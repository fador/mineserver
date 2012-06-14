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

uint8_t pickint;

void BlockTNT::rb(int32_t x,int8_t y,int8_t z,int map, User* user)
{
  uint8_t block, meta, count;
  int16_t item;

  ServerInstance->map(map)->getBlock(x,y,z,&block,&meta);

  BLOCKDROPS[block]->getDrop(item, count, meta);

  // Undestroyable blocks
  if (block == BLOCK_AIR || block == BLOCK_BEDROCK || block == BLOCK_OBSIDIAN)
  {
    return;
  }
  else
  {
    ServerInstance->map(map)->setBlock(x,y,z,0,0);
    ServerInstance->map(map)->sendBlockChange(x,y,z,0,0);
  }
  
  // Pickup Spawn Area
  // The integer "pickint" is used to spawn 1/5 of the blocks, otherwise there would be too much pickups!
  if(pickint == 5)
  {
    if(count) {
      ServerInstance->map(map)->createPickupSpawn(x, y, z, item, count, meta, user);
      pickint=0;
    }
  } else {
    pickint++;
  }

}

void BlockTNT::explode(User* user,int32_t x,int8_t y,int8_t z,int map)
{
  pickint = 0;
  if (rand() % 9 == 5) {
    // There is a chance of 1/10 that the TNT block does'nt explode;
    // this is more realistic ;)
    ServerInstance->map(map)->setBlock(x,y,z,0,0);
    ServerInstance->map(map)->sendBlockChange(x,y,z,0,0);
    // But we want to be fair; let's create a pickup for the TNT block =)
    ServerInstance->map(map)->createPickupSpawn(x,y,z,46,1,0,user);
  } else {
    int number; // Counter in the for(...){...} loops.
    // Layer Y-4
  
    //rb(x,y-4,z,map,user);
    
    // Layer Y-3
    
    rb(x-1,y-3,z+1,map,user);
    rb(x,y-3,z+1,map,user);
    rb(x+1,y-3,z+1,map,user);
    rb(x-1,y-3,z,map,user);
    rb(x,y-3,z,map,user);
    rb(x+1,y-3,z,map,user);
    rb(x-1,y-3,z-1,map,user);
    rb(x,y-3,z-1,map,user);
    rb(x+1,y-3,z-1,map,user);
  
    // Layer Y-2
  
    for (number=-2; number<=2; number++) {
      rb(x+number,y-2,z+1,map,user);
      rb(x+number,y-2,z,map,user);
      rb(x+number,y-2,z-1,map,user);
    }
  
    rb(x-1,y-2,z+2,map,user);
    rb(x,y-2,z+2,map,user);
    rb(x+1,y-2,z+2,map,user);
  
    rb(x-1,y-2,z-2,map,user);
    rb(x,y-2,z-2,map,user);
    rb(x+1,y-2,z-2,map,user);
  
    // Layer Y-1
  
    rb(x-3,y-1,z+2,map,user);
    rb(x-3,y-1,z+1,map,user);
    rb(x-3,y-1,z,map,user);
    rb(x-3,y-1,z-1,map,user);
    rb(x-3,y-1,z-2,map,user);
  
    for (number=-2; number<=2; number++) {
      rb(x-number,y-1,z+3,map,user);
      rb(x-number,y-1,z+2,map,user);
      rb(x-number,y-1,z+1,map,user);
      rb(x-number,y-1,z,map,user);
      rb(x-number,y-1,z-1,map,user);
      rb(x-number,y-1,z-2,map,user);
      rb(x-number,y-1,z-3,map,user);
    }
  
    rb(x+3,y-1,z+2,map,user);
    rb(x+3,y-1,z+1,map,user);
    rb(x+3,y-1,z,map,user);
    rb(x+3,y-1,z-1,map,user);
    rb(x+3,y-1,z-2,map,user);
  
  
    // Layer Y {+,-} 0, same as TNT block
  
    //rb(x-4,y,z,map,user);
  
    rb(x-3,y,z+2,map,user);
    rb(x-3,y,z+1,map,user);
    rb(x-3,y,z,map,user);
    rb(x-3,y,z-1,map,user);
    rb(x-3,y,z-2,map,user);
  
    for (number=-2; number<=2; number++) {
      rb(x-number,y,z+3,map,user);
      rb(x-number,y,z+2,map,user);
      rb(x-number,y,z+1,map,user);
      rb(x-number,y,z,map,user);
      rb(x-number,y,z-1,map,user);
      rb(x-number,y,z-2,map,user);
      rb(x-number,y,z-3,map,user);
    }
  
    rb(x+3,y,z+2,map,user);
    rb(x+3,y,z+1,map,user);
    rb(x+3,y,z,map,user);
    rb(x+3,y,z-1,map,user);
    rb(x+3,y,z-2,map,user);
  
  //rb(x+4,y,z,map,user);
  
  //rb(x,y,z+4,map,user);
  //rb(x,y,z-4,map,user);
  
    // Layer Y+1
  
    rb(x-3,y+1,z+2,map,user);
    rb(x-3,y+1,z+1,map,user);
    rb(x-3,y+1,z,map,user);
    rb(x-3,y+1,z-1,map,user);
    rb(x-3,y+1,z-2,map,user);
  
    for (number=-2; number<=2; number++) {
      rb(x-number,y+1,z+3,map,user);
      rb(x-number,y+1,z+2,map,user);
      rb(x-number,y+1,z+1,map,user);
      rb(x-number,y+1,z,map,user);
      rb(x-number,y+1,z-1,map,user);
      rb(x-number,y+1,z-2,map,user);
      rb(x-number,y+1,z-3,map,user);
    }
  
    rb(x+3,y+1,z+2,map,user);
    rb(x+3,y+1,z+1,map,user);
    rb(x+3,y+1,z,map,user);
    rb(x+3,y+1,z-1,map,user);
    rb(x+3,y+1,z-2,map,user);
  
    // Layer Y+2
  
    for (number=-2; number<=2; number++) {
      rb(x+number,y+2,z+1,map,user);
      rb(x+number,y+2,z,map,user);
      rb(x+number,y+2,z-1,map,user);
    }
  
    rb(x-1,y+2,z+2,map,user);
    rb(x,y+2,z+2,map,user);
    rb(x+1,y+2,z+2,map,user);
  
    rb(x-1,y+2,z-2,map,user);
    rb(x,y+2,z-2,map,user);
    rb(x+1,y+2,z-2,map,user);
  
    // Layer Y+3
  
    rb(x-1,y+3,z+1,map,user);
    rb(x,y+3,z+1,map,user);
    rb(x+1,y+3,z+1,map,user);
    rb(x-1,y+3,z,map,user);
    rb(x,y+3,z,map,user);
    rb(x+1,y+3,z,map,user);
    rb(x-1,y+3,z-1,map,user);
    rb(x,y+3,z-1,map,user);
    rb(x+1,y+3,z-1,map,user);
  
    // Layer Y+4
  
  //rb(x,y+4,z,map,user);
  
    LOG2(INFO,"TNT Block exploded!");
  }
}

bool BlockTNT::onPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  uint8_t oldblock;
  uint8_t oldmeta;
  
  if (!ServerInstance->map(map)->getBlock(x, y, z, &oldblock, &oldmeta))
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
  explode(user,x,y,z,map);
  
}

bool BlockTNT::onInteract(User* user, int32_t x, int8_t y, int32_t z, int map)
{
  uint8_t block, metadata;
  return true;
}
