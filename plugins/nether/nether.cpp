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

#include "nether.h"

#define PLUGIN_NETHER_VERSION 1.1


class block
{
public:
  int x,y,z;
  const char* name;
  block(const char* n, int xx, int yy, int zz){
    x=xx;y=yy;z=zz;name=n;
  }
};

std::vector<block*> blockToDo;
std::vector<block*> blockDone;

const char CHATCMDPREFIX   = '/';
mineserver_pointer_struct* mineserver;

std::string dtos(double n)
{
  std::ostringstream result;
  result << n;
  return result.str();
}

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

void addToDo(const char* user, int x, int y, int z){
  for(int i = 0;i<blockToDo.size();i++){
    block* c=blockToDo[i];
    if(x==c->x && y==c->y && z==c->z)
    {
      return;
    }
  }
  for(int i = 0;i<blockDone.size();i++){
    block* c=blockDone[i];
    if(x==c->x && y==c->y && z==c->z)
    {
      return;
    }
  }
  block* b = new block(user,x,y,z);
  blockToDo.push_back(b);
}

void doPortalFromFrame(const char* user, int x,int y, int z)
{
  bool up=false,down=false,left=false,right=false,forward=false,backward=false;
  for(int c=1; c<10; c++) // Yes, we start at 1 so we DONT see the start again
  {
    if(getBlock(user,x,y+c,z)==49){ up=true; addToDo(user,x,y+c,z);}
    if(getBlock(user,x,y-c,z)==49){ down=true; addToDo(user,x,y-c,z);}
    if(getBlock(user,x+c,y,z)==49){ left=true; addToDo(user,x+c,y,z);}
    if(getBlock(user,x-c,y,z)==49){ right=true; addToDo(user,x-c,y,z);}
    if(getBlock(user,x,y,z+c)==49){ forward=true; addToDo(user,x,y,z+c);}
    if(getBlock(user,x,y,z-c)==49){ backward=true; addToDo(user,x,y,z-c);}
  }
  for(int c=1; c<10; c++)
  {
    if(up)
    {
      if(getBlock(user,x,y+c,z)!=49)
      {
        setBlock(user,x,y+c,z,90);
      }else{
        up=false;
      }
    }
    if(down)
    {
      if(getBlock(user,x,y-c,z)!=49)
      {
        setBlock(user,x,y-c,z,90);
      }else{
        down=false;
      }
    }
    if(left)
    {
      if(getBlock(user,x+c,y,z)!=49)
      {
        setBlock(user,x+c,y,z,90);
      }else{
        left=false;
      }
    }
    if(right)
    {
      if(getBlock(user,x-c,y,z)!=49)
      {
        setBlock(user,x-c,y,z,90);
      }else{
        right=false;
      }
    }
    if(forward)
    {
      if(getBlock(user,x,y,z+c)!=49)
      {
        setBlock(user,x,y,z+c,90);
      }else{
        forward=false;
      }
    }
    if(backward)
    {
      if(getBlock(user,x,y,z-c)!=49)
      {
        setBlock(user,x,y,z-c,90);
      }else{
        backward=false;
      }
    } 
  }
}

void doPortal(const char* user,int x,int y, int z)
{
  int cx=x,cy=y,cz=z;
  addToDo(user,x,y,z);
  while(blockToDo.size()>0)
  {
    doPortalFromFrame(user, blockToDo[0]->x, blockToDo[0]->y, blockToDo[0]->z);
    blockDone.push_back(blockToDo[0]);
    blockToDo.erase(blockToDo.begin());
    cz ++;
  }
  for(int i = 0; i < blockDone.size(); i++)
  {
    delete blockDone[i];
  }
  blockDone.clear();
}

bool blockPlacePreFunction(const char* userIn, int32_t x, int32_t y, int32_t z,int16_t block, int8_t direction)
{
  if(block == 259){
    // Should we trigger Portal creation?
    if(getBlock(userIn,x,y,z)==49){
      std::cout << "Portal triggered" << std::endl;
      doPortal(userIn,x,y,z);
    }
  }
  return true;
}


void timer200Function()
{
  for (int i = 0; i < mineserver->user.getCount(); i++){
    char *name;
    name= mineserver->user.getUserNumbered(i);
    double x,y,z;
    int map;
    mineserver->user.getPositionW(name, &x,&y,&z,&map,NULL,NULL,NULL);
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
  mineserver->plugin.addCallback("BlockPlacePre", (void *)blockPlacePreFunction);
}

PLUGIN_API_EXPORT void CALLCONVERSION command_shutdown(void)
{
  if (mineserver->plugin.getPluginVersion(pluginName.c_str()) <= 0)
  {
    mineserver->logger.log(6, "plugin.nether", "nether is not loaded!");
    return;
  }
}
