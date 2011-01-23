/*
g++ -c crapMobs.cpp
g++ -shared crapMobs.o -o crapMobs.so

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

#include "crapMobs.h"

#define PLUGIN_CRAPMOBS_VERSION 1.1
mineserver_pointer_struct* mineserver;

int myPetGiant;
int myPetGiantz;
bool blah=false;

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

void timer200Function()
{
  mineserver->mob.moveMob(myPetGiant, 0,85,0);
  blah = !blah;
  if(blah){
//    mineserver->mob.despawnMob(myPetGiant);
  }else{
    mineserver->mob.spawnMob(myPetGiant);
  }
  myPetGiantz+=1;
  if(myPetGiantz > 40)
    myPetGiantz = 0;
  std::cout << myPetGiantz << std::endl;
  mineserver->mob.moveMob(myPetGiant, 0,85,myPetGiantz);
}

std::string pluginName = "crapMobs";

PLUGIN_API_EXPORT void CALLCONVERSION crapMobs_init(mineserver_pointer_struct* mineserver_temp)
{
  mineserver = mineserver_temp;

  if (mineserver->plugin.getPluginVersion(pluginName.c_str()) > 0)
  {
    std::string msg = "crapMobs is already loaded";
    mineserver->logger.log(6, "plugin.crapMobs", msg.c_str());
    return;
  }
  std::string msg = "Loaded "+pluginName+"!";
  mineserver->logger.log(6, "plugin.crapMobs", msg.c_str());

  mineserver->plugin.setPluginVersion(pluginName.c_str(), PLUGIN_CRAPMOBS_VERSION);

  mineserver->plugin.addCallback("Timer200", (void *)timer200Function);
  myPetGiant = mineserver->mob.spawnMobN("Giant");
  mineserver->mob.spawnMob(myPetGiant);
  mineserver->mob.moveMob(myPetGiant,0,0,0);
}

PLUGIN_API_EXPORT void CALLCONVERSION command_shutdown(void)
{
  if (mineserver->plugin.getPluginVersion(pluginName.c_str()) <= 0)
  {
    mineserver->logger.log(6, "plugin.crapMobs", "crapMobs is not loaded!");
    return;
  }
}
