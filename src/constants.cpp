/*
   Copyright (c) 2010, The Mineserver Project
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

#include <string>
#include <map>

#include "constants.h"

std::map<std::string, std::string> defaultConf;
std::map<uint8, Drop> BLOCKDROPS;

void initConstants()
{
  // Init configuration
  defaultConf.insert(std::pair<std::string, std::string>("port", "25565"));
  defaultConf.insert(std::pair<std::string, std::string>("servername", "Minecraft server"));
  defaultConf.insert(std::pair<std::string, std::string>("mapdir", "testmap"));
  defaultConf.insert(std::pair<std::string, std::string>("userlimit", "20"));
  defaultConf.insert(std::pair<std::string, std::string>("map_release_time", "10"));
  defaultConf.insert(std::pair<std::string, std::string>("liquid_physics", "1"));
  defaultConf.insert(std::pair<std::string, std::string>("map_flatland", "false"));
  defaultConf.insert(std::pair<std::string, std::string>("oreDensity", "24"));
  defaultConf.insert(std::pair<std::string, std::string>("seaLevel", "63"));
  defaultConf.insert(std::pair<std::string, std::string>("server_full_message",
                                                         "Server is currently full"));
  defaultConf.insert(std::pair<std::string, std::string>("default_kick_message",
                                                         "You have been kicked"));
  defaultConf.insert(std::pair<std::string, std::string>("wrong_protocol_message",
                                                         "Wrong client protocol"));

  // Block drops (10000 = 100%)

  // Blocks that always drop one item
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_STONE, Drop(BLOCK_COBBLESTONE, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_GRASS, Drop(BLOCK_DIRT, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_DIRT, Drop(BLOCK_DIRT, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_COBBLESTONE, Drop(BLOCK_COBBLESTONE, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_WOOD, Drop(BLOCK_WOOD, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_SAPLING, Drop(BLOCK_SAPLING, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_SAND, Drop(BLOCK_SAND, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_COAL_ORE, Drop(ITEM_COAL, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_IRON_ORE, Drop(ITEM_IRON_INGOT, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_GOLD_ORE, Drop(ITEM_GOLD_INGOT, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_DIAMOND_ORE, Drop(ITEM_DIAMOND, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_LOG, Drop(BLOCK_LOG, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_WOODEN_STAIRS, Drop(BLOCK_WOOD, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_RED_ROSE, Drop(BLOCK_RED_ROSE, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_YELLOW_FLOWER, Drop(BLOCK_RED_ROSE, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_BROWN_MUSHROOM, Drop(BLOCK_BROWN_MUSHROOM, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_RED_MUSHROOM, Drop(BLOCK_RED_MUSHROOM, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_LADDER, Drop(BLOCK_LADDER, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_CACTUS, Drop(BLOCK_CACTUS, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_REED, Drop(BLOCK_REED, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_PUMPKIN, Drop(BLOCK_PUMPKIN, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_TORCH, Drop(BLOCK_TORCH, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_REDSTONE_TORCH_OFF, Drop(BLOCK_REDSTONE_TORCH_OFF, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_REDSTONE_TORCH_ON, Drop(BLOCK_REDSTONE_TORCH_ON, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_LIGHTSTONE, Drop(ITEM_LIGHTSTONE_DUST, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_BRICK, Drop(ITEM_CLAY_BRICK, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_JUKEBOX, Drop(BLOCK_JUKEBOX, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_NETHERSTONE, Drop(BLOCK_NETHERSTONE, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_SLOW_SAND, Drop(BLOCK_SLOW_SAND, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_JACK_O_LANTERN, Drop(BLOCK_JACK_O_LANTERN, 10000, 1, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_MINECART_TRACKS, Drop(BLOCK_MINECART_TRACKS, 10000, 1, true)));

  // Always drop but give more then one item
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_REDSTONE_ORE, Drop(ITEM_REDSTONE, 10000, 4, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_CLAY, Drop(ITEM_CLAY_BALLS, 10000, 4, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_FURNACE, Drop(BLOCK_COBBLESTONE, 10000, 3, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_BURNING_FURNACE, Drop(BLOCK_COBBLESTONE, 10000, 3, true)));

  // Blocks that drop items by chance
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_GRAVEL, Drop(ITEM_FLINT, 850, 1, false)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_LEAVES, Drop(BLOCK_SAPLING, 1200, 1, true)));

  /* BLOCK_SPONGE, BLOCK_GLASS, BLOCK_RED_CLOTH,
  BLOCK_ORANGE_CLOTH, BLOCK_YELLOW_CLOTH, BLOCK_LIME_CLOTH, BLOCK_GREEN_CLOTH,
  BLOCK_AQUA_GREEN_CLOTH, BLOCK_CYAN_CLOTH, BLOCK_BLUE_CLOTH, BLOCK_PURPLE_CLOTH,
  BLOCK_INDIGO_CLOTH, BLOCK_VIOLET_CLOTH, BLOCK_MAGENTA_CLOTH, BLOCK_PINK_CLOTH,
  BLOCK_BLACK_CLOTH, BLOCK_GRAY_CLOTH, BLOCK_WHITE_CLOTH, BLOCK_GOLD_BLOCK,
  BLOCK_IRON_BLOCK, BLOCK_DOUBLE_STEP, BLOCK_STEP, , BLOCK_TNT,
  BLOCK_BOOKSHELF, BLOCK_MOSSY_COBBLESTONE, BLOCK_OBSIDIAN,, BLOCK_FIRE,
  BLOCK_MOB_SPAWNER, BLOCK_CHEST, BLOCK_REDSTONE_WIRE,
  BLOCK_DIAMOND_ORE, BLOCK_DIAMOND_BLOCK, BLOCK_WORKBENCH, BLOCK_CROPS, BLOCK_SOIL,
  BLOCK_SIGN_POST, BLOCK_WOODEN_DOOR,
  , , BLOCK_COBBLESTONE_STAIRS, BLOCK_WALL_SIGN,
  BLOCK_LEVER, BLOCK_STONE_PRESSURE_PLATE, BLOCK_IRON_DOOR, BLOCK_WOODEN_PRESSURE_PLATE,
  , BLOCK_STONE_BUTTON, BLOCK_SNOW, BLOCK_ICE, BLOCK_SNOW_BLOCK,
  , BLOCK_FENCE,
  , , BLOCK_PORTAL, 
*/
  // Blocks that drop nothing
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_TNT, Drop(BLOCK_TNT, 10000, 0, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_GLASS, Drop(BLOCK_GLASS, 10000, 0, true)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_MOB_SPAWNER, Drop(BLOCK_MOB_SPAWNER, 10000, 0, true)));

}
