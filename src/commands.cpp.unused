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
#include "plugin.h"
#include "mineserver.h"

namespace
{
  void reportError(User* user, std::string message)
  {
    Mineserver::get()->chat()->sendMsg(user, MC_COLOR_DARK_MAGENTA + "Error! " + MC_COLOR_RED + message, Chat::USER);

    // Let the console know
    if(user->UID == SERVER_CONSOLE_UID)
    {
      Mineserver::get()->screen()->log(LOG_CHAT, message);
    }
  }

  void pluginLoad(User* user, std::string command, std::deque<std::string> args)
  {
    if (args.size() == 2)
    {
      Mineserver::get()->plugin()->loadPlugin(args[0], args[1]);
    }

    if (args.size() == 1)
    {
      Mineserver::get()->plugin()->loadPlugin(args[0]);
    }

    return;
  }

  void pluginUnload(User* user, std::string command, std::deque<std::string> args)
  {
    if (args.size() < 1)
    {
      return;
    }

    Mineserver::get()->plugin()->unloadPlugin(args[0]);
  }

  void playerList(User* user, std::string command, std::deque<std::string> args)
  {
    Mineserver::get()->chat()->sendUserlist(user);
  }

  void about(User* user, std::string command, std::deque<std::string> args)
  {
    if (Mineserver::get()->config()->bData("show_version"))
    {
      Mineserver::get()->chat()->sendMsg(user, MC_COLOR_BLUE + Mineserver::get()->config()->sData("system.server_name") + "Running Mineserver v." + VERSION, Chat::USER);
    }
  }

  void rules(User* user, std::string command, std::deque<std::string> args)
  {
    User* tUser = user;

    if(!args.empty() && IS_ADMIN(user->permissions))
    {
      tUser = User::byNick(args[0]);
    }

    if(tUser != NULL)
    {
      // Send rules
      std::ifstream ifs(Mineserver::get()->config()->sData("rules_file").c_str());
      std::string temp;

      if(ifs.fail())
      {
        Mineserver::get()->screen()->log("> Warning: " + Mineserver::get()->config()->sData("rules_file") + " not found.");
        return;
      }
      else
      {
        while(getline(ifs, temp))
        {
          // If not a comment
          if(!temp.empty() && temp[0] != COMMENTPREFIX)
          {
            Mineserver::get()->chat()->sendMsg(tUser, temp, Chat::USER);
          }
        }
        ifs.close();
      }
    }
    else
    {
      reportError(user, "Player " + args[0] + " not found (see /players)");
    }
  }
}

void home(User* user, std::string command, std::deque<std::string> args)
{
  Mineserver::get()->chat()->sendMsg(user, MC_COLOR_BLUE + "Teleported you home!", Chat::USER);
  user->teleport(Mineserver::get()->map()->spawnPos.x(), Mineserver::get()->map()->spawnPos.y() + 2, Mineserver::get()->map()->spawnPos.z());
}

void saveMap(User* user, std::string command, std::deque<std::string> args)
{
  Mineserver::get()->map()->saveWholeMap();
  Mineserver::get()->chat()->handleMsg(user, "% Saved map.");
}

