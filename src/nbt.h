/*
   Copyright (c) 2010, The Mineserver Project
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

#ifndef _NBT_H
#define _NBT_H

#include <string>
#include <map>
#include <vector>
#include <zlib.h>
#include "tools.h"

class NBT_Value
{
public:
  enum eTAG_Type
  {
    TAG_END        = 0,
    TAG_BYTE       = 1,
    TAG_SHORT      = 2,
    TAG_INT        = 3,
    TAG_LONG       = 4,
    TAG_FLOAT      = 5,
    TAG_DOUBLE     = 6,
    TAG_BYTE_ARRAY = 7,
    TAG_STRING     = 8,
    TAG_LIST       = 9,
    TAG_COMPOUND   = 10
  };

  NBT_Value(eTAG_Type type, eTAG_Type listType = TAG_END);
  NBT_Value(sint8 value);
  NBT_Value(sint16 value);
  NBT_Value(sint32 value);
  NBT_Value(sint64 value);
  NBT_Value(float value);
  NBT_Value(double value);

  NBT_Value(uint8 *buf, sint32 len);
  NBT_Value(const std::string &str);

  NBT_Value(eTAG_Type type, uint8 **buf, int &remaining);

  ~NBT_Value();

  NBT_Value *operator[](const std::string &index);
  NBT_Value *operator[](const char *index);

  void Insert(const std::string &str, NBT_Value *val);

  operator sint8();
  operator sint16();
  operator sint32();
  operator sint64();
  operator float();
  operator double();
  
  NBT_Value &operator =(sint8 val);
  NBT_Value &operator =(sint16 val);
  NBT_Value &operator =(sint32 val);
  NBT_Value &operator =(sint64 val);
  NBT_Value &operator =(float val);
  NBT_Value &operator =(double val);

  std::vector<uint8> *GetByteArray();
  std::string *GetString();
  eTAG_Type GetListType();
  std::vector<NBT_Value*> *GetList();

  void SetType(eTAG_Type type, eTAG_Type listType = TAG_END);

  eTAG_Type GetType();
  void cleanup();

  static NBT_Value * LoadFromFile(const std::string &filename);
  void SaveToFile(const std::string &filename);
  
  void Write(std::vector<uint8> &buffer);

  void Print(const std::string &name = std::string(""), int tabs=0);
private:
  eTAG_Type m_type;
  union
  {
    sint8 byteVal;
    sint16 shortVal;
    sint32 intVal;
    sint64 longVal;
    float floatVal;
    double doubleVal;
    std::string *stringVal;
    std::vector<uint8> *byteArrayVal;
    struct
    {
      eTAG_Type type;
      std::vector<NBT_Value*> *data;
    } listVal;
    std::map<std::string, NBT_Value*> *compoundVal;
  } m_value;
};

#endif
