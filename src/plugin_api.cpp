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

#include "sys/stat.h"

#include "mineserver.h"
#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif


#include "logger.h"

#include "plugin.h"
#include "blocks/default.h"
#include "blocks/falling.h"
#include "blocks/torch.h"
#include "blocks/plant.h"
#include "blocks/snow.h"
#include "blocks/liquid.h"
#include "blocks/fire.h"
#include "blocks/stair.h"
#include "blocks/door.h"
#include "blocks/sign.h"
#include "blocks/tracks.h"
#include "blocks/chest.h"

#define MINESERVER
#include "plugin_api.h"

mineserver_pointer_struct plugin_api_pointers;
std::vector<bool (*)(const std::string& user, std::string msg)> chatPreHook;



User* userFromName(std::string user)
{
  for(unsigned int i = 0; i < Mineserver::get()->users().size(); i++)
  {
    if(Mineserver::get()->users()[i]->fd && Mineserver::get()->users()[i]->logged)
    {
      // Don't send to his user if he is DND and the message is a chat message
      if(user == Mineserver::get()->users()[i]->nick)
      {
        return Mineserver::get()->users()[i];
      }
    }
  }
  return NULL;
}


bool plugin_api_add_hook(std::string name, void *function)
{
  if(name == "ChatPre")
  {
    chatPreHook.push_back((bool (*)(const std::string& user, std::string msg))function);
    return true;
  }
  return false;
}

bool plugin_api_chatpre_callback(std::string user, std::string msg)
{
  for(uint32 i = 0; i < chatPreHook.size(); i++)
  {
    if(chatPreHook[i](user,msg))
    {
      return true;
    }
  }
  return false;
}


float plugin_getPluginVersion(const std::string name)
{
 return Mineserver::get()->plugin()->getPluginVersion(name);
}


void plugin_setPluginVersion(const std::string name, float version)
{
  Mineserver::get()->plugin()->setPluginVersion(name,version);
}

void screen_log(std::string message)
{
  Mineserver::get()->screen()->log(message);
}

bool chat_sendmsgTo(std::string user,std::string msg)
{
  for(unsigned int i = 0; i < Mineserver::get()->users().size(); i++)
  {
    if(Mineserver::get()->users()[i]->fd && Mineserver::get()->users()[i]->logged)
    {
      // Don't send to his user if he is DND and the message is a chat message
      if(user == Mineserver::get()->users()[i]->nick)
      {
        Mineserver::get()->users()[i]->buffer << (sint8)PACKET_CHAT_MESSAGE << (std::string)msg;
        return true;
      }
    }
  }
  return false;
}


bool chat_sendmsg(std::string msg)
{
  for(unsigned int i = 0; i < Mineserver::get()->users().size(); i++)
  {
    if(Mineserver::get()->users()[i]->fd && Mineserver::get()->users()[i]->logged)
    {
      // Don't send to his user if he is DND and the message is a chat message
      if(!(Mineserver::get()->users()[i]->dnd))
      {
        Mineserver::get()->users()[i]->buffer << (sint8)PACKET_CHAT_MESSAGE << (std::string)msg;
      }
    }
  }
  return true;
}

bool map_setTime(std::string timeValue)
{
  Mineserver::get()->map()->mapTime = (sint64)atoi(timeValue.c_str());
  Packet pkt;
  pkt << (sint8)PACKET_TIME_UPDATE << (sint64)Mineserver::get()->map()->mapTime;

  if(User::all().size())
  {
    User::all()[0]->sendAll((uint8*)pkt.getWrite(), pkt.getWriteLen());
  }
  return true;
}


position_struct* user_getPosition(std::string user)
{
  for(unsigned int i = 0; i < Mineserver::get()->users().size(); i++)
  {
    if(Mineserver::get()->users()[i]->fd && Mineserver::get()->users()[i]->logged)
    {
      // Don't send to his user if he is DND and the message is a chat message
      if(user == Mineserver::get()->users()[i]->nick)
      {
        return reinterpret_cast<position_struct*>(&Mineserver::get()->users()[i]->pos);
      }
    }
  }
  return NULL;
}

bool user_teleport(std::string user,double x, double y, double z)
{
  User* tempUser = userFromName(user);
  if(tempUser != NULL)
  {
    tempUser->teleport(x, y, z);
    return true;
  }
  return false;
}

void map_createPickupSpawn(int x, int y, int z, int type, int count, int health, std::string user)
{
  User* tempUser = userFromName(user);
  Mineserver::get()->map()->createPickupSpawn(x,y,z,type,count,health,tempUser);
}

void map_getSpawn(int* x, int* y, int* z)
{  
  *x=Mineserver::get()->map()->spawnPos.x();
  *y=Mineserver::get()->map()->spawnPos.y();
  *z=Mineserver::get()->map()->spawnPos.z();
}

void init_plugin_api(void)
{
  plugin_api_pointers.screen.log              = &screen_log;

  plugin_api_pointers.chat.sendmsg            = &chat_sendmsg;
  plugin_api_pointers.chat.sendmsgTo          = &chat_sendmsgTo;

  plugin_api_pointers.plugin.getPluginVersion = &plugin_getPluginVersion;
  plugin_api_pointers.plugin.setPluginVersion = &plugin_setPluginVersion;

  plugin_api_pointers.map.setTime             = &map_setTime;
  plugin_api_pointers.map.createPickupSpawn   = &map_createPickupSpawn;
  plugin_api_pointers.map.getSpawn            = &map_getSpawn;

  plugin_api_pointers.user.getPosition        = &user_getPosition;
  plugin_api_pointers.user.teleport           = &user_teleport;

  plugin_api_pointers.callback.add_hook       = &plugin_api_add_hook;
}

mineserver_pointer_struct getMineServer(){
    return plugin_api_pointers;
}
