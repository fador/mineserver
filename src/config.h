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

#ifndef _CONFIG_H
#define _CONFIG_H

#include <string>
#include <list>
#include <stdint.h>

#include "tr1.h"
#include TR1INCLUDE(memory)

#include "config/parser.h"
#include "config/node.h"

class ConfigParser;

class Config
{
public:
  Config()
  :
  m_parser(new ConfigParser),
  m_root(new ConfigNode)
  {
  }

  inline bool load(const std::string& file) const
  {
    return m_parser->parse(file, m_root);
  }

  inline bool load(const std::istream& data) const
  {
    return m_parser->parse(data, m_root);
  }

  inline void dump() const
  {
    m_root->dump();
  }

  inline ConfigNode::Ptr root() const
  {
    return m_root;
  }

  inline int iData(const std::string& key)
  {
    return m_root->has(key) ? m_root->get(key, false)->iData() : 0;
  }

  inline int64_t lData(const std::string& key)
  {
    return m_root->has(key) ? m_root->get(key, false)->lData() : 0;
  }

  inline float fData(const std::string& key)
  {
    return m_root->has(key) ? m_root->get(key, false)->fData() : 0;
  }

  inline double dData(const std::string& key)
  {
    return m_root->has(key) ? m_root->get(key, false)->dData() : 0;
  }

  inline std::string sData(const std::string& key)
  {
    return m_root->has(key) ? m_root->get(key, false)->sData() : "";
  }

  inline bool bData(const std::string& key)
  {
    return m_root->has(key) ? m_root->get(key, false)->bData() : false;
  }

  inline ConfigNode::Ptr mData(const std::string& key)
  {
    return m_root->has(key) ? m_root->get(key, false) : ConfigNode::Ptr();
  }

  inline bool has(const std::string& key) const
  {
    return m_root->has(key);
  }

  inline int type(const std::string& key) const
  {
    return m_root->has(key) ? m_root->get(key)->type() : CONFIG_NODE_UNDEFINED;
  }

  inline std::list<std::string> keys(int type = CONFIG_NODE_UNDEFINED) const
  {
    return m_root->keys();
  }

private:
  std::tr1::shared_ptr<ConfigParser> m_parser;
  ConfigNode::Ptr m_root;
};

#endif
