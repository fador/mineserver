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

#include "../mineserver.h"
#include "../map.h"

#include "door.h"

void BlockDoor::onStartedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int8_t direction)
{
    uint8_t block,metadata;
    Mineserver::get()->map()->getBlock(x, y, z, &block, &metadata);

   // Toggle door state
   if (metadata & 0x4)
   {
     metadata &= (0x8 | 0x3);
   }
   else
   {
     metadata |= 0x4;
   }

   uint8_t metadata2, block2;

   int modifier = (metadata & 0x8) ? -1 : 1;

   Mineserver::get()->map()->setBlock(x, y, z, block, metadata);
   Mineserver::get()->map()->sendBlockChange(x, y, z, (char)block, metadata);  

   Mineserver::get()->map()->getBlock(x, y + modifier, z, &block2, &metadata2);

   if (block2 == block)
   {
     metadata2 = metadata;
   
     if(metadata & 0x8)
       metadata2 &= 0x7;
     else
       metadata2 |= 0x8;

     Mineserver::get()->map()->setBlock(x, y + modifier, z, block2, metadata2);
     Mineserver::get()->map()->sendBlockChange(x, y + modifier, z, (char)block, metadata2);
   }
}

void BlockDoor::onDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int8_t direction)
{

}

void BlockDoor::onStoppedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int8_t direction)
{

}

void BlockDoor::onBroken(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int8_t direction)
{
}

void BlockDoor::onNeighbourBroken(User* user, int8_t oldblock, int32_t x, int8_t y, int32_t z, int8_t direction)
{
}

void BlockDoor::onPlace(User* user, int8_t newblock, int32_t x, int8_t y, int32_t z, int8_t direction)
{
   uint8_t oldblock;
   uint8_t oldmeta;

   if (!Mineserver::get()->map()->getBlock(x, y, z, &oldblock, &oldmeta))
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

   // checking for an item rather then a block
   int block = newblock + 256;

   if (block == ITEM_WOODEN_DOOR)
      newblock = BLOCK_WOODEN_DOOR;

   if (block == ITEM_IRON_DOOR)
      newblock = BLOCK_IRON_DOOR;

   direction = user->relativeToBlock(x, y, z);

   switch(direction)
   {
      case BLOCK_EAST:
         direction = BLOCK_WEST;
      break;
      case BLOCK_WEST:
         direction = BLOCK_EAST;
      break;
      case BLOCK_NORTH:
         direction = BLOCK_SOUTH;
      break;
      case BLOCK_SOUTH:
         direction = BLOCK_NORTH;
      break;
   }

   Mineserver::get()->map()->setBlock(x, y, z, (char)newblock, direction);
   Mineserver::get()->map()->sendBlockChange(x, y, z, (char)newblock, direction);

   /* Get correct direction for top of the door */
   direction = 8+(direction);

   Mineserver::get()->map()->setBlock(x, y+1, z, (char)newblock, direction);
   Mineserver::get()->map()->sendBlockChange(x, y+1, z, (char)newblock, direction);
}

void BlockDoor::onNeighbourPlace(User* user, int8_t newblock, int32_t x, int8_t y, int32_t z, int8_t direction)
{
}

void BlockDoor::onReplace(User* user, int8_t newblock, int32_t x, int8_t y, int32_t z, int8_t direction)
{
}
