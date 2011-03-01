/*
  Copyright (c) 2011, The Mineserver Project
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

#ifndef _CONFIG_NODE_H
#define _CONFIG_NODE_H

#include <string>
#include <map>
#include <list>

#include <stdint.h>

#define CONFIG_NODE_UNDEFINED 0
#define CONFIG_NODE_LIST 1
#define CONFIG_NODE_BOOLEAN 2
#define CONFIG_NODE_NUMBER 3
#define CONFIG_NODE_STRING 4

class ConfigNode
{
public:
  ConfigNode();
  ~ConfigNode();
  bool bData() const;
  int iData() const;
  int64_t lData() const;
  float fData() const;
  double dData() const;
  std::string sData() const;
  void setData(bool data);
  void setData(int data);
  void setData(int64_t data);
  void setData(float data);
  void setData(double data);
  void setData(const std::string& data);
  std::list<std::string>* keys(int type = CONFIG_NODE_UNDEFINED);
  int type() const;
  void setType(int type);
  bool has(const std::string& key);
  ConfigNode* get(const std::string& key, bool createMissing = true);
  bool set(const std::string& key, ConfigNode* ptr, bool createMissing = true);
  bool add(ConfigNode* ptr);
  void clear();
  void dump(int indent) const;
  void dump() const
  {
    dump(0);
  }
private:
  int m_type;
  int m_index;
  double m_nData;
  std::string m_sData;
  std::map<std::string, ConfigNode*> m_list;
};

#endif
