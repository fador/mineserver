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

#include <iostream>
#include <sstream>
#include <fstream> // Added for MOTD
#include <string>
#include <deque>
#include <stdint.h>
#include <cstdlib>
#include <ctime>

#include "tr1.h"
#include TR1INCLUDE(unordered_map)
#include TR1INCLUDE(memory)

#define MINESERVER_C_API
#include "plugin_api.h"

#include "commands.h"

enum { PLANE,REPLACE };

#define PLUGIN_COMMANDS_VERSION 1.1
const char CHATCMDPREFIX   = '/';
mineserver_pointer_struct* mineserver;

struct cuboidStruct
{
  int x;
  char y;
  int z;
  unsigned char block;
  bool active;
  int state;
  int action;
  int fromBlock;
  int toBlock;
};

std::tr1::unordered_map<std::string, cuboidStruct> cuboidMap;

std::string dtos(double n)
{
  std::ostringstream result;
  result << n;
  return result.str();
}

typedef void (*CommandCallback)(std::string nick, std::string, std::deque<std::string>);

struct Command
{
  Command(std::deque<std::string> _names, std::string _arguments, std::string _description, CommandCallback _callback)
    : names(_names),
      arguments(_arguments),
      description(_description),
      callback(_callback)
  {}
  std::deque<std::string> names;
  std::string arguments;
  std::string description;
  CommandCallback callback;  
};

typedef std::tr1::shared_ptr<Command> ComPtr;
typedef std::tr1::unordered_map<std::string, ComPtr> CommandList;
CommandList m_Commands;

void registerCommand(ComPtr command)
{
  // Loop thru all the words for this command
  std::string currentWord;
  std::deque<std::string> words = command->names;
  while(!words.empty())
  {
    currentWord = words.front();
    words.pop_front();
    m_Commands[currentWord] = command;
  }
}

bool chatCommandFunction(const char* userIn,const char* cmdIn, int argc, char** argv)
{
  std::string user(userIn);
  std::string command(cmdIn);
  std::deque<std::string> cmd(argv, argv+argc);

  if(command.size() == 0)
  {
    return false;
  }


  std::string logMsg = user + ": " + command;
  mineserver->logger.log(LOG_INFO, "plugin.commands", logMsg.c_str());

  // User commands
  CommandList::iterator iter;
  if((iter = m_Commands.find(command)) != m_Commands.end())
  {
    iter->second->callback(user, command, cmd);
    return true;
  }
  return false;
}

bool isValidItem(int id)
{
  if (id < 1)  // zero or negative items are all invalid
  {
    return false;
  }

  if (id > 96 && id < 256)  // these are undefined blocks and items
  {
    return false;
  }

  if (id == 2256 || id == 2257)  // records are special cased
  {
    return true;
  }

  if (id > 383)  // high items are invalid
  {
    return false;
  }

  return true;
}

int roundUpTo(int x, int nearest)
{
  return (((x + (nearest - 1)) / nearest) * nearest );
}

