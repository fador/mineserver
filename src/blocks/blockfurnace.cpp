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
#include "tools.h"

#include "blockfurnace.h"

bool BlockFurnace::onPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
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

  direction = user->relativeToBlock(x, y, z);

  // Fix orientation

  //switch (direction)
  //{
  //case BLOCK_EAST:
  //  direction = BLOCK_SOUTH;
  //  break;
  //case BLOCK_BOTTOM:
  //  direction = BLOCK_EAST;
  //  break;
  //case BLOCK_NORTH:
  //  direction = BLOCK_NORTH;
  //  break;
  //case BLOCK_SOUTH:
  //  direction = BLOCK_BOTTOM;
  //  break;
  //}

  ServerInstance->map(map)->setBlock(x, y, z, (char)newblock, direction);
  ServerInstance->map(map)->sendBlockChange(x, y, z, (char)newblock, direction);


  int chunk_x = blockToChunk(x);
  int chunk_z = blockToChunk(z);
  sChunk* chunk = ServerInstance->map(map)->loadMap(chunk_x, chunk_z);

  if (chunk == NULL)
  {
    revertBlock(user, x, y, z, map);
  }
  return true;

  for (uint32_t i = 0; i < chunk->furnaces.size(); i++)
  {
    if (chunk->furnaces[i]->x == x &&
        chunk->furnaces[i]->y == y &&
        chunk->furnaces[i]->z == z)
    {
      chunk->furnaces.erase(chunk->furnaces.begin() + i);
      break;
    }
  }

  return false;
}

bool BlockFurnace::onInteract(User* user, int32_t x, int8_t y, int32_t z, int map)
{
  ServerInstance->inventory()->windowOpen(user, WINDOW_FURNACE, x, y, z);
  return true;
}


bool BlockFurnace::onBroken(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map,  int8_t direction)
{
  uint8_t block;
  uint8_t meta;

  if (!ServerInstance->map(map)->getBlock(x, y, z, &block, &meta))
  {
    return true;
  }

  bool destroy = false;

  int chunk_x = blockToChunk(x);
  int chunk_z = blockToChunk(z);

  sChunk* chunk = ServerInstance->map(map)->loadMap(chunk_x, chunk_z);

  if (chunk == NULL)
  {
    return true;
  }

  for (uint32_t i = 0; i < chunk->furnaces.size(); i++)
  {
    if (chunk->furnaces[i]->x == x &&
        chunk->furnaces[i]->y == y &&
        chunk->furnaces[i]->z == z)
    {
      chunk->furnaces.erase(chunk->furnaces.begin() + i);
      break;
    }
  }

  ServerInstance->map(map)->sendBlockChange(x, y, z, BLOCK_AIR, 0);
  ServerInstance->map(map)->setBlock(x, y, z, BLOCK_AIR, 0);
  this->spawnBlockItem(x, y, z, map, block);
  return false;
}

