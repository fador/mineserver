/*
   Copyright (c) 2011, The Mineserver Project
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
#include <sstream>
#include <fstream>
#include <algorithm>
#include <event.h>
#include <ctime>
#include <vector>
#include <zlib.h>
#include <signal.h>
#include <errno.h>

#include "constants.h"
#include "mineserver.h"
#include "logger.h"
#include "sockets.h"
#include "tools.h"
#include "map.h"
#include "user.h"
#include "chat.h"
#include "worldgen/mapgen.h"
#include "worldgen/nethergen.h"
#include "worldgen/heavengen.h"
#include "worldgen/biomegen.h"
#include "config.h"
#include "config/node.h"
#include "nbt.h"
#include "packets.h"
#include "physics.h"
#include "plugin.h"
#include "furnaceManager.h"
#include "cliScreen.h"
#include "hook.h"
#include "mob.h"
//#include "minecart.h"
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

// Handle signals
void sighandler(int sig_num)
{
  Mineserver::get()->stop();
}

#ifndef WIN32
void pipehandler(int sig_num)
{
  //Do nothing
}
#endif

std::string removeChar(std::string str, const char* c)
{
  std::string remove(c);
  std::string replace("");
  std::size_t loc = str.find(remove);
  if (loc != std::string::npos)
  {
    return str.replace(loc, 1, replace);
  }
  return str;
}

int printHelp(int code)
{
  std::cout
    << "Usage: mineserver [CONFIG_FILE]\n"
    << "   or: mineserver -h|--help\n";
  return code;
}


int main(int argc, char* argv[])
{
  signal(SIGTERM, sighandler);
  signal(SIGINT, sighandler);

#ifndef WIN32
  signal(SIGPIPE, pipehandler);
#else
  signal(SIGBREAK, sighandler);
#endif

  srand((uint32_t)time(NULL));

  // accept at most 1 argument
  if (argc > 2)
    return printHelp(EXIT_FAILURE);

  std::string cfg;
  for (int i = 1; i < argc; i++)
  {
    cfg = std::string(argv[i]);
    if (cfg[0] == '-')
      return printHelp(EXIT_SUCCESS);
  }

  bool ret = false;

  ret = Mineserver::get()->run();

  return ret ? EXIT_SUCCESS : EXIT_FAILURE;
}

Mineserver::Mineserver()
{
  m_saveInterval = 0;
  m_lastSave = time(NULL);

  initConstants();

  m_config         = new Config;

  std::string file_config;
  file_config.assign(CONFIG_FILE);

  // Initialize conf
  if (!m_config->load(file_config))
  {
    std::cerr << "Config file error, failed to start Mineserver!" << std::endl;
    exit(1);
  }

  MapGen* mapgen = new MapGen;
  MapGen* nethergen = new NetherGen;
  MapGen* heavengen = new HeavenGen;
  MapGen* biomegen = new BiomeGen;
  m_mapGenNames.push_back(mapgen);
  m_mapGenNames.push_back(nethergen);
  m_mapGenNames.push_back(heavengen);
  m_mapGenNames.push_back(biomegen);

  m_saveInterval = m_config->iData("map.save_interval");

  m_only_helmets = m_config->bData("system.armour.helmet_strict");
  m_pvp_enabled = m_config->bData("system.pvp.enabled");
  m_damage_enabled = m_config->bData("system.damage.enabled");

  const char* key = "map.storage.nbt.directories"; // Prefix for worlds config
  if (m_config->has(key) && (m_config->type(key) == CONFIG_NODE_LIST))
  {
    std::list<std::string>* tmp = m_config->mData(key)->keys();
    std::list<std::string>::iterator it = tmp->begin();
    int n = 0;
    for (; it != tmp->end(); ++it)
    {
      m_map.push_back(new Map());
      Physics* phy = new Physics;
      phy->map = n;
      m_physics.push_back(phy);
      int k = m_config->iData((std::string(key) + ".") + (*it));
      if (k >= m_mapGenNames.size())
      {
        std::cout << "Error! Mapgen number " << k << " in config. " << m_mapGenNames.size() << " Mapgens known" << std::endl;
      }
      MapGen* m = m_mapGenNames[k];
      m_mapGen.push_back(m);
      n++;

    }
    delete tmp;
  }
  else
  {
    // WARNING: winex: infinite loop here
    //LOG2(WARNING, "Cannot find map.storage.nbt.directories.*");
  }
  if (m_map.size() == 0)
  {
    std::cerr << "No worlds in Config!" << std::endl;
    exit(1);
  }
  m_screen         = new CliScreen;
  m_logger         = new Logger;
  m_chat           = new Chat;
  m_furnaceManager = new FurnaceManager;
  m_packetHandler  = new PacketHandler;
  //m_inventory      = new Inventory(std::string(CONFIG_DIR_SHARE) + '/' + "recipes", ".recipe", "ENABLED_RECIPES.cfg");
  m_mobs           = new Mobs;
}

event_base* Mineserver::getEventBase()
{
  return m_eventBase;
}

void Mineserver::saveAll()
{
  for (std::vector<Map*>::size_type i = 0; i < m_map.size(); i++)
  {
    m_map[i]->saveWholeMap();
  }
  saveAllPlayers();
}

void Mineserver::saveAllPlayers()
{
  if (users().size() == 0)
  {
    return;
  }
  for (int32_t i = users().size() - 1; i >= 0; i--)
  {
    if (users()[i]->logged)
    {
      users()[i]->saveData();
    }
  }
}

bool Mineserver::run()
{
  uint32_t starttime = (uint32_t)time(0);
  uint32_t tick      = (uint32_t)time(0);
  m_plugin         = new Plugin;

  init_plugin_api();

  if (config()->bData("system.interface.use_cli"))
  {
    // Init our Screen
    screen()->init(VERSION);
    LOG2(INFO, "Welcome to Mineserver v" + VERSION);
  }

  m_inventory      = new Inventory(std::string(CONFIG_DIR_SHARE) + '/' + "recipes", ".recipe", "ENABLED_RECIPES.cfg");

  if (config()->has("system.plugins") && (config()->type("system.plugins") == CONFIG_NODE_LIST))
  {
    std::list<std::string>* tmp = config()->mData("system.plugins")->keys();
    std::list<std::string>::iterator it = tmp->begin();
    for (; it != tmp->end(); ++it)
    {
      plugin()->loadPlugin(*it, config()->sData("system.plugins." + (*it)));
    }
    delete tmp;
  }

  // Write PID to file
  std::ofstream pid_out((config()->sData("system.pid_file")).c_str());
  if (!pid_out.fail())
  {
#ifdef WIN32
    pid_out << _getpid();
#else
    pid_out << getpid();
#endif
  }
  pid_out.close();

  // Initialize map
  for (int i = 0; i < (int)m_map.size(); i++)
  {
    physics(i)->enabled = (config()->bData("system.physics.enabled"));

    m_map[i]->init(i);
    if (config()->bData("map.generate_spawn.enabled"))
    {
      LOG2(INFO, "Generating spawn area...");
      int size = config()->iData("map.generate_spawn.size");
      bool show_progress = config()->bData("map.generate_spawn.show_progress");
#ifdef __FreeBSD__
      show_progress = false;
#endif

#ifdef WIN32
      DWORD t_begin = 0, t_end = 0;
#else
      clock_t t_begin = 0, t_end = 0;
#endif

      for (int x = -size; x <= size; x++)
      {
#ifdef WIN32
        if (show_progress)
        {
          t_begin = timeGetTime();
        }
#else
        if (show_progress)
        {
          t_begin = clock();
        }
#endif
        for (int z = -size; z <= size; z++)
        {
          m_map[i]->loadMap(x, z);
        }

        if (show_progress)
        {
#ifdef WIN32
          t_end = timeGetTime();
          LOG2(INFO, dtos((x + size + 1) * (size * 2 + 1)) + "/" + dtos((size * 2 + 1) * (size * 2 + 1)) + " done. " + dtos((t_end - t_begin) / (size * 2 + 1)) + "ms per chunk");
#else
          t_end = clock();
          LOG2(INFO, dtos((x + size + 1) * (size * 2 + 1)) + "/" + dtos((size * 2 + 1) * (size * 2 + 1)) + " done. " + dtos(((t_end - t_begin) / (CLOCKS_PER_SEC / 1000)) / (size * 2 + 1)) + "ms per chunk");
#endif
        }
      }
    }
#ifdef DEBUG
    LOG(DEBUG, "Map", "Spawn area ready!");
#endif
  }

  // Initialize packethandler
  packetHandler()->init();

  // Load ip from config
  std::string ip = config()->sData("net.ip");

  // Load port from config
  int port = config()->iData("net.port");

#ifdef WIN32
  WSADATA wsaData;
  int iResult;
  // Initialize Winsock
  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0)
  {
    LOG2(ERROR, std::string("WSAStartup failed with error: ") + iResult);
    screen()->end();
    return false;
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

  if (m_socketlisten < 0)
  {
    LOG2(ERROR, "Failed to create listen socket");
    screen()->end();
    return false;
  }

  memset(&addresslisten, 0, sizeof(addresslisten));

  addresslisten.sin_family      = AF_INET;
  addresslisten.sin_addr.s_addr = inet_addr(ip.c_str());
  addresslisten.sin_port        = htons(port);

  setsockopt(m_socketlisten, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

  // Bind to port
  if (bind(m_socketlisten, (struct sockaddr*)&addresslisten, sizeof(addresslisten)) < 0)
  {
    LOG2(ERROR, "Failed to bind to " + ip + ":" + dtos(port));
    screen()->end();
    return false;
  }

  if (listen(m_socketlisten, 5) < 0)
  {
    LOG2(ERROR, "Failed to listen to socket");
    screen()->end();
    return false;
  }

  setnonblock(m_socketlisten);
  event_set(&m_listenEvent, m_socketlisten, EV_WRITE | EV_READ | EV_PERSIST, accept_callback, NULL);
  event_add(&m_listenEvent, NULL);

  if (ip == "0.0.0.0")
  {
    // Print all local IPs
    char name[255];
    gethostname(name, sizeof(name));
    struct hostent* hostinfo = gethostbyname(name);
    LOG2(INFO, "Listening on: ");
    int ipIndex = 0;
    while (hostinfo && hostinfo->h_addr_list[ipIndex])
    {
      std::string ip(inet_ntoa(*(struct in_addr*)hostinfo->h_addr_list[ipIndex++]));
      LOG2(INFO, ip + ":" + dtos(port));
    }
  }
  else
  {
    std::string myip(ip);
    LOG2(INFO, myip + ":" + dtos(port));
  }

  timeval loopTime;
  loopTime.tv_sec  = 0;
  loopTime.tv_usec = 200000; // 200ms

  m_running = true;
  event_base_loopexit(m_eventBase, &loopTime);

  // Create our Server Console user so we can issue commands

  time_t timeNow = time(NULL);
  while (m_running && event_base_loop(m_eventBase, 0) == 0)
  {
    // Run 200ms timer hook
    static_cast<Hook0<bool>*>(plugin()->getHook("Timer200"))->doAll();
    // Alert any block types that care about timers
    BlockBasic* blockcb;
    for (uint32_t i = 0 ; i < plugin()->getBlockCB().size(); i++)
    {
      blockcb = plugin()->getBlockCB()[i];
      if (blockcb != NULL)
      {
        blockcb->timer200();
      }
    }
    //    for(uint32_t i=0; i<minecarts.size(); i++){
    //      minecarts[i]->timer();
    //    }
    for (std::vector<Map*>::size_type i = 0 ; i < m_map.size(); i++)
    {
      physics(i)->update();
    }



    timeNow = time(0);
    if (timeNow - starttime > 10)
    {
      starttime = (uint32_t)timeNow;

      //Map saving on configurable interval
      if (m_saveInterval != 0 && timeNow - m_lastSave >= m_saveInterval)
      {
        //Save
        for (std::vector<Map*>::size_type i = 0; i < m_map.size(); i++)
        {
          m_map[i]->saveWholeMap();
        }

        m_lastSave = timeNow;
      }

      // If users, ping them
      if (User::all().size() > 0)
      {
        // 0x00 package
        uint8_t data = 0;
        User::all()[0]->sendAll(&data, 1);

        // Send server time
        Packet pkt;
        pkt << (int8_t)PACKET_TIME_UPDATE << (int64_t)m_map[0]->mapTime;
        User::all()[0]->sendAll((uint8_t*)pkt.getWrite(), pkt.getWriteLen());
      }

      //Check for tree generation from saplings
      for (std::vector<Map*>::size_type i = 0; i < m_map.size(); i++)
      {
        m_map[i]->checkGenTrees();
      }

      // TODO: Run garbage collection for chunk storage dealie?

      // Run 10s timer hook
      static_cast<Hook0<bool>*>(plugin()->getHook("Timer10000"))->doAll();
    }

    // Every second
    if (timeNow - tick > 0)
    {
      tick = (uint32_t)timeNow;
      // Loop users
      for (int i = users().size() - 1; i >= 0; i--)
      {
        // No data received in 30s, timeout
        if (users()[i]->logged && (timeNow - users()[i]->lastData) > 30)
        {
          LOG2(INFO, "Player " + users()[i]->nick + " timed out");

          delete users()[i];
        }
        else if (!users()[i]->logged && (timeNow - users()[i]->lastData) > 100)
        {
          delete users()[i];
        }
        else
        {
          if (m_damage_enabled)
          {
            users()[i]->checkEnvironmentDamage();
          }
          users()[i]->pushMap();
          users()[i]->popMap();
        }

        // Minecart hacks!!
        /*
        if (User::all()[i]->attachedTo)
        {
          Packet pkt;
          pkt << PACKET_ENTITY_VELOCITY << (int32_t)User::all()[i]->attachedTo <<  (int16_t)10000       << (int16_t)0 << (int16_t)0;
          // pkt << PACKET_ENTITY_RELATIVE_MOVE << (int32_t)User::all()[i]->attachedTo <<  (int8_t)100       << (int8_t)0 << (int8_t)0;
          User::all()[i]->sendAll((int8_t*)pkt.getWrite(), pkt.getWriteLen());
        }
        */

      }

      for (std::vector<Map*>::size_type i = 0 ; i < m_map.size(); i++)
      {
        m_map[i]->mapTime += 20;
        if (m_map[i]->mapTime >= 24000)
        {
          m_map[i]->mapTime = 0;
        }
      }


      for (int i = users().size() - 1; i >= 0; i--)
      {
        users()[i]->pushMap();
        users()[i]->popMap();
      }

      // Check for Furnace activity
      furnaceManager()->update();

      // Run 1s timer hook
      static_cast<Hook0<bool>*>(plugin()->getHook("Timer1000"))->doAll();
    }

    // Underwater check / drowning
    // ToDo: this could be done a bit differently? - Fador
    int i = 0;
    int s = User::all().size();
    for (i = 0; i < s; i++)
    {
      User::all()[i]->isUnderwater();
      if (User::all()[i]->pos.y < 0)
      {
        User::all()[i]->sethealth(User::all()[i]->health - 5);
      }
    }

    event_base_loopexit(m_eventBase, &loopTime);
  }

