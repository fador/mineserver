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
#include <ctime>
#include <stdint.h>
#include <math.h>

#define MINESERVER_C_API
#include "../../src/plugin_api.h"

#include "passiveMobs.h"

#define PLUGIN_PASSIVEMOBS_VERSION 1.1

#define PI 3.14159265

double sin_lt[3600];
double cos_lt[3600];

const char CHATCMDPREFIX   = '/';
mineserver_pointer_struct* mineserver;

std::string dtos(double n)
{
  std::ostringstream result;
  result << n;
  return result.str();
}

std::string Mobnames[] = {"Chicken","Pig","Sheep","Cow"};
 
// The list of Mobs this plugin has control of
// Note that other plugins may make other mobs, and control them itself
class MyPetMob{
  public:
  int ID;
  double velocity;
  MyPetMob(int ID){ this->ID = ID; }
};
std::vector<MyPetMob*> MyMobs;

int maxMobs = 20; // Maximum ammount of mobs allowed
time_t lastSpawn = time(NULL);

int topBlockSuitable(int x, int z, int w){
  int y;
  unsigned char block, meta;
  for(y=127; y>0; y--){
    mineserver->map.getBlockW(x,y,z,w,&block,&meta);
    if(block!=0){break;}
  }
  if(block == 78 || block == 2 || block == 38 || block == 37){
    return y;
  }
  return 0;
}

bool canStepIn(int type){
  if(type == 0 || type == 6 || type == 8 || type == 9 || type == 10 ||
     type == 11 || type == 37 || type == 38 || type == 39 || type == 40 ||
     type == 50 || type == 50 || type == 51 || type == 55 || type == 59 ||
     type == 69 || type == 70 || type == 72 || type == 75 || type == 76 ||
     type == 77 || type == 78 || type == 83)
    return true;
  return false;
}

bool moveSuitable(double* x, double* y, double* z, int w){
  unsigned char block, meta;
  mineserver->map.getBlockW((int)floor(*x),(int)*y,(int)floor(*z),w,&block,&meta);
  if(canStepIn(block))
    return true;
  mineserver->map.getBlockW((int)floor(*x),(int)(*y)+1,(int)floor(*z),w,&block,&meta);
  if(canStepIn(block)){
    (*y)++;
    return true;
  }
  return false;
}

void fallMob(double* x, double* y, double* z, int w){
  unsigned char block, meta;
  for(int count = 1; count < 128; count ++){
    mineserver->map.getBlockW((int)floor(*x),(int)(*y)-count,(int)floor(*z),w,&block,&meta);
    if(!canStepIn(block)){
      *y=(1+(*y))-count;
      return;
    }
  }
}

