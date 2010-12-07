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
#include <algorithm>

#include "logger.h"
#include "constants.h"
#include "permissions.h"

#include "config.h"
#include "kit.h"


Conf* Conf::_conf;

Conf::~Conf()
{
  for(std::map<std::string, Kit*>::iterator it = m_kits.begin(); it != m_kits.end(); it++)
  {
    delete it->second;
  }
}

void Conf::free()
{
   if (_conf)
   {
      delete _conf;
      _conf = 0;
   }
}

// Load/reload configuration
bool Conf::load(std::string configFile, std::string namePrefix)
{
  #ifdef _DEBUG
  std::cout << "Loading data from " << configFile << std::endl;
  #endif
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
    {
      confofs << iter->first << " = " << iter->second << std::endl;
    }

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
    {
      continue;
    }

    // If commentline -> skip to next
    if(temp[0] == COMMENTPREFIX)
    {
      continue;
    }

    // If not enough characters (Absolute min is 5: "a = s")
    if(temp.length() < 5)
    {
      continue;
    }

    // Init vars
    del = 0;
    line.clear();

    // Process line
    while(temp.length() > 0)
    {
      // Remove white spaces and = characters -_-
      while(temp[0] == ' ' || temp[0] == '=')
      {
        temp = temp.substr(1);
      }

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
    {
      text = line[1];
    }

    if (line[0] == "include")
    {
      #ifdef _DEBUG
      std::cout << "Including config file " << text << std::endl;
      #endif
      load(text);
      continue;
    }

    // Update existing configuration and add new lines
    if(m_confSet.find(namePrefix + line[0]) != m_confSet.end())
    {
      m_confSet[namePrefix + line[0]] = text;
    }
    else
    {
      // Push to configuration
      m_confSet.insert(std::pair<std::string, std::string>(namePrefix + line[0], text));
    }

    // Count line numbers
    lineNum++;
  }
  ifs.close();
  #ifdef _DEBUG
  std::cout << "Loaded " << lineNum << " lines from " << configFile << std::endl;
  #endif

  return true;
}

// Return values
std::string Conf::sValue(std::string name)
{
  if(m_confSet.find(name) != m_confSet.end())
  {
    return m_confSet[name];
  }
  else
  {
    std::cout << "Warning! " << name << " not defined in configuration. Using default value: "<<
    defaultConf[name] << std::endl;
    return defaultConf[name];
  }
}

int Conf::iValue(std::string name)
{
  if(m_confSet.find(name) != m_confSet.end())
  {
    return atoi(m_confSet[name].c_str());
  }
  else
  {
    std::cout << "Warning! " << name << " not defined in configuration. Using default value: "<<
    defaultConf[name] << std::endl;
    return atoi(defaultConf[name].c_str());
  }
}

bool Conf::bValue(std::string name)
{
  if(m_confSet.find(name) != m_confSet.end())
  {
    return (m_confSet[name] == "true")?true:false;
  }
  else
  {
    std::cout << "Warning! " << name << " not defined in configuration. Using default value: "<<
    defaultConf[name] << std::endl;
    return (defaultConf[name] == "true")?true:false;
  }
}

std::vector<int> Conf::vValue(std::string name)
{
  if(m_confSet.find(name) != m_confSet.end())
  {
    return stringToVec(m_confSet[name]);
  }
  else
  {
    std::cout << "Warning! " << name << " not defined in configuration." << std::endl;
    return std::vector<int>();
  }
}

int Conf::commandPermission(std::string commandName)
{
  return permissionByName(sValue(COMMANDS_NAME_PREFIX + commandName));
}

int Conf::permissionByName(std::string permissionName)
{
  if(permissionName == "admin")
  {
    return PERM_ADMIN;
  }

  if(permissionName == "op")
  {
    return PERM_OP;
  }

  if(permissionName == "member")
  {
    return PERM_MEMBER;
  }

  if(permissionName == "guest")
  {
    return PERM_GUEST;
  }

  std::cout << "Warning! Unknown permission name: " << permissionName << " - Using GUEST permission by default!" << std::endl;

  return PERM_GUEST; // default
}

