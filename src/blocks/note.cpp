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

#include "note.h"


bool BlockNote::onPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
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

  ServerInstance->map(map)->setBlock(x, y, z, BLOCK_NOTE_BLOCK, 0);
  ServerInstance->map(map)->sendBlockChange(x, y, z, BLOCK_NOTE_BLOCK, 0);
  return false;
}

void BlockNote::onStartedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  uint8_t block, metadata;
  ServerInstance->map(map)->getBlock(x, y, z, &block, &metadata);
  ServerInstance->map(map)->sendNote(x, y, z, BlockNote::getInstrument(x, y - 1, z, map), metadata);
}

bool BlockNote::onInteract(User* user, int32_t x, int8_t y, int32_t z, int map)
{
  uint8_t block, metadata;
  ServerInstance->map(map)->getBlock(x, y, z, &block, &metadata);
  if (metadata == 0x14)
  {
    metadata = 0x00;
    ServerInstance->map(map)->setBlock(x, y, z, block, metadata);
    ServerInstance->map(map)->sendNote(x, y, z, BlockNote::getInstrument(x, y - 1, z, map), metadata);
  }
  else
  {
    metadata++;
    ServerInstance->map(map)->setBlock(x, y, z, block, metadata);
    ServerInstance->map(map)->sendNote(x, y, z, BlockNote::getInstrument(x, y - 1, z, map), metadata);
  }
  return true;
}

int BlockNote::getInstrument(int32_t x, int8_t y, int32_t z, int map)
{
  uint8_t block, meta;
  ServerInstance->map(map)->getBlock(x, y, z, &block, &meta);
  switch (block)
  {
  case BLOCK_WOOD:
  case BLOCK_PLANK:
    return INSTRUMENT_BASS;
  case BLOCK_SAND:
  case BLOCK_GRAVEL:
  case BLOCK_SLOW_SAND:
    return INSTRUMENT_SNARE;
  case BLOCK_GLASS:
  case BLOCK_GLOWSTONE:
    return INSTRUMENT_STICK;
  case BLOCK_STONE:
  case BLOCK_COBBLESTONE:
  case BLOCK_BRICK:
  case BLOCK_OBSIDIAN:
  case BLOCK_NETHERSTONE:
  case BLOCK_IRON_ORE:
  case BLOCK_DIAMOND_ORE:
  case BLOCK_GOLD_ORE:
  case BLOCK_FURNACE:
  case BLOCK_BURNING_FURNACE:
  case BLOCK_DISPENSER:
    return INSTRUMENT_BASSDRUM;
  }
  return INSTRUMENT_HARP;
}
