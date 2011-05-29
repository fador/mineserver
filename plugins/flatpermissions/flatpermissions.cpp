/*

g++ -c flatpermissions.cpp
g++ -shared flatpermissions.o -o flatpermissions.so

*/
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
#include <string>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>

#define MINESERVER_C_API
#include "../../src/plugin_api.h"

#define PLUGIN_VERSION 1.0
mineserver_pointer_struct* mineserver;
std::string pluginName = "flatpermissions";

std::string dtos(double n)
{
  std::stringstream result;
  result << n;
  return result.str();
}

void loginPost(const char* userIn){
  std::ifstream file;
  file.open("permissions.txt", std::ios::in);
  std::string line;
  if(file.is_open()){
    while(file.good()){
      std::string msg;
      std::getline(file, msg);
      if(msg.size()>1){
        std::string name, rank;
        std::istringstream line(msg);
        std::getline(line,name,':');
        std::getline(line,rank);
        if(name.compare(std::string(userIn))==0){
          std::transform(rank.begin(), rank.end(), rank.begin(), ::tolower);
          if(rank.compare("admin")==0 || rank.compare("admins")==0){
            mineserver->permissions.setAdmin(name.c_str());
          }else if(rank.compare("op")==0 || rank.compare("ops")==0){
            mineserver->permissions.setOp(name.c_str());
          }else if(rank.compare("member")==0 || rank.compare("members")==0){
            mineserver->permissions.setMember(name.c_str());
          }else{
            mineserver->permissions.setGuest(name.c_str());
          }
          break;
        }
      }
    }
    file.close();
  }
}


PLUGIN_API_EXPORT void CALLCONVERSION flatpermissions_shutdown(void)
{
  if (mineserver->plugin.getPluginVersion(pluginName.c_str()) <= 0)
  {
    mineserver->logger.log(6, "plugin.flatpermissions", "flatpermission is not loaded!");
    return;
  }
}

PLUGIN_API_EXPORT void CALLCONVERSION flatpermissions_init(mineserver_pointer_struct* mineserver_temp)
{
  mineserver = mineserver_temp;
  if(mineserver->plugin.getPluginVersion(pluginName.c_str())>0){
    std::string msg = "flatpermission is already loaded, v. "+dtos(mineserver->plugin.getPluginVersion(pluginName.c_str()));
    mineserver->logger.log(6, "plugin.flatpermissions", msg.c_str());
    return;
  }
  std::string msg = "Loaded "+pluginName+"!";
  mineserver->logger.log(6, "plugin.flatpermissions", msg.c_str());

  mineserver->plugin.setPluginVersion(pluginName.c_str(), PLUGIN_VERSION);
  mineserver->plugin.addCallback("PlayerLoginPost", reinterpret_cast<voidF>(loginPost));
}
