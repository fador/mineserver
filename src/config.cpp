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


Conf* Conf::mConf;

void Conf::free()
{
   if (mConf)
   {
      delete mConf;
      mConf = 0;
   }
}

// Load/reload configuration
bool Conf::load(std::string configFile)
{
  std::cout << "Loading data from " << configFile << std::endl;
  std::ifstream ifs(configFile.c_str());

  // If configfile does not exist
  if(ifs.fail() && configFile == CONFIG_FILE)
  {
    // TODO: Load default configuration from the internets!
    std::cout << "Warning: " << configFile << " not found! Generating it now." << std::endl;

    // Open config file
    std::ofstream confofs(configFile.c_str());

    // Write header
    confofs << "# This is the default config, please see http://mineserver.be/wiki/Configuration for more information." << std::endl << std::endl;

    // Write all the default settings
    std::map<std::string, std::string>::iterator iter;
    for(iter=defaultConf.begin();iter!=defaultConf.end();++iter)
      confofs << iter->first << " = " << iter->second << std::endl;

    // Close the config file
    confofs.close();

    this->load(CONFIG_FILE);
  }

  if (ifs.fail())
  {
    std::cout << "Warning: " << configFile << " not found!" << std::endl;
    ifs.close();
    return true;
  }

  std::string temp;

  // Reading row at a time
  int del;
  int lineNum = 0;
  std::vector<std::string> line;
  std::string text;
  while(getline(ifs, temp))
  {
    //If empty line
    if(temp.size() == 0) 
      continue;

    // If commentline -> skip to next
    if(temp[0] == COMMENTPREFIX)
      continue;

    // If not enough characters (Absolute min is 5: "a = s")
    if(temp.length() < 5)
      continue;

    // Init vars
    del = 0;
    line.clear();

    // Process line
    while(temp.length() > 0)
    {
      // Remove white spaces and = characters -_-
      while(temp[0] == ' ' || temp[0] == '=')
        temp = temp.substr(1);

      // Split words
      del = temp.find(' ');
      if(del > -1)
      {
        line.push_back(temp.substr(0, del));
        temp = temp.substr(del+1);
      }
      else
      {
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
    }
    else
      text = line[1];

    if (line[0] == "include")
    {
      std::cout << "Including config file " << text << std::endl;
      load(text);
      continue;
    }

    // Update existing configuration and add new lines
    if(confSet.find(line[0]) != confSet.end())
      confSet[line[0]] = text;
    else
    {
      // Push to configuration
      confSet.insert(std::pair<std::string, std::string>(line[0], text));
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
    return confSet[name];
  else
  {
    std::cout << "Warning! " << name << " not defined in configuration. Using default value: "<<
    defaultConf[name] << std::endl;
    return defaultConf[name];
  }
}

int Conf::iValue(std::string name)
{
  if(confSet.find(name) != confSet.end())
    return atoi(confSet[name].c_str());
  else
  {
    std::cout << "Warning! " << name << " not defined in configuration. Using default value: "<<
    defaultConf[name] << std::endl;
    return atoi(defaultConf[name].c_str());
  }
}

bool Conf::bValue(std::string name)
{
  if(confSet.find(name) != confSet.end())
    return (confSet[name] == "true")?true:false;
  else
  {
    std::cout << "Warning! " << name << " not defined in configuration. Using default value: "<<
    defaultConf[name] << std::endl;
    return (defaultConf[name] == "true")?true:false;
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
      // Remove white spaces characters
      while(tmpStr[0] == ' ')
        tmpStr = tmpStr.substr(1);

      // Split words
      del = tmpStr.find(',');
      if(del > -1)
      {
        temp.push_back(atoi(tmpStr.substr(0, del).c_str()));
        tmpStr = tmpStr.substr(del+1);
      }
      else
      {
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
