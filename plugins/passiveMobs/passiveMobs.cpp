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
#include <vector>
#include <iostream>
#include <ctime>
#include <stdint.h>
#include <cmath>

#include "tr1.h"
#ifdef __APPLE__
#include <tr1/memory>
#else
#include TR1INCLUDE(memory)
#endif


#define MINESERVER_C_API
#include "plugin_api.h"
#include "tools.h"

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

static const int passiveMobs[] = { MOB_SHEEP, MOB_COW, MOB_CHICKEN, MOB_PIG };

// The list of Mobs this plugin has control of
// Note that other plugins may make other mobs, and control them itself
class MyPetMob
{
public:
  int ID;
  int deSpawn;
  double velocity;
  explicit MyPetMob(int ID, double velocity = 0) : ID(ID), deSpawn(0), velocity(velocity) { }
};

typedef std::tr1::shared_ptr<MyPetMob> MyPetMobPtr;

std::vector<MyPetMobPtr> MyMobs;

const unsigned int maxMobs = 15; // Maximum ammount of mobs allowed
time_t lastSpawn = time(NULL);

int topBlockSuitable(int x, int z, int w)
{
  int y;
  unsigned char block, meta;
  for(y = 127; y > 0; y--)
  {
    mineserver->map.getBlockW(x,y,z,w,&block,&meta);
    if(block!=0){break;}
  }
  if(block == 78 || block == 2 || block == 38 || block == 37)
  {
    return y;
  }
  return 0;
}

bool canStepIn(int type)
{
  if(type == 0 || type == 6 || type == 8 || type == 9 || type == 10 ||
    type == 11 || type == 37 || type == 38 || type == 39 || type == 40 ||
    type == 50 || type == 51 || type == 55 || type == 59 ||
    type == 69 || type == 70 || type == 72 || type == 75 || type == 76 ||
    type == 77 || type == 78 || type == 83)
    return true;
  return false;
}

bool moveSuitable(double* x, double* y, double* z, int w)
{
  unsigned char block, meta;
  mineserver->map.getBlockW((int)floor(*x),(int)*y,(int)floor(*z),w,&block,&meta);
  return canStepIn(block);
}

void fallMob(double* x, double* y, double* z, int w)
{
  unsigned char block, meta;
  for(int count = 1; count < 128; count ++)
  {
    mineserver->map.getBlockW((int)floor(*x),(int)(*y)-count,(int)floor(*z),w,&block,&meta);
    if (!canStepIn(block))
    {
      *y=(1+(*y))-count;
      return;
    }
  }
}

int defaultHealth(int mob)
{
  switch (mob)
  {
    case MOB_PIG:
    case MOB_COW:
    case MOB_SHEEP:
      return 10;
    case MOB_CHICKEN:
      return 4;
    default:
      return 10;
  }
}

int defaultDamage(int item)
{
  switch (item)
  {
    case ITEM_WOODEN_SWORD:
    case ITEM_GOLD_SWORD:
      return 5;
    case ITEM_STONE_SWORD:
      return 7;
    case ITEM_IRON_SWORD:
      return 9;
    case ITEM_DIAMOND_SWORD:
      return 11;
    default:
      return 2;
  }
}

void spawn()
{
  if (MyMobs.size() < maxMobs &&
      time(NULL)-lastSpawn > 2 &&
      mineserver->user.getCount() > 0 &&
      mineserver->map.getTime() <= 18000)
  {
    int randomPlayer = mineserver->tools.uniformInt(0, mineserver->user.getCount() - 1);
    double x,y,z;
    int w;
    if (mineserver->user.getPositionW(mineserver->user.getUserNumbered(randomPlayer),&x,&y,&z,&w,NULL,NULL,NULL))
    {
      x += mineserver->tools.uniformInt(-50, 50);
      z += mineserver->tools.uniformInt(-50, 50);
      y = topBlockSuitable(x,z,w);
      if (y > 0)
      {
        y+=1;
        int type = passiveMobs[mineserver->tools.uniformInt(0, sizeof(passiveMobs) / sizeof(passiveMobs[0]) - 1)];
        int newMob = mineserver->mob.createMob(type);
        mineserver->mob.setHealth(newMob, defaultHealth(type));
        MyPetMobPtr newMobData(new MyPetMob(newMob));
        MyMobs.push_back(newMobData);
        mineserver->mob.moveMobW(newMob,x,y,z,w);
        mineserver->mob.spawnMob(newMob);
        if (type == MOB_SHEEP)
        {
          int color = mineserver->tools.uniformInt(0, 15);
          mineserver->mob.setByteMetadata(newMob, 16, (int8_t)color);
          mineserver->mob.updateMetadata(newMob);
        }
        lastSpawn=time(NULL);
      }
    }
  }
}