void giveItemsName(std::string userIn, int id, int count, int health)
{
  if (isValidItem(id))
  {
    int itemCount = 1, itemStacks = 1;

    if(count != 1)
    {
      itemCount = count;
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
      mineserver->user.addItem(userIn.c_str(), id, amount, health);
    }
  }
  else
  {
    mineserver->chat.sendmsgTo(userIn.c_str(),  "Not a valid item");
  }
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
    if(args.size()==2)
    {
      giveItemsName(user, itemId, 1,0);
    }
    else
    {
      giveItemsName(user, itemId, atoi(args[2].c_str()),0);
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
    if(args.size()==1)
    {
      giveItemsName(user, itemId, 1,0);
    }
    else
    {
      giveItemsName(user, itemId, atoi(args[1].c_str()),0);
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

void setSpawn(std::string user, std::string command, std::deque<std::string> args)
{
  if(args.size() == 0) {
    double x,y,z;
    mineserver->user.getPosition(user.c_str(), &x,&y,&z,NULL,NULL,NULL);
    
    mineserver->chat.sendmsgTo(user.c_str(),"Set spawn!");
    mineserver->map.setSpawn(x,y,z);
    mineserver->user.teleport(user.c_str(),x, y + 2, z);
  }
}

void userWorld(std::string user, std::string command, std::deque<std::string> args)
{
  if(args.size() == 1)
  {
     double x,y,z;
     mineserver->user.getPosition(user.c_str(), &x,&y,&z,NULL,NULL,NULL);
     mineserver->logger.log(LOG_INFO, "plugin.commands", (user + args[0]).c_str());
     mineserver->user.teleportMap(user.c_str(), x,y+2,z,atoi(args[0].c_str()));
  }

}

void coordinateTeleport(std::string user, std::string command, std::deque<std::string> args)
{
  if(args.size() == 3)
  {
    double x = atof(args[0].c_str());
    double y = atof(args[1].c_str());
    double z = atof(args[2].c_str());
    mineserver->user.teleport(user.c_str(), x, y, z);
  }
  else
  {
    mineserver->chat.sendmsgTo(user.c_str(),"Usage: /ctp x y z");
  }
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

void replace(std::string user, std::string command, std::deque<std::string> args)
{
  if (cuboidMap.find(user) != cuboidMap.end())
  {
    cuboidMap.erase(user);
  }
  if (args.size() == 2)
  {
    cuboidMap[user].active = 1;
    cuboidMap[user].state = 0;
    cuboidMap[user].action = REPLACE;

    int blockID = atoi(args[0].c_str());

    //If item was not a number, search the name from config
    if (blockID == 0)
    {
      blockID = mineserver->config.iData(args[0].c_str());
    }

    if(blockID < 1 || blockID > 255)
    {
      cuboidMap.erase(user);
      return;
    }

    cuboidMap[user].fromBlock = blockID;

    blockID = atoi(args[1].c_str());

    //If item was not a number, search the name from config
    if (blockID == 0 && args[1] != "0")
    {
      blockID = mineserver->config.iData(args[1].c_str());
    }

    if(blockID < 0 || blockID > 255)
    {
      cuboidMap.erase(user);
      return;
    }

    cuboidMap[user].toBlock = blockID;

    mineserver->chat.sendmsgTo(user.c_str(),"Cuboid replace start, hit first block");
  }
}


void replacechunk(std::string user, std::string command, std::deque<std::string> args)
{
  if(args.size() == 2)
  {
    double x,y,z;
    if(mineserver->user.getPosition(user.c_str(), &x,&y,&z,NULL,NULL,NULL))
    {

      int fromBlock = atoi(args[0].c_str());

      //If item was not a number, search the name from config
      if (fromBlock == 0)
      {
        fromBlock = mineserver->config.iData(args[0].c_str());
      }

      if(fromBlock < 1 || fromBlock > 255)
      {
        return;
      }


      int toBlock = atoi(args[1].c_str());

      //If item was not a number, search the name from config
      if (toBlock == 0 && args[1] != "0")
      {
        toBlock = mineserver->config.iData(args[1].c_str());
      }

      if(toBlock < 0 || toBlock > 255)
      {
        return;
      }

      int chunkx,chunkz;
      chunkx = ((int)x)>>4;
      chunkz = ((int)z)>>4;
      unsigned char* blocks = mineserver->map.getMapData_block(chunkx,chunkz);
      for(int bX = 0; bX < 16; bX++)
      {
        for(int bZ = 0; bZ < 16; bZ++)
        {
          for(int bY = 0; bY < 128; bY++)
          {
            if(blocks[bY + ((bZ << 7) + (bX << 11))] == fromBlock)
            {
              blocks[bY + ((bZ << 7) + (bX << 11))] = toBlock;
            }
          }
        }
      }

      // TODO: Send chunk update to all players.
      mineserver->chat.sendmsgTo(user.c_str(),"Replace chunk done");
    }
  }
}

void flattenchunk(std::string user, std::string command, std::deque<std::string> args)
{
  if(args.size() == 1)
  {
    double x,y,z;
    if(mineserver->user.getPosition(user.c_str(), &x,&y,&z,NULL,NULL,NULL))
    {

      int topBlock = atoi(args[0].c_str());

      //If item was not a number, search the name from config
      if (topBlock == 0)
      {
        topBlock = mineserver->config.iData(args[0].c_str());
      }

      if(topBlock < 1 || topBlock > 255)
      {
        return;
      }

      int chunkx,chunkz;
      chunkx = ((int)x)>>4;
      chunkz = ((int)z)>>4;
      unsigned char* blocks = mineserver->map.getMapData_block(chunkx,chunkz);
      for(int bX = 0; bX < 16; bX++)
      {
        for(int bZ = 0; bZ < 16; bZ++)
        {
          for(int bY = 127; bY >= 0; bY--)
          {
            if(bY >= y)
            {
              blocks[bY + ((bZ << 7) + (bX << 11))] = 0;
            }
            else if(bY == y - 1)
            {
              blocks[bY + ((bZ << 7) + (bX << 11))] = topBlock;
            }
            else
            {
              // Do nothing; leave this block untouched
            }
          }
        }
      }

      // TODO: Send chunk update to all players.
      mineserver->chat.sendmsgTo(user.c_str(),"Flatten chunk done");
    }
  }
}

void cuboid(std::string user, std::string command, std::deque<std::string> args)
{
  if(cuboidMap.find(user) != cuboidMap.end())
  {
    cuboidMap.erase(user);
  }
  cuboidMap[user].active = 1;
  cuboidMap[user].state = 0;
  cuboidMap[user].action = PLANE;
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

void getTime(std::string user, std::string command, std::deque<std::string> args)
{
  std::string msg = "The current server time is " + dtos(mineserver->map.getTime());
  mineserver->chat.sendmsgTo(user.c_str(), msg.c_str());
}

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
      default:                    break;
    }
  return true;
}


bool startedDiggingFunction(const char* userIn, int32_t x,int8_t y,int32_t z,int8_t direction)
{
  //translateDirection(&x,&y,&z,direction);
  std::string user(userIn);
  int map = 0;
  mineserver->user.getPositionW(userIn, NULL, NULL, NULL, &map,NULL, NULL, NULL);

  if(cuboidMap.find(user) != cuboidMap.end())
  {
    if(cuboidMap[user].active)
    {
      if(cuboidMap[user].action == REPLACE)
      {
        if(cuboidMap[user].state == 0)
        {
          cuboidMap[user].state = 1;
          cuboidMap[user].x     = x;
          cuboidMap[user].y     = y;
          cuboidMap[user].z     = z;
          mineserver->chat.sendmsgTo(user.c_str(),"First block done, now second");
        }
        else if(cuboidMap[user].state == 1)
        {
          int xstart,xend;
          int ystart,yend;
          int zstart,zend;
          xstart=(x<cuboidMap[user].x)?x:cuboidMap[user].x;
          xend=(x<cuboidMap[user].x)?cuboidMap[user].x:x;

          ystart=(y<cuboidMap[user].y)?y:cuboidMap[user].y;
          yend=(y<cuboidMap[user].y)?cuboidMap[user].y:y;

          zstart=(z<cuboidMap[user].z)?z:cuboidMap[user].z;
          zend=(z<cuboidMap[user].z)?cuboidMap[user].z:z;

          unsigned char block,meta;

          if((xend-xstart) * (yend-ystart) * (zend-zstart) > 10000)
          {
            mineserver->chat.sendmsgTo(user.c_str(),"Area too large!");
            return true;
          }
          for(int xpos = xstart; xpos <= xend; xpos ++)
          {
            for(int zpos = zstart;  zpos <= zend; zpos ++)
            {     
              for(int ypos = ystart;  ypos <= yend; ypos ++)
              {  
                if(mineserver->map.getBlock(xpos,ypos,zpos,&block,&meta) && block == cuboidMap[user].fromBlock)
                {
                  mineserver->map.setBlock(xpos,ypos,zpos,cuboidMap[user].toBlock,map);
                }
              }
            }
          }
          mineserver->chat.sendmsgTo(user.c_str(),"Replace done");
          cuboidMap.erase(user);
        }        
      }
    }
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
      if(cuboidMap[user].action == PLANE)
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
			  int xstart,xend;
			  int ystart,yend;
			  int zstart,zend;
			  xstart=(x<cuboidMap[user].x)?x:cuboidMap[user].x;
			  xend=(x<cuboidMap[user].x)?cuboidMap[user].x:x;

			  ystart=(y<cuboidMap[user].y)?y:cuboidMap[user].y;
			  yend=(y<cuboidMap[user].y)?cuboidMap[user].y:y;

			  zstart=(z<cuboidMap[user].z)?z:cuboidMap[user].z;
			  zend=(z<cuboidMap[user].z)?cuboidMap[user].z:z;
			  if((xend-xstart) * (yend-ystart) * (zend-zstart) > 10000)
			  {
			    mineserver->chat.sendmsgTo(user.c_str(),"Area too large!");
			    return true;
			  }
			  for(int xpos = xstart; xpos <= xend; xpos ++)
			  {
			    for(int zpos = zstart;  zpos <= zend; zpos ++)
			    {                
				  for(int ypos = ystart;  ypos <= yend; ypos ++)
				  {
				    mineserver->map.setBlock(xpos,ypos,zpos,block,0);
				  }
			    }
			  }
		    mineserver->chat.sendmsgTo(user.c_str(),"Cuboid done");
		    cuboidMap.erase(user);
	  	  }
	    }
	  }
    }
  }
  return true;
}

