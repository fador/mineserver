#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>

#include "logger.h"
#include "constants.h"

#include "config.h"

Conf &Conf::get()
{
  static Conf instance;
  return instance;
}

// Load/reload configuration
bool Conf::load(std::string configFile)
{
  std::cout << "Loading configuration from " << configFile << std::endl;
  std::ifstream ifs(configFile.c_str());

  // If file does not exist
  if(ifs.fail())
  {
    std::cout << ">>> " << configFile << " not found. Creating using default values..." << std::endl;

    std::ofstream confofs(configFile.c_str());
    confofs << "#"                                                                          << std::endl
            << "# Default Mineserver configuration"                                         << std::endl
            << "#"                                                                          << std::endl
                                                                                            << std::endl
            << "# Server name"                                                              << std::endl
            << "servername = \"Mineserver alpha testserver\""                               << std::endl
                                                                                            << std::endl
            << "# Userlimit"                                                                << std::endl
            << "userlimit = 50"                                                             << std::endl
                                                                                            << std::endl
            << "# Port"                                                                     << std::endl
            << "port = 25565"                                                               << std::endl
                                                                                            << std::endl
            << "# Map Release time - time in seconds to keep unused map chunks in memory"   << std::endl
            << "# Memory vs. CPU tradeoff. Reloading map data takes a bit of CPU each time" << std::endl
            << "# but the map in memory consumes it around 100kb/chunk"                     << std::endl
            << "map_release_time = 10 << std::endl"                                         << std::endl;

    confofs.close();

    this->load(CONFIGFILE);
  }

  std::string temp;

  // Clear config (to allow configuration reload)
  confSet.clear();

  // Reading row at a time
  int del;
  int lineNum = 0;
  std::vector<std::string> line;
  std::string text;
  while(getline(ifs, temp))
  {
    // Count line numbers
    lineNum++;

    // If not enough characters (Absolute min is 5: "a = s")
    if(temp.length() < 5) continue;

    // If commentline -> skip to next
    if(temp[0] == COMMENTPREFIX) continue;

    // Init vars
    del = 0;
    line.clear();

    // Process line
    while(temp.length() > 0)
    {
      // Remove white spaces and = characters -_-
      while(temp[0] == ' ' || temp[0] == '=') temp = temp.substr(1);

      // Split words
      del = temp.find(' ');
      if(del > -1)
      {
        line.push_back(temp.substr(0,del));
        temp = temp.substr(del+1);
      } else {
        line.push_back(temp);
        break;
      }
    }

    // If under two words skip the line and log skipping.
    if(line.size() < 2)
    {
      std::cout << "Invalid configuration at line " << lineNum << " of " << configFile;
      break;
    }

    // Construct strings if needed
    text = "";
    if(line[1][0] == '"')
    {
      // Append to text
      for(unsigned int i = 1; i < line.size(); i++)
      {
        text += line[i] + " ";
      }
      // Remove ""
      text = text.substr(1, text.length()-3);
    } else
    {
      text = line[1];
    }

    // TODO: Validate configline

    // Push to configuration
    confSet.insert(std::pair<std::string, std::string>(line[0], text));

    // DEBUG
    std::cout << "> " << line[0] << " = " << text << std::endl;
  }
  ifs.close();

  std::cout << "Configuration loaded!" << std::endl;

  return true;
}

// Return value
std::string Conf::value(std::string name)
{
  if(confSet.find(name) != confSet.end())
  {
    return confSet[name];
  }
  else
  {
    return "Not found!";
  }
}
