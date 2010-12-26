#ifndef _CONFIG_LEXER_H
#define _CONFIG_LEXER_H

#include <string>
#include <stack>
#include <utility>
#include "scanner.h"

#define CONFIG_TOKEN_ENTITY 1
#define CONFIG_TOKEN_LABEL 2
#define CONFIG_TOKEN_NUMBER 3
#define CONFIG_TOKEN_STRING 4

#define CONFIG_TOKEN_LIST_OPEN 10
#define CONFIG_TOKEN_LIST_CLOSE 11
#define CONFIG_TOKEN_LIST_DELIMITER 12
#define CONFIG_TOKEN_LIST_LABEL 13

#define CONFIG_TOKEN_OPERATOR_ASSIGN 20
#define CONFIG_TOKEN_OPERATOR_APPEND 21

#define CONFIG_TOKEN_TERMINATOR 30

class ConfigLexer
{
public:
  bool get_token(int* type, std::string* data);
  void put_token(int type, std::string& data);
  void setScanner(ConfigScanner* scanner);
  ConfigScanner* scanner();
private:
  ConfigScanner* m_scanner;
  std::stack<std::pair<int, std::string>*> m_tokenStack;
};

#endif
