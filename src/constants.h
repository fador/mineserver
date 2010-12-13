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

#ifndef _CONSTANTS_H
#define _CONSTANTS_H

#include <map>
#include <string>
#include "tools.h"

//
// Mineserver constants
//

//Player digging status
enum
{
  BLOCK_STATUS_STARTED_DIGGING,
  BLOCK_STATUS_DIGGING,
  BLOCK_STATUS_STOPPED_DIGGING,
  BLOCK_STATUS_BLOCK_BROKEN
};

// Chat colors
#define MC_COLOR_BLACK std::string("§0")
#define MC_COLOR_DARK_BLUE std::string("§1")
#define MC_COLOR_DARK_GREEN std::string("§2")
#define MC_COLOR_DARK_CYAN std::string("§3")
#define MC_COLOR_DARK_RED std::string("§4")
#define MC_COLOR_DARK_MAGENTA std::string("§5")
#define MC_COLOR_DARK_ORANGE std::string("§6")
#define MC_COLOR_GREY std::string("§7")
#define MC_COLOR_DARK_GREY std::string("§8")
#define MC_COLOR_BLUE std::string("§9")
#define MC_COLOR_GREEN std::string("§a")
#define MC_COLOR_CYAN std::string("§b")
#define MC_COLOR_RED std::string("§c")
#define MC_COLOR_MAGENTA std::string("§d")
#define MC_COLOR_YELLOW std::string("§e")
#define MC_COLOR_WHITE std::string("§f")

// Direction
enum Direction
{
   BLOCK_BOTTOM, BLOCK_NORTH, BLOCK_SOUTH, BLOCK_EAST, BLOCK_WEST, BLOCK_TOP
};

// Blocks
enum Block
{
  BLOCK_AIR, BLOCK_STONE, BLOCK_GRASS, BLOCK_DIRT, BLOCK_COBBLESTONE, BLOCK_WOOD,
  BLOCK_SAPLING, BLOCK_BEDROCK, BLOCK_WATER, BLOCK_STATIONARY_WATER, BLOCK_LAVA,
  BLOCK_STATIONARY_LAVA, BLOCK_SAND, BLOCK_GRAVEL, BLOCK_GOLD_ORE, BLOCK_IRON_ORE,
  BLOCK_COAL_ORE, BLOCK_LOG, BLOCK_LEAVES, BLOCK_SPONGE, BLOCK_GLASS, BLOCK_RED_CLOTH,
  BLOCK_ORANGE_CLOTH, BLOCK_YELLOW_CLOTH, BLOCK_LIME_CLOTH, BLOCK_GREEN_CLOTH,
  BLOCK_AQUA_GREEN_CLOTH, BLOCK_CYAN_CLOTH, BLOCK_BLUE_CLOTH, BLOCK_PURPLE_CLOTH,
  BLOCK_INDIGO_CLOTH, BLOCK_VIOLET_CLOTH, BLOCK_MAGENTA_CLOTH, BLOCK_PINK_CLOTH,
  BLOCK_BLACK_CLOTH, BLOCK_GRAY_CLOTH, BLOCK_WHITE_CLOTH, BLOCK_YELLOW_FLOWER,
  BLOCK_RED_ROSE, BLOCK_BROWN_MUSHROOM, BLOCK_RED_MUSHROOM, BLOCK_GOLD_BLOCK,
  BLOCK_IRON_BLOCK, BLOCK_DOUBLE_STEP, BLOCK_STEP, BLOCK_BRICK, BLOCK_TNT,
  BLOCK_BOOKSHELF, BLOCK_MOSSY_COBBLESTONE, BLOCK_OBSIDIAN, BLOCK_TORCH, BLOCK_FIRE,
  BLOCK_MOB_SPAWNER, BLOCK_WOODEN_STAIRS, BLOCK_CHEST, BLOCK_REDSTONE_WIRE,
  BLOCK_DIAMOND_ORE, BLOCK_DIAMOND_BLOCK, BLOCK_WORKBENCH, BLOCK_CROPS, BLOCK_SOIL,
  BLOCK_FURNACE, BLOCK_BURNING_FURNACE, BLOCK_SIGN_POST, BLOCK_WOODEN_DOOR,
  BLOCK_LADDER, BLOCK_MINECART_TRACKS, BLOCK_COBBLESTONE_STAIRS, BLOCK_WALL_SIGN,
  BLOCK_LEVER, BLOCK_STONE_PRESSURE_PLATE, BLOCK_IRON_DOOR, BLOCK_WOODEN_PRESSURE_PLATE,
  BLOCK_REDSTONE_ORE, BLOCK_GLOWING_REDSTONE_ORE, BLOCK_REDSTONE_TORCH_OFF,
  BLOCK_REDSTONE_TORCH_ON, BLOCK_STONE_BUTTON, BLOCK_SNOW, BLOCK_ICE, BLOCK_SNOW_BLOCK,
  BLOCK_CACTUS, BLOCK_CLAY, BLOCK_REED, BLOCK_JUKEBOX, BLOCK_FENCE, BLOCK_PUMPKIN,
  BLOCK_NETHERSTONE, BLOCK_SLOW_SAND, BLOCK_LIGHTSTONE, BLOCK_PORTAL, BLOCK_JACK_O_LANTERN
};

