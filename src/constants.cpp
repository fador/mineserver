/*
   Copyright (c) 2012, The Mineserver Project
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

#include <map>

#include "constants.h"
#include "random.h"

std::map<uint8_t, DropPtr> BLOCKDROPS;

const unsigned int SERVER_CONSOLE_UID = -1;

const std::string VERSION = "0.2.0 (Alpha)";

const int PROTOCOL_VERSION = 47;
const std::string MINECRAFT_VERSION = "1.8.9";

const char COMMENTPREFIX  = '#';

// Configuration
const std::string CONFIG_FILE = "config.cfg";

// PID file
const std::string PID_FILE = "mineserver.pid";

//allocate 1 MB for chunk files
const int ALLOCATE_NBTFILE = 1048576;

const int kMaxChatMessageLength = 119;

void initConstants()
{
  // Block drops (10000 = 100%)

  // Blocks that always drop one item
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_STONE, DropPtr(new Drop(BLOCK_COBBLESTONE, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_GRASS, DropPtr(new Drop(BLOCK_DIRT, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_DIRT, DropPtr(new Drop(BLOCK_DIRT, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_COBBLESTONE, DropPtr(new Drop(BLOCK_COBBLESTONE, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_PLANK, DropPtr(new Drop(BLOCK_PLANK, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_SAPLING, DropPtr(new Drop(BLOCK_SAPLING, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_SAND, DropPtr(new Drop(BLOCK_SAND, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_COAL_ORE, DropPtr(new Drop(ITEM_COAL, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_IRON_ORE, DropPtr(new Drop(BLOCK_IRON_ORE, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_GOLD_ORE, DropPtr(new Drop(BLOCK_GOLD_ORE, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_DIAMOND_ORE, DropPtr(new Drop(ITEM_DIAMOND, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_WOOD, DropPtr(new Drop(BLOCK_WOOD, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_WOOL, DropPtr(new Drop(BLOCK_WOOL, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_WOODEN_STAIRS, DropPtr(new Drop(BLOCK_PLANK, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_RED_ROSE, DropPtr(new Drop(BLOCK_RED_ROSE, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_YELLOW_FLOWER, DropPtr(new Drop(BLOCK_YELLOW_FLOWER, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_BROWN_MUSHROOM, DropPtr(new Drop(BLOCK_BROWN_MUSHROOM, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_RED_MUSHROOM, DropPtr(new Drop(BLOCK_RED_MUSHROOM, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_LADDER, DropPtr(new Drop(BLOCK_LADDER, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_CACTUS, DropPtr(new Drop(BLOCK_CACTUS, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_REED, DropPtr(new Drop(ITEM_REED, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_PUMPKIN, DropPtr(new Drop(BLOCK_PUMPKIN, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_TORCH, DropPtr(new Drop(BLOCK_TORCH, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_REDSTONE_TORCH_OFF, DropPtr(new Drop(BLOCK_REDSTONE_TORCH_ON, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_REDSTONE_TORCH_ON, DropPtr(new Drop(BLOCK_REDSTONE_TORCH_ON, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_REDSTONE_WIRE, DropPtr(new Drop(BLOCK_REDSTONE_WIRE, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_LEVER, DropPtr(new Drop(BLOCK_LEVER, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_GLOWSTONE, DropPtr(new Drop(ITEM_GLOWSTONE_DUST, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_BRICK, DropPtr(new Drop(ITEM_CLAY_BRICK, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_JUKEBOX, DropPtr(new Drop(BLOCK_JUKEBOX, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_NETHERSTONE, DropPtr(new Drop(BLOCK_NETHERSTONE, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_SLOW_SAND, DropPtr(new Drop(BLOCK_SLOW_SAND, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_JACK_O_LANTERN, DropPtr(new Drop(BLOCK_JACK_O_LANTERN, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_MINECART_TRACKS, DropPtr(new Drop(BLOCK_MINECART_TRACKS, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_MOSSY_COBBLESTONE, DropPtr(new Drop(BLOCK_MOSSY_COBBLESTONE, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_OBSIDIAN, DropPtr(new Drop(BLOCK_OBSIDIAN, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_STEP, DropPtr(new Drop(BLOCK_STEP, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_WOODEN_DOOR, DropPtr(new Drop(ITEM_WOODEN_DOOR, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_IRON_DOOR, DropPtr(new Drop(ITEM_IRON_DOOR, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_SNOW, DropPtr(new Drop(ITEM_SNOWBALL, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_IRON_BLOCK, DropPtr(new Drop(BLOCK_IRON_BLOCK, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_GOLD_BLOCK, DropPtr(new Drop(BLOCK_GOLD_BLOCK, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_DIAMOND_BLOCK, DropPtr(new Drop(BLOCK_DIAMOND_BLOCK, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_LAPIS_BLOCK, DropPtr(new Drop(BLOCK_LAPIS_BLOCK, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_BED, DropPtr(new Drop(ITEM_BED, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_FURNACE, DropPtr(new Drop(BLOCK_FURNACE, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_BURNING_FURNACE, DropPtr(new Drop(BLOCK_FURNACE, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_CHEST, DropPtr(new Drop(BLOCK_CHEST, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_WORKBENCH, DropPtr(new Drop(BLOCK_WORKBENCH, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_SIGN_POST, DropPtr(new Drop(ITEM_SIGN, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_WALL_SIGN, DropPtr(new Drop(ITEM_SIGN, 10000, 1))));
  

  // Always drop but give more than one item
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_REDSTONE_ORE, DropPtr(new Drop(ITEM_REDSTONE, 10000, 4))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_CLAY, DropPtr(new Drop(ITEM_CLAY_BALLS, 10000, 4))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_SNOW_BLOCK, DropPtr(new Drop(ITEM_SNOWBALL, 10000, 4))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_DOUBLE_STEP, DropPtr(new Drop(BLOCK_STEP, 10000, 2))));

  // Blocks that drop items by chance
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_GRAVEL, DropPtr(new Drop(ITEM_FLINT, 850, 1, 0, DropPtr(new Drop(BLOCK_GRAVEL, 10000, 1))))));
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_LEAVES, DropPtr(new Drop(BLOCK_SAPLING, 1200, 1))));

  // Blocks that drop nothing:
  // BLOCK_TNT, BLOCK_GLASS, BLOCK_MOB_SPAWNER

  // Blocks that drop items with metadata
  BLOCKDROPS.insert(std::pair<uint8_t, DropPtr>(BLOCK_LAPIS_ORE, DropPtr(new Drop(ITEM_DYE, 10000, 1, DYE_LAPIS_LAZULI))));
}

void Drop::getDrop(int16_t& item, uint8_t& count, uint8_t& meta)
{
  Drop *cur = this;
  while (cur)
  {
    if (cur->probability >= uniformUINT(0, 9999))
    {
      item = cur->item_id;
      count = cur->count;
      if (cur->meta != -1) meta = (uint8_t)cur->meta;
      return;
    }
    else
    {
      cur = cur->alt_drop.get();
    }
  }
  count = 0;
}
