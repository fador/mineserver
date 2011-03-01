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

#include <iostream>
#include <vector>
#include <set>

#ifdef WIN32
// This is needed for event to work on Windows.
#include <Winsock2.h>
#else
//Do not remove!! Required on Debian
#include <sys/types.h>
#endif
#include <event.h>

class User;
class Map;
class Chat;
class Plugin;
class Screen;
class Config;
class FurnaceManager;
class PacketHandler;
class Physics;
class MapGen;
class Logger;
class Inventory;
class Mobs;
class Mob;

#define MINESERVER
#include "plugin_api.h"
#undef MINESERVER


struct event_base;

uint32_t generateEID();

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

  std::vector<User*>& users()
  {
    return m_users;
  }

  struct event m_listenEvent;
  int m_socketlisten;
  int m_saveInterval;
  time_t m_lastSave;
  bool m_pvp_enabled;
  bool m_damage_enabled;
  bool m_only_helmets;

  Map* map(int n);
  void setMap(Map* map, int n = 0);
  int mapCount();

  Chat* chat() const
  {
    return m_chat;
  }
  void setChat(Chat* chat)
  {
    m_chat = chat;
  }
  Mobs* mobs() const
  {
    return m_mobs;
  }
  Plugin* plugin() const
  {
    return m_plugin;
  }
  void setPlugin(Plugin* plugin)
  {
    m_plugin = plugin;
  }
  Screen* screen() const
  {
    return m_screen;
  }
  void setScreen(Screen* screen)
  {
    m_screen = screen;
  }
  Physics* physics(int n);
  Config* config() const
  {
    return m_config;
  }
  void setConfig(Config* config)
  {
    m_config = config;
  }
  FurnaceManager* furnaceManager() const
  {
    return m_furnaceManager;
  }
  void setFurnaceManager(FurnaceManager* furnaceManager)
  {
    m_furnaceManager = furnaceManager;
  }
  PacketHandler* packetHandler() const
  {
    return m_packetHandler;
  }
  void setPacketHandler(PacketHandler* packetHandler)
  {
    m_packetHandler = packetHandler;
  }
  MapGen* mapGen(int n);
  Logger* logger() const
  {
    return m_logger;
  }
  void setLogger(Logger* logger)
  {
    m_logger = logger;
  }
  Inventory* inventory() const
  {
    return m_inventory;
  }
  void setInventory(Inventory* inventory)
  {
    m_inventory = m_inventory;
  }

  void saveAllPlayers();
  void saveAll();

  void parseCommandLine(int argc, char* argv[]);

private:
  Mineserver();
  event_base* m_eventBase;
  bool m_running;
  // holds all connected users
  std::vector<User*> m_users;

  static Mineserver* m_instance;

  std::vector<Map*> m_map;
  std::vector<Physics*> m_physics;
  std::vector<MapGen*> gennames;
  Chat* m_chat;
  Plugin* m_plugin;
  Screen* m_screen;
  Config* m_config;
  FurnaceManager* m_furnaceManager;
  PacketHandler* m_packetHandler;
  std::vector<MapGen*> m_mapGen;
  Logger* m_logger;
  Inventory* m_inventory;
  Mobs* m_mobs;
};

#endif
