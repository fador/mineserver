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
#include "physics.h"
#include "liquid.h"


void BlockLiquid::onStartedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
}

void BlockLiquid::onDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
}

void BlockLiquid::onStoppedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
}

bool BlockLiquid::onBroken(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  return false;
}

void BlockLiquid::onNeighbourBroken(User* user, int16_t oldblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  physics(x, y, z, map);
}

bool BlockLiquid::onPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  uint8_t oldblock;
  uint8_t oldmeta;

  if (!ServerInstance->map(map)->getBlock(x, y, z, &oldblock, &oldmeta))
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
  if (!ServerInstance->map(map)->getBlock(x, y, z, &oldblock, &oldmeta))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }


  Item* item = &user->inv[user->curItem + 36];
  if (newblock > 255)
  {
    if (item->getCount() > 1)
    {
      // Too many bugs working with multi-buckets
      revertBlock(user, x, y, z, map);
      item->sendUpdate();
      return true;
    }
  }
  // Remove liquid from map, add to bucket.
  if (newblock == ITEM_BUCKET && oldmeta == 0 && affectedBlock(oldblock))
  {
    int new_item = -1;
    if (oldblock == BLOCK_LAVA || oldblock == BLOCK_STATIONARY_LAVA)
    {
      new_item = ITEM_LAVA_BUCKET;
    }
    else
    {
      new_item = ITEM_WATER_BUCKET;
    }
    item->setType(new_item);

    ServerInstance->map(map)->setBlock(x, y, z, BLOCK_AIR, 0);
    ServerInstance->map(map)->sendBlockChange(x, y, z, BLOCK_AIR, 0);
    return true;
  }
  if (!this->isBlockEmpty(x, y, z, map))
  {
    revertBlock(user, x, y, z, map);
    item->sendUpdate();
    return true;
  }
  if (oldblock != BLOCK_AIR)
  {
    // Shouldnt replace liquids.
    revertBlock(user, x, y, z, map);
    item->sendUpdate();
    return true;
  }

  direction = user->relativeToBlock(x, y, z);

  if (newblock == ITEM_WATER_BUCKET)
  {
    newblock = BLOCK_WATER;
    item->setType(ITEM_BUCKET);
  }
  else if (newblock == ITEM_LAVA_BUCKET)
  {
    newblock = BLOCK_LAVA;
    item->setType(ITEM_BUCKET);
  }
  else if (newblock > 255)
  {
    revertBlock(user, x, y, z, map);
    item->sendUpdate();
    return true;
  }
  else
  {
    item->decCount();
  }

  ServerInstance->map(map)->setBlock(x, y, z, (char)newblock, 0);
  ServerInstance->map(map)->sendBlockChange(x, y, z, (char)newblock, 0);

  physics(x, y, z, map);
  return true;
}

void BlockLiquid::onNeighbourPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  physics(x, y, z, map);
}

void BlockLiquid::onReplace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  uint8_t oldblock;
  uint8_t oldmeta;
  if (newblock > 255)
  {
    return;
  }
  if (!ServerInstance->map(map)->getBlock(x, y, z, &oldblock, &oldmeta))
  {
    return;
  }

  ServerInstance->physics(map)->removeSimulation(vec(x, y, z));

  ServerInstance->map(map)->sendBlockChange(x, y, z, BLOCK_AIR, 0);
  ServerInstance->map(map)->setBlock(x, y, z, BLOCK_AIR, 0);
}

void BlockLiquid::physics(int32_t x, int8_t y, int32_t z, int map)
{
  ServerInstance->physics(map)->addSimulation(vec(x, y, z));
  //ServerInstance->physics()->checkSurrounding(vec(x, y, z));
}
