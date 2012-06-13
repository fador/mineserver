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

#ifndef _MINESERVER_H
#define _MINESERVER_H

#include <vector>
#include <set>
#include <string>

#include "tr1.h"
#include TR1INCLUDE(memory)

#ifdef WIN32
// This is needed for event to work on Windows.
#define NOMINMAX
#include <winsock2.h>
#else
//Do not remove!! Required on Debian
#include <sys/types.h>
#endif
#include <event.h>

#include "extern.h"

#define MINESERVER
#include "plugin_api.h"
#undef MINESERVER

struct event_base;

class Mineserver
{
public:
  // Variables
  char** argv;
  int argc;
  int m_socketlisten;
  int m_saveInterval;
  time_t m_lastSave;
  bool m_pvp_enabled;
  bool m_damage_enabled;
  bool m_only_helmets;
  struct event m_listenEvent;

  // Constructor/Destructor
  Mineserver(int, char**);
  ~Mineserver();

  // Non-inline functions
  bool run();
  bool stop();
  event_base* getEventBase();
  Map* map(size_t n) const;
  void saveAllPlayers();
  void saveAll();
  size_t getLoggedUsersCount();
  bool configDirectoryPrepare(const std::string& path);
  
  static uint32_t generateEID()
  {
    static uint32_t m_EID = 0;
    return ++m_EID;
  }

  // Inline functions
  // Get the total number of connected users
  inline std::set<User*>& users()
  {
    return m_users;
  }

  inline const std::set<User*>& users() const
  {
    return m_users;
  }

  inline void setMap(Map* map, size_t n = 0)
  {
    m_map[n] = map;
  }
  
  inline size_t mapCount()
  {
    return m_map.size();
  }
  
  inline Chat* chat() const
  {
    return m_chat;
  }
  
  inline void setChat(Chat* chat)
  {
    m_chat = chat;
  }
  
  inline Mobs* mobs() const
  {
    return m_mobs;
  }
  
  inline Plugin* plugin() const
  {
    return m_plugin;
  }
  
  inline void setPlugin(Plugin* plugin)
  {
    m_plugin = plugin;
  }
  
  inline std::tr1::shared_ptr<Screen> screen() const
  {
    return m_screen;
  }
  
  inline void setScreen(std::tr1::shared_ptr<Screen> screen)
  {
    m_screen = screen;
  }
  
  inline Physics* physics(size_t n) const
  {
    return m_physics[n];
  }

  // Get the configuration pointer
  inline std::tr1::shared_ptr<Config> config() const
  {
    return m_config;
  }

  // Set a pointer to the configuration file
  inline void setConfig(std::tr1::shared_ptr<Config> config)
  {
    m_config = config;
  }

  inline FurnaceManager* furnaceManager() const
  {
    return m_furnaceManager;
  }
  
  inline void setFurnaceManager(FurnaceManager* furnaceManager)
  {
    m_furnaceManager = furnaceManager;
  }
  
  inline PacketHandler* packetHandler() const
  {
    return m_packetHandler;
  }
  
  inline void setPacketHandler(PacketHandler* packetHandler)
  {
    m_packetHandler = packetHandler;
  }
  
  inline MapGen* mapGen(size_t n) const
  {
    return m_mapGen[n];
  }
  
  inline std::tr1::shared_ptr<Logger> logger() const
  {
    return m_logger;
  }

  // Set the logger to use
  inline void setLogger(std::tr1::shared_ptr<Logger> logger)
  {
    m_logger = logger;
  }
  
  inline Inventory* inventory() const
  {
    return m_inventory;
  }

  // Set a pointer to the inventory
  inline void setInventory(Inventory* inventory)
  {
    m_inventory = m_inventory;
  }

private:

  bool m_running;

  event_base* m_eventBase;

  // holds all connected users
  std::set<User*>    m_users;

  std::vector<Map*>     m_map;
  std::vector<Physics*> m_physics;
  std::vector<MapGen*>  m_mapGenNames;
  std::vector<MapGen*>  m_mapGen;

  // core modules
  std::tr1::shared_ptr<Config> m_config;
  std::tr1::shared_ptr<Screen> m_screen;
  std::tr1::shared_ptr<Logger> m_logger;

  Plugin*         m_plugin;
  Chat*           m_chat;
  FurnaceManager* m_furnaceManager;
  PacketHandler*  m_packetHandler;
  Inventory*      m_inventory;
  Mobs*           m_mobs;
};

#endif
