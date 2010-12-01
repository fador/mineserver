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

#include "fire.h"

void BlockFire::onStartedDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

void BlockFire::onDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

void BlockFire::onStoppedDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

void BlockFire::onBroken(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{
}

void BlockFire::onNeighbourBroken(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

void BlockFire::onPlace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
   uint8 topblock;
   uint8 topmeta;
   uint8 oldblock;
   uint8 oldmeta;

   if (Map::get()->getBlock(x, y, z, &oldblock, &oldmeta))
   {
      switch(oldblock)
      {
         case BLOCK_FIRE:
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
            if (Map::get()->getBlock(x, y+1, z, &topblock, &topmeta) && topblock == BLOCK_AIR)
            {
               Map::get()->setBlock(x, y+1, z, (char)newblock, 0);
               Map::get()->sendBlockChange(x, y+1, z, (char)newblock, 0);
            }
         break;
      }
   }
}

void BlockFire::onNeighbourPlace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
}

void BlockFire::onReplace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

