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

enum
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

struct NBT_value
{
  uint8 type;
  std::string name;
  void *value;
};

struct NBT_list
{
  std::string name;
  char tagId;
  int length;
  void **items;
};

struct NBT_byte_array
{
  std::string name;
  int length;
  uint8 *data;
};

struct NBT_struct
{
  uint8 *blocks;
  uint8 *data;
  uint8 *blocklight;
  uint8 *skylight;
  sint16 x;
  sint16 z;

  std::string name;
  std::vector<NBT_value> values;
  std::vector<NBT_list> lists;
  std::vector<NBT_byte_array> byte_arrays;
  std::vector<NBT_struct> compounds;
};

//Data reading
int TAG_Byte(uint8 *input, char *output);
int TAG_Short(uint8 *input, int *output);
int TAG_Int(uint8 *input, int *output);
int TAG_Long(uint8 *input, sint64 *output);
int TAG_Float(uint8 *input, float *output);
int TAG_Double(uint8 *input, double *output);
int TAG_String(uint8 *input, std::string *output);

int TAG_Byte_Array(uint8 *input, NBT_byte_array *output);
int TAG_List(uint8 *input, NBT_list *output);
int TAG_Compound(uint8 *input, NBT_struct *output, bool start = false);

//Get data from struct
uint8 *get_NBT_pointer(NBT_struct *input, std::string TAG);
//template <typename customType>
//inline bool get_NBT_value(NBT_struct *input, std::string TAG, customType *value);
template <typename customType>
bool get_NBT_value(NBT_struct *input, std::string TAG, customType *value)
{
  for(unsigned i = 0; i < input->values.size(); i++)
  {
    if(input->values[i].name == TAG)
    {
      *value = *(customType *)input->values[i].value;
      return true;
    }
  }

  for(unsigned j = 0; j < input->compounds.size(); j++)
  {
    return get_NBT_value(&input->compounds[j], TAG, value);
  }
  return false;
}

NBT_list *get_NBT_list(NBT_struct *input, std::string TAG);

int dumpNBT_string(uint8 *buffer, std::string name);
int dumpNBT_value(NBT_value *input, uint8 *buffer);
int dumpNBT_struct(NBT_struct *input, uint8 *buffer, bool list = false);
int dumpNBT_byte_array(NBT_byte_array *input, uint8 *buffer, bool list = false);
int dumpNBT_list(NBT_list *input, uint8 *buffer);

bool freeNBT_struct(NBT_struct *input);
bool freeNBT_list(NBT_list *input);

#endif
