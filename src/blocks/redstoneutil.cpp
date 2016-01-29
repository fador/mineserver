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
#include "logger.h"
#include "redstoneSimulation.h"
#include "vec.h"

#include "redstoneutil.h"


void BlockRedstoneUtil::timer200() {
  // Loop activated buttons -list
  for (PositionList::iterator iter = activated.begin(); iter != activated.end();) {
    if (!(--iter->timeleft)) {
      uint8_t block;
      uint8_t meta;
      if (ServerInstance->map(iter->map)->getBlock(iter->x, iter->y, iter->z, &block, &meta)) {
        // Enable button
        meta ^= 0x08;

        // Save the block
        ServerInstance->map(iter->map)->setBlock(iter->x, iter->y, iter->z, char(block), meta);
        ServerInstance->map(iter->map)->sendBlockChange(iter->x, iter->y, iter->z, char(block), meta);
      }

      // Remove button from list
      iter = activated.erase(iter);
    }
    else {
      iter++;
    }
  }
}

void BlockRedstoneUtil::onStartedDigging(User* user, int8_t status, int32_t x, int16_t y, int32_t z, int map, int8_t direction)
{
  this->onInteract(user, x, y, z, map);
}

void BlockRedstoneUtil::onDigging(User* user, int8_t status, int32_t x, int16_t y, int32_t z, int map, int8_t direction)
{

}

void BlockRedstoneUtil::onStoppedDigging(User* user, int8_t status, int32_t x, int16_t y, int32_t z, int map, int8_t direction)
{

}

void BlockRedstoneUtil::onNeighbourBroken(User* user, int16_t oldblock, int32_t x, int16_t y, int32_t z, int map, int8_t direction)
{
  uint8_t block;
  uint8_t meta;
  bool destroy = false;

  if (!ServerInstance->map(map)->getBlock(x, y, z, &block, &meta))
  {
    return;
  }

  if (direction == BLOCK_TOP && meta == BLOCK_TOP && this->isBlockEmpty(x, y - 1, z, map))
  {
    destroy = true;
    // Crude fix for weird sign destruction
    uint8_t tempblock;
    uint8_t tempmeta;
    if (ServerInstance->map(map)->getBlock(x, y, z, &tempblock, &tempmeta) && tempblock == BLOCK_WALL_SIGN)
    {
      destroy = false;
    }
  }
  else if (direction == BLOCK_NORTH && meta == BLOCK_SOUTH && this->isBlockEmpty(x + 1, y, z, map))
  {
    destroy = true;
  }
  else if (direction == BLOCK_SOUTH && meta == BLOCK_NORTH && this->isBlockEmpty(x - 1, y, z, map))
  {
    destroy = true;
  }
  else if (direction == BLOCK_EAST && meta == BLOCK_WEST && this->isBlockEmpty(x, y, z + 1, map))
  {
    destroy = true;
  }
  else if (direction == BLOCK_WEST && meta == BLOCK_EAST && this->isBlockEmpty(x, y, z - 1, map))
  {
    destroy = true;
  }

  if (destroy)
  {
    // Break switch and spawn item
    ServerInstance->map(map)->sendBlockChange(x, y, z, BLOCK_AIR, 0);
    ServerInstance->map(map)->setBlock(x, y, z, BLOCK_AIR, 0);
    this->spawnBlockItem(x, y, z, map, block);
  }
}

bool BlockRedstoneUtil::onPlace(User* user, int16_t newblock, int32_t x, int16_t y, int32_t z, int map, int8_t direction)
{
  uint8_t block;
  uint8_t meta = 0;
  if (!ServerInstance->map(map)->getBlock(x, y, z, &block, &meta))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

  /* Check block below allows blocks placed on top */
  if (!isBlockStackable(block))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

  /* move the x,y,z coords dependent upon placement direction */
  if (!translateDirection(&x, &y, &z, map, direction))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

  if (!isBlockEmpty(x, y, z, map))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }


  // Check that switch is placed on legal direction
  if ((newblock == BLOCK_STONE_BUTTON && (direction == BLOCK_TOP || direction == BLOCK_BOTTOM))
  || ((newblock == BLOCK_STONE_PRESSURE_PLATE || newblock == BLOCK_WOODEN_PRESSURE_PLATE) && direction != BLOCK_TOP)) {
  revertBlock(user, x, y, z, map);
    return true;
  }

  // Set metadata for the position
  if (newblock == BLOCK_STONE_BUTTON || newblock == BLOCK_LEVER) {
    // Wall switch
    if (direction != BLOCK_TOP && direction != BLOCK_BOTTOM) {
    meta = direction;
    }
    // Ceiling lever
    else if (direction == BLOCK_BOTTOM) {
      int yaw = abs(int(user->pos.yaw)) % 360;
      if ((yaw > 45 && yaw <= 135) || (yaw > 225 && yaw <= 315)) {
        meta = 0x00;
      }
      else {
        meta = 0x07;
      }
    }
    // Floor lever
    else {
      int yaw = abs(int(user->pos.yaw)) % 360;
      if ((yaw > 45 && yaw <= 135) || (yaw > 225 && yaw <= 315)) {
        meta = 0x06;
      }
      else {
        meta = 0x05;
      }
    }
  }

  ServerInstance->map(map)->setBlock(x, y, z, char(newblock), meta);
  ServerInstance->map(map)->sendBlockChange(x, y, z, char(newblock), meta);

  return false;
}

void BlockRedstoneUtil::onNeighbourPlace(User* user, int16_t newblock, int32_t x, int16_t y, int32_t z, int map, int8_t direction)
{

}

void BlockRedstoneUtil::onReplace(User* user, int16_t newblock, int32_t x, int16_t y, int32_t z, int map, int8_t direction)
{

}

void BlockRedstoneUtil::onNeighbourMove(User* user, int16_t oldblock, int32_t x, int16_t y, int32_t z, int map, int8_t direction)
{

}

bool BlockRedstoneUtil::onInteract(User* user, int32_t x, int16_t y, int32_t z, int map) {
  uint8_t block = 0;
  uint8_t meta = 0;

  if (!ServerInstance->map(map)->getBlock(x, y, z, &block, &meta))
  {
    return true;
  }

  if (block == BLOCK_STONE_BUTTON) {
    // Enable button if it's not already enabled
    if ((meta & 0x08) != 0) {
    return true;
    }
    
    meta |= 0x08;

    // Save button position
    activated.push_back(Position(x, y, z, map));
  }
  else if (block == BLOCK_LEVER) {
    // Enable/disable lever
    meta ^= 0x08;
  }
  else {
    return true;
  }

  ServerInstance->map(map)->setBlock(x, y, z, char(block), meta);
  ServerInstance->map(map)->sendBlockChange(x, y, z, char(block), meta);

  return true;
}