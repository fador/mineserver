/*
   Copyright (c) 2012, The Mineserver Project
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

#ifdef WIN32
#include <process.h>
#include <direct.h>
#else
#include <netdb.h>  // for gethostbyname()
#endif

#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include <sstream>
#include <fstream>

#include "mineserver.h"
#include "signalhandler.h"
#include "configure.h"
#include "constants.h"
#include "logger.h"
#include "sockets.h"
#include "tools.h"
#include "random.h"
#include "map.h"
#include "user.h"
#include "chat.h"
#include "worldgen/mapgen.h"
#include "worldgen/nethergen.h"
#include "worldgen/heavengen.h"
#include "worldgen/biomegen.h"
#include "worldgen/eximgen.h"
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
#include "protocol.h"
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

Mineserver *ServerInstance = NULL;

std::string removeChar(std::string str, const char* c)
{
  const size_t loc = str.find(c);
  if (loc != std::string::npos)
  {
    return str.replace(loc, 1, "");
  }
  return str;
}

// What is the purpose of "code"? -- louisdx
// God I have no clue.. some day someone will fix it - Justasic
int printHelp(int code)
{
  std::cout
      << "Mineserver " << VERSION << "\n"
      << "Usage: mineserver [CONFIG_FILE] [OVERRIDE]...\n"
      << "   or: mineserver -h|--help\n"
      << "\n"
      << "Syntax for overrides is: +VARIABLE=VALUE\n"
      << "\n"
      << "Examples:\n"
      << "  mineserver /etc/mineserver/config.cfg +system.path.home=\"/var/lib/mineserver\" +net.port=25565\n";
  return code;
}

// Main :D
int main(int argc, char* argv[])
{
  bool ret = false;
  // Try and start a new server instance
  try
  {
    new Mineserver(argc, argv);
    ret = ServerInstance->run();
  }
  catch (const CoreException &e)
  {
    LOG2(ERROR, e.GetReason());
    return EXIT_FAILURE;
  }

  delete ServerInstance;
  
  return ret ? EXIT_SUCCESS : EXIT_FAILURE;
}


Mineserver::Mineserver(int args, char **argarray)
  :  m_socketlisten  (0),
     m_saveInterval  (0),
     m_lastSave      (std::time(NULL)),
     m_pvp_enabled   (false),
     m_damage_enabled(false),
     m_only_helmets  (false),
     m_running       (false),
     m_eventBase     (NULL),
     argc(args),
     argv(argarray),

     // core modules
     m_config        (new Config()),
     m_screen        (new CliScreen()),
     m_logger        (new Logger()),

     m_plugin        (NULL),
     m_chat          (NULL),
     m_furnaceManager(NULL),
     m_packetHandler (NULL),
     m_inventory     (NULL),
     m_mobs          (NULL)
{
  ServerInstance = this;
  InitSignals();
  
  std::srand((uint32_t)std::time(NULL));
  initPRNG();
  
  std::string cfg;
  std::vector<std::string> overrides;
  
  for (int i = 1; i < argc; i++)
  {
    const std::string arg(argv[i]);
    
    switch (arg[0])
    {
      case '-':   // option
      // we have only '-h' and '--help' now, so just return with help
      printHelp(0);
      throw CoreException();
      
      case '+':   // override
      overrides.push_back(arg.substr(1));
      break;
      
      default:    // otherwise, it is config file
      if (!cfg.empty())
	throw CoreException("Only single CONFIG_FILE argument is allowed!");
      cfg = arg;
      break;
    }
  }
  
  const std::string path_exe = pathOfExecutable();
  
  // If config file is provided as an argument
  if (!cfg.empty())
  {
    std::cout << "Searching for configuration file..." << std::endl;
    if (fileExists(cfg))
    {
      const std::pair<std::string, std::string> fullpath = pathOfFile(cfg);
      cfg = fullpath.first + PATH_SEPARATOR + fullpath.second;
      this->config()->config_path = fullpath.first;
    }
    else
    {
      std::cout << "Config not found...\n";;
      cfg.clear();
    }
  }
  
  if (cfg.empty())
  {
    if (fileExists(path_exe + PATH_SEPARATOR + CONFIG_FILE))
    {
      cfg = path_exe + PATH_SEPARATOR + CONFIG_FILE;
      this->config()->config_path = path_exe;
    }
    else
    {
      std::cout << "Config not found\n";
    }
  }
  
  // load config
  Config &configvar = *this->config();
  if (!configvar.load(cfg))
  {
    throw CoreException("Could not load config!");
  }
  
  LOG2(INFO, "Using config: " + cfg);
  
  if (overrides.size())
  {
    std::stringstream override_config;
    for (size_t i = 0; i < overrides.size(); i++)
    {
      LOG2(INFO, "Overriden: " + overrides[i]);
      override_config << overrides[i] << ';' << std::endl;
    }
    // override config
    if (!configvar.load(override_config))
      throw CoreException("Error when parsing overrides: maybe you forgot to doublequote string values?");
  }
  
  memset(&m_listenEvent, 0, sizeof(event));
  initConstants();
  // Write PID to file
  std::ofstream pid_out((config()->sData("system.pid_file")).c_str());
  if (!pid_out.fail())
  {
    pid_out << getpid();
  }
  pid_out.close();


  // screen::init() needs m_plugin
  m_plugin = new Plugin();

  init_plugin_api();

  if (config()->bData("system.interface.use_cli"))
  {
    // Init our Screen
    screen()->init(VERSION);
  }


  LOG2(INFO, "Welcome to Mineserver v" + VERSION);
  LOG2(INFO, "Using zlib "+std::string(ZLIB_VERSION)+" libevent "+std::string(event_get_version()));

  MapGen* mapgen = new MapGen();
  MapGen* nethergen = new NetherGen();
  MapGen* heavengen = new HeavenGen();
  MapGen* biomegen = new BiomeGen();
  MapGen* eximgen = new EximGen();
  m_mapGenNames.push_back(mapgen);
  m_mapGenNames.push_back(nethergen);
  m_mapGenNames.push_back(heavengen);
  m_mapGenNames.push_back(biomegen);
  m_mapGenNames.push_back(eximgen);

  m_saveInterval = m_config->iData("map.save_interval");

  m_only_helmets = m_config->bData("system.armour.helmet_strict");
  m_pvp_enabled = m_config->bData("system.pvp.enabled");
  m_damage_enabled = m_config->bData("system.damage.enabled");

  const char* key = "map.storage.nbt.directories"; // Prefix for worlds config
  if (m_config->has(key) && (m_config->type(key) == CONFIG_NODE_LIST))
  {
    std::list<std::string> tmp = m_config->mData(key)->keys();
    int n = 0;
    for (std::list<std::string>::const_iterator it = tmp.begin(); it != tmp.end(); ++it)
    {
      m_map.push_back(new Map());
      Physics* phy = new Physics;
      phy->map = n;
      m_physics.push_back(phy);
      int k = m_config->iData((std::string(key) + ".") + (*it));
      if ((uint32_t)k >= m_mapGenNames.size())
      {
        std::ostringstream s;
        s << "Error! Mapgen number " << k << " in config. " << m_mapGenNames.size() << " Mapgens known";
        LOG2(INFO, s.str());
      }
      // WARNING: if k is too big this will be an access error! -- louisdx
      MapGen* m = m_mapGenNames[k];
      m_mapGen.push_back(m);
      n++;
    }
  }
  else
  {
    LOG2(WARNING, "Cannot find map.storage.nbt.directories.*");
  }

  if (m_map.size() == 0)
    throw CoreException("No worlds in Config");

  m_chat           = new Chat;
  m_furnaceManager = new FurnaceManager;
  m_packetHandler  = new PacketHandler;
  m_inventory      = new Inventory(m_config->sData("system.path.data") + '/' + "recipes", ".recipe", "ENABLED_RECIPES.cfg");
  m_mobs           = new Mobs;

} // End Mineserver constructor

Mineserver::~Mineserver()
{
  // Let the user know we're shutting the server down cleanly
  LOG2(INFO, "Shutting down...");

  // Close the cli session if its in use
  if (config() && config()->bData("system.interface.use_cli"))
    screen()->end();

  // Free memory
  for (std::vector<Map*>::size_type i = 0; i < m_map.size(); i++)
  {
    delete m_map[i];
    delete m_physics[i];
    delete m_mapGen[i];
  }

  delete m_chat;
  delete m_furnaceManager;
  delete m_packetHandler;
  delete m_inventory;
  delete m_mobs;

  if (m_plugin)
  {
    delete m_plugin;
    m_plugin = NULL;
  }

  // Remove the PID file
  unlink((config()->sData("system.pid_file")).c_str());
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
  for (std::set<User*>::const_iterator it = users().begin(); it != users().end(); ++it)
  {
    if ((*it)->logged) (*it)->saveData();
  }
}

size_t Mineserver::getLoggedUsersCount()
{
  size_t count = 0;
  for(std::set<User*>::const_iterator it = users().begin(); it != users().end(); ++it) {
    if((*it)->logged) count++;
  }
  return count;
}


bool Mineserver::run()
{
  uint32_t starttime = (uint32_t)time(0);
  uint32_t tick      = (uint32_t)time(0);


  // load plugins
  if (config()->has("system.plugins") && (config()->type("system.plugins") == CONFIG_NODE_LIST))
  {
    std::list<std::string> tmp = config()->mData("system.plugins")->keys();
    for (std::list<std::string>::const_iterator it = tmp.begin(); it != tmp.end(); ++it)
    {
      std::string path  = config()->sData("system.path.plugins");
      std::string name  = config()->sData("system.plugins." + (*it));
      std::string alias = *it;
      if (name[0] == '_')
      {
        path = "";
        alias = name;
        name = name.substr(1);
      }

      plugin()->loadPlugin(name, path, alias);
    }
  }

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
        if (show_progress)
        {
#ifdef WIN32
          t_begin = timeGetTime();
#else
          t_begin = clock();
#endif
        }
        for (int z = -size; z <= size; z++)
        {
          m_map[i]->loadMap(x, z);
        }

        if (show_progress)
        {
#ifdef WIN32
          t_end = timeGetTime();
          LOG2(INFO, dtos((x + size + 1) *(size * 2 + 1)) + "/" + dtos((size * 2 + 1) *(size * 2 + 1)) + " done. " + dtos((t_end - t_begin) / (size * 2 + 1)) + "ms per chunk");
#else
          t_end = clock();
          LOG2(INFO, dtos((x + size + 1) *(size * 2 + 1)) + "/" + dtos((size * 2 + 1) *(size * 2 + 1)) + " done. " + dtos(((t_end - t_begin) / (CLOCKS_PER_SEC / 1000)) / (size * 2 + 1)) + "ms per chunk");
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
  const std::string ip = config()->sData("net.ip");

  // Load port from config
  const int port = config()->iData("net.port");

#ifdef WIN32
  WSADATA wsaData;
  int iResult;
  // Initialize Winsock
  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0)
  {
    LOG2(ERROR, std::string("WSAStartup failed with error: " + iResult));
    return false;
  }
#endif

  struct sockaddr_in addresslisten;
  int reuse = 1;

  m_eventBase = reinterpret_cast<event_base*>(event_init());
#ifdef WIN32
  m_socketlisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
  m_socketlisten = socket(AF_INET, SOCK_STREAM, 0);
#endif

  if (m_socketlisten < 0)
  {
    LOG2(ERROR, "Failed to create listen socket");
    return false;
  }

  memset(&addresslisten, 0, sizeof(addresslisten));

  addresslisten.sin_family      = AF_INET;
  addresslisten.sin_addr.s_addr = inet_addr(ip.c_str());
  addresslisten.sin_port        = htons(port);

  //Reuse the socket
  setsockopt(m_socketlisten, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

  // Bind to port
  if (bind(m_socketlisten, (struct sockaddr*)&addresslisten, sizeof(addresslisten)) < 0)
  {
    LOG2(ERROR, "Failed to bind to " + ip + ":" + dtos(port));
    return false;
  }

  if (listen(m_socketlisten, 5) < 0)
  {
    LOG2(ERROR, "Failed to listen to socket");
    return false;
  }

  setnonblock(m_socketlisten);
  event_set(&m_listenEvent, m_socketlisten, EV_WRITE | EV_READ | EV_PERSIST, accept_callback, NULL);
  event_add(&m_listenEvent, NULL);

  LOG2(INFO, "Listening on: ");
  if (ip == "0.0.0.0")
  {
    // Print all local IPs
    char name[255];
    gethostname(name, sizeof(name));
    struct hostent* hostinfo = gethostbyname(name);
    int ipIndex = 0;
    while (hostinfo && hostinfo->h_addr_list[ipIndex])
    {
      const std::string ip(inet_ntoa(*(struct in_addr*)hostinfo->h_addr_list[ipIndex++]));
      LOG2(INFO, ip + ":" + dtos(port));
    }
  }
  else
  {
    LOG2(INFO, ip + ":" + dtos(port));
  }

  //Let event_base_loop lock for 200ms
  timeval loopTime;
  loopTime.tv_sec  = 0;
  loopTime.tv_usec = 200000; // 200ms

  m_running = true;
  event_base_loopexit(m_eventBase, &loopTime);

  // Create our Server Console user so we can issue commands

  time_t timeNow = time(NULL);
  while (m_running && event_base_loop(m_eventBase, 0) == 0)
  {
    event_base_loopexit(m_eventBase, &loopTime);

    // Run 200ms timer hook
    static_cast<Hook0<bool>*>(plugin()->getHook("Timer200"))->doAll();

    // Alert any block types that care about timers
    for (size_t i = 0 ; i < plugin()->getBlockCB().size(); ++i)
    {
      const BlockBasicPtr blockcb = plugin()->getBlockCB()[i];
      if (blockcb != NULL)
      {
        blockcb->timer200();
      }
    }

    //Update physics every 200ms
    for (std::vector<Map*>::size_type i = 0 ; i < m_map.size(); i++)
    {
      physics(i)->update();
    }

    //Every 10 seconds..
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
      if (!User::all().empty())
      {
        // Send server time and keepalive
        Packet pkt;
        pkt << Protocol::timeUpdate(m_map[0]->mapTime);        
        pkt << Protocol::keepalive(0);
        pkt << Protocol::playerlist();
        (*User::all().begin())->sendAll(pkt);
      }

      //Check for tree generation from saplings
      for (size_t i = 0; i < m_map.size(); ++i)
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
      for (std::set<User*>::iterator it = users().begin(), it_end = users().end(); it != it_end;)
      {
	// NOTE: iterators corrupt when you delete their objects, therefore we have to iterate in a special way - Justasic
	User *u = *it;
	++it;
        // No data received in 30s, timeout
        if (u->logged && timeNow - u->lastData > 30)
        {
          LOG2(INFO, "Player " + u->nick + " timed out");
          delete u;
        }
        else if (!u->logged && timeNow - u->lastData > 100)
          delete u;
        else
        {
          if (m_damage_enabled)
          {
            u->checkEnvironmentDamage();
          }
          u->pushMap();
          u->popMap();
        }

      }

      for (std::vector<Map*>::size_type i = 0 ; i < m_map.size(); i++)
      {
        m_map[i]->mapTime += 20;
        if (m_map[i]->mapTime >= 24000)
        {
          m_map[i]->mapTime = 0;
        }
      }

      for (std::set<User*>::const_iterator it = users().begin(); it != users().end(); ++it)
      {
        (*it)->pushMap();
        (*it)->popMap();
      }

      // Check for Furnace activity
      furnaceManager()->update();

      // Run 1s timer hook
      static_cast<Hook0<bool>*>(plugin()->getHook("Timer1000"))->doAll();
    }

    // Underwater check / drowning
    // ToDo: this could be done a bit differently? - Fador
    // -- User::all() == users() - louisdx

    for (std::set<User*>::const_iterator it = users().begin(); it != users().end(); ++it)
    {
      (*it)->isUnderwater();
      if ((*it)->pos.y < 0)
      {
        (*it)->sethealth((*it)->health - 5);
      }
    }
  }
  #ifdef WIN32
  closesocket(m_socketlisten);
  #else
  close(m_socketlisten);
  #endif

  saveAll();

  event_base_free(m_eventBase);

  return true;
}

bool Mineserver::stop()
{
  m_running = false;
  return true;
}

Map* Mineserver::map(size_t n) const
{
  if (n < m_map.size())
  {
    return m_map[n];
  }
  LOG2(WARNING, "Nonexistent map requested. Map 0 passed");
  return m_map[0];
}