void timer200Function()
{
  if(MyMobs.size() < maxMobs && time(NULL)-lastSpawn>2){
    if(mineserver->user.getCount()>0){
      int randomPlayer = rand()%mineserver->user.getCount();
      double x,y,z;
      int w;
      mineserver->user.getPositionW(mineserver->user.getUserNumbered(randomPlayer),&x,&y,&z,&w,NULL,NULL,NULL);
      x += ((rand()%100)-50);
      z += ((rand()%100)-50);
      y = topBlockSuitable(x,z,w);
      if(y>0){
        y+=1;
        int randomMob = rand()%4;
        int newMob = mineserver->mob.createMob(Mobnames[randomMob].c_str());
        MyPetMob* newMobData = new MyPetMob(newMob);
        MyMobs.push_back(newMobData);
        mineserver->mob.moveMobW(newMob,x,y,z,w);
        mineserver->mob.spawnMob(newMob);
        lastSpawn=time(NULL);
      }
    }
  }
  for(int i = MyMobs.size()-1; i>=0; i--){
    double x,y,z;
    int w;
    mineserver->mob.getMobPositionW(MyMobs[i]->ID,&x,&y,&z,&w);


    if(mineserver->mob.getHealth(MyMobs[i]->ID)==0){
      int type = mineserver->mob.getType(MyMobs[i]->ID);
      int item = 0,count = 0;
      if(type == 90){ item = 319; count = (rand()%4); }
      if(type == 91){ item = 35; count = (rand()%5);  }
      if(type == 92){ item = 334; count = (rand()%6); }
      if(type == 93){ item = 288; count = (rand()%8); }
      if(item != 0){
        mineserver->map.createPickupSpawn((int)floor(x),(int)floor(y),(int)floor(z),
                                         item, count, 0,NULL);
      }
      // TODO : Fix obvious gaping memory leak
      MyMobs.erase(MyMobs.begin()+i);
      continue;
    }
    int nearest = 10000;
    for (int j = 0; j < mineserver->user.getCount(); j++){
      char *name;
      name= mineserver->user.getUserNumbered(j);
      double px,py,pz;
      int pmap;
      mineserver->user.getPositionW(name, &px,&py,&pz,&pmap,NULL,NULL,NULL);

      if(w!=pmap){ continue; }
      int distance = abs(int(px-x)) + abs(int(py-y)) + abs(int(pz-z));
      if(distance < nearest){ nearest=distance; }
    }
    if(nearest < 0 || nearest > 200){
      MyMobs.erase(MyMobs.begin()+i);
      continue;
    }
    int action = rand()%100;
    double yaw, pitch;
    double forward=0;
    mineserver->mob.getLook(MyMobs[i]->ID, &yaw, &pitch);
    if(action < 5){
      yaw += 30;
    }else if(action <10){
      yaw += 15;
      while(yaw>=360){ yaw-=360;}
      forward=0.3;
    }else if(action <15){
      yaw -= 30;
    }else if(action <20){
      yaw -= 15;
      while(yaw <= 0){ yaw += 360; }
      forward=0.3;
    }else if(action < 30){
      forward=+0.6;
    }else if(action < 50){
      forward=-0.6;;
    }
    MyMobs[i]->velocity+=forward;
    if(MyMobs[i]->velocity>2.0){ MyMobs[i]->velocity = 2.0; }
    if(MyMobs[i]->velocity<0.0){ MyMobs[i]->velocity = 0.0; }
    forward = MyMobs[i]->velocity;
    
    if(yaw<=0){yaw+=360;}
    if( yaw>=360 ) { yaw-=360; }

    if(forward>0.1){
      double incz = cos_lt[(int)(yaw*10)] * forward;
      double incx = sin_lt[(int)(yaw*10)] * forward;
      x-=incx; z+=incz;
      if(moveSuitable(&x,&y,&z,w)){
        fallMob(&x,&y,&z,w);
        mineserver->mob.moveMobW(MyMobs[i]->ID,x,y,z,w);
      }
      fallMob(&x,&y,&z,w); // Even if they dont move, make them fall
    }
    mineserver->mob.setLook(MyMobs[i]->ID, yaw, pitch);
      
  }
        
    

}

std::string pluginName = "passiveMobs";

PLUGIN_API_EXPORT void CALLCONVERSION passiveMobs_init(mineserver_pointer_struct* mineserver_temp)
{
  mineserver = mineserver_temp;

  if (mineserver->plugin.getPluginVersion(pluginName.c_str()) > 0)
  {
    std::string msg = "passiveMobs is already loaded v."+dtos(mineserver->plugin.getPluginVersion(pluginName.c_str()));
    mineserver->logger.log(6, "plugin.passiveMobs", msg.c_str());
    return;
  }
  std::string msg = "Loaded "+pluginName+"!";
  mineserver->logger.log(6, "plugin.passiveMobs", msg.c_str());

  mineserver->plugin.setPluginVersion(pluginName.c_str(), PLUGIN_PASSIVEMOBS_VERSION);

  for(int i=0; i<3600; i ++){
    sin_lt[i] = sin(((double)(i/10)*PI/180));
    cos_lt[i] = cos(((double)(i/10)*PI/180));
  }
  mineserver->plugin.addCallback("Timer200", (void *)timer200Function);
}

PLUGIN_API_EXPORT void CALLCONVERSION passiveMobs_shutdown(void)
{
  if (mineserver->plugin.getPluginVersion(pluginName.c_str()) <= 0)
  {
    mineserver->logger.log(6, "plugin.passiveMobs", "pluginMobs is not loaded!");
    return;
  }
}
