#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>

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
  std::cout << "Loading data from " << configFile << std::endl;
  std::ifstream ifs(configFile.c_str());

  // If configfile does not exist
  if(ifs.fail() && configFile == CONFIGFILE)
  {
    std::cout << ">>> " << configFile << " not found. Generating default configuration.." << std::endl;

    std::ofstream confofs(configFile.c_str());
    confofs << "#"                                                  << std::endl
            << "# Load configuration example from: <address here>"   << std::endl
            << "#"                                                  << std::endl;
    confofs.close();

    this->load(CONFIGFILE);
  }

  std::string temp;

  // Reading row at a time
  int del;
  int lineNum = 0;
  std::vector<std::string> line;
  std::string text;
  while(getline(ifs, temp))
  {
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
      continue;
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

    // Update existing configuration and add new lines
    if(confSet.find(line[0]) != confSet.end())
    {
      confSet[line[0]] = text;
      //std::cout << "Updated> " << line[0] << " = " << text << std::endl;
    }
    else
    {
      // Push to configuration
      confSet.insert(std::pair<std::string, std::string>(line[0], text));
      //std::cout << "Added> " << line[0] << " = " << text << std::endl;
    }
    
    // Count line numbers
    lineNum++;
  }
  ifs.close();

  std::cout << "Loaded " << lineNum << " lines from " << configFile << std::endl;

  return true;
}

// Return values
std::string Conf::sValue(std::string name)
{
  if(confSet.find(name) != confSet.end())
  { 
    return confSet[name];
  }
  else
  {
    std::cout << "Warning! " << name << " not defined in configuration. Using default value: " << defaultConf[name] << std::endl;
    return defaultConf[name];
  }
}

int Conf::iValue(std::string name)
{
  if(confSet.find(name) != confSet.end())
  {
    return atoi(confSet[name].c_str());
  }
  else
  {
    std::cout << "Warning! " << name << " not defined in configuration. Using default value: " << defaultConf[name] << std::endl;
    return atoi(defaultConf[name].c_str());
  }
}

std::vector<int> Conf::vValue(std::string name)
{
  std::vector<int> temp;
  std::string tmpStr;
  int del;
  if(confSet.find(name) != confSet.end())
  {
    tmpStr = confSet[name];
    
    // Process "array"
    while(tmpStr.length() > 0)
    {
      // Remove white spaces and = characters -_-
      while(tmpStr[0] == ' ') tmpStr = tmpStr.substr(1);

      // Split words
      del = tmpStr.find(',');
      if(del > -1)
      {
        temp.push_back(atoi(tmpStr.substr(0,del).c_str()));
        tmpStr = tmpStr.substr(del+1);
      } else {
        temp.push_back(atoi(tmpStr.c_str()));
        break;
      }
    }
    
    return temp;
  }
  else
  {
    std::cout << "Warning! " << name << " not defined in configuration." << std::endl;
    return temp;
  }

}
