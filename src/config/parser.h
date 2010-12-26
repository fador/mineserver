#ifndef _CONFIG_PARSER_H
#define _CONFIG_PARSER_H

#include <string>
#include <map>
#include <iostream>

#include "scanner.h"
#include "lexer.h"
#include "node.h"

class ConfigParser
{
public:
  bool parse(std::string file, ConfigNode* ptr);
};

#endif
