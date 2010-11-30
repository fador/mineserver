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

#include "plant.h"

void BlockPlant::onStartedDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

void BlockPlant::onDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

void BlockPlant::onStoppedDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

void BlockPlant::onBroken(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{
}

void BlockPlant::onNeighbourBroken(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{
   uint8 block; uint8 meta;
   uint8 nblock; uint8 nmeta;
   bool destroy = false;  
   if (!Map::get()->getBlock(x, y, z, &block, &meta))
      return;
      
   if (meta == BLOCK_TOP && Map::get()->getBlock(x, y-1, z, &nblock, &nmeta) && nblock == BLOCK_AIR)
   {
      // block broken under plant
      destroy = true;
   }

   if (destroy)
   {
      // Break plant and spawn plant item
      Map::get()->sendBlockChange(x, y, z, BLOCK_AIR, 0);
      Map::get()->setBlock(x, y, z, BLOCK_AIR, 0);
      Map::get()->createPickupSpawn(x, y, z, block, 1);
   }   
}

void BlockPlant::onPlace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
   uint8 oldblock;
   uint8 oldmeta;
   uint8 topblock;
   uint8 topmeta;

   if (Map::get()->getBlock(x, y, z, &oldblock, &oldmeta))
   {
      /* Only place of dirt or grass */
      switch(oldblock)
      {
         case BLOCK_GRASS:
         case BLOCK_SOIL:
         case BLOCK_DIRT:
            if (Map::get()->getBlock(x, y+1, z, &topblock, &topmeta) && topblock == BLOCK_AIR)
            {
               Map::get()->setBlock(x, y+1, z, (char)newblock, 0);
               Map::get()->sendBlockChange(x, y+1, z, (char)newblock, 0);
            }
         break;
         default:
            return;
         break;
      }
   }
}

void BlockPlant::onNeighbourPlace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
}

void BlockPlant::onReplace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}
