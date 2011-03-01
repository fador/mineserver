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
#include <deque>
#include <iostream>
#include <cstdlib>

#include "scanner.h"
#include "lexer.h"
#include "parser.h"
#include "node.h"

bool ConfigParser::parse(const std::string& file, ConfigNode* ptr)
{
  ConfigScanner scanner;
  ConfigLexer lexer;
  ConfigNode* root = ptr;

  if (!scanner.read(file))
  {
    std::cerr << "Couldn't find config file: " << file << "\n";
    return false;
  }

  lexer.setScanner(&scanner);

  int token_type;
  std::string token_data;
  std::string token_label;
  std::deque<ConfigNode*> nodeStack;
  ConfigNode* currentNode = root;
  nodeStack.push_back(currentNode);
  while (lexer.get_token(&token_type, &token_data))
  {
    if (!token_type)
    {
      std::cerr << "Unrecognised data!\n";
      return false;
    }

    // Include other files only if we're in the root node
    if ((token_type == CONFIG_TOKEN_ENTITY) && (token_data == "include") && (currentNode == root))
    {
      int tmp_type;
      std::string tmp_data;

      lexer.get_token(&tmp_type, &tmp_data);
      if (tmp_type == CONFIG_TOKEN_STRING)
      {
        if (tmp_data == file)
        {
          std::cerr << "Warning: recursion detected! Not including `" << tmp_data << "`.\n";
          continue;
        }

        if (!parse(tmp_data, root))
        {
          return false;
        }

        continue;
      }
      else
      {
        lexer.put_token(tmp_type, tmp_data);
      }
    }

    if ((token_type == CONFIG_TOKEN_ENTITY) || (token_type == CONFIG_TOKEN_LABEL))
    {
      token_label.assign(token_data);
    }

    if (token_type == CONFIG_TOKEN_OPERATOR_ASSIGN)
    {
      if (currentNode != root)
      {
        currentNode->clear();
      }
    }

    if (token_type == CONFIG_TOKEN_BOOLEAN)
    {
      ConfigNode* newNode = (token_label.size() && currentNode->has(token_label)) ? currentNode->get(token_label) : new ConfigNode;

      newNode->setData(token_data == "true");

      if (token_label.size())
      {
        currentNode->set(token_label, newNode, true);
        token_label.clear();
      }
      else
      {
        currentNode->add(newNode);
      }
    }

    if (token_type == CONFIG_TOKEN_STRING)
    {
      ConfigNode* newNode = (token_label.size() && currentNode->has(token_label)) ? currentNode->get(token_label) : new ConfigNode;

      newNode->setData(token_data);

      if (token_label.size())
      {
        currentNode->set(token_label, newNode, true);
        token_label.clear();
      }
      else
      {
        currentNode->add(newNode);
      }
    }

    if (token_type == CONFIG_TOKEN_NUMBER)
    {
      ConfigNode* newNode = (token_label.size() && currentNode->has(token_label)) ? currentNode->get(token_label) : new ConfigNode;

      newNode->setData((double)::atof(token_data.c_str()));

      if (token_label.size())
      {
        currentNode->set(token_label, newNode, true);
        token_label.clear();
      }
      else
      {
        currentNode->add(newNode);
      }
    }

    if (token_type == CONFIG_TOKEN_LIST_OPEN)
    {
      ConfigNode* newNode = (token_label.size() && currentNode->has(token_label)) ? currentNode->get(token_label) : new ConfigNode;

      newNode->setType(CONFIG_NODE_LIST);

      if (token_label.size())
      {
        currentNode->set(token_label, newNode, true);

        newNode = currentNode->get(token_label, true);

        token_label.clear();
      }
      else
      {
        currentNode->add(newNode);
      }

      nodeStack.push_back(currentNode);
      currentNode = newNode;
    }

    if (token_type == CONFIG_TOKEN_LIST_CLOSE)
    {
      currentNode = nodeStack.back();
      nodeStack.pop_back();
    }
  }

  return true;
}
