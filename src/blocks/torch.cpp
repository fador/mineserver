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

#include <cmath>

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

void BlockTorch::onNeighbourBroken(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{
   uint8 block; uint8 meta;
   uint8 nblock; uint8 nmeta;
   bool destroy = false;

   if (!Map::get()->getBlock(x, y, z, &block, &meta))
      return;

   if (direction == BLOCK_TOP && Map::get()->getBlock(x, y-1, z, &nblock, &nmeta) && nblock == BLOCK_AIR)
   {
      // block broken under torch
      destroy = true;
   }
   else if (direction == BLOCK_NORTH && Map::get()->getBlock(x-1, y, z, &nblock, &nmeta) && nblock == BLOCK_AIR)
   {
      destroy = true;
   }
   else if (direction == BLOCK_SOUTH && Map::get()->getBlock(x+1, y, z, &nblock, &nmeta) && nblock == BLOCK_AIR)
   {
      destroy = true;
   }
   else if (direction == BLOCK_EAST && Map::get()->getBlock(x, y, z-1, &nblock, &nmeta) && nblock == BLOCK_AIR)
   {
      destroy = true;
   }
   else if (direction == BLOCK_WEST && Map::get()->getBlock(x, y, z+1, &nblock, &nmeta) && nblock == BLOCK_AIR)
   {
      destroy = true;
   }

   if (destroy)
   {
      // Break torch and spawn torch item
      Map::get()->sendBlockChange(x, y, z, BLOCK_AIR, 0);
      Map::get()->setBlock(x, y, z, BLOCK_AIR, 0);
      int count = 1;
      if (BLOCKDROPS.count(block) && BLOCKDROPS[block].probability >= rand() % 10000)
      {
          uint16 item_id = BLOCKDROPS[block].item_id;
          count = BLOCKDROPS[block].count;
          Map::get()->createPickupSpawn(x, y, z, item_id, count);
      }
   }   
}

void BlockTorch::onPlace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
   uint8 oldblock;
   uint8 oldmeta;

   if (Map::get()->getBlock(x, y-1, z, &oldblock, &oldmeta))
   {
      /* Check block below allows blocks placed on top */
      switch(oldblock)
      {
         case BLOCK_WORKBENCH:
         case BLOCK_FURNACE:
         case BLOCK_BURNING_FURNACE:
         case BLOCK_CHEST:
         case BLOCK_JUKEBOX:
         case BLOCK_TORCH:
         case BLOCK_REDSTONE_TORCH_OFF:
         case BLOCK_REDSTONE_TORCH_ON:
         case BLOCK_WATER:
         case BLOCK_STATIONARY_WATER:
         case BLOCK_LAVA:
         case BLOCK_STATIONARY_LAVA:
          return;
         break;
         default:
            switch(direction)
            {
               case BLOCK_SOUTH:
                  x--;
               break;
               case BLOCK_NORTH:
                  x++;
               break;
               case BLOCK_EAST:
                  z++;
               break;
               case BLOCK_WEST:
                  z--;
               break;
               case BLOCK_TOP:
                  y++;
               break;
               default:
                  return;
               break;
            }

            uint8 block;
            uint8 meta;
            if (Map::get()->getBlock(x, y, z, &block, &meta) && block == BLOCK_AIR)
            {
               Map::get()->setBlock(x, y, z, (char)newblock, direction);
               Map::get()->sendBlockChange(x, y, z, (char)newblock, direction);
            }
         break;
      }
   }
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
            Map::get()->createPickupSpawn(x, y, z, oldblock, 1);
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