void timer200Function()
{
  spawn();

  for (int i = MyMobs.size() - 1; i >= 0; i--)
  {
    double x,y,z;
    int w;
    // get position and world
    mineserver->mob.getMobPositionW(MyMobs[i]->ID, &x, &y, &z, &w);
    // kill dead mobs
    if(mineserver->mob.getHealth(MyMobs[i]->ID) == 0)
    {
      if (MyMobs[i]->deSpawn < 12)
      {
        MyMobs[i]->deSpawn++;
      }
      else
      {
        mineserver->mob.despawnMob(MyMobs[i]->ID);
        /* TODO: fix memory leak */
        MyMobs.erase(MyMobs.begin()+i);
      }
      continue;
    }
    else
    {
      MyMobs[i]->deSpawn=0;
    }
    // get the nearest user
    int nearest = 10000;
    double nearest_x = .0, nearest_y = .0, nearest_z = .0;
    for (int j = 0; j < mineserver->user.getCount(); j++)
    {
      const char * const name = mineserver->user.getUserNumbered(j);
      double px,py,pz;
      int pmap;
      mineserver->user.getPositionW(name, &px,&py,&pz,&pmap,NULL,NULL,NULL);

      if (w != pmap) { continue; }
      int distance = abs(int(px-x)) + abs(int(py-y)) + abs(int(pz-z));
      if(distance < nearest) {
        nearest = distance;
        nearest_x = px;
        nearest_y = py;
        nearest_z = pz;
      }
    }
    if(nearest < 0 || nearest > 200)
    {
      // if there is no user here, despawn the mob
      mineserver->mob.despawnMob(MyMobs[i]->ID);
      MyMobs.erase(MyMobs.begin()+i);
      continue;
    }
    // do something, my little mob
    int action = rand() % 150;
    double yaw, pitch, head_yaw;
    float forward = 0;
    mineserver->mob.getLook(MyMobs[i]->ID, &yaw, &pitch, &head_yaw);
    if (action < 5)
    {
      yaw += 30;
    }
    else if (action < 10)
    {
      yaw += 15;
      forward = 0.3;
    }
    else if (action < 15)
    {
      yaw -= 30;
    }
    else if (action < 20)
    {
      yaw -= 15;
      forward = 0.3;
    }
    else if (action < 30)
    {
      forward = 0.6;
    }
    else if (action < 40)
    {
      // for now, just look around stupidly
      head_yaw += rand() % 40;
    }
    else if (action < 50)
    {
      forward = -0.6;
      // turn around!
      yaw -= 180;
    }
    MyMobs[i]->velocity += forward;
    if (MyMobs[i]->velocity > 2.0){ MyMobs[i]->velocity = 2.0; }
    if (MyMobs[i]->velocity < 0.0){ MyMobs[i]->velocity = 0.0; }
    forward = MyMobs[i]->velocity;

    if (yaw <= 0) { yaw += 360; }
    if (yaw >= 360) { yaw -= 360; }

    // TODO: make it look at the player if he's near enough.
    // (nearest_x, nearest_y, nearest_z).
/*    if(nearest_z != z) {
      head_yaw = RADIANS_TO_DEGREES(tan((nearest_x - x) / (nearest_z - z)));
    }*/

    if (forward>0.1 && rand()%6 == 3)
    {
      float incz = cos((yaw*PI)/180)* forward;
      float incx = sin((yaw*PI)/180)* forward;
      x-=incx; z+=incz;

      if(moveSuitable(&x,&y,&z,w))
      {
        fallMob(&x,&y,&z,w);
        mineserver->mob.moveMobW(MyMobs[i]->ID,x,y,z,w);
      }
      fallMob(&x,&y,&z,w); // Even if they dont move, make them fall
    }
    mineserver->mob.setLook(MyMobs[i]->ID, yaw, pitch, head_yaw);

  }
}

