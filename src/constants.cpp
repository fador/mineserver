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
  defaultConf.insert(std::pair<std::string, std::string>("server_full_message", "Server is currently full"));
  defaultConf.insert(std::pair<std::string, std::string>("default_kick_message", "You have been kicked"));
  defaultConf.insert(std::pair<std::string, std::string>("wrong_protocol_message", "Wrong client protocol"));

  // Block drops (10000 = 100%)
  BLOCKDROPS.insert(std::pair<uint8, Drop>((uint8)BLOCK_STONE, Drop((uint8)BLOCK_COBBLESTONE, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>((uint8)BLOCK_GRASS, Drop((uint8)BLOCK_DIRT, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>((uint8)BLOCK_GRAVEL, Drop((uint8)ITEM_FLINT, 850, 1)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>((uint8)BLOCK_COAL_ORE, Drop((uint8)ITEM_COAL, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>((uint8)BLOCK_LEAVES, Drop((uint8)BLOCK_SAPLING, 1200, 1)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>((uint8)BLOCK_DIAMOND_ORE, Drop((uint8)ITEM_DIAMOND, 10000, 1)));
  BLOCKDROPS.insert(std::pair<uint8, Drop>((uint8)BLOCK_REDSTONE_ORE, Drop((uint8)ITEM_REDSTONE, 10000, 4)));

}