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
  Mineserver::get()->stop();
}

int main(int argc, char* argv[])
{
  signal(SIGTERM, sighandler);
  signal(SIGINT, sighandler);

  srand(time(NULL));

  return Mineserver::get()->run(argc, argv);
}

event_base* Mineserver::getEventBase()
{
  return m_eventBase;
}

void Mineserver::updatePlayerList()
{
  // Update the player window
  Mineserver::get()->screen()->updatePlayerList(users());
}

int Mineserver::run(int argc, char *argv[])
{
  uint32 starttime = (uint32)time(0);
  uint32 tick      = (uint32)time(0);

  // Init our Screen
  screen()->init(VERSION);
  screen()->log("Welcome to Mineserver v" + VERSION);
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
  Mineserver::get()->conf()->load(file_config);
  Mineserver::get()->conf()->load(file_commands, COMMANDS_NAME_PREFIX);

  // Write PID to file
  std::ofstream pid_out((Mineserver::get()->conf()->sValue("pid_file")).c_str());
  if (!pid_out.fail())
  {
#ifdef WIN32
     pid_out << _getpid();
#else
     pid_out << getpid();
#endif
  }
  pid_out.close();

  // Load MOTD
  Mineserver::get()->chat()->checkMotd(Mineserver::get()->conf()->sValue("motd_file"));

  // Set physics enable state according to config
  Mineserver::get()->physics()->enabled = (Mineserver::get()->conf()->bValue("liquid_physics"));

  // Initialize map
  Mineserver::get()->map()->init();

  if (Mineserver::get()->conf()->bValue("map_generate_spawn"))
  {
    Mineserver::get()->screen()->log("Generating spawn area...");
    int size = Mineserver::get()->conf()->iValue("map_generate_spawn_size");
    bool show_progress = Mineserver::get()->conf()->bValue("map_generate_spawn_show_progress");
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
        Mineserver::get()->map()->loadMap(x, z);
      }

      if(show_progress)
      {
#ifdef WIN32
        t_end = timeGetTime ();
        Mineserver::get()->screen()->log(dtos((x+size+1)*(size*2+1)) + "/" + dtos((size*2+1)*(size*2+1)) + " done. " + dtos((t_end-t_begin)/(size*2+1)) + "ms per chunk");
#else
        t_end = clock();
        Mineserver::get()->screen()->log(dtos((x+size+1)*(size*2+1)) + "/" + dtos((size*2+1)*(size*2+1)) + " done. " + dtos(((t_end-t_begin)/(CLOCKS_PER_SEC/1000))/(size*2+1)) + "ms per chunk");
#endif
      }
    }
#ifdef _DEBUG
    Mineserver::get()->screen()->log("Spawn area ready!");
#endif
  }

  // Initialize packethandler
  PacketHandler::get()->init();

  // Load ip from config
  std::string ip = Mineserver::get()->conf()->sValue("ip");

  // Load port from config
  int port = Mineserver::get()->conf()->iValue("port");

  // Initialize plugins
  Mineserver::get()->plugin()->init();

#ifdef WIN32
  WSADATA wsaData;
  int iResult;
  // Initialize Winsock
  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if(iResult != 0)
  {
    printf("WSAStartup failed with error: %d\n", iResult);
    Mineserver::get()->screen()->end();
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
    Mineserver::get()->screen()->log(LOG_ERROR, "Failed to create listen socket");
    Mineserver::get()->screen()->end();
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
    Mineserver::get()->screen()->log(LOG_ERROR, "Failed to bind");
    return 1;
  }

  if(listen(m_socketlisten, 5) < 0)
  {
    Mineserver::get()->screen()->log(LOG_ERROR, "Failed to listen to socket");
    Mineserver::get()->screen()->end();
    return 1;
  }

  setnonblock(m_socketlisten);
  event_set(&m_listenEvent, m_socketlisten, EV_WRITE|EV_READ|EV_PERSIST, accept_callback, NULL);
  event_add(&m_listenEvent, NULL);

  if(ip == "0.0.0.0")
  {
    // Print all local IPs
    char name[255];
    gethostname ( name, sizeof(name));
    struct hostent* hostinfo = gethostbyname(name);
    Mineserver::get()->screen()->log("Listening on: ");
    int ipIndex = 0;
    while(hostinfo && hostinfo->h_addr_list[ipIndex])
    {
      std::string ip(inet_ntoa(*(struct in_addr*)hostinfo->h_addr_list[ipIndex++]));
      Mineserver::get()->screen()->log(" " + ip + ":" + dtos(port));
    }
  }
  else
  {
    std::string myip(ip);
    Mineserver::get()->screen()->log("Listening on " + myip + ":" + dtos(port));
  }
  //std::cout << std::endl;

  timeval loopTime;
  loopTime.tv_sec  = 0;
  loopTime.tv_usec = 200000; //200ms

  m_running = true;
  event_base_loopexit(m_eventBase, &loopTime);

  // Create our Server Console user so we can issue commands
  User* serverUser = new User(-1, SERVER_CONSOLE_UID);
  serverUser->changeNick("[Server]");

  while(m_running && event_base_loop(m_eventBase, 0) == 0)
  {
    // Append current command and check if user entered return
    if(Mineserver::get()->screen()->hasCommand())
    {
      // Now handle this command as normal
      Mineserver::get()->chat()->handleMsg(serverUser, Mineserver::get()->screen()->getCommand().c_str());
    }

    if(time(0)-starttime > 10)
    {
      starttime = (uint32)time(0);

      //If users, ping them
      if(User::all().size() > 0)
      {
        //0x00 package
        uint8 data = 0;
        User::all()[0]->sendAll(&data, 1);

        //Send server time
        Packet pkt;
        pkt << (sint8)PACKET_TIME_UPDATE << (sint64)Mineserver::get()->map()->mapTime;
        User::all()[0]->sendAll((uint8*)pkt.getWrite(), pkt.getWriteLen());
      }

      // TODO: Run garbage collection for chunk storage dealie?
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

      map()->mapTime+=20;
      if (map()->mapTime >= 24000)
      {
        map()->mapTime = 0;
      }

      map()->checkGenTrees();

      // Check for Furnace activity
      Mineserver::get()->furnaceManager()->update();
    }

    // Physics simulation every 200ms
    Mineserver::get()->physics()->update();

    // Underwater check / drowning
    int i = 0;
    int s = User::all().size();
    for(i=0;i<s;i++)
    {
      User::all()[i]->isUnderwater();
    }

//    event_set(&m_listenEvent, m_socketlisten, EV_WRITE|EV_READ|EV_PERSIST, accept_callback, NULL);
//    event_add(&m_listenEvent, NULL);

    event_base_loopexit(m_eventBase, &loopTime);
  }

#ifdef WIN32
  closesocket(m_socketlisten);
#else
  close(m_socketlisten);
#endif

  // Remove the PID file
#ifdef WIN32
  _unlink((Mineserver::get()->conf()->sValue("pid_file")).c_str());
#else
  unlink((Mineserver::get()->conf()->sValue("pid_file")).c_str());
#endif

  // End our NCurses session
  screen()->end();

  /* Free memory */
  delete m_map;
  delete m_chat;
  delete m_plugin;
  delete m_screen;
  delete m_physics;
  delete m_conf;
  delete m_furnaceManager;

  PacketHandler::get()->free();
  Logger::get()->free();
  MapGen::get()->free();

  return EXIT_SUCCESS;
}

bool Mineserver::stop()
{
  m_running=false;

  return true;
}
