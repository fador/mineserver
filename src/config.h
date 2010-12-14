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

#ifndef _CONFIG_H
#define _CONFIG_H

#include <string>
#include <map>
#include <vector>
#include <deque>
#include "mineserver.h"

struct Kit;

class Conf
{
 public:
  static Conf* get()
  {
    if(!_conf) {
      _conf = new Conf();
    }
    return _conf;
  }

  ~Conf();

  void free();

  bool load(std::string configFile, std::string namePrefix = "");
  int iValue(std::string name);
  std::string sValue(std::string name);
  bool bValue(std::string name);
  std::vector<int> vValue(std::string name);
  int commandPermission(std::string commandName);
  int permissionByName(std::string permissionName);
  Kit* kit(const std::string& kitname);

  bool loadRoles();
  bool loadBanned();
  bool loadWhitelist();

  std::deque<std::string>& admins() { return m_admins; }
  std::deque<std::string>& ops() { return m_ops; }
  std::deque<std::string>& members() { return m_members; }
  std::deque<std::string>& banned() { return m_banned; }
  std::deque<std::string>& whitelist() { return m_whitelist; }

 private:
  static Conf *_conf;

  Conf() {}
  std::vector<int> stringToVec(std::string& val);

  std::map<std::string, std::string> m_confSet;
  std::map<std::string, Kit*> m_kits;

  // predefined usernames in roles.txt, banned.txt & whitelist.txt
  std::deque<std::string> m_admins;
  std::deque<std::string> m_ops;
  std::deque<std::string> m_members;
  std::deque<std::string> m_banned;
  std::deque<std::string> m_whitelist;

};

#endif
