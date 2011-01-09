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

#ifndef _MINESERVER_H
#define _MINESERVER_H

#include <vector>
#ifdef WIN32
  #include <winsock2.h>
#else
  #include <netinet/in.h>
#endif

#include <iostream>

#include <event.h>

#include "user.h"
#include "map.h"
#include "chat.h"
#include "plugin.h"
#include "physics.h"
#include "config.h"
#include "logger.h"
#include "furnaceManager.h"
#include "worldgen/mapgen.h"
#include "inventory.h"

#ifdef FADOR_PLUGIN
#define MINESERVER
#include "plugin_api.h"
#undef MINESERVER
#endif

struct event_base;

class Mineserver
{
public:
  static Mineserver* get()
  {
    static Mineserver* m_instance;
    if (!m_instance)
    {
      m_instance = new Mineserver;
    }
    return m_instance;
  }

  int run(int argc, char* argv[]);
  bool stop();
  event_base* getEventBase();

  std::vector<User*>& users() { return m_users; }

  struct event m_listenEvent;
  int m_socketlisten;
  void updatePlayerList();

  Map* map() { if (!m_map) { m_map = new Map; } return m_map; }
  void setMap(Map* map) { m_map = map; }
  Chat* chat() { if (!m_chat) { m_chat = new Chat; } return m_chat; }
  void setChat(Chat* chat) { m_chat = chat; }
  Plugin* plugin() { if (!m_plugin) { m_plugin = new Plugin; } return m_plugin; }
  void setPlugin(Plugin* plugin) { m_plugin = plugin; }
  Screen* screen() { if (!m_screen) { m_screen = new Screen; } return m_screen; }
  void setScreen(Screen* screen) { m_screen = screen; }
  Physics* physics() { if (!m_physics) { m_physics = new Physics; } return m_physics; }
  void setPhysics(Physics* physics) { m_physics = physics; }
  Config* config() { if (!m_config) { m_config = new Config; } return m_config; }
  void setConfig(Config* config) { m_config = config; }
  FurnaceManager* furnaceManager() { if (!m_furnaceManager) { m_furnaceManager = new FurnaceManager; } return m_furnaceManager; }
  void setFurnaceManager(FurnaceManager* furnaceManager) { m_furnaceManager = furnaceManager; }
  PacketHandler* packetHandler() { if (!m_packetHandler) { m_packetHandler = new PacketHandler; } return m_packetHandler; }
  void setPacketHandler(PacketHandler* packetHandler) { m_packetHandler = packetHandler; }
  MapGen* mapGen() { if (!m_mapGen) { m_mapGen = new MapGen; } return m_mapGen; }
  void setMapGen(MapGen* mapGen) { m_mapGen = mapGen; }
  Logger* logger() { if (!m_logger) { m_logger = new Logger; } return m_logger; }
  void setLogger(Logger* logger) { m_logger = logger; }

  Inventory* inventory() { if (!m_inventory) { m_inventory = new Inventory; } return m_inventory; }
  void setInventory(Inventory* inventory) { m_inventory = m_inventory; }

private:
  Mineserver()
  {
    m_map            = NULL;
    m_chat           = NULL;
    m_plugin         = NULL;
    m_screen         = NULL;
    m_physics        = NULL;
    m_config         = NULL;
    m_furnaceManager = NULL;
    m_packetHandler  = NULL;
    m_mapGen         = NULL;
    m_logger         = NULL;
    m_inventory      = NULL;
  }
  event_base* m_eventBase;
  bool m_running;
  // holds all connected users
  std::vector<User*> m_users;

  static Mineserver* m_instance;

  Map* m_map;
  Chat* m_chat;
  Plugin* m_plugin;
  Screen* m_screen;
  Physics* m_physics;
  Config* m_config;
  FurnaceManager* m_furnaceManager;
  PacketHandler* m_packetHandler;
  MapGen* m_mapGen;
  Logger* m_logger;
  Inventory* m_inventory;
};

#endif
