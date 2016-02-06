/*
  Copyright (c) 2016, The Mineserver Project
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

#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>

#include "mineserver.h"
#include "json.h"
#include "tools.h"

std::string JSON_Val::readString(uint8_t* buf, int32_t& len) {
  bool done = false;
  bool escapeChar = false;
  int32_t readBytes = 0;
  std::string str;

  while (!done) {
    char byte = buf[0];    
    buf++;
    readBytes++;
    switch (byte) {
      case  '\\':
        escapeChar = true;
      break;
      case '\"':
        if (!escapeChar) {
          done = true;
        } else {
          str += byte;
        }
      break;
      default:
        str+=byte;
      break;
    }
    if (!len) done = true;
  }
  len -= readBytes;
  return str;
}

void JSON_Val::dump(std::string &str) {
  switch (m_type) {
    case JSON_COMPOUND:
      str += "{\n";
      {
        bool first = true;
        for (auto val : *m_value.compoundVal) {          
          if (!first) {
            str += ",\n";
          }
          first = false;
          val.second->dump(str);
        }
      }
      str += "}\n";
    break;
    case JSON_LIST:
      if (m_name != "") {
        str += "\"" + m_name + "\": ";
      }
      str += "[\n";
      {
        bool first = true;
        for (JSON_Val *val : *m_value.listVal) {
          if (!first) {
            str += ",\n";
          }
          first = false;
          val->dump(str);
        }
      }
      str += "],\n";
    break;
    case JSON_INT:
      if (m_name != "") {
        str += "\"" + m_name + "\": ";
      }
      str += dtos(m_value.intVal);
    break;
    case JSON_STRING:
      if (m_name != "") {
        str += "\"" + m_name + "\": ";
      }
      str += "\"" + *m_value.stringVal + "\"";
    break;
    case JSON_BOOL:
      if (m_name != "") {
        str += "\"" + m_name + "\": ";
      }
      str += m_value.boolVal ? "true": "false";
    break;
  }
}

JSON_Val::JSON_Val(uint8_t* buf, int32_t& len, JSON_type type) : m_type(type)
{
  bool readingString = false;
  bool readingArray = false;
  bool done = false;
  bool escapeChar = false;
  bool nameRead = false;

  std::string temp;
  int32_t readBytes = 0;
  int32_t len_temp;

  switch (m_type) {
    case JSON_COMPOUND:
      m_value.compoundVal = new std::map<std::string, JSON_Val*>();
      while (!done) {
        char byte = buf[0];    
        buf++;
        readBytes++;

        switch (byte) {
          case '\"':
            if (!nameRead) {
              len_temp = len-readBytes;
              temp = readString(buf, len_temp);              
              buf += len-readBytes - len_temp;
              readBytes += len-readBytes - len_temp;
            } else {
              len_temp = len-readBytes;
              std::string value = readString(buf, len_temp);
              (*m_value.compoundVal)[temp] = new JSON_Val(temp, value);
              buf += len-readBytes - len_temp;
              readBytes += len-readBytes - len_temp;
              nameRead = false;
            }
          break;
          case ':':
            nameRead = true;
          break;
          case '[':
            if (nameRead) {
              len_temp = len-readBytes;
              (*m_value.compoundVal)[temp] = new JSON_Val(buf, len_temp, JSON_LIST);
              (*m_value.compoundVal)[temp]->setName(temp);
              buf += len-readBytes - len_temp;
              readBytes += len-readBytes - len_temp;
              nameRead = false;
            }
          break;
          case '}':
           done = true;
          break;
          case '{':
            if (nameRead) {
              len_temp = len-readBytes;
              (*m_value.compoundVal)[temp] = new JSON_Val(buf, len_temp, JSON_COMPOUND);
              buf += len-readBytes - len_temp;
              readBytes += len-readBytes - len_temp;
              nameRead = false;
            }
          break;
          case ',':
            // next value
          break;
          default:
            if (!len) done = true;
            else {
              if (nameRead && byte >= '0' && byte <= '9') {
                std::string val;              
                while (byte >= '0' && byte <= '9') {
                  val += byte;  
                  byte = buf[0];    
                  buf++;
                  readBytes++;
                }
                (*m_value.compoundVal)[temp] = new JSON_Val(temp, atoi(val.c_str()));
                buf--;
                readBytes--;
                nameRead = false;
              } else if (nameRead && strncmp((char*)(buf-1), "true", 4) == 0) {
                buf+=3;
                readBytes += 3;
                (*m_value.compoundVal)[temp] = new JSON_Val(temp, true);
                nameRead = false;
              }
              else if (nameRead && strncmp((char*)(buf-1), "false", 5) == 0) {
                buf+=4;
                readBytes += 4;
                (*m_value.compoundVal)[temp] = new JSON_Val(temp, false);
                nameRead = false;
              }
            }
        }
      }
    break;
    case JSON_LIST:
      m_value.listVal = new std::vector<JSON_Val*>();
      while (!done) {
        char byte = buf[0];    
        buf++;
        readBytes++;

        switch (byte) {
          case '\"':
            len_temp = len-readBytes;
            temp = readString(buf, len_temp);
            m_value.listVal->push_back(new JSON_Val("", temp));
            buf += len-readBytes - len_temp;
            readBytes += len-readBytes - len_temp;
          break;
          case '[':
            len_temp = len-readBytes;
            m_value.listVal->push_back(new JSON_Val(buf, len_temp, JSON_LIST));
            buf += len-readBytes - len_temp;
            readBytes += len-readBytes - len_temp;
          break;
          case ']':
            done = true;
          break;
          case '{':
            len_temp = len-readBytes;
            m_value.listVal->push_back(new JSON_Val(buf, len_temp, JSON_COMPOUND));
            buf += len-readBytes - len_temp;
            readBytes += len-readBytes - len_temp;
          break;
      
        }
      }
    break;
  }
  len -= readBytes;
}
