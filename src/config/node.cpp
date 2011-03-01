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

ConfigNode::ConfigNode() : m_type(0), m_index(0), m_nData(0)
{
}

ConfigNode::~ConfigNode()
{
  std::map<std::string, ConfigNode*>::iterator iter_a = m_list.begin();
  std::map<std::string, ConfigNode*>::iterator iter_b = m_list.end();

  for (; iter_a != iter_b; ++iter_a)
  {
    delete iter_a->second;
  }
}



bool ConfigNode::bData() const
{
  bool tmp = false;
  if (m_type == CONFIG_NODE_BOOLEAN)
  {
    tmp = m_nData != 0.0;
  }
  return tmp;
}

int ConfigNode::iData() const
{
  if (m_type == CONFIG_NODE_NUMBER)
  {
    return (int)m_nData;
  }
  return 0;
}

int64_t ConfigNode::lData() const
{
  if (m_type == CONFIG_NODE_NUMBER)
  {
    return (int64_t)m_nData;
  }
  return 0;
}

float ConfigNode::fData() const
{
  if (m_type == CONFIG_NODE_NUMBER)
  {
    return (float)m_nData;
  }
  return 0.0f;
}

double ConfigNode::dData() const
{
  if (m_type == CONFIG_NODE_NUMBER)
  {
    return m_nData;
  }
  return 0.0;
}

std::string ConfigNode::sData() const
{
  std::string tmp("");
  if (m_type == CONFIG_NODE_STRING)
  {
    tmp.assign(m_sData);
  }
  else
  {
    tmp.clear();
  }
  return tmp;
}

std::list<std::string>* ConfigNode::keys(int type)
{
  std::map<std::string, ConfigNode*>::iterator iter_a = m_list.begin();
  std::map<std::string, ConfigNode*>::iterator iter_b = m_list.end();

  std::list<std::string>* keys = new std::list<std::string>;

  for (; iter_a != iter_b; ++iter_a)
  {
    if ((type == CONFIG_NODE_UNDEFINED) || (iter_a->second->type() == type))
    {
      keys->push_back(iter_a->first);
    }

    if (iter_a->second->type() == CONFIG_NODE_LIST)
    {
      std::list<std::string>* tmp_list = iter_a->second->keys(type);
      std::list<std::string>::iterator tmp_iter = tmp_list->begin();

      for (; tmp_iter != tmp_list->end(); ++tmp_iter)
      {
        keys->push_back((iter_a->first) + "." + (*tmp_iter));
      }

      delete tmp_list;
    }
  }

  return keys;
}

void ConfigNode::setData(bool data)
{
  m_type = CONFIG_NODE_BOOLEAN;
  m_nData = (double)data;
}

void ConfigNode::setData(int data)
{
  m_type = CONFIG_NODE_NUMBER;
  m_nData = (double)data;
}

void ConfigNode::setData(int64_t data)
{
  m_type = CONFIG_NODE_NUMBER;
  m_nData = (double)data;
}

void ConfigNode::setData(float data)
{
  m_type = CONFIG_NODE_NUMBER;
  m_nData = (double)data;
}

void ConfigNode::setData(double data)
{
  m_type = CONFIG_NODE_NUMBER;
  m_nData = (double)data;
}

void ConfigNode::setData(const std::string& data)
{
  m_type = CONFIG_NODE_STRING;
  m_sData.assign(data);
}

int ConfigNode::type() const
{
  return m_type;
}

void ConfigNode::setType(int type)
{
  m_type = type;
}

bool ConfigNode::has(const std::string& key)
{
  if (m_type != CONFIG_NODE_LIST)
  {
    return false;
  }

  size_t pos = key.find('.');

  if (pos != std::string::npos)
  {
    const std::string keyA(key.substr(0, pos));
    const std::string keyB(key.substr(pos + 1));
    ConfigNode* tmp;

    if (m_list.count(keyA) == 0)
    {
      return false;
    }
    else
    {
      tmp = m_list[keyA];
      return tmp->has(keyB);
    }
  }
  else if (m_list.count(key) == 1)
  {
    return true;
  }
  else
  {
    return false;
  }
}

ConfigNode* ConfigNode::get(const std::string& key, bool createMissing)
{
  if (m_type != CONFIG_NODE_LIST)
  {
    return NULL;
  }

  size_t pos = key.find('.');

  if (pos != std::string::npos)
  {
    std::string keyA(key.substr(0, pos));
    std::string keyB(key.substr(pos + 1));
    ConfigNode* tmp;

    if (m_list.count(keyA))
    {
      tmp = m_list[keyA];
    }
    else
    {
      if (createMissing == false)
      {
        return NULL;
      }
      else
      {
        tmp = new ConfigNode();
        m_list[keyA] = tmp;
      }
    }

    return tmp->get(keyB, createMissing);
  }
  else
  {
    if (m_list.count(key) == 0)
    {
      if (createMissing == true)
      {
        m_list[key] = new ConfigNode;
      }
      else
      {
        return NULL;
      }
    }

    return m_list[key];
  }
}

bool ConfigNode::set(const std::string& key, ConfigNode* ptr, bool createMissing)
{
  m_type = CONFIG_NODE_LIST;

  size_t pos = key.find('.');

  if (pos != std::string::npos)
  {
    std::string keyA(key.substr(0, pos));
    std::string keyB(key.substr(pos + 1));
    ConfigNode* tmp;

    if (m_list.count(keyA) == 0)
    {
      if (createMissing == false)
      {
        return false;
      }
      else
      {
        m_list[keyA] = new ConfigNode;
      }
    }

    tmp = m_list[keyA];
    return tmp->set(keyB, ptr, createMissing);
  }
  else
  {
    m_index++;
    m_list[key] = ptr;
    return true;
  }
}

bool ConfigNode::add(ConfigNode* ptr)
{
  std::string key;
  std::stringstream ss;
  ss << m_index;
  ss >> key;

  return set(key, ptr);
}

void ConfigNode::clear()
{
  m_type = 0;
  m_nData = 0;
  m_sData.clear();
  m_list.clear();
}

void ConfigNode::dump(int indent = 0) const
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

    std::map<std::string, ConfigNode*>::const_iterator iter_a = m_list.begin();
    std::map<std::string, ConfigNode*>::const_iterator iter_b = m_list.end();

    for (; iter_a != iter_b; ++iter_a)
    {
      for (int i = 0; i < (indent + 1); i++)
      {
        std::cout << "  ";
      }
      std::cout << iter_a->first << " =>\n";
      iter_a->second->dump(indent + 1);
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
