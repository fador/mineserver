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

#include "default.h"
#include "mineserver.h"
#include "map.h"
#include "constants.h"
#include "logger.h"

#include "tnt.h"
#include <protocol.h>
#include <vehicle.h>
#include <explosion.h>

bool BlockTNT::onPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  uint8_t oldblock;
  uint8_t oldmeta;
  
  if (!Mineserver::get()->map(map)->getBlock(x, y, z, &oldblock, &oldmeta))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }
  /* Check block below allows blocks placed on top */
  if (!this->isBlockStackable(oldblock))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }
  
  /* move the x,y,z coords dependent upon placement direction */
  if (!this->translateDirection(&x, &y, &z, map, direction))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }
  
  if (this->isUserOnBlock(x, y, z, map))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }
  
  if (!this->isBlockEmpty(x, y, z, map))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }
  
  return false;
}

void BlockTNT::onStartedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  uint8_t block, metadata;

  //Remove tnt block so it doesn't mess up the explosion
  Mineserver::get()->map(map)->setBlock(x,y,z,0,0);
  Mineserver::get()->map(map)->sendBlockChange(x,y,z,0,0);

  if (rand() % 9 == 5)
  {
    // There is a chance of 1/10 that the TNT block does'nt explode;
    // this is more realistic ;)
    // But we want to be fair; let's create a pickup for the TNT block =)
    Mineserver::get()->map(map)->createPickupSpawn(x,y,z,46,1,0,user);
    return;
  } 

  tntQueue.push_front(tntTimer(x, y, z, map, user));
}

bool BlockTNT::onInteract(User* user, int32_t x, int8_t y, int32_t z, int map)
{
  uint8_t block, metadata;
  return true;
}

void BlockTNT::timer200()
{
  if(tntQueue.size() == 0)
  {return;}
    if ((time(0) - tntQueue.front().timerStart) > 3)
    {
      tntQueue.front().primedTNT.deSpawnToAll();
      //TODO: TNT physics so the explosion is created where the primedTNT entity is.
      //The primedTNT entity has client-sided physics so people will notice if we screw up the server-sided pysics.
      Explosion::explode(tntQueue.front().user, tntQueue.front().x, tntQueue.front().y, tntQueue.front().z, tntQueue.front().map);
      tntQueue.pop_front();
    }
}
