/*
g++ -c command.cpp
g++ -shared command.o -o command.so

copy command.so to Mineserver bin directory.
*/
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

#include <string>
#include <deque>
#include <ctime>
#include <sstream>
#include <cstdlib>
#include <map>
#include <iostream>
#include <stdint.h>

#define MINESERVER_C_API
#include "../../src/plugin_api.h"
//#include "../../src/logtype.h"

#include "command.h"

#define PLUGIN_COMMAND_VERSION 1.1
const char CHATCMDPREFIX   = '/';
mineserver_pointer_struct* mineserver;

struct cuboidStruct
{
  int x;
  char y;
  int z;
  char block;
  bool active;
  int state;
};

std::map<std::string,cuboidStruct> cuboidMap;

std::string dtos(double n)
{
  std::ostringstream result;
  result << n;
  return result.str();
}

std::deque<std::string> parseCmd(std::string cmd)
{
  int del;
  std::deque<std::string> temp;

  while(cmd.length() > 0)
  {
    while(cmd[0] == ' ')
    {
      cmd = cmd.substr(1);
    }

    del = cmd.find(' ');

    if(del > -1)
    {
      temp.push_back(cmd.substr(0, del));
      cmd = cmd.substr(del+1);
    }
    else
    {
      temp.push_back(cmd);
      break;
    }
  }

  if(temp.empty())
  {
    temp.push_back("empty");
  }

  return temp;
}

typedef void (*CommandCallback)(std::string nick, std::string, std::deque<std::string>);

struct Command
{
  CommandCallback callback;  
  Command(std::deque<std::string> _names, std::string _arguments, std::string _description, CommandCallback _callback) : names(_names),arguments(_arguments),description(_description),callback(_callback) {}
  std::deque<std::string> names;
  std::string arguments;
  std::string description;
};

typedef std::map<std::string, Command*> CommandList;
CommandList m_Commands;

void registerCommand(Command* command)
{
  // Loop thru all the words for this command
  std::string currentWord;
  std::deque<std::string> words = command->names;
  while(!words.empty())
  {
    currentWord = words[0];
    words.pop_front();
    m_Commands[currentWord] = command;
  }
}

#define LOG_INFO 6
bool chatPreFunction(const char* userIn,time_t timestamp, const char* msgIn)
{
  std::string user(userIn);
  std::string msg(msgIn);

  if(msg.size() == 0)
  {
    return false;
  }

  char prefix = msg[0];

  std::string logMsg = "Command Plugin got from "+user+": " + msg;
  mineserver->logger.log(LOG_INFO, "plugin.command", logMsg.c_str());

  if (prefix == CHATCMDPREFIX)
  {
    // Timestamp
    time_t rawTime = timestamp;
    struct tm* Tm  = localtime(&rawTime);
    std::string timeStamp (asctime(Tm));
    timeStamp = timeStamp.substr(11, 5);

    std::deque<std::string> cmd = parseCmd(msg.substr(1));

    std::string command = cmd[0];
    cmd.pop_front();

    // User commands
    CommandList::iterator iter;
    if((iter = m_Commands.find(command)) != m_Commands.end())
    {
      iter->second->callback(user, command, cmd);
      return false;
    }
  }

  return true;
}

bool isValidItem(int id)
{
  if (id < 1)  // zero or negative items are all invalid
  {
    return false;
  }

  if (id > 91 && id < 256)  // these are undefined blocks and items
  {
    return false;
  }

  if (id == 2256 || id == 2257)  // records are special cased
  {
    return true;
  }

  if (id > 350)  // high items are invalid
  {
    return false;
  }

  return true;
}

int roundUpTo(int x, int nearest)
{
  return (((x + (nearest - 1)) / nearest) * nearest );
}

