/*
g++ -c nBreak.cpp
g++ -shared nBreak.o -o nBreak.so

copy nether.so to Mineserver bin directory.
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

#include <stdlib.h>
#include <string>
#include <deque>
#include <ctime>
#include <sstream>
#include <cstdlib>
#include <map>
#include <vector>
#include <iostream>
#include <stdint.h>
#include <math.h>

#define MINESERVER_C_API
#include "../../src/plugin_api.h"

#include "nBreak.h"

#define PLUGIN_NBREAK_VERSION 1.1
mineserver_pointer_struct* mineserver;

void setBlock(const char* name,int x,int y,int z,unsigned char id)
{
  int map;
  mineserver->user.getPositionW(name,NULL,NULL,NULL,&map,NULL,NULL,NULL);
  mineserver->map.setBlockW(x,y,z,map,(unsigned char)id,0);
}

int getBlock(const char* name,int x,int y,int z)
{
  int map;
  mineserver->user.getPositionW(name,NULL,NULL,NULL,&map,NULL,NULL,NULL);
  unsigned char type, meta;
  if(mineserver->map.getBlockW(x,y,z,map,&type,&meta))
  {
    return type;
  }
  return 0;
}

bool blockBreakPreFunction(const char* userIn, int32_t x, int32_t y, int32_t z,int16_t block, int8_t direction)
{
  int id = getBlock(userIn,x,y,z);
  int item = mineserver->user.getItemInHand(userIn);
  if(id == 4 || id == 1 || id == 16 || id == 48 || id == 67)
  {
    if(item == 270 || item == 278 || item == 257 || item == 285 || item == 274)
    {
      return true;
    }
    else
    {
      setBlock(userIn,x,y,z,0);
      return false;
    }
  }
  if(id == 15)
  {
    if(item == 278 || item == 257 || item == 285 || item == 274)
    {
      return true;
    }
    else
    {
      setBlock(userIn,x,y,z,0);
      return false;
    }
  }
  if(id == 14)
  {
    if(item == 278 || item == 257 || item == 285)
    {
      return true;
    }
    else
    {
      setBlock(userIn,x,y,z,0);
      return false;
    }
  }
  if(id == 49 || id == 21 || id == 56 || id == 73 || id ==74 || id == 52)
  {
    if(item == 278 || item == 257)
    {
      return true;
    }
    else
    {
      setBlock(userIn,x,y,z,0);
      return false;
    }
  }

  
  return true;
}

std::string pluginName = "nBreak";

PLUGIN_API_EXPORT void CALLCONVERSION nBreak_init(mineserver_pointer_struct* mineserver_temp)
{
  mineserver = mineserver_temp;

  if (mineserver->plugin.getPluginVersion(pluginName.c_str()) > 0)
  {
    std::string msg = "nBreak is already loaded";
    mineserver->logger.log(6, "plugin.nether", msg.c_str());
    return;
  }
  std::string msg = "Loaded "+pluginName+"!";
  mineserver->logger.log(6, "plugin.nBreak", msg.c_str());

  mineserver->plugin.setPluginVersion(pluginName.c_str(), PLUGIN_NBREAK_VERSION);

  mineserver->plugin.addCallback("BlockBreakPre", (void *)blockBreakPreFunction);
}

PLUGIN_API_EXPORT void CALLCONVERSION command_shutdown(void)
{
  if (mineserver->plugin.getPluginVersion(pluginName.c_str()) <= 0)
  {
    mineserver->logger.log(6, "plugin.nBreak", "nBreak is not loaded!");
    return;
  }
}