void drop(int mobID)
{
  int type = mineserver->mob.getType(mobID);
  double x,y,z;
  int w;
  mineserver->mob.getMobPositionW(mobID,&x,&y,&z,&w);
  int item = 0, count = 1;
  switch (type)
  {
    case MOB_PIG:
      item = ITEM_PORK;
      break;
    case MOB_SHEEP:
      /* sheep drops on first hit */
      break;
    case MOB_COW:
      item = ITEM_LEATHER;
      break;
    case MOB_CHICKEN:
      item = ITEM_FEATHER;
      break;
  }
  if (item)
  {
    mineserver->map.createPickupSpawn((int)floor(x),(int)floor(y),(int)floor(z),
                                      item, count, 0,NULL);
  }
}

void gotAttacked(const char* userIn,int mobID)
{
  std::string user(userIn);
  int atk_item, _meta, _quant;
  mineserver->user.getItemInHand(userIn, &atk_item, &_meta, &_quant);
  int mobHealth = mineserver->mob.getHealth((int)mobID);

  if (mobHealth <= 0) return;

  mobHealth -= defaultDamage(atk_item);

  if (mobHealth <= 0)
  {
    drop(mobID);
  }
  mineserver->mob.setHealth((int)mobID, (int)mobHealth);
}

void interact(const char* userIn,int mobID)
{
  // Certain mobs drop something when the user interacts with them.
  // Sheep drop wool.
  std::string user(userIn);
  int atk_item, _meta, _quant;
  mineserver->user.getItemInHand(userIn, &atk_item, &_meta, &_quant);
  int mobHealth = mineserver->mob.getHealth((int)mobID);

  if (mobHealth <= 0) return;

  int type = mineserver->mob.getType(mobID);
  double x, y, z; int w;
  mineserver->mob.getMobPositionW(mobID, &x, &y, &z, &w);
  if (type == MOB_SHEEP)
  {
    // On unsheared sheeps, use a shear to obtain 1-3 wool blocks.
    int8_t meta = mineserver->mob.getByteMetadata(mobID, 16);
    if (atk_item == ITEM_SHEARS && !(meta & 0x10))
    {
      size_t amount = rand() % 3 + 1;
      mineserver->map.createPickupSpawn((int)floor(x),(int)floor(y),(int)floor(z),
                                        BLOCK_WOOL, amount, meta, NULL);
      meta |= 0x10;
      mineserver->mob.setByteMetadata(mobID, 16, meta);
      mineserver->mob.updateMetadata(mobID);
    }
  } else if(type == MOB_COW)
  {
    if(atk_item == ITEM_BUCKET)
    {
      // give him milk, take the bucket!
      _quant--;
      mineserver->user.setItemInHand(userIn, atk_item, _meta, _quant);
      mineserver->user.addItem(userIn, ITEM_MILK_BUCKET, 1, 0);
    }
  }
}

std::string pluginName = "passiveMobs";

PLUGIN_API_EXPORT void CALLCONVERSION passiveMobs_init(mineserver_pointer_struct* mineserver_temp)
{
  mineserver = mineserver_temp;

  if (mineserver->plugin.getPluginVersion(pluginName.c_str()) > 0)
  {
    std::string msg = atoi(pluginName.c_str())+ " is already loaded v."+dtos(mineserver->plugin.getPluginVersion(pluginName.c_str()));
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
  mineserver->plugin.addCallback("Timer200", reinterpret_cast<voidF>(timer200Function));
  mineserver->plugin.addCallback("gotAttacked", reinterpret_cast<voidF>(gotAttacked));
  mineserver->plugin.addCallback("interact", reinterpret_cast<voidF>(interact));
}

PLUGIN_API_EXPORT void CALLCONVERSION passiveMobs_shutdown(void)
{
  if (mineserver->plugin.getPluginVersion(pluginName.c_str()) <= 0)
  {
    mineserver->logger.log(6, "plugin.passiveMobs", atoi(pluginName.c_str()) + " is not loaded!");
    return;
  }
}
