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
#include <sstream>
#include <map>
#include <list>
#include <iostream>

#include "node.h"

ConfigNode::ConfigNode()
  :
  m_type(0),
  m_index(0),
  m_nData(0)
{
}

std::list<std::string> ConfigNode::keys(int type) const
{
  std::list<std::string> keys;

  for (Map::const_iterator it = m_list.begin(); it != m_list.end(); ++it)
  {
    if (type == CONFIG_NODE_UNDEFINED || it->second->type() == type)
    {
      keys.push_back(it->first);
    }

    if (it->second->type() == CONFIG_NODE_LIST)
    {
      std::list<std::string> tmp_list = it->second->keys(type);
      for (std::list<std::string>::const_iterator tmp_iter = tmp_list.begin(); tmp_iter != tmp_list.end(); ++tmp_iter)
      {
        keys.push_back(it->first + "." + *tmp_iter);
      }
    }
  }

  return keys;
}

bool ConfigNode::has(const std::string& key) const
{
  if (m_type != CONFIG_NODE_LIST)
  {
    return false;
  }

  const size_t pos = key.find('.');

  if (pos != std::string::npos)
  {
    const std::string keyA(key.substr(0, pos));
    const std::string keyB(key.substr(pos + 1));

    const Map::const_iterator it = m_list.find(keyA);
    return it == m_list.end() ? false : it->second->has(keyB);
  }
  else
  {
    return m_list.count(key) > 0;
  }
}

ConfigNode::Ptr ConfigNode::get(const std::string& key, bool createMissing)
{
  if (m_type != CONFIG_NODE_LIST)
  {
    return Ptr();
  }

  size_t pos = key.find('.');

  if (pos != std::string::npos)
  {
    std::string keyA(key.substr(0, pos));
    std::string keyB(key.substr(pos + 1));

    const Map::const_iterator it = m_list.find(keyA);

    if (it != m_list.end())
    {
      return it->second->get(keyB, createMissing);
    }
    else if (createMissing == false)
    {
      return Ptr();
    }
    else
    {
      return m_list.insert(Map::value_type(keyA, std::shared_ptr<ConfigNode>(new ConfigNode))).first->second->get(keyB, createMissing);
    }
  }
  else
  {
    const Map::const_iterator it = m_list.find(key);

    if (it != m_list.end())
    {
      return it->second;
    }
    else if (createMissing == false)
    {
      return Ptr();
    }
    else
    {
      return m_list.insert(Map::value_type(key, Map::mapped_type(new ConfigNode))).first->second;
    }
  }
}

bool ConfigNode::set(const std::string& key, ConfigNode::Ptr ptr, bool createMissing)
{
  m_type = CONFIG_NODE_LIST;

  const size_t pos = key.find('.');

  if (pos != std::string::npos)
  {
    const std::string keyA(key.substr(0, pos));
    const std::string keyB(key.substr(pos + 1));

    const Map::const_iterator it = m_list.find(keyA);

    if (it == m_list.end())
    {
      if (createMissing == false)
      {
        return false;
      }
      else
      {
        return m_list.insert(Map::value_type(keyA, Map::mapped_type(new ConfigNode))).first->second->set(keyB, ptr, createMissing);
      }
    }
    else
    {
      return it->second->set(keyB, ptr, createMissing);
    }
  }
  else
  {
    ++m_index;
    m_list[key] = ptr;
    return true;
  }
}

bool ConfigNode::add(ConfigNode::Ptr ptr)
{
  std::stringstream ss;
  ss << m_index;

  return set(ss.str(), ptr);
}

void ConfigNode::clear()
{
  m_type = 0;
  m_nData = 0;
  m_sData.clear();
  m_list.clear();
}

void ConfigNode::dump(int indent) const
{
  for (int i = 0; i < indent; i++)
  {
    std::cout << "  ";
  }

  switch (m_type)
  {
  case CONFIG_NODE_UNDEFINED:
  {
    std::cout << "undefined\n";
    break;
  }

  case CONFIG_NODE_LIST:
  {
    std::cout << "list:\n";

    for (Map::const_iterator it = m_list.begin(); it != m_list.end(); ++it)
    {
      for (int i = 0; i < indent + 1; ++i)
      {
        std::cout << "  ";
      }
      std::cout << it->first << " =>\n";
      it->second->dump(indent + 1);
    }

    break;
  }

  case CONFIG_NODE_BOOLEAN:
  {
    std::cout << "boolean: " << ((m_nData == 0) ? "true" : "false") << "\n";
    break;
  }

  case CONFIG_NODE_NUMBER:
  {
    std::cout << "number: " << m_nData << "\n";
    break;
  }

  case CONFIG_NODE_STRING:
  {
    std::cout << "string: " << m_sData << "\n";
    break;
  }

  default:
  {
    std::cout << "unknown\n";
    break;
  }
  }
}