Kit* Conf::kit(const std::string& kitname)
{
  if(m_kits.find(kitname) != m_kits.end()) {
    return m_kits[kitname];
  } else {
    std::string keyname = "kit_" + kitname;
    if(m_confSet.find(keyname) != m_confSet.end())
    {
      std::string valueString, permissionName, itemsString;
      valueString = m_confSet[keyname];
      size_t pos = valueString.find_first_of(",");

      permissionName = valueString.substr(0, pos);
      itemsString = valueString.substr(pos + 1);
      std::vector<int> items = stringToVec(itemsString);

      Kit* kit = new Kit(kitname, items, permissionByName(permissionName));
      m_kits[kitname] = kit; // save kit for later, if used again
      return kit;
    } else {
      std::cout << "Warning! " << keyname << " not defined in configuration." << std::endl;
      return NULL;
    }
  }
}

std::vector<int> Conf::stringToVec(std::string& str)
{
  std::vector<int> temp;
  int del;
  // Process "array"
  while(str.length() > 0)
  {
    // Remove white spaces characters
    while(str[0] == ' ')
    {
      str = str.substr(1);
    }

    // Split words
    del = str.find(',');
    if(del > -1)
    {
      temp.push_back(atoi(str.substr(0, del).c_str()));
      str = str.substr(del+1);
    }
    else
    {
      temp.push_back(atoi(str.c_str()));
      break;
    }
  }

  return temp;
}



bool Conf::loadRoles()
{
  std::string rolesFile = sValue("roles_file");

  // Clear current admin-vector
  m_admins.clear();
  m_ops.clear();
  m_members.clear();

  // Read admins to deque
  std::ifstream ifs(rolesFile.c_str());

  // If file does not exist
  if(ifs.fail())
  {
    std::cout << "> Warning: " << rolesFile << " not found. Creating..." << std::endl;

    std::ofstream adminofs(rolesFile.c_str());
    adminofs << ROLES_CONTENT << std::endl;
    adminofs.close();

    return true;
  }

  std::deque<std::string> *role_list = &m_members; // default is member role
  std::string temp;
  while(getline(ifs, temp))
  {
    if(temp[0] == COMMENTPREFIX)
    {
      temp = temp.substr(1); // ignore COMMENTPREFIX
      temp.erase(std::remove(temp.begin(), temp.end(), ' '), temp.end());

      // get the name of the role from the comment
      if(temp == "admins")
      {
        role_list = &m_admins;
      }
      if(temp == "ops")
      {
        role_list = &m_ops;
      }
      if(temp == "members")
      {
        role_list = &m_members;
      }
    }
    else
    {
      temp.erase(std::remove(temp.begin(), temp.end(), ' '), temp.end());
      if(temp != "")
      {
        role_list->push_back(temp);
      }
    }
  }
  ifs.close();
#ifdef _DEBUG
  std::cout << "Loaded roles from " << rolesFile << std::endl;
#endif

  return true;
}

bool Conf::loadBanned()
{
  std::string bannedFile = sValue("banned_file");

  // Clear current banned-vector
  m_banned.clear();

  // Read banned to deque
  std::ifstream ifs(bannedFile.c_str());

  // If file does not exist
  if(ifs.fail())
  {
    std::cout << "> Warning: " << bannedFile << " not found. Creating..." << std::endl;

    std::ofstream bannedofs(bannedFile.c_str());
    bannedofs << BANNED_CONTENT << std::endl;
    bannedofs.close();

    return true;
  }

  std::string temp;
  while(getline(ifs, temp))
  {
    // If not commentline
    if(temp[0] != COMMENTPREFIX)
    {
      m_banned.push_back(temp);
    }
  }
  ifs.close();
#ifdef _DEBUG
  std::cout << "Loaded banned users from " << bannedFile << std::endl;
#endif

  return true;
}

bool Conf::loadWhitelist()
{
  std::string whitelistFile = sValue("whitelist_file");

  // Clear current whitelist-vector
  m_whitelist.clear();

  // Read whitelist to deque
  std::ifstream ifs(whitelistFile.c_str());

  // If file does not exist
  if(ifs.fail())
  {
    std::cout << "> Warning: " << whitelistFile << " not found. Creating..." << std::endl;

    std::ofstream whitelistofs(whitelistFile.c_str());
    whitelistofs << WHITELIST_CONTENT << std::endl;
    whitelistofs.close();

    return true;
  }

  std::string temp;
  while(getline(ifs, temp))
  {
    // If not commentline
    if(temp[0] != COMMENTPREFIX)
    {
      m_whitelist.push_back(temp);
    }
  }
  ifs.close();
#ifdef _DEBUG
  std::cout << "Loaded whitelisted users from " << whitelistFile << std::endl;
#endif

  return true;
}
