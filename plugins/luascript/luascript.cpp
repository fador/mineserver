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

#include <iostream>
#include <sstream>
#include <string>
#include <deque>
#include <stdint.h>
#include <cstdlib>
#include <ctime>
#include <map>
#include <vector>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

struct LuaScript
{
  std::string script;
  lua_State* pL;
};

std::vector<LuaScript> luaScripts;


#define MINESERVER_C_API
#include "plugin_api.h"

std::deque<std::string> parseCmd(std::string cmd);
const char CHATCMDPREFIX   = '/';
#define PLUGIN_LUASCRIPT_VERSION 0.1
#define MSG_PREFIX std::string("[LuaScript] ")

mineserver_pointer_struct* mineserver;

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

typedef Command* ComPtr;
typedef std::map<std::string, ComPtr> CommandList;
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

  // User commands
  CommandList::iterator iter;
  if((iter = m_Commands.find(command)) != m_Commands.end())
  {
    iter->second->callback(user, command, cmd);
    return true;
  }
  return false;
}


bool startedDiggingFunction(const char* userIn, int32_t x,int8_t y,int32_t z,int8_t direction)
{
  //translateDirection(&x,&y,&z,direction);
  std::string user(userIn);
  int map = 0;
  mineserver->user.getPositionW(userIn, NULL, NULL, NULL, &map,NULL, NULL, NULL);
  

  return true;
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

bool blockPlacePreFunction(const char* userIn, int32_t x,int8_t y,int32_t z,int16_t block,int8_t direction)
{  
  translateDirection(&x,&y,&z,direction);
  std::string user(userIn);
  
  return true;
}

void sendHelp(std::string user, std::string command, std::deque<std::string> args)
{
  CommandList* commandList = &m_Commands; // defaults
  //std::string commandColor = MC_COLOR_BLUE;

  if (args.size() == 0)
  {
    for(CommandList::iterator it = commandList->begin();it != commandList->end();++it)
    {
      if(it->first != "help")
      {
        std::string args = it->second->arguments;
        std::string description = it->second->description;
        std::string msg = CHATCMDPREFIX + it->first + " " + args + " : " + description;
        mineserver->chat.sendmsgTo(user.c_str(), msg.c_str());
      }
    }
  }
  else
  {
    CommandList::iterator iter;
    if ((iter = commandList->find(args.front())) != commandList->end())
    {
      if(iter->first != "help")
      {
        std::string args = iter->second->arguments;
        std::string description = iter->second->description;
        std::string msg = CHATCMDPREFIX + iter->first + " " + args;
        mineserver->chat.sendmsgTo(user.c_str(), msg.c_str());
        msg = CHATCMDPREFIX + description;
        mineserver->chat.sendmsgTo(user.c_str(), msg.c_str());
      }
    }
  }
}

//Calltest, string a, string b
int lua_calltest(lua_State *pL)
{
  int n = lua_gettop(pL);
  if(n != 2)
  {
    return 0;
  }
  if(!lua_isstring(pL, 1) || !lua_isstring(pL, 2))
  {
    return 0;
  }

  std::string msg=lua_tostring(pL,1);
  std::string msg2=lua_tostring(pL,2);

  mineserver->logger.log(LOG_INFO, "plugin.luascript", msg.c_str());
  return 0;
}

void luaLoad(std::string user, std::string command, std::deque<std::string> args)
{

  if (args.size() == 0)
  {
    std::string msg = MSG_PREFIX+"Usage: /luaload <scriptname>";
    mineserver->chat.sendmsgTo(user.c_str(), msg.c_str());
  }
  else
  {
    std::string msg = "Trying to load " + args[0];
    mineserver->logger.log(LOG_INFO, "plugin.luascript", msg.c_str());
    luaScripts.push_back(LuaScript());
    unsigned int last = luaScripts.size()-1;

    luaScripts[last].pL = lua_open();
    luaScripts[last].script = args[0];
    
    luaL_openlibs(luaScripts[last].pL);

    if(luaL_dofile(luaScripts[last].pL,  luaScripts[last].script.c_str()) != 0)
    {
      lua_close(luaScripts[last].pL);
      luaScripts.pop_back();
      std::string msg = MSG_PREFIX+"failed to load script "+args[0];
      mineserver->chat.sendmsgTo(user.c_str(), msg.c_str());
    }
    else
    {
      //Init stuff
      lua_register(luaScripts[last].pL, "calltest", lua_calltest);
      std::string msg = MSG_PREFIX+"script "+args[0]+" loaded and running!";
      mineserver->chat.sendmsgTo(user.c_str(), msg.c_str());
    }
  }
}

void luaUnload(std::string user, std::string command, std::deque<std::string> args)
{
  if (args.size() == 0)
  {
    std::string msg = MSG_PREFIX+"Usage: /luaunload <scriptname>";
    mineserver->chat.sendmsgTo(user.c_str(), msg.c_str());
  }
  else
  {
    bool found = false;
    for(unsigned int i = 0;  i < luaScripts.size(); i++)
    {
      if(luaScripts[i].script == args[0])
      {
        lua_close(luaScripts[i].pL);
        luaScripts.erase(luaScripts.begin()+i);
        std::string msg = MSG_PREFIX+"unloaded script "+args[0];
        mineserver->chat.sendmsgTo(user.c_str(), msg.c_str());
        found = true;
        break;
      }
    }
    if(!found)
    {
      std::string msg = MSG_PREFIX+"unable to unload script "+args[0];
      mineserver->chat.sendmsgTo(user.c_str(), msg.c_str());
    }
  }
}


void luaStatus(std::string user, std::string command, std::deque<std::string> args)
{

}

void timer200Function()
{
  for(unsigned int i = 0;  i < luaScripts.size(); i++)
  {
    lua_getglobal(luaScripts[i].pL, "timer200");
    if(lua_isfunction(luaScripts[i].pL, -1))
    {
      lua_call(luaScripts[i].pL, 0, 0);
    }
  }
}

std::string pluginName = "luascript";

PLUGIN_API_EXPORT void CALLCONVERSION luascript_init(mineserver_pointer_struct* mineserver_temp)
{
  mineserver = mineserver_temp;

  if (mineserver->plugin.getPluginVersion(pluginName.c_str()) > 0)
  {
    std::string msg = "luascript is already loaded v."+dtos(mineserver->plugin.getPluginVersion(pluginName.c_str()));
    mineserver->logger.log(LOG_INFO, "plugin.luascript", msg.c_str());
    return;
  }
  std::string msg = "Loaded "+pluginName+"!";
  mineserver->logger.log(LOG_INFO, "plugin.luascript", msg.c_str());

  mineserver->plugin.setPluginVersion(pluginName.c_str(), PLUGIN_LUASCRIPT_VERSION);

  mineserver->plugin.addCallback("PlayerChatCommand", reinterpret_cast<voidF>(chatCommandFunction));
  mineserver->plugin.addCallback("BlockPlacePre", reinterpret_cast<voidF>(blockPlacePreFunction));
  mineserver->plugin.addCallback("PlayerDiggingStarted", reinterpret_cast<voidF>(startedDiggingFunction));
  mineserver->plugin.addCallback("Timer200", reinterpret_cast<voidF>(timer200Function));

  registerCommand(ComPtr(new Command(parseCmd("help"), "[<commandName>]", "Display this help message.", sendHelp)));
  registerCommand(ComPtr(new Command(parseCmd("luaload"), "<scriptname>", "Load Lua script", luaLoad)));
  registerCommand(ComPtr(new Command(parseCmd("luaunload"), "<scriptname>", "Unload Lua script", luaUnload)));
  registerCommand(ComPtr(new Command(parseCmd("luastatus"), "", "Give status of luascript", luaStatus)));
}

PLUGIN_API_EXPORT void CALLCONVERSION commands_shutdown(void)
{
  if (mineserver->plugin.getPluginVersion(pluginName.c_str()) <= 0)
  {
    mineserver->logger.log(LOG_INFO, "plugin.luascript", "luascript is not loaded!");
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