void giveItems(std::string userIn, std::string command, std::deque<std::string> args)
{
  if (args.size() == 2 || args.size() == 3)
  {
    std::string user = args[0];
    int itemId = 0;

    //First check if item is a number
    itemId = atoi(args[1].c_str());

    //If item was not a number, search the name from config
    if (itemId == 0)
    {
      itemId = mineserver->config.iData(args[1].c_str());
    }

    // Check item validity
    if (isValidItem(itemId))
    {
      double x,y,z;
      if (mineserver->user.getPosition(user.c_str(),&x,&y,&z,NULL,NULL,NULL))
      {
        int itemCount = 1, itemStacks = 1;

        if (args.size() == 3)
        {
          itemCount = atoi(args[2].c_str());
          if (itemCount>1024) itemCount=1024;
          // If multiple stacks
          itemStacks = roundUpTo(itemCount, 64) / 64;
          itemCount  -= (itemStacks-1) * 64;
        }

        int amount = 64;
        for (int i = 0; i < itemStacks; i++)
        {
          // if last stack
          if (i == itemStacks - 1)
          {
            amount = itemCount;
          }

          mineserver->map.createPickupSpawn((int)x,(int)y,(int)z,itemId,amount,0,user.c_str());
        }
      }

    }
    else
    {
      mineserver->chat.sendmsgTo(userIn.c_str(),  "Not a valid item");      
    }
  }
  else
  {
    mineserver->chat.sendmsgTo(userIn.c_str(),"Usage: /give player item [count]");
  }
}

void giveItemsSelf(std::string user, std::string command, std::deque<std::string> args)
{
  if (args.size() == 1 || args.size() == 2)
  {
    int itemId = 0;

    //First check if item is a number
    itemId = atoi(args[0].c_str());

    //If item was not a number, search the name from config
    if (itemId == 0)
    {
      itemId = mineserver->config.iData(args[0].c_str());
    }

    // Check item validity
    if (isValidItem(itemId))
    {
      double x,y,z;
      if (mineserver->user.getPosition(user.c_str(),&x,&y,&z,NULL,NULL,NULL))
      {
        int itemCount = 1, itemStacks = 1;

        if (args.size() == 2)
        {
          itemCount = atoi(args[1].c_str());
          if(itemCount>1024) itemCount=1024;
          // If multiple stacks
          itemStacks = roundUpTo(itemCount, 64) / 64;
          itemCount  -= (itemStacks-1) * 64;
        }

        int amount = 64;
        for (int i = 0; i < itemStacks; i++)
        {
          // if last stack
          if (i == itemStacks - 1)
          {
            amount = itemCount;
          }

          mineserver->map.createPickupSpawn((int)x,(int)y,(int)z,itemId,amount,0,user.c_str());
        }
      }

    }
    else
    {
      mineserver->chat.sendmsgTo(user.c_str(),"Not a valid item");      
    }
  }
  else
  {
    mineserver->chat.sendmsgTo(user.c_str(),"Usage: /igive item [count]");
  }
}

void home(std::string user, std::string command, std::deque<std::string> args)
{
  mineserver->chat.sendmsgTo(user.c_str(),"Teleported you home!");
  int x,y,z;
  mineserver->map.getSpawn(&x,&y,&z);
  mineserver->user.teleport(user.c_str(),x, y + 2, z);
}

void userTeleport(std::string user, std::string command, std::deque<std::string> args)
{
  if(args.size() == 1)
  {
    std::string tUser = args[0];
    double x,y,z;
    if(mineserver->user.getPosition(tUser.c_str(), &x,&y,&z,NULL,NULL,NULL))
    {
      mineserver->user.teleport(user.c_str(),x,y+2,z);
      mineserver->chat.sendmsgTo(user.c_str(), "Teleported!");
    }
    else
    {
      std::string msg = "User " + args[0] + " not found (see /players)";
      mineserver->chat.sendmsgTo(user.c_str(), msg.c_str());
    }
  }
  else if(args.size() == 2)
  {
    std::string whoUser = args[0];
    std::string toUser  = args[1];

    double x,y,z;
    if(mineserver->user.getPosition(toUser.c_str(), &x,&y,&z,NULL,NULL,NULL))
    {
      mineserver->user.teleport(whoUser.c_str(),x,y+2,z);
      mineserver->chat.sendmsgTo(user.c_str(), "Teleported!");
    }
    else
    {
      std::string msg = "User " + args[0] + " not found (see /players)";
      mineserver->chat.sendmsgTo(user.c_str(), msg.c_str());
    }
  }
  else
  {
    mineserver->chat.sendmsgTo(user.c_str(), "Usage: /tp [player] targetplayer");
  }
}

