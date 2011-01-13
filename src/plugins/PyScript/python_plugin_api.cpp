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

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string>
#include <errno.h>
#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include "PyScript.h"
#include "python_plugin_api.h"

PyMineserver* pymines= new PyMineserver();

PyMineserver* getMS(){
    return pymines;
}

float PyPlugin::getPluginVersion(const std::string name)
{
  return getMineServer()->plugin.getPluginVersion(name.c_str());
}
void PyPlugin::setPluginVersion(const std::string name, float version)
{
  getMineServer()->plugin.setPluginVersion(name.c_str(),version);
}

void PymyMap::createPickupSpawn(int x, int y, int z, int type, int count, int health, std::string user)
{
  getMineServer()->map.createPickupSpawn(x,y,z,type,count,health,user.c_str());
}
mineserver_pointer_struct* magical(){
  return getMineServer();
}
bool set_time(mineserver_pointer_struct* m, int timeValue)
{
 return m->map.setTime(timeValue);
}

bool PymyMap::setTime(int timeValue)
{
 cout << (int)getMineServer();
 cout << (int)::ms << endl;
 return ::ms->map.setTime(timeValue);
}
PyLoc* PymyMap::getSpawn()
{
  PyLoc* L= new PyLoc();
  int *x,*y,*z;
  double dx,dy,dz;
  getMineServer()->map.getSpawn(x,y,z);
  dx = (double)*x; dy = (double)*y; dz = (double)*z;
  L->x=dx; L->y=dy; L->z=dz;
  return L;
}
bool PymyMap::getBlock(int x, int y, int z, unsigned char* type,unsigned char* meta)
{
  return getMineServer()->map.getBlock(x,y,z,type,meta);
}
bool PymyMap::setBlock(int x, int y, int z, unsigned char type,unsigned char meta)
{
  return getMineServer()->map.setBlock(x,y,z,type,meta);
}

void PyScreen::log(const char* message)
{
  std::string m2(message);
  getMineServer()->logger.log(6,"Plugin",m2.c_str());
}

//mineserver_pointer_struct* PyMineserver::getMineServer(){
//  return pyms;
//}

//void PyMineserver::setMineServer(mineserver_pointer_struct* MS){
//  pyms = MS;
//}
