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

#include "tr1.h"
#include TR1INCLUDE(memory)

#include <stdint.h>

#define CONFIG_NODE_UNDEFINED 0
#define CONFIG_NODE_LIST 1
#define CONFIG_NODE_BOOLEAN 2
#define CONFIG_NODE_NUMBER 3
#define CONFIG_NODE_STRING 4

class ConfigNode
{
public:

  typedef std::tr1::shared_ptr<ConfigNode> Ptr;
  typedef std::map<std::string, Ptr> Map;

  ConfigNode();

  inline bool bData() const
  {
    return m_type == CONFIG_NODE_BOOLEAN ? (m_nData != 0) : false;
  }

  inline int iData() const
  {
    return m_type == CONFIG_NODE_NUMBER ? (int)m_nData : 0;
  }

  inline int64_t lData() const
  {
    return m_type == CONFIG_NODE_NUMBER ? (int64_t)m_nData : 0;
  }

  inline float fData() const
  {
    return m_type == CONFIG_NODE_NUMBER ? (float)m_nData : 0;
  }

  inline double dData() const
  {
    return m_type == CONFIG_NODE_NUMBER ? m_nData : 0;
  }

  inline std::string sData() const
  {
    return m_type == CONFIG_NODE_STRING ? m_sData : "";
  }

  inline void setData(bool data)
  {
    m_type = CONFIG_NODE_BOOLEAN;
    m_nData = data ? 1 : 0;
  }

  inline void setData(int data)
  {
    m_type = CONFIG_NODE_NUMBER;
    m_nData = data;
  }

  inline void setData(int64_t data)
  {
    m_type = CONFIG_NODE_NUMBER;
    m_nData = data;
  }

  inline void setData(float data)
  {
    m_type = CONFIG_NODE_NUMBER;
    m_nData = data;
  }

  inline void setData(double data)
  {
    m_type = CONFIG_NODE_NUMBER;
    m_nData = data;
  }

  inline void setData(const std::string& data)
  {
    m_type = CONFIG_NODE_STRING;
    m_sData = data;
  }

  std::list<std::string> keys(int type = CONFIG_NODE_UNDEFINED) const;

  inline int type() const
  {
    return m_type;
  }

  inline void setType(int type)
  {
    m_type = type;
  }

  bool has(const std::string& key) const;
  Ptr  get(const std::string& key, bool createMissing = true);
  bool set(const std::string& key, Ptr ptr, bool createMissing = true);
  bool add(Ptr ptr);
  void clear();
  void dump(int indent = 0) const;

private:
  int m_type;
  int m_index;
  double m_nData;
  std::string m_sData;
  Map m_list;
};

#endif