void cuboid(std::string user, std::string command, std::deque<std::string> args)
{
  if(cuboidMap.find(user) != cuboidMap.end())
  {
    cuboidMap.erase(cuboidMap.find(user));
  }
  cuboidMap[user].active = 1;
  cuboidMap[user].state = 0;
  mineserver->chat.sendmsgTo(user.c_str(),"Cuboid start, place first block");
}

void playerList(std::string user, std::string command, std::deque<std::string> args)
{
  mineserver->chat.sendUserlist(user.c_str());
}

void saveMap(std::string user, std::string command, std::deque<std::string> args)
{
  mineserver->map.saveWholeMap();
  mineserver->chat.sendmsgTo(user.c_str(),"Saved map!");
}

void setTime(std::string user, std::string command, std::deque<std::string> args)
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

    mineserver->map.setTime(atoi(timeValue.c_str()));

    mineserver->chat.sendmsgTo(user.c_str(),"World time changed.");
  }
  else
  {
    mineserver->chat.sendmsgTo(user.c_str(),"Usage: /settime time (time = 0-24000)");
  }
}

// Direction
enum Direction
{
   BLOCK_BOTTOM, BLOCK_NORTH, BLOCK_SOUTH, BLOCK_EAST, BLOCK_WEST, BLOCK_TOP
};

bool translateDirection(int32_t *x, int8_t *y, int32_t *z, int8_t direction)
{
    switch(direction)
    {
    case BLOCK_BOTTOM: (*y)--;  break;
    case BLOCK_TOP:    (*y)++;  break;
    case BLOCK_NORTH:  (*x)++;  break;
    case BLOCK_SOUTH:  (*x)--;  break;
    case BLOCK_EAST:   (*z)++;  break;
    case BLOCK_WEST:   (*z)--;  break;
    default:                  break;
    }
  return true;
}

bool blockPlacePreFunction(const char* userIn, int32_t x,int8_t y,int32_t z,int16_t block,int8_t direction)
{  
  translateDirection(&x,&y,&z,direction);
  std::string user(userIn);
  if(cuboidMap.find(user) != cuboidMap.end())
  {
    if(cuboidMap[user].active)
    {
      if(cuboidMap[user].state == 0)
      {
        cuboidMap[user].state = 1;
        cuboidMap[user].x     = x;
        cuboidMap[user].y     = y;
        cuboidMap[user].z     = z;
        cuboidMap[user].block = block;
        mineserver->chat.sendmsgTo(user.c_str(),"First block done, place second");
      }
      else if(cuboidMap[user].state == 1)
      {
        if(cuboidMap[user].block == block)
        {
          if(cuboidMap[user].y == y)
          {
            int xstart,xend;
            int zstart,zend;
            xstart=(x<cuboidMap[user].x)?x:cuboidMap[user].x;
            xend=(x<cuboidMap[user].x)?cuboidMap[user].x:x;

            zstart=(z<cuboidMap[user].z)?z:cuboidMap[user].z;
            zend=(z<cuboidMap[user].z)?cuboidMap[user].z:z;
            for(int xpos = xstart; xpos <= xend; xpos ++)
            {
              for(int zpos = zstart;  zpos <= zend; zpos ++)
              {                
                mineserver->map.setBlock(xpos,y,zpos,block,0);
              }
            }
            mineserver->chat.sendmsgTo(user.c_str(),"Cuboid done");
          }
        }
        cuboidMap.erase(cuboidMap.find(user));
      }
    }
  }

  return true;
}

