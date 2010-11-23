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

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <deque>
#include <fstream>
#include <vector>
#include <ctime>
#include <math.h>
#ifdef WIN32
  #include <winsock2.h>
#else
  #include <netinet/in.h>
  #include <string.h>
#endif

#include "logger.h"
#include "constants.h"

#include "tools.h"
#include "map.h"
#include "user.h"
#include "chat.h"
#include "config.h"
#include "physics.h"

namespace
{

  void reportError(User *user, std::string message)
  {
    Chat::get().sendMsg(user, COLOR_DARK_MAGENTA + "Error! " + COLOR_RED + message, Chat::USER);
  }

  void playerList(User *user, std::string command, std::deque<std::string> args)
  {
    Chat::get().sendUserlist(user);
  }

          void about(User *user, std::string command, std::deque<std::string> args)
{
  Chat::get().sendMsg(user, COLOR_DARK_MAGENTA + Conf::get().sValue("servername")+
                      "Running Mineserver v." + VERSION, Chat::USER);
}

  // TODO: Check if rulesfile exists
  void rules(User *user, std::string command, std::deque<std::string> args)
  {
    User *tUser = user;
    if(!args.empty() && user->admin)
      tUser = getUserByNick(args[0]);
    if(tUser != NULL)
    {
      // Send rules
      std::ifstream ifs( RULESFILE.c_str());
      std::string temp;

      if(ifs.fail())
      {
        std::cout << "> Warning: " << RULESFILE << " not found." << std::endl;
        return;
      }
      else
      {
        while(getline(ifs, temp))
        {
          // If not a comment
          if(!temp.empty() && temp[0] != COMMENTPREFIX)
            Chat::get().sendMsg(tUser, temp, Chat::USER);
        }
        ifs.close();
      }
    }
    else
      reportError(user, "User " + args[0] + " not found (see /players)");
  }
}

void home(User *user, std::string command, std::deque<std::string> args)
{
  Chat::get().sendMsg(user, COLOR_DARK_MAGENTA + "Teleported you home!", Chat::USER);
  user->teleport(Map::get().spawnPos.x(), Map::get().spawnPos.y() + 2, Map::get().spawnPos.z());
}

void kit(User *user, std::string command, std::deque<std::string> args)
{
  if(!args.empty())
  {
    std::vector<int> kitItems = Conf::get().vValue("kit_" + args[0]);
    // If kit is found
    if(!kitItems.empty())
    {
      for(uint32 i=0;i<kitItems.size();i++)
      {
        spawnedItem item;
        item.EID     = generateEID();
        item.item    = kitItems[i];
        item.count   = 1;
        item.health=0;
        item.pos.x() = static_cast<int>(user->pos.x*32 + (rand() % 30));
        item.pos.y() = static_cast<int>(user->pos.y*32);
        item.pos.z() = static_cast<int>(user->pos.z*32 + (rand() % 30));
        Map::get().sendPickupSpawn(item);
      }
    }
    else
      reportError(user, "Kit " + args[0] + " not found");
  }
}

void saveMap(User *user, std::string command, std::deque<std::string> args)
{
  Map::get().saveWholeMap();
  Chat::get().sendMsg(user, COLOR_DARK_MAGENTA + "SERVER:" + COLOR_RED + " Saved map to disc",
                      Chat::USER);
}

void kick(User *user, std::string command, std::deque<std::string> args)
{
  if(!args.empty())
  {
    std::string victim = args[0];

    User *tUser        = getUserByNick(victim);

    if(tUser != NULL)
    {
      args.pop_front();
      std::string kickMsg;
      if(args.empty())
        kickMsg = Conf::get().sValue("default_kick_message");
      else
      {
        while(!args.empty())
        {
          kickMsg += args[0] + " ";
          args.pop_front();
        }
      }

      tUser->kick(kickMsg);
    }
    else
      reportError(user, "User " + victim + " not found (see /players)");
  }
  else
    reportError(user, "Usage: /kick user [reason]");
}

void coordinateTeleport(User *user, std::string command, std::deque<std::string> args)
{
  if(args.size() > 2)
  {
    LOG(user->nick + " teleport to: " + args[0] + " " + args[1] + " " + args[2]);
    double x = atof(args[0].c_str());
    double y = atof(args[1].c_str());
    double z = atof(args[2].c_str());
    user->teleport(x, y, z);
  }
}