#ifdef WIN32
  closesocket(m_socketlisten);
#else
  close(m_socketlisten);
#endif

  // Remove the PID file
#ifdef WIN32
  _unlink((config()->sData("system.pid_file")).c_str());
#else
  unlink((config()->sData("system.pid_file")).c_str());
#endif

  // Let the user know we're shutting the server down cleanly
  LOG2(INFO, "Shutting down...");

  // Close the cli session if its in use
  if (config()->bData("system.interface.use_cli"))
  {
    screen()->end();
  }

  saveAll();

  /* Free memory */
  for (std::vector<Map*>::size_type i = 0; i < m_map.size(); i++)
  {
    delete m_map[i];
    delete m_physics[i];
    delete m_mapGen[i];
  }

  delete m_chat;
  delete m_plugin;
  delete m_screen;
  delete m_config;
  delete m_furnaceManager;
  delete m_packetHandler;
  delete m_logger;
  delete m_inventory;

  freeConstants();

  event_base_free(m_eventBase);

  return true;
}

bool Mineserver::stop()
{
  m_running = false;
  return true;
}


bool Mineserver::homePrepare(const std::string& path)
{
  struct stat st;
  if (stat(path.c_str(), &st) != 0)
  {
    LOG2(INFO, "Creating: " + path);
#ifdef WIN32
    if (_mkdir(path.c_str() == -1)
#else
    if (mkdir(path.c_str(), 0755) == -1)
#endif
    {
      LOG2(ERROR, path + ": " + strerror(errno));
      return false;
    }
  }

  // copy example configs
  const std::string files[] = {
    "banned.txt",
    "commands.cfg",
    "config.cfg",
    "item_alias.cfg",
    "ENABLED_RECIPES.cfg",
    "motd.txt",
    "permissions.txt",
    "roles.txt",
    "rules.txt",
    "whitelist.txt",
  };
  for (uint32_t i = 0; i < sizeof(files)/sizeof(files[0]); i++)
  {
    // TODO: winex: hardcoded path won't work on installation
    std::string namein  = std::string("../files") + '/' + files[i];
    std::string nameout = path + '/' + files[i];

    // don't overwrite existing files
    if ( (stat(nameout.c_str(), &st) == 0) && S_ISREG(st.st_mode) )
      continue;

    std::ifstream fin(namein.c_str(), std::ios_base::binary | std::ios_base::in);
    if (fin.fail())
    {
      LOG2(ERROR, "Failed to open: " + namein);
      continue;
    }

    LOG2(INFO, "Copying: " + nameout);
    std::ofstream fout(nameout.c_str(), std::ios_base::binary);
    if (fout.fail())
    {
      LOG2(ERROR, "Failed to write to: " + nameout);
      continue;
    }

    fout << fin.rdbuf();
  }

  return true;
}

Physics* Mineserver::physics(int n)
{
  return m_physics[n];
}

MapGen* Mineserver::mapGen(int n)
{
  return m_mapGen[n];
}

Map* Mineserver::map(int n)
{
  if (m_map.size() > n)
  {
    return m_map[n];
  }
  LOG2(WARNING, "Nonexistent map requested. Map 0 passed");
  return m_map[0];
}

int Mineserver::mapCount()
{
  return m_map.size();
}

void Mineserver::setMap(Map* map, int n)
{
  m_map[n] = map;
}