void sendHelp(std::string user, std::string command, std::deque<std::string> args)
{
  // TODO: Add paging support, since not all commands will fit into
  // the screen at once.

  CommandList* commandList = &m_Commands; // defaults
  //std::string commandColor = MC_COLOR_BLUE;

  if (args.size() == 0)
  {
    for(CommandList::iterator it = commandList->begin();it != commandList->end();++it)
    {
      std::string args = it->second->arguments;
      std::string description = it->second->description;
      std::string msg = /*commandColor +*/ CHATCMDPREFIX + it->first + " " + args + " : " /*+ MC_COLOR_YELLOW*/ + description;
      mineserver->chat.sendmsgTo(user.c_str(), msg.c_str());
    }
  }
  else
  {
    CommandList::iterator iter;
    if ((iter = commandList->find(args.front())) != commandList->end())
    {
      std::string args = iter->second->arguments;
      std::string description = iter->second->description;
      std::string msg = /*commandColor +*/ CHATCMDPREFIX + iter->first + " " + args;
      mineserver->chat.sendmsgTo(user.c_str(), msg.c_str());
      msg = /*MC_COLOR_YELLOW + */CHATCMDPREFIX + description;
      mineserver->chat.sendmsgTo(user.c_str(), msg.c_str());
    }
    else
    {
      std::string msg = /*MC_COLOR_RED +*/ "Unknown Command: " + args.front();
      mineserver->chat.sendmsgTo(user.c_str(),msg.c_str());
    }
  }
}

std::string pluginName = "command";

PLUGIN_API_EXPORT void CALLCONVERSION command_init(mineserver_pointer_struct* mineserver_temp)
{
  mineserver = mineserver_temp;

  if (mineserver->plugin.getPluginVersion(pluginName.c_str()) > 0)
  {
    std::string msg = "command is already loaded v."+dtos(mineserver->plugin.getPluginVersion(pluginName.c_str()));
    mineserver->logger.log(LOG_INFO, "plugin.command", msg.c_str());
    return;
  }
  std::string msg = "Loaded "+pluginName+"!";
  mineserver->logger.log(LOG_INFO, "plugin.command", msg.c_str());

  mineserver->plugin.setPluginVersion(pluginName.c_str(), PLUGIN_COMMAND_VERSION);

  mineserver->plugin.addCallback("PlayerChatPre", (void *)chatPreFunction);
  mineserver->plugin.addCallback("BlockPlacePre", (void *)blockPlacePreFunction);

  registerCommand(new Command(parseCmd("igive i"), "<id/alias> [count]", "Gives self [count] pieces of <id/alias>. By default [count] = 1", giveItemsSelf));
  registerCommand(new Command(parseCmd("home"), "", "Teleport to map spawn location", home));
  registerCommand(new Command(parseCmd("settime"), "<time>", "Sets server time. (<time> = 0-24000, 0 & 24000 = day, ~15000 = night)", setTime));
  registerCommand(new Command(parseCmd("cuboid"), "", "type in the command and place two blocks, it will fill the space between them", cuboid));  
  registerCommand(new Command(parseCmd("players who names list"), "", "Lists online players", playerList));
  registerCommand(new Command(parseCmd("give"), "<player> <id/alias> [count]", "Gives <player> [count] pieces of <id/alias>. By default [count] = 1", giveItems));
  registerCommand(new Command(parseCmd("save"), "", "Manually save map to disc", saveMap));  
  registerCommand(new Command(parseCmd("help"), "[<commandName>]", "Display this help message.", sendHelp));
  registerCommand(new Command(parseCmd("tp"), "<player> [<anotherPlayer>]", "Teleport yourself to <player>'s position or <player> to <anotherPlayer>", userTeleport));
}

PLUGIN_API_EXPORT void CALLCONVERSION command_shutdown(void)
{
  if (mineserver->plugin.getPluginVersion(pluginName.c_str()) <= 0)
  {
    mineserver->logger.log(LOG_INFO, "plugin.command", "command is not loaded!");
    return;
  }
}