// Items
enum
{
  ITEM_IRON_SPADE = 256, ITEM_IRON_PICKAXE, ITEM_IRON_AXE, ITEM_FLINT_AND_STEEL, ITEM_APPLE,
  ITEM_BOW, ITEM_ARROW, ITEM_COAL, ITEM_DIAMOND, ITEM_IRON_INGOT, ITEM_GOLD_INGOT, ITEM_IRON_SWORD,
  ITEM_WOODEN_SWORD, ITEM_WOODEN_SPADE, ITEM_WOODEN_PICKAXE, ITEM_WOODEN_AXE, ITEM_STONE_SWORD,
  ITEM_STONE_SPADE, ITEM_STONE_PICKAXE, ITEM_STONE_AXE, ITEM_DIAMOND_SWORD,
  ITEM_DIAMOND_SPADE, ITEM_DIAMOND_PICKAXE, ITEM_DIAMOND_AXE, ITEM_STICK, ITEM_BOWL,
  ITEM_MUSHROOM_SOUP, ITEM_GOLD_SWORD, ITEM_GOLD_SPADE, ITEM_GOLD_PICKAXE, ITEM_GOLD_AXE,
  ITEM_STRING, ITEM_FEATHER, ITEM_GUNPOWDER, ITEM_WOODEN_HOE, ITEM_STONE_HOE,
  ITEM_IRON_HOE, ITEM_DIAMOND_HOE, ITEM_GOLD_HOE, ITEM_SEEDS, ITEM_WHEAT, ITEM_BREAD,
  ITEM_LEATHER_HELMET, ITEM_LEATHER_CHESTPLATE, ITEM_LEATHER_LEGGINGS, ITEM_LEATHER_BOOTS,
  ITEM_CHAINMAIL_HELMET, ITEM_CHAINMAIL_CHESTPLATE, ITEM_CHAINMAIL_LEGGINGS,
  ITEM_CHAINMAIL_BOOTS, ITEM_IRON_HELMET, ITEM_IRON_CHESTPLATE, ITEM_IRON_LEGGINGS,
  ITEM_IRON_BOOTS, ITEM_DIAMOND_HELMET, ITEM_DIAMOND_CHESTPLATE, ITEM_DIAMOND_LEGGINGS,
  ITEM_DIAMOND_BOOTS, ITEM_GOLD_HELMET, ITEM_GOLD_CHESTPLATE, ITEM_GOLD_LEGGINS,
  ITEM_GOLD_BOOTS, ITEM_FLINT, ITEM_PORK, ITEM_GRILLED_PORK, ITEM_PAINTINGS,
  ITEM_GOLDEN_APPLE, ITEM_SIGN, ITEM_WOODEN_DOOR, ITEM_BUCKET, ITEM_WATER_BUCKET,
  ITEM_LAVA_BUCKET, ITEM_MINECART, ITEM_SADDLE, ITEM_IRON_DOOR, ITEM_REDSTONE,
  ITEM_SNOWBALL, ITEM_BOAT, ITEM_LEATHER, ITEM_MILK_BUCKET, ITEM_CLAY_BRICK,
  ITEM_CLAY_BALLS, ITEM_REED, ITEM_PAPER, ITEM_BOOK, ITEM_SLIME_BALL,
  ITEM_STORAGE_MINECART, ITEM_POWERED_MINECART, ITEM_EGG, ITEM_COMPASS, ITEM_FISHING_ROD,
  ITEM_WATCH, ITEM_LIGHTSTONE_DUST, ITEM_RAW_FISH, ITEM_COOKED_FISH
};

// Records
enum
{
  ITEM_GOLD_RECORD = 2256, ITEM_GREEN_RECORD
};

const std::string VERSION = "0.1.14 (Alpha)";

const int PROTOCOL_VERSION = 6;

const char COMMENTPREFIX  = '#';

// Configuration
const std::string CONFIG_FILE = "config.cfg";

// Commands file
const std::string COMMANDS_FILE    = "commands.cfg";
const std::string COMMANDS_NAME_PREFIX = "cmd_";
const std::string COMMANDS_CONTENT = "# This is default command permissions file. Add minimum permission rank after each command.";

// Roles file
const std::string ROLES_FILE    = "roles.txt";
const std::string ROLES_CONTENT = "# This is default player roles file. Add nicknames here for admins, operators or members, one per line";

// Banned users file
const std::string BANNED_FILE    = "banned.txt";
const std::string BANNED_CONTENT = "# This is default banned users file. Add nicknames here, one per line.";

// Whitelist users file
const std::string WHITELIST_FILE    = "whitelist.txt";
const std::string WHITELIST_CONTENT = "# This is default whitelist file. Add nicknames here, one per line.";

// Motdfile
const std::string MOTD_FILE    = "motd.txt";
const std::string MOTD_CONTENT = "This is the default motd! Please edit motd.txt to change this message.";

// Server rules
const std::string RULES_FILE    = "rules.txt";
const std::string RULES_CONTENT = "This is the default rules file. Please edit rules.txt to change this message.";

// PID file
const std::string PID_FILE = "mineserver.pid";

// Default configuration
extern std::map<std::string, std::string> defaultConf;

//
// Blocks that drop special things!
//
struct Drop
{
  uint16 item_id;
  sint32 probability;
  uint8 count;
  bool exclusive;

  Drop() {}

  Drop(uint16 item_id, uint32 probability, uint8 count, bool exclusive)
  {
    this->item_id     = item_id;
    this->probability = probability;
    this->count       = count;
    this->exclusive   = exclusive;
  }
};

extern std::map<uint8, Drop> BLOCKDROPS;

// Chat prefixes
const char SERVERMSGPREFIX = '%';
const char CHATCMDPREFIX   = '/';
const char ADMINCHATPREFIX = '&';

const int SERVER_CONSOLE_UID = -1;

void initConstants();

const int ALLOCATE_NBTFILE = 200000;

#endif
