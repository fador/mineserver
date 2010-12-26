#include <string>
#include <map>
#include <deque>
#include <iostream>
#include <cstdlib>

#include "scanner.h"
#include "lexer.h"
#include "parser.h"
#include "node.h"

bool ConfigParser::parse(std::string file, ConfigNode* ptr)
{
  ConfigScanner* scanner = new ConfigScanner();
  ConfigLexer* lexer = new ConfigLexer();
  ConfigNode* root = ptr;

  if (!scanner->read(file))
  {
    std::cerr << "Couldn't find config file: " << file << "\n";
    return false;
  }

  lexer->setScanner(scanner);

  int token_type;
  std::string token_data;
  std::string token_label;
  std::deque<ConfigNode*> nodeStack;
  ConfigNode* currentNode = root;
  nodeStack.push_back(currentNode);
  while (lexer->get_token(&token_type, &token_data))
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

      lexer->get_token(&tmp_type, &tmp_data);
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
        lexer->put_token(tmp_type, tmp_data);
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

    if (token_type == CONFIG_TOKEN_STRING)
    {
      ConfigNode* newNode = new ConfigNode;
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
      ConfigNode* newNode = new ConfigNode;
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
      ConfigNode* newNode;
      if (token_label.size())
      {
        if (!currentNode->has(token_label))
        {
          newNode = new ConfigNode;
          newNode->setType(CONFIG_NODE_LIST);
          currentNode->set(token_label, newNode, true);
        }

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
