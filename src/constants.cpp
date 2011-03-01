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

#include <map>

#include "constants.h"

std::map<uint8_t, Drop*> BLOCKDROPS;

void initConstants()
{
  // Block drops (10000 = 100%)

  // Blocks that always drop one item
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_STONE, new Drop(BLOCK_COBBLESTONE, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_GRASS, new Drop(BLOCK_DIRT, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_DIRT, new Drop(BLOCK_DIRT, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_COBBLESTONE, new Drop(BLOCK_COBBLESTONE, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_PLANK, new Drop(BLOCK_PLANK, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_SAPLING, new Drop(BLOCK_SAPLING, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_SAND, new Drop(BLOCK_SAND, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_COAL_ORE, new Drop(ITEM_COAL, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_IRON_ORE, new Drop(BLOCK_IRON_ORE, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_GOLD_ORE, new Drop(BLOCK_GOLD_ORE, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_DIAMOND_ORE, new Drop(ITEM_DIAMOND, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_WOOD, new Drop(BLOCK_WOOD, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_GRAY_CLOTH, new Drop(BLOCK_GRAY_CLOTH, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_WOODEN_STAIRS, new Drop(BLOCK_PLANK, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_RED_ROSE, new Drop(BLOCK_RED_ROSE, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_YELLOW_FLOWER, new Drop(BLOCK_YELLOW_FLOWER, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_BROWN_MUSHROOM, new Drop(BLOCK_BROWN_MUSHROOM, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_RED_MUSHROOM, new Drop(BLOCK_RED_MUSHROOM, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_LADDER, new Drop(BLOCK_LADDER, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_CACTUS, new Drop(BLOCK_CACTUS, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_REED, new Drop(ITEM_REED, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_PUMPKIN, new Drop(BLOCK_PUMPKIN, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_TORCH, new Drop(BLOCK_TORCH, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_REDSTONE_TORCH_OFF, new Drop(BLOCK_REDSTONE_TORCH_OFF, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_REDSTONE_TORCH_ON, new Drop(BLOCK_REDSTONE_TORCH_ON, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_GLOWSTONE, new Drop(ITEM_GLOWSTONE_DUST, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_BRICK, new Drop(ITEM_CLAY_BRICK, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_JUKEBOX, new Drop(BLOCK_JUKEBOX, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_NETHERSTONE, new Drop(BLOCK_NETHERSTONE, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_SLOW_SAND, new Drop(BLOCK_SLOW_SAND, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_JACK_O_LANTERN, new Drop(BLOCK_JACK_O_LANTERN, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_MINECART_TRACKS, new Drop(BLOCK_MINECART_TRACKS, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_MOSSY_COBBLESTONE, new Drop(BLOCK_MOSSY_COBBLESTONE, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_OBSIDIAN, new Drop(BLOCK_OBSIDIAN, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_STEP, new Drop(BLOCK_STEP, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_DOUBLE_STEP, new Drop(BLOCK_STEP, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_WOODEN_DOOR, new Drop(ITEM_WOODEN_DOOR, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_IRON_DOOR, new Drop(ITEM_IRON_DOOR, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_SNOW, new Drop(ITEM_SNOWBALL, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_IRON_BLOCK, new Drop(BLOCK_IRON_BLOCK, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_GOLD_BLOCK, new Drop(BLOCK_GOLD_BLOCK, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_DIAMOND_BLOCK, new Drop(BLOCK_DIAMOND_BLOCK, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_LAPIS_BLOCK, new Drop(BLOCK_LAPIS_BLOCK, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_LAPIS_ORE, new Drop(BLOCK_LAPIS_ORE, 10000, 1)));


  // Always drop but give more than one item
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_REDSTONE_ORE, new Drop(ITEM_REDSTONE, 10000, 4)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_CLAY, new Drop(ITEM_CLAY_BALLS, 10000, 4)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_SNOW_BLOCK, new Drop(ITEM_SNOWBALL, 10000, 4)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_FURNACE, new Drop(BLOCK_FURNACE, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_BURNING_FURNACE, new Drop(BLOCK_FURNACE, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_CHEST, new Drop(BLOCK_CHEST, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_WORKBENCH, new Drop(BLOCK_WORKBENCH, 10000, 1)));

  // Blocks that drop items by chance
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_GRAVEL, new Drop(ITEM_FLINT, 850, 1, new Drop(BLOCK_GRAVEL, 10000, 1))));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_LEAVES, new Drop(BLOCK_SAPLING, 1200, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_SIGN_POST, new Drop(ITEM_SIGN, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8_t, Drop*>(BLOCK_WALL_SIGN, new Drop(ITEM_SIGN, 10000, 1)));

  // Blocks that drop nothing:
  // BLOCK_TNT, BLOCK_GLASS, BLOCK_MOB_SPAWNER
}

void freeConstants()
{
  std::map<uint8_t, Drop*>::iterator it_a = BLOCKDROPS.begin();
  std::map<uint8_t, Drop*>::iterator it_b = BLOCKDROPS.end();
  for (; it_a != it_b; ++it_a)
  {
    delete it_a->second;
  }
}
