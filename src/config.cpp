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

#include <string>

#include "config/parser.h"
#include "config/node.h"
#include "config.h"

Config::Config()
{
  m_parser = new ConfigParser;
  m_root = new ConfigNode;
}

Config::~Config()
{
  delete m_parser;
  delete m_root;
}

bool Config::load(const std::string& file)
{
  return m_parser->parse(file, m_root);
}

void Config::dump()
{
  m_root->dump();
}

ConfigNode* Config::root()
{
  return m_root;
}

bool Config::bData(const std::string& key)
{
  if (m_root->has(key))
  {
    return m_root->get(key, false)->bData();
  }

  return false;
}

int Config::iData(const std::string& key)
{
  if (m_root->has(key))
  {
    return m_root->get(key, false)->iData();
  }

  return 0;
}

int64_t Config::lData(const std::string& key)
{
  if (m_root->has(key))
  {
    return m_root->get(key, false)->lData();
  }

  return 0L;
}

float Config::fData(const std::string& key)
{
  if (m_root->has(key))
  {
    return m_root->get(key, false)->fData();
  }

  return 0.0f;
}

double Config::dData(const std::string& key)
{
  if (m_root->has(key))
  {
    return m_root->get(key, false)->dData();
  }

  return 0.0;
}

std::string Config::sData(const std::string& key)
{
  if (m_root->has(key))
  {
    return m_root->get(key, false)->sData();
  }

  return "";
}

ConfigNode* Config::mData(const std::string& key)
{
  if (m_root->has(key))
  {
    return m_root->get(key, false);
  }

  return NULL;
}

bool Config::has(const std::string& key)
{
  return m_root->has(key);
}

int Config::type(const std::string& key) const
{
  if (m_root->has(key))
  {
    return m_root->get(key)->type();
  }
  else
  {
    return CONFIG_NODE_UNDEFINED;
  }
}

std::list<std::string>* Config::keys(int type)
{
  return m_root->keys();
}
