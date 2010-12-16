/*
   Copyright (c) 2010, The Mineserver Project
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
 * Neither the name of the The Mineserver Project nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdlib.h>
#ifdef WIN32
  #include <conio.h>
  #include <winsock2.h>
  #include <process.h>
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <string.h>
  #include <netdb.h>
  #include <unistd.h>
  #include <sys/times.h>
#endif
#include <sys/types.h>
#include <fcntl.h>
#include <cassert>
#include <deque>
#include <map>
#include <iostream>
#include <fstream>
#include <event.h>
#include <ctime>
#include <vector>
#include <zlib.h>
#include <signal.h>

#include "constants.h"
#include "mineserver.h"
#include "logger.h"
#include "sockets.h"
#include "tools.h"
#include "map.h"
#include "user.h"
#include "chat.h"
#include "worldgen/mapgen.h"
#include "config.h"
#include "nbt.h"
#include "packets.h"
#include "physics.h"
#include "plugin.h"
#include "furnaceManager.h"
#include "screen.h"

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

  flags  = fcntl(fd, F_GETFL);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  #endif

  return 1;
}

//Handle signals
void sighandler(int sig_num)
{
  Mineserver::get().stop();
}

int main(int argc, char* argv[])
{
  signal(SIGTERM, sighandler);
  signal(SIGINT, sighandler);

  srand(time(NULL));

  return Mineserver::get().run(argc, argv);
}

Mineserver::Mineserver()
{
}

event_base* Mineserver::getEventBase()
{
  return m_eventBase;
}

void Mineserver::updatePlayerList()
{
	// Update the player window
	Screen::get()->updatePlayerList(users());
}
int Mineserver::run(int argc, char *argv[])
{
  uint32 starttime = (uint32)time(0);
  uint32 tick      = (uint32)time(0);

	// Init our Screen
	Screen::get()->init(VERSION);
	Screen::get()->log("Welcome to Mineserver v" + VERSION);
	updatePlayerList();

  initConstants();

  std::string file_config;
  file_config.assign(CONFIG_FILE);
  std::string file_commands;
  file_commands.assign(COMMANDS_FILE);

  if (argc > 1)
  {
    file_config.assign(argv[1]);
  }

  // Initialize conf
  Conf::get()->load(file_config);
  Conf::get()->load(file_commands, COMMANDS_NAME_PREFIX);

  // Write PID to file
  std::ofstream pid_out((Conf::get()->sValue("pid_file")).c_str());
  if (!pid_out.fail())
  {
#ifdef WIN32
     pid_out << _getpid();
#else
     pid_out << getpid();
#endif
  }
  pid_out.close();

  // Load admin, banned and whitelisted users
  Conf::get()->loadRoles();
  Conf::get()->loadBanned();
  Conf::get()->loadWhitelist();
  // Load MOTD
  Chat::get()->checkMotd(Conf::get()->sValue("motd_file"));

  // Set physics enable state according to config
  Physics::get()->enabled = (Conf::get()->bValue("liquid_physics"));

  // Initialize map
  Map::get()->init();

  if (Conf::get()->bValue("map_generate_spawn"))
  {
    Screen::get()->log("Generating spawn area...");
    int size = Conf::get()->iValue("map_generate_spawn_size");
    bool show_progress = Conf::get()->bValue("map_generate_spawn_show_progress");
#ifdef WIN32
    DWORD t_begin,t_end;
#else
    clock_t t_begin,t_end;
#endif

    for (int x=-size;x<=size;x++)
    {
#ifdef WIN32
      if(show_progress)
      {
        t_begin = timeGetTime();
      }
#else
      if(show_progress)
      {
        t_begin = clock();
      }
#endif
      for (int z = -size; z <= size; z++)
      {
        Map::get()->loadMap(x, z);
      }

      if(show_progress)
      {
#ifdef WIN32
        t_end = timeGetTime ();
        Screen::get()->log(dtos((x+size+1)*(size*2+1)) + "/" + dtos((size*2+1)*(size*2+1)) + " done. " + dtos((t_end-t_begin)/(size*2+1)) + "ms per chunk");
#else
        t_end = clock();
        Screen::get()->log(dtos((x+size+1)*(size*2+1)) + "/" + dtos((size*2+1)*(size*2+1)) + " done. " + dtos(((t_end-t_begin)/(CLOCKS_PER_SEC/1000))/(size*2+1)) + "ms per chunk");
#endif
      }
    }
#ifdef _DEBUG
    Screen::get()->log("Spawn area ready!");
#endif
  }

  // Initialize packethandler
  PacketHandler::get()->init();

  // Load ip from config
  std::string ip = Conf::get()->sValue("ip");

  // Load port from config
  int port = Conf::get()->iValue("port");

  // Initialize plugins
  Plugin::get()->init();

#ifdef WIN32
  WSADATA wsaData;
  int iResult;
  // Initialize Winsock
  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if(iResult != 0)
  {
    printf("WSAStartup failed with error: %d\n", iResult);
    Screen::get()->end();
    return EXIT_FAILURE;
  }
#endif

  struct sockaddr_in addresslisten;
  int reuse = 1;

  m_eventBase = (event_base*)event_init();
#ifdef WIN32
  m_socketlisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
  m_socketlisten = socket(AF_INET, SOCK_STREAM, 0);
#endif

  if(m_socketlisten < 0)
  {
    Screen::get()->log(LOG_ERROR, "Failed to create listen socket");
    Screen::get()->end();
    return 1;
  }

  memset(&addresslisten, 0, sizeof(addresslisten));

  addresslisten.sin_family      = AF_INET;
  addresslisten.sin_addr.s_addr = inet_addr(ip.c_str());
  addresslisten.sin_port        = htons(port);

  setsockopt(m_socketlisten, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

  //Bind to port
  if(bind(m_socketlisten, (struct sockaddr*)&addresslisten, sizeof(addresslisten)) < 0)
  {
    Screen::get()->log(LOG_ERROR, "Failed to bind");
    return 1;
  }

  if(listen(m_socketlisten, 5) < 0)
  {
    Screen::get()->log(LOG_ERROR, "Failed to listen to socket");
    Screen::get()->end();
    return 1;
  }

  setnonblock(m_socketlisten);
  event_set(&m_listenEvent, m_socketlisten, EV_WRITE|EV_READ|EV_PERSIST, accept_callback, NULL);
  event_add(&m_listenEvent, NULL);

 /* std::cout <<
  "   _____  .__  "<<
  std::endl<<
  "  /     \\ |__| ____   ____   ______ ______________  __ ___________ "<<
  std::endl<<
  " /  \\ /  \\|  |/    \\_/ __ \\ /  ___// __ \\_  __ \\  \\/ // __ \\_  __ \\"<<
  std::endl<<
  "/    Y    \\  |   |  \\  ___/ \\___ \\\\  ___/|  | \\/\\   /\\  ___/|  | \\/"<<
  std::endl<<
  "\\____|__  /__|___|  /\\___  >____  >\\___  >__|    \\_/  \\___  >__|   "<<
  std::endl<<
  "        \\/        \\/     \\/     \\/     \\/                 \\/       "<<
  std::endl<<
  "Version " << VERSION <<" by The Mineserver Project"<<
  std::endl << std::endl;
*/
  if(ip == "0.0.0.0")
  {
    // Print all local IPs
    char name[255];
    gethostname ( name, sizeof(name));
    struct hostent* hostinfo = gethostbyname(name);
    Screen::get()->log("Listening on: ");
    int ipIndex = 0;
    while(hostinfo && hostinfo->h_addr_list[ipIndex])
    {
      std::string ip(inet_ntoa(*(struct in_addr*)hostinfo->h_addr_list[ipIndex++]));
      Screen::get()->log(" " + ip + ":" + dtos(port));
    }
  }
  else
  {
    std::string myip(ip);
    Screen::get()->log("Listening on " + myip + ":" + dtos(port));
  }
  //std::cout << std::endl;

  timeval loopTime;
  loopTime.tv_sec  = 0;
  loopTime.tv_usec = 200000; //200ms

  m_running=true;
  event_base_loopexit(m_eventBase, &loopTime);

  // Create our Server Console user so we can issue commands
  User* serverUser = new User(-1, SERVER_CONSOLE_UID);
  serverUser->changeNick("[Server]");

  while(m_running && event_base_loop(m_eventBase, 0) == 0)
  {
    // Append current command and check if user entered return
    if(Screen::get()->hasCommand())
    {
      // Now handle this command as normal
      Chat::get()->handleMsg(serverUser, Screen::get()->getCommand().c_str());
    }

    if(time(0)-starttime > 10)
    {
      starttime = (uint32)time(0);
      //Screen::get()->log("Currently " + User::all().size() + " users in!");

      //If users, ping them
      if(User::all().size() > 0)
      {
        //0x00 package
        uint8 data = 0;
        User::all()[0]->sendAll(&data, 1);

        //Send server time
        Packet pkt;
        pkt << (sint8)PACKET_TIME_UPDATE << (sint64)Map::get()->mapTime;
        User::all()[0]->sendAll((uint8*)pkt.getWrite(), pkt.getWriteLen());
      }

      //Try to load release time from config
      //int map_release_time = Conf::get()->iValue("map_release_time"); // Unused variable

      //Release chunks not used in <map_release_time> seconds
   /*   std::vector<uint32> toRelease;
      for(std::map<uint32, int>::const_iterator it = Map::get()->mapLastused.begin();
          it != Map::get()->mapLastused.end();
          ++it)
      {
        if(Map::get()->mapLastused[it->first] <= time(0)-map_release_time)
          toRelease.push_back(it->first);
      }

      int x_temp, z_temp;
      for(unsigned i = 0; i < toRelease.size(); i++)
      {
        Map::get()->idToPos(toRelease[i], &x_temp, &z_temp);
        Map::get()->releaseMap(x_temp, z_temp);
      } */
    }

    //Every second
    if(time(0)-tick > 0)
    {
      tick = (uint32)time(0);
      //Loop users
      for(unsigned int i = 0; i < User::all().size(); i++)
      {
        User::all()[i]->pushMap();
        User::all()[i]->popMap();

        //Minecart hacks!!
        if(User::all()[i]->attachedTo)
        {
          Packet pkt;
          pkt << PACKET_ENTITY_VELOCITY << (sint32)User::all()[i]->attachedTo <<  (sint16)10000       << (sint16)0 << (sint16)0;
          //pkt << PACKET_ENTITY_RELATIVE_MOVE << (sint32)User::all()[i]->attachedTo <<  (sint8)100       << (sint8)0 << (sint8)0;
          User::all()[i]->sendAll((uint8*)pkt.getWrite(), pkt.getWriteLen());
        }
      }
      Map::get()->mapTime+=20;
      if(Map::get()->mapTime>=24000) Map::get()->mapTime=0;

      Map::get()->checkGenTrees();

      // Check for Furnace activity
      FurnaceManager::get()->update();

    }

    //Physics simulation every 200ms
    Physics::get()->update();

    //Underwater check / drowning
    for( unsigned int i = 0; i < User::all().size(); i++ )
      User::all()[i]->isUnderwater();

 //   event_set(&m_listenEvent, m_socketlisten, EV_WRITE|EV_READ|EV_PERSIST, accept_callback, NULL);
 //   event_add(&m_listenEvent, NULL);

    event_base_loopexit(m_eventBase, &loopTime);
  }

#ifdef WIN32
  closesocket(m_socketlisten);
#else
  close(m_socketlisten);
#endif

  // Remove the PID file
#ifdef WIN32
  _unlink((Conf::get()->sValue("pid_file")).c_str());
#else
  unlink((Conf::get()->sValue("pid_file")).c_str());
#endif

  /* Free memory */
  PacketHandler::get()->free();
  Map::get()->free();
  Physics::get()->free();
  FurnaceManager::get()->free();
  Chat::get()->free();
  Conf::get()->free();
  Plugin::get()->free();
  Logger::get()->free();
  MapGen::get()->free();

	// End our NCurses session
	Screen::get()->end();


  return EXIT_SUCCESS;
}

bool Mineserver::stop()
{
  m_running=false;

  return true;
}

