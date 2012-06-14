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

#include "step.h"


void BlockStep::onStartedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{

}

void BlockStep::onDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{

}

void BlockStep::onStoppedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{

}

bool BlockStep::onBroken(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  uint8_t block, meta;
  ServerInstance->map(map)->getBlock(x, y, z, &block, &meta);

  ServerInstance->map(map)->setBlock(x, y, z, BLOCK_AIR, 0);
  ServerInstance->map(map)->sendBlockChange(x, y, z, BLOCK_AIR, 0);

  this->spawnBlockItem(x, y, z, map, block, meta);
  return false;
}

void BlockStep::onNeighbourBroken(User* user, int16_t oldblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
}

bool BlockStep::onPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  uint8_t oldblock;
  uint8_t oldmeta;

  if (!ServerInstance->map(map)->getBlock(x, y, z, &oldblock, &oldmeta))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

  //Combine two steps
  if (newblock == BLOCK_STEP && oldblock == BLOCK_STEP && direction == BLOCK_TOP)
  {
    Item item = user->inv[user->curItem + 36];

    if (item.getHealth() == oldmeta)
    {
      ServerInstance->map(map)->setBlock(x, y, z, (char)BLOCK_DOUBLE_STEP, oldmeta);
      ServerInstance->map(map)->sendBlockChange(x, y, z, (char)BLOCK_DOUBLE_STEP, oldmeta);
      revertBlock(user, x, y, z, map);
      return true;
    }
    else
    {
      revertBlock(user, x, y, z, map);
      return true;
    }
  }

  /* Check block below allows blocks placed on top */
  if (!this->isBlockStackable(oldblock))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

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

  Item item = user->inv[user->curItem + 36];

  ServerInstance->map(map)->setBlock(x, y, z, (char)newblock, item.getHealth());
  ServerInstance->map(map)->sendBlockChange(x, y, z, (char)newblock, item.getHealth());
  return false;
}

void BlockStep::onNeighbourPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
}

void BlockStep::onReplace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
}