void userTeleport(User *user, std::string command, std::deque<std::string> args)
{
  if(args.size() == 1)
  {
    LOG(user->nick + " teleport to: " + args[0]);
    User *tUser = getUserByNick(args[0]);
    if(tUser != NULL)
      user->teleport(tUser->pos.x, tUser->pos.y + 2, tUser->pos.z);
    else
      reportError(user, "User " + args[0] + " not found (see /players)");
  }
  else if(args.size() == 2)
  {
    LOG(user->nick + ": teleport " + args[0] + " to " + args[1]);

    User *whoUser = getUserByNick(args[0]);
    User *toUser   = getUserByNick(args[1]);

    if(whoUser != NULL && toUser != NULL)
    {
      whoUser->teleport(toUser->pos.x, toUser->pos.y + 2, toUser->pos.z);
      Chat::get().sendMsg(user, COLOR_MAGENTA + "Teleported!", Chat::USER);
    }
    else
    {
      reportError(user, "User " + (whoUser == NULL ? args[0] : args[1])+
                  " not found (see /players");
    }
  }
}

void reloadConfiguration(User *user, std::string command, std::deque<std::string> args)
{
  Chat::get().loadAdmins(ADMINFILE);
  Conf::get().load(CONFIGFILE);

  // Set physics enable state based on config
  Physics::get().enabled = ((Conf::get().iValue("liquid_physics") == 0) ? false : true);

  Chat::get().sendMsg(user, COLOR_DARK_MAGENTA + "SERVER:" + COLOR_RED+
                      " Reloaded admins and config", Chat::USER);

  // Note that the MOTD is loaded on-demand each time it is requested
}

bool isValidItem(int id)
{
  if(id < 1)  // zero or negative items are all invalid
    return false;

  if(id > 91 && id < 256)  // these are undefined blocks and items
    return false;

  if(id == 2256 || id == 2257)  // records are special cased
    return true;

  if(id > 350)  // high items are invalid
    return false;

  if(id >= BLOCK_RED_CLOTH && id <= BLOCK_GRAY_CLOTH)  // coloured cloth causes client crashes
    return false;

  return true;
}

int roundUpTo(int x, int nearest) 
{ 
  x += (nearest - 1); 
  x /= nearest; 
  x *= nearest; 
  return x; 
} 

void giveItems(User *user, std::string command, std::deque<std::string> args)
{
  User *tUser = NULL;
  int itemId = 0, itemCount = 1, itemStacks = 1;

  if(args.size() > 1)
  {
    tUser = getUserByNick(args[0]);

    //First check if item is a number
    itemId = atoi(args[1].c_str());

    //If item was not a number, search the name from config
    if(itemId == 0)
      itemId = Conf::get().iValue(args[1]);

    // Check item validity
    if(!isValidItem(itemId))
    {
      reportError(user, "Item " + args[1] + " not found.");
      return;
    }

    if(args.size() > 2)
    {
      itemCount = atoi(args[2].c_str());
      // If multiple stacks
      itemStacks = roundUpTo(itemCount, 64) / 64; 
      itemCount  -= (itemStacks-1) * 64; 
    }
  }
  else
  {
    reportError(user, "Too few parameters.");
	  return;
  }

  if(tUser)
  {
    int amount = 64;
    for(int i = 0; i < itemStacks; i++)
    {
      // if last stack
      if(i == itemStacks - 1)
        amount = itemCount;

      spawnedItem item;
      item.EID     = generateEID();
      item.item    = itemId;
      item.health  = 0;
      item.count   = amount;
      item.pos.x() = static_cast<int>(tUser->pos.x * 32);
      item.pos.y() = static_cast<int>(tUser->pos.y * 32);
      item.pos.z() = static_cast<int>(tUser->pos.z * 32);

      Map::get().sendPickupSpawn(item);
    }

    Chat::get().sendMsg(user, COLOR_RED + user->nick + " spawned " + args[1], Chat::ADMINS);
  }
  else
    reportError(user, "User " + args[0] + " not found (see /players)");
}

void Chat::registerStandardCommands()
{
  registerCommand("players", playerList, false);
  registerCommand("about", about, false);
  registerCommand("rules", rules, false);
  registerCommand("home", home, false);
  registerCommand("kit", kit, false);
  registerCommand("save", saveMap, true);
  registerCommand("kick", kick, true);
  registerCommand("ctp", coordinateTeleport, true);
  registerCommand("tp", userTeleport, true);
  registerCommand("reload", reloadConfiguration, true);
  registerCommand("give", giveItems, true);
}
