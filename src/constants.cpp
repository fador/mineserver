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

  // Block drops
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_STONE, {BLOCK_COBBLESTONE, 1000, 1});
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_GRASS, {BLOCK_DIRT, 1000, 1});
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_GRAVEL, {ITEM_FLINT, 85, 1});
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_COAL_ORE, {ITEM_COAL, 1000, 1});
  BLOCKDROPS.insert(std::pair<uint8, Drop>(BLOCK_LEAVES, {BLOCK_SAPLING, 125, 1});
}