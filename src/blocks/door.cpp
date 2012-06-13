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

#include "mineserver.h"
#include "map.h"
//#include "logger.h"

#include "door.h"


void BlockDoor::onStartedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  uint8_t block, metadata;
  ServerInstance->map(map)->getBlock(x, y, z, &block, &metadata);
  
  if (block != BLOCK_IRON_DOOR)
  {

  // Toggle door state
  metadata ^= 0x4;

  uint8_t metadata2, block2;

  int modifier = ((metadata & 0x8) == 0x8) ? -1 : 1;
  ServerInstance->map(map)->setBlock(x, y, z, block, metadata);
  ServerInstance->map(map)->sendBlockChange(x, y, z, (char)block, metadata);

  ServerInstance->map(map)->getBlock(x, y + modifier, z, &block2, &metadata2);

  if (block2 == block)
  {
    if ((metadata2 & 0x4) && !(metadata & 0x4))
    {
      metadata2 ^= 0x4;
    }
    else if (!(metadata2 & 0x4) && (metadata & 0x4))
    {
      metadata2 ^= 0x4;
    }

    ServerInstance->map(map)->setBlock(x, y + modifier, z, block2, metadata2);
    ServerInstance->map(map)->sendBlockChange(x, y + modifier, z, (char)block, metadata2);
  }
  }
}

void BlockDoor::onDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
}

void BlockDoor::onStoppedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{

}

bool BlockDoor::onBroken(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  // get block info
  uint8_t block, metadata;
  uint8_t metadata2, block2;
  ServerInstance->map(map)->getBlock(x, y, z, &block, &metadata);
  // check if block is the upper part of the door
  int modifier = ((metadata & 0x8) == 0x8) ? -1 : 1;
  ServerInstance->map(map)->getBlock(x, y + modifier, z, &block2, &metadata2);
  // if upper part, remove it
  if (block2 == block)
  {
    ServerInstance->map(map)->setBlock(x, y + modifier, z, BLOCK_AIR, 0);
    ServerInstance->map(map)->sendBlockChange(x, y + modifier, z, BLOCK_AIR, 0);
  }
  // remove lower part
  ServerInstance->map(map)->setBlock(x, y, z, BLOCK_AIR, 0);
  ServerInstance->map(map)->sendBlockChange(x, y, z, BLOCK_AIR, 0);
  this->spawnBlockItem(x, y, z, map, block, 0);
  return false;
}

void BlockDoor::onNeighbourBroken(User* user, int16_t oldblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  // get block info
  uint8_t block, metadata;
  uint8_t metadata2, block2;
  ServerInstance->map(map)->getBlock(x, y, z, &block, &metadata);
  // check if block is the upper part of the door
  int modifier = ((metadata & 0x8) == 0x8) ? -1 : 1;

  ServerInstance->map(map)->getBlock(x, y + modifier, z, &block2, &metadata2);
  if (metadata & 0x4)
  {
    metadata &= (0x8 | 0x3);
  }
  else
  {
    metadata |= 0x4;
  }
  // do nothing if the block is not below door
  if (direction == BLOCK_BOTTOM && (block == BLOCK_WOODEN_DOOR || block == BLOCK_IRON_DOOR))
  {
    return;
  }
  // break the door if the block is below it
  if (direction == BLOCK_TOP && (block == BLOCK_WOODEN_DOOR || block == BLOCK_IRON_DOOR))
  {
    ServerInstance->map(map)->setBlock(x, y, z, BLOCK_AIR, 0);
    ServerInstance->map(map)->sendBlockChange(x, y, z, BLOCK_AIR, 0);
    this->spawnBlockItem(x, y, z, map, block, 0);

    ServerInstance->map(map)->getBlock(x, y + modifier, z, &block2, &metadata2);

    if (block2 == block)
    {
      metadata2 = metadata;

      if (metadata & 0x8)
      {
        metadata2 &= 0x7;
      }
      else
      {
        metadata2 |= 0x8;
      }

      ServerInstance->map(map)->setBlock(x, y + modifier, z, BLOCK_AIR, 0);
      ServerInstance->map(map)->sendBlockChange(x, y + modifier, z, BLOCK_AIR, 0);
    }
  }
}

bool BlockDoor::onPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
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

  // checking for an item rather then a block
  if (newblock == ITEM_WOODEN_DOOR)
  {
    newblock = BLOCK_WOODEN_DOOR;
  }
  else if (newblock == ITEM_IRON_DOOR)
  {
    newblock = BLOCK_IRON_DOOR;
  }

  direction = user->relativeToBlock(x, y, z);

  switch (direction)
  {
  case BLOCK_EAST:
  //LOG(INFO, "Map", "EAST");
    direction = BLOCK_EAST;
    break;
  case BLOCK_WEST:
  //LOG(INFO, "Map", "WEST");
    direction = BLOCK_WEST;
    break;
  case BLOCK_NORTH:
  //LOG(INFO, "Map", "NORTH");
    direction = BLOCK_SOUTH;
    break;
  case BLOCK_SOUTH:
  //LOG(INFO, "Map", "SOUTH");
    direction = BLOCK_NORTH;
    break;
  }

  ServerInstance->map(map)->setBlock(x, y, z, (char)newblock, direction);
  ServerInstance->map(map)->sendBlockChange(x, y, z, (char)newblock, direction);

  /* Get correct direction for top of the door */
  direction ^= 8;

  ServerInstance->map(map)->setBlock(x, y + 1, z, (char)newblock, direction);
  ServerInstance->map(map)->sendBlockChange(x, y + 1, z, (char)newblock, direction);
  return false;
}

void BlockDoor::onNeighbourPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
}

void BlockDoor::onReplace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
}

bool BlockDoor::onInteract(User* user, int32_t x, int8_t y, int32_t z, int map)
{
  uint8_t block, metadata;
  uint8_t metadata2, block2;
  ServerInstance->map(map)->getBlock(x, y, z, &block, &metadata);
  int modifier = ((metadata & 0x8) == 0x8) ? -1 : 1;

  if (block != BLOCK_IRON_DOOR)
  {

  ServerInstance->map(map)->getBlock(x, y + modifier, z, &block2, &metadata2);
  if (metadata & 0x4)
  {
    metadata &= (0x8 | 0x3);
  }
  else
  {
    metadata |= 0x4;
  }

  ServerInstance->map(map)->setBlock(x, y, z, block, metadata);
  ServerInstance->map(map)->sendBlockChange(x, y, z, (char)block, metadata);

  ServerInstance->map(map)->getBlock(x, y + modifier, z, &block2, &metadata2);

  if (block2 == block)
  {
    metadata2 = metadata;

    if (metadata & 0x8)
    {
      metadata2 &= 0x7;
    }
    else
    {
      metadata2 |= 0x8;
    }

    ServerInstance->map(map)->setBlock(x, y + modifier, z, block2, metadata2);
    ServerInstance->map(map)->sendBlockChange(x, y + modifier, z, (char)block, metadata2);
  }
 }
  return false;
}
