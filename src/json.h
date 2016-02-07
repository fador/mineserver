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

#pragma once

#include <map>
#include <string>
#include <vector>

#include <stdint.h>


class JSON_Val
{
public:
  enum JSON_type
  {
    JSON_COMPOUND,
    JSON_LIST,
    JSON_INT,
    JSON_STRING,
    JSON_BOOL,
    JSON_FLOAT
  };

  JSON_Val(uint8_t* buf, int32_t& len, JSON_type type = JSON_COMPOUND);

  std::string readString(uint8_t* buf, int32_t& len);

  JSON_Val(const std::string& name, const std::string& val) {
    m_type = JSON_STRING;
    m_name = name;
    m_value.stringVal = new std::string(val);
  }

  JSON_Val(const std::string& name, int32_t val) {
    m_type = JSON_INT;
    m_name = name;
    m_value.intVal = val;
  }

  JSON_Val(const std::string& name, double val) {
    m_type = JSON_FLOAT;
    m_name = name;
    m_value.floatVal = val;
  }

  JSON_Val(const std::string& name, bool val) {
    m_type = JSON_BOOL;
    m_name = name;
    m_value.boolVal = val;
  }

  ~JSON_Val() {
    switch (m_type) {
      case JSON_COMPOUND:
        for (auto val : *m_value.compoundVal)
          delete val.second;
        delete m_value.compoundVal;
      break;
      case JSON_LIST:
        for (JSON_Val *val : *m_value.listVal)
          delete val;
        delete m_value.listVal;
      break;
      case JSON_STRING:
        delete m_value.stringVal;
      break;
    }
  }
  
  JSON_Val* operator[](const std::string& index) {
      if (m_type != JSON_COMPOUND) {
        return nullptr;
      }

      if (!m_value.compoundVal->count(index)) {
        return nullptr;
      }
      return (*m_value.compoundVal)[index];
  }

  void Insert(const std::string& str, JSON_Val* val) {
    if (m_type != JSON_COMPOUND) {
      return;
    }

    if (m_value.compoundVal == 0) {
      m_value.compoundVal = new std::map<std::string, JSON_Val*>();
    }

    if ((*m_value.compoundVal)[str] != 0) {
      delete(*m_value.compoundVal)[str];
    }
    (*m_value.compoundVal)[str] = val;
  }

  JSON_Val& operator =(int32_t val) {
    m_type = JSON_INT;
    m_value.intVal = val;
    return *this;
  }

  JSON_Val& operator =(const std::string& val) {
    m_type = JSON_INT;
    if (m_value.stringVal != nullptr) delete m_value.stringVal;
    m_value.stringVal = new std::string(val);
    return *this;
  }

  std::string* getString() {
    return m_value.stringVal;
  }

  JSON_type getType() {
    return m_type;
  }

  std::vector<JSON_Val*>* getList() {
    if (m_type != JSON_LIST) {
        return nullptr;
      }
    if (m_value.listVal == nullptr) {
      m_value.listVal = new std::vector<JSON_Val*>();
    }
    return m_value.listVal;
  }

  void dump(std::string &str);

  void setName(std::string str) {
    m_name = str;
  }

private:
  JSON_type m_type;
  std::string m_name;
  union
  {
    bool boolVal;
    int32_t intVal;
    double floatVal;
    std::string* stringVal;
    std::vector<JSON_Val*>* listVal;
    std::map<std::string, JSON_Val*>* compoundVal;
  } m_value;
};
