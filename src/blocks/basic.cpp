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

#include <cmath>
#include <cstdlib>

#include "mineserver.h"
#include "plugin.h"
#include "map.h"

#include "basic.h"


bool BlockBasic::isBlockStackable(const uint8_t block)
{
  /* Check block below allows blocks placed on top */
  switch (block)
  {
  case BLOCK_WORKBENCH:
  case BLOCK_CHEST:
  case BLOCK_JUKEBOX:
  case BLOCK_TORCH:
  case BLOCK_REDSTONE_TORCH_OFF:
  case BLOCK_REDSTONE_TORCH_ON:
  case BLOCK_REDSTONE_WIRE:
  case BLOCK_WATER:
  case BLOCK_STATIONARY_WATER:
  case BLOCK_LAVA:
  case BLOCK_STATIONARY_LAVA:
  case BLOCK_AIR:
  case BLOCK_MINECART_TRACKS:
  case BLOCK_WOODEN_DOOR:
  case BLOCK_IRON_DOOR:
  case BLOCK_ICE:
  case BLOCK_CAKE:
  case BLOCK_BED:
    return false;
    break;
  default:
    break;
  }

  return true;
}

bool BlockBasic::isUserOnBlock(const int32_t x, const int8_t y, const int32_t z, const int map)
{
  /* TODO: Get Users by chunk rather than whole list */
  for (std::set<User*>::iterator it = Mineserver::get()->users().begin(); it != Mineserver::get()->users().end(); ++it)
  {
    /* don't allow block placement on top of player */
    if ((*it)->checkOnBlock(x, y, z) || (*it)->checkOnBlock(x, y - 1, z))
    {
      return true;
    }
  }

  return false;
}

bool BlockBasic::translateDirection(int32_t* x, int8_t* y, int32_t* z, int map, const int8_t direction)
{
  uint8_t block, meta;
  Mineserver::get()->map(map)->getBlock(*x, *y, *z, &block, &meta);
  if (block != BLOCK_SNOW)
  {
    switch (direction)
    {
    case BLOCK_SOUTH:
      *x -= 1;
      break;
    case BLOCK_NORTH:
      *x += 1;
      break;
    case BLOCK_EAST:
      *z += 1;
      break;
    case BLOCK_WEST:
      *z -= 1;
      break;
    case BLOCK_TOP:
      *y += 1;
      break;
    case BLOCK_BOTTOM:
      *y -= 1;
      break;
    default:
      return false;
      break;
    }
  }
  return true;
}

bool BlockBasic::isBlockEmpty(const int32_t x, const int8_t y, const int32_t z, const int map)
{
  uint8_t block;
  uint8_t meta;
  if (!Mineserver::get()->map(map)->getBlock(x, y, z, &block, &meta))
  {
    return false;
  }
  return (block == BLOCK_AIR || block == BLOCK_WATER || block == BLOCK_STATIONARY_WATER || block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA || block == BLOCK_SNOW);
}

bool BlockBasic::spawnBlockItem(const int32_t x, const int8_t y, const int32_t z, int map, const uint8_t block, const uint8_t meta)
{
  DropPtr drop;
  int16_t item; uint8_t count, item_meta = meta;
  BLOCKDROPS[block]->getDrop(item, count, item_meta);
  if (count)
  {
    Mineserver::get()->map(map)->createPickupSpawn(x, y, z, item, count, item_meta, NULL);
    return true;
  }
  return false;
}

