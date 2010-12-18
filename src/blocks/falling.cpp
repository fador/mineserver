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

#include "falling.h"
#include "../plugin.h"

void BlockFalling::onStartedDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{
}

void BlockFalling::onDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{
}

void BlockFalling::onStoppedDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{
}

void BlockFalling::onBroken(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{
}

void BlockFalling::onNeighbourBroken(User* user, sint8 oldblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
   this->onNeighbourMove(user, oldblock, x, y, z, direction);
}

void BlockFalling::onPlace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
   uint8 oldblock;
   uint8 oldmeta;

   if (!Map::get()->getBlock(x, y, z, &oldblock, &oldmeta))
      return;

   /* Check block below allows blocks placed on top */
   if (!this->isBlockStackable(oldblock))
      return;

   /* move the x,y,z coords dependent upon placement direction */
   if (!this->translateDirection(&x,&y,&z,direction))
      return;

   if (this->isUserOnBlock(x,y,z))
      return;

   if (!this->isBlockEmpty(x,y,z))
      return;

   direction = user->relativeToBlock(x, y, z);

   Map::get()->setBlock(x, y, z, (char)newblock, direction);
   Map::get()->sendBlockChange(x, y, z, (char)newblock, direction);

   applyPhysics(user,x,y,z);
}

void BlockFalling::onNeighbourPlace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
}

void BlockFalling::onReplace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
}

void BlockFalling::onNeighbourMove(User* user, sint8 oldblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
  uint8 block;
  uint8 meta;

  if (!Map::get()->getBlock(x, y, z, &block, &meta))
  {
    return;
  }

  applyPhysics(user,x,y,z);
}

void BlockFalling::applyPhysics(User* user, sint32 x, sint8 y, sint32 z)
{
  uint8 fallblock, block;
  uint8 fallmeta, meta;
   
  if (!Map::get()->getBlock(x, y, z, &fallblock, &fallmeta))
  {
    return;
  }
   
  while(Map::get()->getBlock(x, y-1, z, &block, &meta))
  {
    switch(block)
    {
      case BLOCK_AIR:
      case BLOCK_WATER:
      case BLOCK_STATIONARY_WATER:
      case BLOCK_LAVA:
      case BLOCK_STATIONARY_LAVA:
        break;
      default:
        // stop falling
        return;
        break;
     }

     // Destroy original block
     Map::get()->sendBlockChange(x, y, z, BLOCK_AIR, 0);
     Map::get()->setBlock(x, y, z, BLOCK_AIR, 0);

     y--;

     Map::get()->setBlock(x, y, z, fallblock, fallmeta);
     Map::get()->sendBlockChange(x, y, z, fallblock, fallmeta);

     this->notifyNeighbours(x, y+1, z, "onNeighbourMove", user, block, BLOCK_BOTTOM);
   }
}

