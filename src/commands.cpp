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
#include "tools.h"
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

  void rules(User *user, std::string command, std::deque<std::string> args)
  {
    User *tUser = user;
    if(!args.empty() && user->admin)
      tUser = getUserByNick(args[0]);
    if(tUser != NULL)
    {
      // Send rules
      std::ifstream ifs(Conf::get().sValue("rules_file").c_str());
      std::string temp;

      if(ifs.fail())
      {
        std::cout << "> Warning: " << Conf::get().sValue("rules_file") << " not found." << std::endl;
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

void ban(User *user, std::string command, std::deque<std::string> args)
{
	std::string victim = args[0];
    User *tUser        = getUserByNick(victim);

	std::fstream bannedf;
	bannedf.open("banned.txt",std::fstream::app);
	bannedf << victim << std::endl;
	bannedf.close();

	if(tUser != NULL)
    {
      args.pop_front();
      std::string kickMsg;
      if(args.empty())
        kickMsg = Conf::get().sValue("default_banned_message");
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
		Chat::get().sendMsg(user, COLOR_DARK_MAGENTA + victim +" was banned in his absence!", Chat::USER);

	// Reload list with banned users
	Chat::get().loadBanned(BANNEDFILE);
}

void unban(User *user, std::string command, std::deque<std::string> args)
{
	std::string victim = args[0];
    User *tUser        = getUserByNick(victim);

	std::string line;
    std::ifstream in("banned.txt");

    std::ofstream out("banned.tmp");

    while( getline(in,line) )
    {
        if(line != victim)
            out << line << "\n";
    }
    in.close();
    out.close();

    remove("banned.txt");
    rename("banned.tmp","banned.txt");

	// Reload list with banned users
	Chat::get().loadBanned(BANNEDFILE);
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

void mute(User *user, std::string command, std::deque<std::string> args)
{
  if(!args.empty())
  {
    std::string victim = args[0];

    User *tUser        = getUserByNick(victim);

    if(tUser != NULL)
    {
      args.pop_front();
      std::string muteMsg;
      while(!args.empty())
      {
        muteMsg += args[0] + " ";
        args.pop_front();
      }
      
      tUser->mute(muteMsg);
      Chat::get().sendMsg(user, COLOR_RED + tUser->nick + " was muted by " + user->nick + ": " + muteMsg, Chat::ADMINS);
    }
    else
      reportError(user, "User " + victim + " not found (see /players)");
  }
  else
    reportError(user, "Usage: /mute user [reason]");
}
void unmute(User *user, std::string command, std::deque<std::string> args)
{
  if(!args.empty())
  {
    std::string victim = args[0];

    User *tUser        = getUserByNick(victim);

    if(tUser != NULL)
    {      
      tUser->unmute();
      Chat::get().sendMsg(user, COLOR_RED + tUser->nick + " was unmuted by " + user->nick + ".", Chat::ADMINS);
    }
    else
      reportError(user, "User " + victim + " not found (see /players)");
  }
  else
    reportError(user, "Usage: /unmute user");
}
void showMOTD(User *user, std::string command, std::deque<std::string> args)
{
  // Open MOTD file
  std::ifstream motdfs(Conf::get().sValue("motd_file").c_str());
  
  // Load MOTD into string and send to user if not a comment
  std::string msgLine;
  while(getline( motdfs, msgLine ))
  {
    // If not commentline
    if(msgLine[0] != COMMENTPREFIX)
    {
      Chat::get().sendMsg(user, msgLine, Chat::USER);
    }
  }
  
  // Close motd file
  motdfs.close();
  
}
void setTime(User *user, std::string command, std::deque<std::string> args)
{
  if(args.size() == 1)
  {
    Map::get().mapTime = (sint64)atoi(args[0].c_str());
    Packet pkt;
    pkt << (sint8)PACKET_TIME_UPDATE << (sint64)Map::get().mapTime;
    if(Users.size())
      Users[0]->sendAll((uint8*)pkt.getWrite(), pkt.getWriteLen());
    Chat::get().sendMsg(user, COLOR_MAGENTA + "Time set to " + args[0], Chat::USER);
  } 
  else
    reportError(user, "Usage: /settime time (time = 0-24000)");
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

std::string getHeadingString(User *user)
{
  // Compass heading labels
  std::string headingLabels[8] = { "North", "Northeast", "East", "Southeast", "South", "Southwest", "West", "Northwest" };
  
  // Get the user's heading and normalize
  int headingAngle = int(user->pos.yaw) % 360;
  if(headingAngle < 0)
    headingAngle = 360 + headingAngle;
      
  // Work out the text heading based on 8 points of the compass
  std::string headingText = "Unknown";
  if((headingAngle > 337.5 && headingAngle <= 360) || (headingAngle >= 0 && headingAngle <= 22.5))   // Special case for North as it spans 360 to 0
    headingText = headingLabels[0];
  else {
    int index = int(floor((headingAngle + 22.5) / 45));     // Add 22.5 so that we can identify the compass box correctly
    headingText = headingLabels[index];
  }
  return headingText + " (" + dtos(headingAngle) + "')";

}
void showPosition(User *user, std::string command, std::deque<std::string> args)
{
  if(args.size() == 1)
  {
    User *tUser = getUserByNick(args[0]);
    if(tUser != NULL)
      Chat::get().sendMsg(user, COLOR_MAGENTA + args[0] + " is at: " + dtos(tUser->pos.x)
                                                                     + " " 
                                                                     + dtos(tUser->pos.y)
                                                                     + " " 
                                                                     + dtos(tUser->pos.z)
                                                                     + " Heading: " + getHeadingString(tUser)
                                                                     , Chat::USER);
    else
      reportError(user, "User " + args[0] + " not found (see /players)");
  }
  else
  {
    Chat::get().sendMsg(user, COLOR_MAGENTA + "You are at: " + dtos(user->pos.x) 
                                                             + " " 
                                                             + dtos(user->pos.y) 
                                                             + " " 
                                                             + dtos(user->pos.z)
                                                             + " Heading: " + getHeadingString(user)
                                                             , Chat::USER);
  }
}

void regenerateLighting(User *user, std::string command, std::deque<std::string> args)
{
  printf("Regenerating lighting for chunk %d,%d\n", blockToChunk((sint32)user->pos.x), blockToChunk((sint32)user->pos.z));
  //First load the map
  if(Map::get().loadMap(blockToChunk((sint32)user->pos.x), blockToChunk((sint32)user->pos.z)))
  {
    //Then regenerate lighting
    Map::get().generateLight(blockToChunk((sint32)user->pos.x), blockToChunk((sint32)user->pos.z));
  }
}

void reloadConfiguration(User *user, std::string command, std::deque<std::string> args)
{
  Chat::get().loadAdmins(Conf::get().sValue("admin_file"));
  Chat::get().loadBanned(BANNEDFILE);
  Chat::get().loadWhitelist(WHITELISTFILE);
  Conf::get().load(CONFIG_FILE);

  // Set physics enable state based on config
  Physics::get().enabled = (Conf::get().bValue("liquid_physics"));

  Chat::get().sendMsg(user, COLOR_DARK_MAGENTA + "SERVER: "+COLOR_RED+"Reloaded admins and config", Chat::USER);

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
  registerCommand("ban", ban, true);
  registerCommand("unban", unban, true);
  registerCommand("kit", kit, false);
  registerCommand("save", saveMap, true);
  registerCommand("kick", kick, true);
  registerCommand("ctp", coordinateTeleport, true);
  registerCommand("tp", userTeleport, true);
  registerCommand("reload", reloadConfiguration, true);
  registerCommand("give", giveItems, true);
  registerCommand("gps", showPosition, true);
  registerCommand("settime", setTime, true);  
  registerCommand("regen", regenerateLighting, true);
  registerCommand("mute", mute, true);
  registerCommand("unmute", unmute, true);
  registerCommand("motd", showMOTD, false);
}
