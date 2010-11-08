#include <stdlib.h>
#ifdef WIN32
  #define _CRTDBG_MAP_ALLOC
  #include <crtdbg.h>
  #include <conio.h>
  #include <winsock2.h>
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <string.h>
#endif

#include <sys/types.h>
#include <fcntl.h>
#include <cstdio>
#include <deque>
#include <iostream>
#include <event.h>
#include <ctime>
#include <vector>
#include <zlib.h>

#include "constants.h"

#include "logger.h"

#include "sockets.h"
#include "tools.h"
#include "map.h"
#include "user.h"
#include "chat.h"
#include "config.h"
#include "nbt.h"
#include "packets.h"


#ifdef WIN32
static bool quit = false;
#endif

int setnonblock(int fd)
{
  #ifdef WIN32
  u_long iMode = 1;
  ioctlsocket(fd, FIONBIO, &iMode);
  #else
  int flags;

  flags = fcntl(fd, F_GETFL);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  #endif

  return 1;
}

int main(void)
{
  uint32 starttime=(uint32)time(0);
  uint32 tick=(uint32)time(0);


  Chat::get().loadAdmins(ADMINFILE);
  Chat::get().checkMotd(MOTDFILE);

  //Initialize conf
  Conf::get().load(CONFIGFILE);
  // Load item aliases
  Conf::get().load(ITEMALIASFILE);

  //Initialize map
  Map::get().initMap();

  //Initialize packethandler
  PacketHandler::get().initPackets();

  //Try to load port from config
  int port=atoi(Conf::get().value("port").c_str());
  //If failed, use default
  if(port==0)
    port=DEFAULT_PORT;

#ifdef WIN32
  _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
  _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
  WSADATA wsaData;
  int iResult;
  // Initialize Winsock
  iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
  if (iResult != 0) {
    printf("WSAStartup failed with error: %d\n", iResult);
    return EXIT_FAILURE;
  }
#endif

  int socketlisten;
  struct sockaddr_in addresslisten;
  struct event accept_event;
  int reuse = 1;

  event_base *eventbase=(event_base *)event_init();
#ifdef WIN32
  socketlisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
  socketlisten = socket(AF_INET, SOCK_STREAM, 0);
#endif

  if (socketlisten < 0)
  {
    fprintf(stderr,"Failed to create listen socket\n");
    return 1;
  }

  memset(&addresslisten, 0, sizeof(addresslisten));

  addresslisten.sin_family = AF_INET;
  addresslisten.sin_addr.s_addr = INADDR_ANY;
  addresslisten.sin_port = htons(port);

  //Bind to port
  if (bind(socketlisten, (struct sockaddr *)&addresslisten, sizeof(addresslisten)) < 0)
  {
    fprintf(stderr, "Failed to bind\n");
    return 1;
  }

  if (listen(socketlisten, 5) < 0)
  {
    fprintf(stderr, "Failed to listen to socket\n");
    return 1;
  }

  setsockopt(socketlisten, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
  setnonblock(socketlisten);
  event_set(&accept_event, socketlisten, EV_READ|EV_PERSIST, accept_callback, NULL);
  event_add(&accept_event, NULL);

  std::cout << std::endl
            << "   _____  .__  "                                                              << std::endl
            << "  /     \\ |__| ____   ____   ______ ______________  __ ___________ "         << std::endl
            << " /  \\ /  \\|  |/    \\_/ __ \\ /  ___// __ \\_  __ \\  \\/ // __ \\_  __ \\" << std::endl
            << "/    Y    \\  |   |  \\  ___/ \\___ \\\\  ___/|  | \\/\\   /\\  ___/|  | \\/" << std::endl
            << "\\____|__  /__|___|  /\\___  >____  >\\___  >__|    \\_/  \\___  >__|   "     << std::endl
            << "        \\/        \\/     \\/     \\/     \\/                 \\/       "    << std::endl
            << "Version " << VERSION <<" by Fador & Psoden"                                   << std::endl << std::endl;

  std::cout << "Listening at port " << port << std::endl;

  timeval loopTime;
  loopTime.tv_sec=1;
  loopTime.tv_usec=0;

  event_base_loopexit(eventbase,&loopTime);
  while (event_base_loop(eventbase, 0)==0)
  {
    if(time(0)-starttime>10)
    {
      starttime=(uint32)time(0);
      //Logger::get().log("Currently " + h.GetCount()-1 + " users in!");
      std::cout << "Currently " << Users.size() << " users in!" << std::endl;

      //If users, ping them
      if(Users.size()>0)
      {
        //0x00 package
        uint8 data=0;
        Users[0]->sendAll(&data, 1);

        //Send server time (after dawn)
        uint8 data3[9]={0x04, 0x00, 0x00, 0x00,0x00,0x00,0x00,0x0e,0x00};
        Users[0]->sendAll((uint8 *)&data3[0], 9);
      }

      //Try to load port from config
      int map_release_time=atoi(Conf::get().value("map_release_time").c_str());
      //If failed, use default
      if(map_release_time==0) map_release_time=DEFAULT_MAP_RELEASE_TIME;

      //Release chunks not used in <map_release_time> seconds
      std::vector<uint32> toRelease;
      for (std::map<uint32, int>::const_iterator it = Map::get().mapLastused.begin(); it != Map::get().mapLastused.end(); ++it)
      {
        if(Map::get().mapLastused[it->first] <= time(0)-map_release_time)
        {
          toRelease.push_back(it->first);
        }
      }

      int x_temp,z_temp;
      for(unsigned i=0;i<toRelease.size();i++)
      {
        Map::get().idToPos(toRelease[i], &x_temp,&z_temp);
        Map::get().releaseMap(x_temp,z_temp);
      }
    }

    //Every second
    if(time(0)-tick>0)
    {
      tick=(uint32)time(0);
      //Loop users
      for(unsigned int i=0;i<Users.size();i++)
      {
        //for(uint8 j=0;j<10;j++)
        {
          //Push new map data
          Users[i]->pushMap();
        }
        //for(uint8 j=0;j<20;j++)
        {
          //Remove map far away
          Users[i]->popMap();
        }
      }
    }
#ifdef WIN32
    if(_kbhit())
        quit = 1;
#endif

    event_base_loopexit(eventbase,&loopTime);
  }

  Map::get().freeMap();

  //event_dispatch();

#ifdef WIN32
  closesocket(socketlisten);
#else
  close(socketlisten);
#endif

  //Windows debug
#ifdef WIN32
  _CrtDumpMemoryLeaks();
#endif
  
  return EXIT_SUCCESS;
}