void kick(User* user, std::string command, std::deque<std::string> args)
{
  if(!args.empty())
  {
    User* tUser = User::byNick(args[0]);

    if(tUser != NULL)
    {
      args.pop_front();
      std::string kickMsg;
      if(args.empty())
      {
        kickMsg = Mineserver::get()->config()->sData("default_kick_message");
      }
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
    {
      reportError(user, "User " + args[0] + " not found (see /players)");
    }
  }
  else
  {
    reportError(user, "Usage: /kick player [reason]");
  }
}

void mute(User* user, std::string command, std::deque<std::string> args)
{
  if(!args.empty())
  {
    std::string victim = args[0];

    User* tUser        = User::byNick(victim);

    if(tUser != NULL)
    {
      args.pop_front();
      std::string muteMsg;
      while(!args.empty())
      {
        muteMsg += args[0] + " ";
        args.pop_front();
      }

      // Tell the user they're muted
      tUser->mute(muteMsg);

      // Tell the admins what happened
      std::string adminMsg = MC_COLOR_RED + tUser->nick + " was muted by " + user->nick + ". ";
      if(!muteMsg.empty())
      {
        adminMsg += " Reason: " + muteMsg;
      }

      Mineserver::get()->chat()->sendMsg(user, adminMsg, Chat::ADMINS);
    }
    else
    {
      reportError(user, "User " + victim + " not found (see /players)");
    }
  }
  else
  {
    reportError(user, "Usage: /mute player [reason]");
  }
}

void unmute(User* user, std::string command, std::deque<std::string> args)
{
  if(!args.empty())
  {
    std::string victim = args[0];

    User* tUser        = User::byNick(victim);

    if(tUser != NULL)
    {
      tUser->unmute();
      Mineserver::get()->chat()->sendMsg(user, MC_COLOR_RED + tUser->nick + " was unmuted by " + user->nick + ".", Chat::ADMINS);
    }
    else
    {
      reportError(user, "User " + victim + " not found (see /players)");
    }
  }
  else
  {
    reportError(user, "Usage: /unmute player");
  }
}

void showMOTD(User* user, std::string command, std::deque<std::string> args)
{
  // Open MOTD file
  std::ifstream motdfs(Mineserver::get()->config()->sData("motd_file").c_str());

  // Load MOTD into string and send to user if not a comment
  std::string msgLine;
  while(getline( motdfs, msgLine ))
  {
    // If not commentline
    if(msgLine[0] != COMMENTPREFIX)
    {
      Mineserver::get()->chat()->sendMsg(user, msgLine, Chat::USER);
    }
  }

  // Close motd file
  motdfs.close();
}

void emote(User* user, std::string command, std::deque<std::string> args)
{
  if(user->isAbleToCommunicate(command) == false)
  {
    return;
  }

  std::string emoteMsg;
  while(!args.empty())
  {
    emoteMsg += args[0] + " ";
    args.pop_front();
  }

  if(emoteMsg.empty())
  {
    reportError(user, "Usage: /" + command + " message");
  }
  else
  {
    Mineserver::get()->screen()->log(LOG_CHAT, "* "+ user->nick + " " + emoteMsg);
    Mineserver::get()->chat()->sendMsg(user, MC_COLOR_DARK_ORANGE + "* " + user->nick + " " + emoteMsg, Chat::ALL);
  }
}

void whisper(User* user, std::string command, std::deque<std::string> args)
{
  if(user->isAbleToCommunicate(command) == false)
  {
    return;
  }

  if(!args.empty())
  {
    std::string targetNick = args[0];

    User* tUser = User::byNick(targetNick);

    // Don't whisper or tell if DND is set
    if(tUser != NULL && tUser->dnd)
    {
      Mineserver::get()->chat()->sendMsg(user, MC_COLOR_YELLOW + tUser->nick + " currently doesn't want to be disturbed.", Chat::USER);
      Mineserver::get()->chat()->sendMsg(user, MC_COLOR_YELLOW + "Message not sent.", Chat::USER);
      return;
    }

    if(tUser != NULL)
    {
      args.pop_front();
      std::string whisperMsg;
      while(!args.empty())
      {
        whisperMsg += args[0] + " ";
        args.pop_front();
      }

      Mineserver::get()->chat()->sendMsg(tUser, MC_COLOR_YELLOW + user->nick + " whispers: " + MC_COLOR_GREEN + whisperMsg, Chat::USER);
      Mineserver::get()->chat()->sendMsg(user, MC_COLOR_YELLOW + "You whisper to " + tUser->nick + ": " + MC_COLOR_GREEN + whisperMsg, Chat::USER);
    }
    else
    {
      reportError(user, "User " + targetNick + " not found (see /players)");
    }
  }
  else
  {
    reportError(user, "Usage: /" + command + " player [message]");
  }
}

void doNotDisturb(User* user, std::string command, std::deque<std::string> args)
{
  user->toggleDND();
}

void help(User* user, std::string command, std::deque<std::string> args)
{
  Mineserver::get()->chat()->sendHelp(user, args);
}

void setTime(User* user, std::string command, std::deque<std::string> args)
{
  if(args.size() == 1)
  {
    std::string timeValue = args[0];

    // Check for time labels
    if(timeValue == "day" || timeValue == "morning")
    {
      timeValue = "24000";
    }
    else if (timeValue == "dawn")
    {
      timeValue = "22500";
    }
    else if (timeValue == "noon")
    {
      timeValue = "6000";
    }
    else if (timeValue == "dusk")
    {
      timeValue = "12000";
    }
    else if (timeValue == "night" || timeValue == "midnight")
    {
      timeValue = "18000";
    }

    Mineserver::get()->map()->mapTime = (sint64)atoi(timeValue.c_str());
    Packet pkt;
    pkt << (sint8)PACKET_TIME_UPDATE << (sint64)Mineserver::get()->map()->mapTime;

    if(User::all().size())
    {
      User::all()[0]->sendAll((uint8*)pkt.getWrite(), pkt.getWriteLen());
    }

    Mineserver::get()->chat()->handleMsg(user, "% World time changed.");
  }
  else
  {
    reportError(user, "Usage: /settime time (time = 0-24000)");
  }
}

void coordinateTeleport(User* user, std::string command, std::deque<std::string> args)
{
  if(args.size() == 3)
  {
    LOG(user->nick + " teleport to: " + args[0] + " " + args[1] + " " + args[2]);
    double x = atof(args[0].c_str());
    double y = atof(args[1].c_str());
    double z = atof(args[2].c_str());
    user->teleport(x, y, z);
    Mineserver::get()->chat()->sendMsg(user, MC_COLOR_BLUE + "Teleported!", Chat::USER);
  }
  else
  {
    reportError(user, "Usage: /ctp x y z");
  }
}

void userTeleport(User* user, std::string command, std::deque<std::string> args)
{
  if(args.size() == 1)
  {
    LOG(user->nick + " teleport to: " + args[0]);
    User* tUser = User::byNick(args[0]);
    if(tUser != NULL)
    {
      user->teleport(tUser->pos.x, tUser->pos.y + 2, tUser->pos.z);
      Mineserver::get()->chat()->sendMsg(user, MC_COLOR_BLUE + "Teleported!", Chat::USER);
    }
    else
    {
      reportError(user, "User " + args[0] + " not found (see /players)");
    }
  }
  else if(args.size() == 2)
  {
    LOG(user->nick + ": teleport " + args[0] + " to " + args[1]);

    User* whoUser = User::byNick(args[0]);
    User* toUser   = User::byNick(args[1]);

    if(whoUser != NULL && toUser != NULL)
    {
      whoUser->teleport(toUser->pos.x, toUser->pos.y + 2, toUser->pos.z);
      Mineserver::get()->chat()->sendMsg(user, MC_COLOR_BLUE + "Teleported!", Chat::USER);
    }
    else
    {
      reportError(user, "User " + (whoUser == NULL ? args[0] : args[1])+
                  " not found (see /players");
    }
  }
  else
  {
    reportError(user, "Usage: /tp [player] targetplayer");
  }
}

std::string getHeadingString(User* user)
{
  // Compass heading labels
  std::string headingLabels[8] = { "North", "Northeast", "East", "Southeast", "South", "Southwest", "West", "Northwest" };

  // Get the user's heading and normalize
  int headingAngle = int(user->pos.yaw) % 360;
  if(headingAngle < 0)
  {
    headingAngle = 360 + headingAngle;
  }

  // Work out the text heading based on 8 points of the compass
  std::string headingText = "Unknown";
  if((headingAngle > 337.5 && headingAngle <= 360) || (headingAngle >= 0 && headingAngle <= 22.5))   // Special case for North as it spans 360 to 0
  {
    headingText = headingLabels[0];
  }
  else
  {
    headingText = headingLabels[int(floor((headingAngle + 22.5) / 45))]; // Add 22.5 so that we can identify the compass box correctly
  }

  return headingText + " (" + dtos(headingAngle) + "')";
}

void showPosition(User* user, std::string command, std::deque<std::string> args)
{
  if(args.size() == 1)
  {
    User* tUser = User::byNick(args[0]);
    if(tUser != NULL)
    {
      Mineserver::get()->chat()->sendMsg(user, MC_COLOR_BLUE + args[0] + " is at: " + dtos(tUser->pos.x)
                                                                      + " "
                                                                      + dtos(tUser->pos.y)
                                                                      + " "
                                                                      + dtos(tUser->pos.z)
                                                                      + " Heading: " + getHeadingString(tUser)
                                                                      , Chat::USER);
    }
    else
    {
      reportError(user, "User " + args[0] + " not found (see /players)");
    }
  }
  else if(args.size() == 0)
  {
    Mineserver::get()->chat()->sendMsg(user, MC_COLOR_BLUE + "You are at: " + dtos(user->pos.x)
                                                              + " "
                                                              + dtos(user->pos.y)
                                                              + " "
                                                              + dtos(user->pos.z)
                                                              + " Heading: " + getHeadingString(user)
                                                              , Chat::USER);
  }
  else
  {
    reportError(user, "Usage: /gps [player]");
  }
}

void regenerateLighting(User* user, std::string command, std::deque<std::string> args)
{
  printf("Regenerating lighting for chunk %d,%d\n", blockToChunk((sint32)user->pos.x), blockToChunk((sint32)user->pos.z));
  //First load the map
  if(Mineserver::get()->map()->loadMap(blockToChunk((sint32)user->pos.x), blockToChunk((sint32)user->pos.z)))
  {
    //Then regenerate lighting
    Mineserver::get()->map()->generateLight(blockToChunk((sint32)user->pos.x), blockToChunk((sint32)user->pos.z));
  }
}

void reloadConfiguration(User* user, std::string command, std::deque<std::string> args)
{
  Mineserver::get()->config()->load(CONFIG_FILE);

  // Set physics enable state based on config
  Mineserver::get()->physics()->enabled = (Mineserver::get()->config()->bData("liquid_physics"));

  Mineserver::get()->chat()->sendMsg(user, MC_COLOR_DARK_MAGENTA + "Reloaded admins and config", Chat::USER);

  // Note that the MOTD is loaded on-demand each time it is requested
}

bool isValidItem(int id)
{
  if(id < 1)  // zero or negative items are all invalid
  {
    return false;
  }

  if(id > 91 && id < 256)  // these are undefined blocks and items
  {
    return false;
  }

  if(id == 2256 || id == 2257)  // records are special cased
  {
    return true;
  }

  if(id > 350)  // high items are invalid
  {
    return false;
  }

  if(id >= BLOCK_RED_CLOTH && id <= BLOCK_GRAY_CLOTH)  // coloured cloth causes client crashes
  {
    return false;
  }

  return true;
}

int roundUpTo(int x, int nearest)
{
  return (((x + (nearest - 1)) / nearest) * nearest );
}

void giveItems(User* user, std::string command, std::deque<std::string> args)
{
  if(args.size() == 2 || args.size() == 3)
  {
    User* tUser = User::byNick(args[0]);
    int itemId = 0;

    //First check if item is a number
    itemId = atoi(args[1].c_str());

    //If item was not a number, search the name from config
    if(itemId == 0)
    {
      itemId = Mineserver::get()->config()->iData(args[1]);
    }

    // Check item validity
    if(isValidItem(itemId))
    {
      if(tUser)
      {
        int itemCount = 1, itemStacks = 1;

        if(args.size() == 3)
        {
          itemCount = atoi(args[2].c_str());
          if(itemCount>1024) itemCount=1024;
          // If multiple stacks
          itemStacks = roundUpTo(itemCount, 64) / 64;
          itemCount  -= (itemStacks-1) * 64;
        }

        int amount = 64;
        for(int i = 0; i < itemStacks; i++)
        {
          // if last stack
          if(i == itemStacks - 1)
          {
            amount = itemCount;
          }

          spawnedItem item;
          item.EID     = generateEID();
          item.item    = itemId;
          item.health  = 0;
          item.count   = amount;
          item.pos.x() = static_cast<int>(tUser->pos.x * 32);
          item.pos.y() = static_cast<int>(tUser->pos.y * 32);
          item.pos.z() = static_cast<int>(tUser->pos.z * 32);
          Mineserver::get()->map()->sendPickupSpawn(item);
        }

        Mineserver::get()->chat()->sendMsg(user, MC_COLOR_RED + user->nick + " spawned " + args[1], Chat::ADMINS);
      }
      else
      {
        reportError(user, "User " + args[0] + " not found (see /players)");
      }
    }
    else
    {
      reportError(user, "Item " + args[1] + " not found.");
    }
  }
  else
  {
    reportError(user, "Usage: /give player item [count]");
  }
}

void giveItemsSelf(User* user, std::string command, std::deque<std::string> args)
{
  if(args.size() == 1 || args.size() == 2)
  {
    User* tUser = user;
    int itemId = 0;

    //First check if item is a number
    itemId = atoi(args[0].c_str());

    //If item was not a number, search the name from config
    if(itemId == 0)
    {
      itemId = Mineserver::get()->config()->iData(args[0]);
    }

    // Check item validity
    if(isValidItem(itemId))
    {
      if(tUser)
      {
        int itemCount = 1, itemStacks = 1;

        if(args.size() == 2)
        {
          itemCount = atoi(args[1].c_str());
          if(itemCount>1024) itemCount=1024;
          // If multiple stacks
          itemStacks = roundUpTo(itemCount, 64) / 64;
          itemCount  -= (itemStacks-1) * 64;
        }

        int amount = 64;
        for(int i = 0; i < itemStacks; i++)
        {
          // if last stack
          if(i == itemStacks - 1)
          {
            amount = itemCount;
          }

          spawnedItem item;
          item.EID     = generateEID();
          item.item    = itemId;
          item.health  = 0;
          item.count   = amount;
          item.pos.x() = static_cast<int>(tUser->pos.x * 32);
          item.pos.y() = static_cast<int>(tUser->pos.y * 32);
          item.pos.z() = static_cast<int>(tUser->pos.z * 32);
          Mineserver::get()->map()->sendPickupSpawn(item);
        }

        Mineserver::get()->chat()->sendMsg(user, MC_COLOR_RED + user->nick + " spawned " + args[0], Chat::ADMINS);
      }
    }
    else
    {
      reportError(user, "Item " + args[0] + " not found.");
    }
  }
  else
  {
    reportError(user, "Usage: /igive item [count]");
  }
}

void setHealth(User* user, std::string command, std::deque<std::string> args)
{
  if(args.size() == 2)
  {
    User* tUser = User::byNick(args[0]);

    if(tUser)
    {
      tUser->sethealth(atoi(args[1].c_str()));
    }
    else
    {
      reportError(user, "User " + args[0] + " not found (see /players)");
    }
  }
  else
  {
    reportError(user, "Usage: /sethealth [player] health (health = 0-20)");
  }
}

void Chat::registerStandardCommands()
{
  // Players
  registerCommand(new Command(parseCmd("about"), "", "Display server name & version", about, Mineserver::get()->config()->commandPermission("about")));
  registerCommand(new Command(parseCmd("home"), "", "Teleport to map spawn location", home, Mineserver::get()->config()->commandPermission("home")));
  registerCommand(new Command(parseCmd("motd"), "", "Display Message Of The Day", showMOTD, Mineserver::get()->config()->commandPermission("motd")));
  registerCommand(new Command(parseCmd("players who"), "", "Lists online players", playerList, Mineserver::get()->config()->commandPermission("who")));
  registerCommand(new Command(parseCmd("rules"), "", "Display server rules", rules, Mineserver::get()->config()->commandPermission("rules")));
  registerCommand(new Command(parseCmd("e em emote me"), "", "Emote", emote, Mineserver::get()->config()->commandPermission("emote")));
  registerCommand(new Command(parseCmd("whisper w tell t"), "<player> <message>", "Send a private message", whisper, Mineserver::get()->config()->commandPermission("whisper")));
  registerCommand(new Command(parseCmd("dnd"), "", "Do Not Disturb - toggles receiving chat messages", doNotDisturb, Mineserver::get()->config()->commandPermission("dnd")));
  registerCommand(new Command(parseCmd("help"), "[<commandName>]", "Display this help message.", help, Mineserver::get()->config()->commandPermission("help")));

  // Admins Only
  registerCommand(new Command(parseCmd("ctp"), "<x> <y> <z>", "Teleport to coordinates (eg. /ctp 100 100 100)", coordinateTeleport, Mineserver::get()->config()->commandPermission("ctp")));
  registerCommand(new Command(parseCmd("give"), "<player> <id/alias> [count]", "Gives <player> [count] pieces of <id/alias>. By default [count] = 1", giveItems, Mineserver::get()->config()->commandPermission("give")));
  registerCommand(new Command(parseCmd("igive i"), "<id/alias> [count]", "Gives self [count] pieces of <id/alias>. By default [count] = 1", giveItemsSelf, Mineserver::get()->config()->commandPermission("igive")));
  registerCommand(new Command(parseCmd("gps"), "[<player>]", "Show own coordinates or show <player>'s coordinates", showPosition, Mineserver::get()->config()->commandPermission("gps")));
  registerCommand(new Command(parseCmd("kick"), "<player>", "Kicks a player with optional kick message", kick, Mineserver::get()->config()->commandPermission("kick")));
  registerCommand(new Command(parseCmd("mute"), "<player>", "Mutes a player with optional message", mute, Mineserver::get()->config()->commandPermission("mute")));
  registerCommand(new Command(parseCmd("regen"), "", "Regenerates lightning", regenerateLighting, Mineserver::get()->config()->commandPermission("regen")));
  registerCommand(new Command(parseCmd("reload"), "", "Reload admins and configuration", reloadConfiguration, Mineserver::get()->config()->commandPermission("reload")));
  registerCommand(new Command(parseCmd("save"), "", "Manually save map to disc", saveMap, Mineserver::get()->config()->commandPermission("save")));
  registerCommand(new Command(parseCmd("sethealth"), "<player> <health>", "Set a player's health. <health> = 0-20", setHealth, Mineserver::get()->config()->commandPermission("sethealth")));
  registerCommand(new Command(parseCmd("settime"), "<time>", "Sets server time. (<time> = 0-24000, 0 & 24000 = day, ~15000 = night)", setTime, Mineserver::get()->config()->commandPermission("settime")));
  registerCommand(new Command(parseCmd("tp"), "<player> [<anotherPlayer>]", "Teleport yourself to <player>'s position or <player> to <anotherPlayer>", userTeleport, Mineserver::get()->config()->commandPermission("tp")));
  registerCommand(new Command(parseCmd("unmute"), "<player>", "Unmutes a given player", unmute, Mineserver::get()->config()->commandPermission("unmute")));
  registerCommand(new Command(parseCmd("load"), "<name> <file>", "Loads a plugin from a file", pluginLoad, Mineserver::get()->config()->commandPermission("load")));
  registerCommand(new Command(parseCmd("unload"), "<name>", "Unloads a plugin", pluginUnload, Mineserver::get()->config()->commandPermission("unload")));
}