void BlockBasic::notifyNeighbours(const int32_t x, const int8_t y, const int32_t z, const int map, const std::string callback, User* user, const uint8_t oldblock, const int8_t ignore_direction)
{
  uint8_t block;
  uint8_t meta;

  //why is this commented out?

  /*  Function::invoker_type inv(user, oldblock, x, y, z, 0);

    if (ignore_direction != BLOCK_SOUTH && Mineserver::get()->map(map)->getBlock(x+1, y, z, &block, &meta) && block != BLOCK_AIR)
    {
      inv = Function::invoker_type(user, oldblock, x+1, y, z, BLOCK_SOUTH);
      Mineserver::get()->plugin()->runBlockCallback(block, callback, inv);
    }

    if (ignore_direction != BLOCK_NORTH && Mineserver::get()->map(map)->getBlock(x-1, y, z, &block, &meta) && block != BLOCK_AIR)
    {
      inv = Function::invoker_type(user, oldblock, x-1, y, z, BLOCK_NORTH);
      Mineserver::get()->plugin()->runBlockCallback(block, callback, inv);
    }

    if (y < 127 && ignore_direction != BLOCK_TOP && Mineserver::get()->map(map)->getBlock(x, y+1, z, &block, &meta) && block != BLOCK_AIR)
    {
      inv = Function::invoker_type(user, oldblock, x, y+1, z, BLOCK_TOP);
      Mineserver::get()->plugin()->runBlockCallback(block, callback, inv);
    }

    if (y > 0 && ignore_direction != BLOCK_BOTTOM && Mineserver::get()->map(map)->getBlock(x, y-1, z, &block, &meta) && block != BLOCK_AIR)
    {
      inv = Function::invoker_type(user, oldblock, x, y-1, z, BLOCK_BOTTOM);
      Mineserver::get()->plugin()->runBlockCallback(block, callback, inv);
    }

    if (ignore_direction != BLOCK_WEST && Mineserver::get()->map(map)->getBlock(x, y, z+1, &block, &meta) && block != BLOCK_AIR)
    {
      inv = Function::invoker_type(user, oldblock, x, y, z+1, BLOCK_WEST);
      Mineserver::get()->plugin()->runBlockCallback(block, callback, inv);
    }

    if (ignore_direction != BLOCK_EAST && Mineserver::get()->map(map)->getBlock(x, y, z-1, &block, &meta) && block != BLOCK_AIR)
    {
      inv = Function::invoker_type(user, oldblock, x, y, z-1, BLOCK_EAST);
      Mineserver::get()->plugin()->runBlockCallback(block, callback, inv);
    }*/
}

void BlockBasic::onStartedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
}

void BlockBasic::onDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map,  int8_t direction)
{
}

void BlockBasic::onStoppedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map,  int8_t direction)
{
}

bool BlockBasic::onBroken(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map,  int8_t direction)
{
  //Clear block on destroy
  uint8_t block, meta;
  Mineserver::get()->map(map)->getBlock(x, y, z, &block, &meta);
  Mineserver::get()->map(map)->sendBlockChange(x, y, z, BLOCK_AIR, 0);
  Mineserver::get()->map(map)->setBlock(x, y, z, BLOCK_AIR, 0);
  spawnBlockItem(x, y, z, map, block, meta);
  return false;
}

void BlockBasic::onNeighbourBroken(User* user, int16_t oldblock, int32_t x, int8_t y, int32_t z, int map,  int8_t direction)
{
}

bool BlockBasic::onPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map,  int8_t direction)
{
  return false;
}

void BlockBasic::onNeighbourPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map,  int8_t direction)
{
}

void BlockBasic::onReplace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map,  int8_t direction)
{
}

void BlockBasic::onNeighbourMove(User* user, int16_t oldblock, int32_t x, int8_t y, int32_t z, int8_t direction)
{
}

void BlockBasic::onNeighbourMove(User* user, int16_t oldblock, int32_t x, int8_t y, int32_t z, int8_t direction, int map)
{
}

bool BlockBasic::onInteract(User* user, int32_t x, int8_t y, int32_t z, int map)
{
  return false;
}

void BlockBasic::revertBlock(User* user, int32_t x, int8_t y, int32_t z, int map)
{
  unsigned char block, meta;
  Mineserver::get()->map(map)->getBlock((int)x, (int)y, (int)z, &block, &meta);
  user->buffer << (int8_t)PACKET_BLOCK_CHANGE << (int32_t)x << (int8_t)y << (int32_t)z << (int8_t)block << (int8_t)meta;
}
