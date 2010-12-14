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

#include "torch.h"

void BlockTorch::onStartedDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

void BlockTorch::onDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

void BlockTorch::onStoppedDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

void BlockTorch::onBroken(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{
}

void BlockTorch::onNeighbourBroken(User* user, sint8 oldblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
   uint8 block;
   uint8 meta;
   bool destroy = false;

   if (!Map::get()->getBlock(x, y, z, &block, &meta))
      return;

   if (direction == BLOCK_TOP && meta == BLOCK_TOP && this->isBlockEmpty(x, y-1, z))
   {
      destroy = true;
      // Crude fix for weird sign destruction
      uint8 tempblock;
      uint8 tempmeta;
      if(Map::get()->getBlock(x, y, z, &tempblock, &tempmeta) && tempblock == BLOCK_WALL_SIGN) 
      {
        destroy = false;
      }
   }
   else if (direction == BLOCK_NORTH && meta == BLOCK_SOUTH && this->isBlockEmpty(x+1, y, z))
   {
      destroy = true;
   }
   else if (direction == BLOCK_SOUTH && meta == BLOCK_NORTH && this->isBlockEmpty(x-1, y, z))
   {
      destroy = true;
   }
   else if (direction == BLOCK_EAST && meta == BLOCK_WEST && this->isBlockEmpty(x, y, z+1))
   {
      destroy = true;
   }
   else if (direction == BLOCK_WEST && meta == BLOCK_EAST && this->isBlockEmpty(x, y, z-1))
   {
      destroy = true;
   }

   if (destroy)
   {
      // Break torch and spawn torch item
      Map::get()->sendBlockChange(x, y, z, BLOCK_AIR, 0);
      Map::get()->setBlock(x, y, z, BLOCK_AIR, 0);
      this->spawnBlockItem(x, y, z, block);
   }
}

void BlockTorch::onPlace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
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

   if (!this->isBlockEmpty(x,y,z))
      return;

   Map::get()->setBlock(x, y, z, (char)newblock, direction);
   Map::get()->sendBlockChange(x, y, z, (char)newblock, direction);
}

void BlockTorch::onNeighbourPlace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
}

void BlockTorch::onReplace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
   uint8 oldblock;
   uint8 oldmeta;
   
   switch(newblock)
   {
      case BLOCK_WATER:
      case BLOCK_STATIONARY_WATER:
         if (Map::get()->getBlock(x, y, z, &oldblock, &oldmeta))
         {
            // spawn item
            Map::get()->sendBlockChange(x, y, z, 0, 0);
            Map::get()->setBlock(x, y, z, 0, 0);
            this->spawnBlockItem(x, y, z, oldblock);
         }
      break;
      case BLOCK_LAVA:
      case BLOCK_STATIONARY_LAVA:
         if (Map::get()->getBlock(x, y, z, &oldblock, &oldmeta))
         {
            // destroy
            Map::get()->sendBlockChange(x, y, z, 0, 0);
            Map::get()->setBlock(x, y, z, 0, 0);
         }
      break;
      default:
         return;
      break;
   }
}

void BlockTorch::onNeighbourMove(User* user, sint8 oldblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
   this->onNeighbourBroken(user, oldblock, x, y, z, direction);
}
