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

#ifndef _CONFIG_LEXER_H
#define _CONFIG_LEXER_H

#include <string>
#include <stack>
#include <utility>

#define CONFIG_TOKEN_ENTITY 1
#define CONFIG_TOKEN_LABEL 2
#define CONFIG_TOKEN_NUMBER 3
#define CONFIG_TOKEN_STRING 4
#define CONFIG_TOKEN_BOOLEAN 5

#define CONFIG_TOKEN_LIST_OPEN 10
#define CONFIG_TOKEN_LIST_CLOSE 11
#define CONFIG_TOKEN_LIST_DELIMITER 12
#define CONFIG_TOKEN_LIST_LABEL 13

#define CONFIG_TOKEN_OPERATOR_ASSIGN 20
#define CONFIG_TOKEN_OPERATOR_APPEND 21

#define CONFIG_TOKEN_TERMINATOR 30

class ConfigScanner;

class ConfigLexer
{
public:
  bool get_token(int* type, std::string* data);
  void put_token(int type, const std::string& data);
  void setScanner(ConfigScanner* scanner);
  ConfigScanner* scanner();
private:
  ConfigScanner* m_scanner;
  std::stack<std::pair<int, std::string>*> m_tokenStack;
};

#endif
