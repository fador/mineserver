#include <string>
#include <map>

#include "constants.h"


std::map<std::string, std::string> defaultConf;

void initDefaultConf()
{
  defaultConf.insert(std::pair<std::string, std::string>("port", "25565"));
  defaultConf.insert(std::pair<std::string, std::string>("servername", "Minecraft server"));
  defaultConf.insert(std::pair<std::string, std::string>("mapdir", "testmap"));
  defaultConf.insert(std::pair<std::string, std::string>("userlimit", "20"));
  defaultConf.insert(std::pair<std::string, std::string>("map_release_time", "10"));
}