void doNotDisturb(std::string user, std::string command, std::deque<std::string> args)
{
  mineserver->user.toggleDND(user.c_str());
}

void gps(std::string user, std::string command, std::deque<std::string> args) 
{
  double x,y,z,stance;
  float yaw,pitch;
  mineserver->user.getPosition(user.c_str(), &x, &y, &z, &yaw, &pitch, &stance);
  std::string msg = "X: " + dtos(x) + " Y: " + dtos(y) + " Z: " + dtos(z);
  mineserver->chat.sendmsgTo(user.c_str(), msg.c_str());
}

void banUser(std::string user, std::string command, std::deque<std::string> args)
{
  return;
}

void unbanUser(std::string user, std::string command, std::deque<std::string> args) 
{
  return;
}

void sendRules(std::string user, std::string command, std::deque<std::string> args) 
{
  std::string line;
  std::ifstream rules("rules.txt");
  
  if(rules.is_open()) {
    while(rules.good()) {
      std::getline(rules, line);
      if(line.size() > 0 && line.at(0) != '#')
        mineserver->chat.sendmsgTo(user.c_str(), line.c_str());
    }
    rules.close();
  }
}
void about(std::string user, std::string command, std::deque<std::string> args)
{
  std::ostringstream msg;
  if (mineserver->config.bData("system.show_version"))
  {
    msg  << "§9" << mineserver->config.sData("system.server_name") << " Running Mineserver v." << VERSION_SIMPLE;
    mineserver->chat.sendmsgTo(user.c_str(), msg.str().c_str());
  }
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

void sendMOTD(std::string user, std::string command, std::deque<std::string> args)
{
  std::string line;
  std::ifstream MOTDFile("motd.txt");
  if (MOTDFile.is_open())
    {
    while (MOTDFile.good() )
    {
      std::getline(MOTDFile, line);
      if(line.size() > 0 && line.at(0) != '#')
        mineserver->chat.sendmsgTo(user.c_str(), line.c_str());
    }
    MOTDFile.close();
  }
}

std::string pluginName = "commands";

PLUGIN_API_EXPORT void CALLCONVERSION commands_init(mineserver_pointer_struct* mineserver_temp)
{
  mineserver = mineserver_temp;

  if (mineserver->plugin.getPluginVersion(pluginName.c_str()) > 0)
  {
    std::string msg = "commands is already loaded v."+dtos(mineserver->plugin.getPluginVersion(pluginName.c_str()));
    mineserver->logger.log(LOG_INFO, "plugin.commands", msg.c_str());
    return;
  }
  std::string msg = "Loaded "+pluginName+"!";
  mineserver->logger.log(LOG_INFO, "plugin.commands", msg.c_str());

  mineserver->plugin.setPluginVersion(pluginName.c_str(), PLUGIN_COMMANDS_VERSION);

  mineserver->plugin.addCallback("PlayerChatCommand", reinterpret_cast<voidF>(chatCommandFunction));
  mineserver->plugin.addCallback("BlockPlacePre", reinterpret_cast<voidF>(blockPlacePreFunction));

  mineserver->plugin.addCallback("PlayerDiggingStarted", reinterpret_cast<voidF>(startedDiggingFunction));

  registerCommand(ComPtr(new Command(parseCmd("about"), "", "Displays server name and software version", about)));
  registerCommand(ComPtr(new Command(parseCmd("ctp"), "<x> <y> <z>", "Teleport to coordinates (eg. /ctp 100 100 100)", coordinateTeleport)));
  registerCommand(ComPtr(new Command(parseCmd("cuboid"), "", "Type in the command and place two blocks, it will fill the space between them", cuboid)));
  registerCommand(ComPtr(new Command(parseCmd("dnd"), "", "Toggles Do Not Disturb mode", doNotDisturb)));
  registerCommand(ComPtr(new Command(parseCmd("flattenchunk"), "<id/alias>", "Erases all blocks above you and changes all blocks at your Y-level to your block of choice", flattenchunk)));
  registerCommand(ComPtr(new Command(parseCmd("gettime"), "", "Gets the world time", getTime)));
  registerCommand(ComPtr(new Command(parseCmd("give"), "<player> <id/alias> [count]", "Gives <player> [count] pieces of <id/alias>. By default [count] = 1", giveItems)));
  registerCommand(ComPtr(new Command(parseCmd("gps"), "", "Display current coordinates", gps)));
  registerCommand(ComPtr(new Command(parseCmd("help"), "[<commandName>]", "Display this help message.", sendHelp)));
  registerCommand(ComPtr(new Command(parseCmd("home"), "", "Teleports you to this world's spawn location", home)));
  registerCommand(ComPtr(new Command(parseCmd("igive i item"), "<id/alias> [count]", "Gives self [count] pieces of <id/alias>. By default [count] = 1", giveItemsSelf)));
  registerCommand(ComPtr(new Command(parseCmd("motd"), "", "Displays the server's MOTD", sendMOTD)));
  registerCommand(ComPtr(new Command(parseCmd("players who names list"), "", "Lists online players", playerList)));
  registerCommand(ComPtr(new Command(parseCmd("replace"), "<from-id/alias> <to-id/alias>", "Type in the command and left-click two blocks, it will replace the selected blocks with the new blocks", replace)));
  registerCommand(ComPtr(new Command(parseCmd("replacechunk"), "<from-id/alias> <to-id/alias>", "Replaces the chunk you are at with the block you specify", replacechunk)));
  registerCommand(ComPtr(new Command(parseCmd("rules"), "", "Displays server rules", sendRules)));
  registerCommand(ComPtr(new Command(parseCmd("save"), "", "Manually saves map to disc", saveMap)));
  registerCommand(ComPtr(new Command(parseCmd("setspawn"), "", "Sets home to your current coordinates", setSpawn)));
  registerCommand(ComPtr(new Command(parseCmd("settime"), "<time>", "Sets the world time. (<time> = 0-24000, 0 & 24000 = day, ~15000 = night)", setTime)));
  registerCommand(ComPtr(new Command(parseCmd("tp"), "<player> [<anotherPlayer>]", "Teleport yourself to <player>'s position or <player> to <anotherPlayer>", userTeleport)));
  registerCommand(ComPtr(new Command(parseCmd("world"), "<world-id>", "Moves you between worlds", userWorld)));
}

PLUGIN_API_EXPORT void CALLCONVERSION commands_shutdown(void)
{
  if (mineserver->plugin.getPluginVersion(pluginName.c_str()) <= 0)
  {
    mineserver->logger.log(LOG_INFO, "plugin.commands", "commands is not loaded!");
    return;
  }
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
