#include <string>
#include <sstream>
#include <map>
#include <list>
#include <iostream>

#include "node.h"

ConfigNode::ConfigNode() : m_type(0),m_index(0),m_nData(0)
{
}

ConfigNode::~ConfigNode()
{
  std::map<std::string, ConfigNode*>::iterator iter_a = m_list.begin();
  std::map<std::string, ConfigNode*>::iterator iter_b = m_list.end();

  for (;iter_a!=iter_b;++iter_a)
  {
    delete iter_a->second;
  }
}

bool ConfigNode::data(bool* ptr)
{
  if (m_type == CONFIG_NODE_BOOLEAN)
  {
    *ptr = (bool)m_nData;
    return true;
  }
  else
  {
    ptr = false;
    return false;
  }
}

bool ConfigNode::data(int* ptr)
{
  if (m_type == CONFIG_NODE_NUMBER)
  {
    *ptr = (int)m_nData;
    return true;
  }
  else
  {
    ptr = NULL;
    return false;
  }
}

bool ConfigNode::data(long* ptr)
{
  if (m_type == CONFIG_NODE_NUMBER)
  {
    *ptr = (long)m_nData;
    return true;
  }
  else
  {
    ptr = NULL;
    return false;
  }
}

bool ConfigNode::data(float* ptr)
{
  if (m_type == CONFIG_NODE_NUMBER)
  {
    *ptr = (float)m_nData;
    return true;
  }
  else
  {
    ptr = NULL;
    return false;
  }
}

bool ConfigNode::data(double* ptr)
{
  if (m_type == CONFIG_NODE_NUMBER)
  {
    *ptr = (double)m_nData;
    return true;
  }
  else
  {
    ptr = NULL;
    return false;
  }
}

bool ConfigNode::data(std::string* ptr)
{
  if (m_type == CONFIG_NODE_STRING)
  {
    ptr->assign(m_sData);
    return true;
  }
  else
  {
    ptr->clear();
    return false;
  }
}

bool ConfigNode::bData()
{
  bool tmp;
  data(&tmp);
  return tmp;
}

int ConfigNode::iData()
{
  int tmp;
  data(&tmp);
  return tmp;
}

long ConfigNode::lData()
{
  long tmp;
  data(&tmp);
  return tmp;
}

float ConfigNode::fData()
{
  float tmp;
  data(&tmp);
  return tmp;
}

double ConfigNode::dData()
{
  double tmp;
  data(&tmp);
  return tmp;
}

std::string ConfigNode::sData()
{
  std::string tmp;
  data(&tmp);
  return tmp;
}

std::list<std::string>* ConfigNode::keys(int type)
{
  std::map<std::string, ConfigNode*>::iterator iter_a = m_list.begin();
  std::map<std::string, ConfigNode*>::iterator iter_b = m_list.end();

  std::list<std::string>* keys = new std::list<std::string>;

  for (;iter_a!=iter_b;++iter_a)
  {
    if ((type == CONFIG_NODE_UNDEFINED) || (iter_a->second->type() == type))
    {
      keys->push_back(iter_a->first);
    }

    if (iter_a->second->type() == CONFIG_NODE_LIST)
    {
      std::list<std::string>* tmp_list = iter_a->second->keys(type);
      std::list<std::string>::iterator tmp_iter = tmp_list->begin();

      for (;tmp_iter!=tmp_list->end();++tmp_iter)
      {
        keys->push_back((iter_a->first)+"."+(*tmp_iter));
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

void ConfigNode::setData(long data)
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

void ConfigNode::setData(std::string& data)
{
  m_type = CONFIG_NODE_STRING;
  m_sData.assign(data);
}

int ConfigNode::type()
{
  return m_type;
}

void ConfigNode::setType(int type)
{
  m_type = type;
}

bool ConfigNode::has(std::string& key)
{
  if (m_type != CONFIG_NODE_LIST)
  {
    return false;
  }

  size_t pos = key.find('.');

  if (pos != std::string::npos)
  {
    std::string keyA(key.substr(0, pos));
    std::string keyB(key.substr(pos+1));
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

ConfigNode* ConfigNode::get(std::string& key, bool createMissing)
{
  if (m_type != CONFIG_NODE_LIST)
  {
    return false;
  }

  size_t pos = key.find('.');

  if (pos != std::string::npos)
  {
    std::string keyA(key.substr(0, pos));
    std::string keyB(key.substr(pos+1));
    ConfigNode* tmp;

    if (m_list.count(keyA))
    {
      tmp = m_list[keyA];
    }
    else
    {
      if (createMissing == false)
      {
        return false;
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
        return false;
      }
    }

    return m_list[key];
  }
}

bool ConfigNode::set(std::string& key, ConfigNode* ptr, bool createMissing)
{
  m_type = CONFIG_NODE_LIST;

  size_t pos = key.find('.');

  if (pos != std::string::npos)
  {
    std::string keyA(key.substr(0, pos));
    std::string keyB(key.substr(pos+1));
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

void ConfigNode::dump(int indent=0)
{
  for (int i=0;i<indent;i++)
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

      std::map<std::string, ConfigNode*>::iterator iter_a = m_list.begin();
      std::map<std::string, ConfigNode*>::iterator iter_b = m_list.end();

      for (;iter_a!=iter_b;++iter_a)
      {
        for (int i=0;i<(indent+1);i++)
        {
          std::cout << "  ";
        }
        std::cout << iter_a->first << " =>\n";
        iter_a->second->dump(indent+1);
      }

      break;
    }

    case CONFIG_NODE_BOOLEAN:
    {
      std::cout << "boolean: " << ((bool)m_nData ? "true" : "false") << "\n";
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
