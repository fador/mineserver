/*
g++ -c nether.cpp
g++ -shared nether.o -o nether.so

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

#include <string>
#include <deque>
#include <ctime>
#include <sstream>
#include <cstdlib>
#include <map>
#include <iostream>
#include <stdint.h>
#include <math.h>

#define MINESERVER_C_API
#include "../../plugin_api.h"

#include "nether.h"

#define PLUGIN_NETHER_VERSION 1.1
const char CHATCMDPREFIX   = '/';
mineserver_pointer_struct* mineserver;

std::string dtos(double n)
{
  std::ostringstream result;
  result << n;
  return result.str();
}

void timer200Function()
{
  for (int i = 0; i < mineserver->user.getCount(); i++){
    char *name;
    name= mineserver->user.getUserNumbered(i);
    double x,y,z;
    int map;
    mineserver->user.getPositionW(name, &x,&y,&z,&map,NULL,NULL,NULL);
    std::cout << x << " " << y << " " << z << " " << map << std::endl;
    if(map<0 || map > 100)
    {
      // Watch out for this, means uninitialized!
      continue;
    }
    unsigned char type, meta;
    mineserver->map.getBlockW((int)floor(x),(int)floor(y),(int)floor(z),map,&type,&meta);
    if((int)type == 90)
    {
      // They're on a portal TP TIEM
      if(map==0){
        // Normal > Nether
        double nx, nz;
        nx = x /16;
        nz = z /16;
        mineserver->user.teleportMap(name, nx,y+1,nz,1);
      }else if(map==1){
        double nx, nz;
        nx = x * 16;
        nz = z * 16;
        mineserver->user.teleportMap(name, nx,y+1,nz,0);
      }
      // You're on another world? FIND YOUR OWN WAY BACK
    }
  }
}

void userWorld(std::string user, std::string command, std::deque<std::string> args)
{
  if(args.size() == 1)
  {
     double x,y,z;
     mineserver->user.getPosition(user.c_str(), &x,&y,&z,NULL,NULL,NULL);
     std::cout << user.c_str() << atoi(args[0].c_str())<< std::endl;
     mineserver->user.teleportMap(user.c_str(), x,y+2,z,atoi(args[0].c_str()));
  }

}

std::string pluginName = "nether";

PLUGIN_API_EXPORT void CALLCONVERSION nether_init(mineserver_pointer_struct* mineserver_temp)
{
  mineserver = mineserver_temp;

  if (mineserver->plugin.getPluginVersion(pluginName.c_str()) > 0)
  {
    std::string msg = "nether is already loaded v."+dtos(mineserver->plugin.getPluginVersion(pluginName.c_str()));
    mineserver->logger.log(6, "plugin.nether", msg.c_str());
    return;
  }
  std::string msg = "Loaded "+pluginName+"!";
  mineserver->logger.log(6, "plugin.nether", msg.c_str());

  mineserver->plugin.setPluginVersion(pluginName.c_str(), PLUGIN_NETHER_VERSION);

  mineserver->plugin.addCallback("Timer200", (void *)timer200Function);
}

PLUGIN_API_EXPORT void CALLCONVERSION command_shutdown(void)
{
  if (mineserver->plugin.getPluginVersion(pluginName.c_str()) <= 0)
  {
    mineserver->logger.log(6, "plugin.nether", "nether is not loaded!");
    return;
  }
}
