#include <string>
#include <stack>
#include <utility>
#include <iostream>

#include "scanner.h"
#include "lexer.h"

void ConfigLexer::setScanner(ConfigScanner* scanner)
{
  m_scanner = scanner;
}

ConfigScanner* ConfigLexer::scanner()
{
  return m_scanner;
}

void ConfigLexer::put_token(int type, std::string& data)
{
  std::pair<int, std::string>* tmp = new std::pair<int, std::string>;
  tmp->first = type;
  tmp->second.assign(data);
  m_tokenStack.push(tmp);
}

bool ConfigLexer::get_token(int* type, std::string* data)
{
  *type = 0;
  data->clear();

  if (m_tokenStack.size())
  {
    std::pair<int, std::string>* tmp = m_tokenStack.top();
    *type = tmp->first;
    data->assign(tmp->second);
    m_tokenStack.pop();
    delete tmp;
    return true;
  }

  char buf;
  buf = m_scanner->get();

  // Skip past spaces and newlines
  while ((buf == ' ') || (buf == '\n') || (buf == '\r') || (buf == '\t'))
  {
    m_scanner->move(1);
    buf = m_scanner->get();
  }

  // Nothing left to parse! Bail out!
  if (m_scanner->left() <= 0)
  {
    return false;
  }

  // Entity
  // Always starts with a letter, can contain only letters, numbers, periods and underscores
  if (((buf >= 'a') && (buf <= 'z')) || ((buf >= 'A') && (buf <= 'Z')))
  {
    while (((buf >= 'a') && (buf <= 'z')) || ((buf >= 'A') && (buf <= 'Z')) || ((buf >= '0') && (buf <= '9')) || (buf == '.') || (buf == '_'))
    {
      data->append(&buf, 1);
      m_scanner->move(1);
      buf = m_scanner->get();
    }

    *type = CONFIG_TOKEN_ENTITY;
    return true;
  }
  // Number
  // Always starts with a digit, may contain at most one decimal point
  else if ((buf >= '0') && (buf <= '9'))
  {
    bool found = false;

    while (((buf >= '0') && (buf <= '9')) || ((buf == '.') && (found == false)) || (buf == '_'))
    {
      if (buf == '.')
      {
        found = true;
      }

      if (buf != '_')
      {
        data->append(&buf, 1);
      }

      m_scanner->move(1);
      buf = m_scanner->get();
    }

    *type = CONFIG_TOKEN_NUMBER;
    return true;
  }
  // Assignment operator
  else if (buf == '=')
  {
    *type = CONFIG_TOKEN_OPERATOR_ASSIGN;
    m_scanner->move(1);
    return true;
  }
  // Addition operator
  else if ((buf == '+') && (m_scanner->at(m_scanner->pos()+1) == '='))
  {
    *type = CONFIG_TOKEN_OPERATOR_APPEND;
    m_scanner->move(2);
    return true;
  }
  // Quoted string
  else if ((buf == '"') || (buf == '\''))
  {
    // Save the type of quote
    int quote = buf;

    // Move forward one character
    m_scanner->move(1);

    // Parse the string and any escape characters
    char temp;
    while (m_scanner->left() > 0)
    {
      // Avoid a couple of get() calls
      temp = m_scanner->get();

      // We've found the end of the string
      if (temp == quote)
      {
        // Time to stop parsing!
        break;
      }

      // This is used to escape other characters or itself
      if (temp == '\\')
      {
        // Skip past the slash
        m_scanner->move(1);

        // Get the next character regardless of what it is
        temp = m_scanner->get();

        // Control characters (more can/will be added)
        switch (temp)
        {
        // New line
        case 'n':
          temp = '\n';
          break;
        }
      }

      // Add the character to the data string
      data->append(&temp, 1);

      // Move forward one
      m_scanner->move(1);
    }

    // Skip past the ending quote
    m_scanner->move(1);

    // Record where the scanner was at before
    int old_pos = m_scanner->pos();
    // Skip past any whitespace
    buf = m_scanner->get();
    while ((buf == ' ') || (buf == '\n') || (buf == '\r') || (buf == '\t'))
    {
      m_scanner->move(1);
      buf = m_scanner->get();
    }

    // A quoted string followed by a colon is a label
    if (buf == ':')
    {
      *type = CONFIG_TOKEN_LABEL;
      m_scanner->move(1);
    }
    else
    {
      *type = CONFIG_TOKEN_STRING;
      m_scanner->move(0-(m_scanner->pos()-old_pos));
    }

    return true;
  }
  // End of statement
  else if (buf == ';')
  {
    *type = CONFIG_TOKEN_TERMINATOR;
    m_scanner->move(1);
    return true;
  }
  // Delimiter for naming list items
  else if (buf == ':')
  {
    *type = CONFIG_TOKEN_LIST_DELIMITER;
    m_scanner->move(1);
    return true;
  }
  // Delimiter for separating list items
  else if (buf == ',')
  {
    *type = CONFIG_TOKEN_LIST_DELIMITER;
    m_scanner->move(1);
    return true;
  }
  // Start of list
  else if (buf == '(')
  {
    *type = CONFIG_TOKEN_LIST_OPEN;
    m_scanner->move(1);
    return true;
  }
  // End of list
  else if (buf == ')')
  {
    *type = CONFIG_TOKEN_LIST_CLOSE;
    m_scanner->move(1);
    return true;
  }
  // Skip commented characters
  else if (buf == '#')
  {
    while (buf != '\n')
    {
      m_scanner->move(1);
      buf = m_scanner->get();
    }

    return get_token(type, data);
  }
  // Unknown data, throw warnings everywhere!
  else
  {
    *type = 0;
    m_scanner->move(1);
    return true;
  }
}
