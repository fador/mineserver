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
#include <map>
#include <stack>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

#include "scanner.h"
#include "lexer.h"
#include "parser.h"
#include "node.h"

#include "../tools.h"

ConfigParser::ConfigParser()
  : m_includes(0)
{
}

bool ConfigParser::parse(const std::string& file, ConfigNode::Ptr ptr)
{
  std::ifstream ifs(file.c_str(), std::ios_base::binary);

  std::cout << "Parser opening file " << file << std::endl;
  if (!ifs)
  {
    std::cerr << "Couldn't open file: " << file << "\n";
    return false;
  }

  return parse(ifs, ptr);
}

bool ConfigParser::parse(std::istream& data, ConfigNode::Ptr ptr)
{
  ConfigScanner scanner;
  ConfigLexer lexer(scanner);
  ConfigNode::Ptr root = ptr;

  data.seekg(0, std::ios::end);
  const size_t data_size = data.tellg();
  data.seekg(0, std::ios::beg);

  char* buf = new char[data_size];
  data.read(buf, data_size);
  std::string data_str(buf, data_size);
  delete[] buf;

  if (!scanner.read(data_str))
  {
    std::cerr << "Couldn't read data!\n";
    return false;
  }

  int token_type;
  std::string token_data;
  std::string token_label;
  std::stack<ConfigNode::Ptr> nodeStack;
  ConfigNode::Ptr currentNode = root;
  nodeStack.push(currentNode);

  while (lexer.get_token(token_type, token_data))
  {
    if (!token_type)
    {
      std::cerr << "Unrecognised data!\n";
      return false;
    }

    // Include other files only if we're in the root node
    if (token_type == CONFIG_TOKEN_ENTITY && token_data == "include" && currentNode == root)
    {
      int tmp_type;
      std::string tmp_data;

      lexer.get_token(tmp_type, tmp_data);
      if (tmp_type == CONFIG_TOKEN_STRING)
      {
        if (m_includes >= MAX_INCLUDES)
        {
          std::cerr << "reached maximum number of include directives: " << m_includes << "\n";
          return false;
        }

        // allow only filename without path
        if ((tmp_data.find('/')  != std::string::npos)
            || (tmp_data.find('\\') != std::string::npos))
        {
          std::cerr << "include directive accepts only filename: " << tmp_data << "\n";
          return false;
        }

        // prepend home path
        const std::string var  = "system.path.home";
        const ConfigNode::Ptr node = root->get(var, false);
        std::string home;
        if (!node || (home = node->sData()).empty())
        {
          std::cerr << "include directive is not allowed before: " << var << "\n";
          return false;
        }

        tmp_data = pathExpandUser(home) + '/' + tmp_data;
        if (!parse(tmp_data, root))
        {
          return false;
        }
        m_includes++;

        continue;
      }
      else
      {
        lexer.put_token(tmp_type, tmp_data);
      }
    }

    if (token_type == CONFIG_TOKEN_ENTITY || token_type == CONFIG_TOKEN_LABEL)
    {
      token_label = token_data;
    }

    else if (token_type == CONFIG_TOKEN_OPERATOR_ASSIGN)
    {
      if (currentNode != root)
      {
        currentNode->clear();
      }
    }

    else if (token_type == CONFIG_TOKEN_BOOLEAN)
    {
      ConfigNode::Ptr newNode(!token_label.empty() && currentNode->has(token_label) ? currentNode->get(token_label) : ConfigNode::Ptr(new ConfigNode));

      newNode->setData(token_data == "true");

      if (!token_label.empty())
      {
        currentNode->set(token_label, newNode, true);
        token_label.clear();
      }
      else
      {
        currentNode->add(newNode);
      }
    }

    else if (token_type == CONFIG_TOKEN_STRING)
    {
      ConfigNode::Ptr newNode(!token_label.empty() && currentNode->has(token_label) ? currentNode->get(token_label) : ConfigNode::Ptr(new ConfigNode));

      newNode->setData(token_data);

      if (!token_label.empty())
      {
        currentNode->set(token_label, newNode, true);
        token_label.clear();
      }
      else
      {
        currentNode->add(newNode);
      }
    }

    else if (token_type == CONFIG_TOKEN_NUMBER)
    {
      ConfigNode::Ptr newNode(token_label.size() && currentNode->has(token_label) ? currentNode->get(token_label) : ConfigNode::Ptr(new ConfigNode));

      newNode->setData((double)::atof(token_data.c_str()));

      if (!token_label.empty())
      {
        currentNode->set(token_label, newNode, true);
        token_label.clear();
      }
      else
      {
        currentNode->add(newNode);
      }
    }

    else if (token_type == CONFIG_TOKEN_LIST_OPEN)
    {
      ConfigNode::Ptr newNode(token_label.size() && currentNode->has(token_label) ? currentNode->get(token_label) : ConfigNode::Ptr(new ConfigNode));

      newNode->setType(CONFIG_NODE_LIST);

      if (!token_label.empty())
      {
        currentNode->set(token_label, newNode, true);

        newNode = currentNode->get(token_label, true);

        token_label.clear();
      }
      else
      {
        currentNode->add(newNode);
      }

      nodeStack.push(currentNode);
      currentNode = newNode;
    }

    else if (token_type == CONFIG_TOKEN_LIST_CLOSE)
    {
      currentNode = nodeStack.top();
      nodeStack.pop();
    }
  }

  return true;
}
