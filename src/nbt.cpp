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

#include <iostream>
#include <fstream>
#include <deque>
#include <vector>
#include <string>
#ifdef WIN32
  #include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <event.h>

#endif

#include "tools.h"
#include "nbt.h"
#include "map.h"

//NBT level file reading
//More info: http://www.minecraft.net/docs/NBT.txt

int TAG_Byte(uint8 *input, char *output)
{
  *output = input[0];
  return 1;
}

int TAG_Short(uint8 *input, int *output)
{
  *output = getSint16(input);
  return 2;
}

int TAG_Int(uint8 *input, int *output)
{
  *output = getSint32(input);
  return 4;
}

int TAG_Long(uint8 *input, long long *output)
{
  *output = getSint64(input);
  return 8;
}

int TAG_Float(uint8 *input, float *output)
{
  *output = getFloat(input);
  return 4;
}

int TAG_Double(uint8 *input, double *output)
{
  *output = getDouble(input);
  return 8;
}

int TAG_String(uint8 *input, std::string *output)
{
  int strLen = getUint16(&input[0]);
  *output = "";

  for(int i = 0; i < strLen; i++)
  {
    *output += input[i+2];
  }
  return strLen+2;
}

int TAG_Byte_Array(uint8 *input, NBT_byte_array *output)
{
  int curpos = 0;
  curpos      += TAG_Int(&input[curpos], &output->length);
  output->data = new uint8[output->length];
  memcpy(output->data, &input[curpos], output->length);
  curpos      += output->length;
  return curpos;
}

int TAG_List(uint8 *input, NBT_list *output)
{
  int curpos = 0;
  curpos += TAG_Byte(&input[curpos], &output->tagId);
  curpos += TAG_Int(&input[curpos], &output->length);

  //If zero length list
  if(!output->length)
    return curpos;


  switch(output->tagId)
  {
  case TAG_BYTE:
    output->items = (void **)new char *[output->length];
    for(int i = 0; i < output->length; i++)
    {
      output->items[i] = (void *)new char;
      curpos          += TAG_Byte(&input[curpos], (char *)output->items[i]);
    }
    break;

  case TAG_SHORT:
    output->items = (void **)new int *[output->length];
    for(int i = 0; i < output->length; i++)
    {
      output->items[i] = (void *)new int;
      curpos          += TAG_Short(&input[curpos], (int *)output->items[i]);
    }
    break;

  case TAG_INT:
    output->items = (void **)new int *[output->length];
    for(int i = 0; i < output->length; i++)
    {
      output->items[i] = (void *)new int;
      curpos          += TAG_Int(&input[curpos], (int *)output->items[i]);
    }
    break;

  case TAG_LONG:
    output->items = (void **)new long long *[output->length];
    for(int i = 0; i < output->length; i++)
    {
      output->items[i] = (void *)new long long;
      curpos          += TAG_Long(&input[curpos], (long long *)output->items[i]);
    }
    break;

  case TAG_FLOAT:
    output->items = (void **)new float *[output->length];
    for(int i = 0; i < output->length; i++)
    {
      output->items[i] = (void *)new float;
      curpos          += TAG_Float(&input[curpos], (float *)output->items[i]);
    }
    break;

  case TAG_DOUBLE:
    output->items = (void **)new double *[output->length];
    for(int i = 0; i < output->length; i++)
    {
      output->items[i] = (void *)new double;
      curpos          += TAG_Double(&input[curpos], (double *)output->items[i]);
    }
    break;

  case TAG_BYTE_ARRAY:
    output->items = (void **)new NBT_byte_array *[output->length];
    for(int i = 0; i < output->length; i++)
    {
      output->items[i] = (void *)new NBT_byte_array;
      curpos          += TAG_Byte_Array(&input[curpos], (NBT_byte_array *)output->items[i]);
    }
    break;

  case TAG_STRING:
    output->items = (void **)new std::string *[output->length];
    for(int i = 0; i < output->length; i++)
    {
      output->items[i] = (void *)new std::string;
      curpos          += TAG_String(&input[curpos], (std::string *)output->items[i]);
    }
    break;

  case TAG_LIST:
    output->items = (void **)new NBT_list *[output->length];
    for(int i = 0; i < output->length; i++)
    {
      output->items[i] = (void *)new NBT_list;
      curpos          += TAG_List(&input[curpos], (NBT_list *)output->items[i]);
    }
    break;

  case TAG_COMPOUND:
    output->items = (void **)new NBT_struct *[output->length];
    for(int i = 0; i < output->length; i++)
    {
      output->items[i] = (void *)new NBT_struct;
      curpos          += TAG_Compound(&input[curpos], (NBT_struct *)output->items[i]);
    }
    break;
  }

  return curpos;
}


int TAG_Compound(uint8 *input, NBT_struct *output, bool start)
{
  char tagType = 1;
  std::string name;
  int curpos   = 0;

  while(tagType != 0)
  {
    curpos += TAG_Byte(&input[curpos], &tagType);
    if(tagType == 0)
    {
      //std::cout << "TAG_end" << std::endl;
      return curpos;
    }
    curpos += TAG_String(&input[curpos], &name);
    //std::cout << "Name: " << name << std::endl;
    NBT_value value;
    value.type     = tagType;
    value.name     = name;
    NBT_byte_array bytearray;
    bytearray.name = name;
    NBT_list list;
    list.name      = name;
    NBT_struct compound;
    compound.name  = name;

    switch(tagType)
    {
    case TAG_BYTE:
      value.value = (void *)new char;
      curpos     += TAG_Byte(&input[curpos], (char *)value.value);
      output->values.push_back(value);
      break;

    case TAG_SHORT:
      value.value = (void *)new int;
      curpos     += TAG_Short(&input[curpos], (int *)value.value);
      output->values.push_back(value);
      break;

    case TAG_INT:
      value.value = (void *)new int;
      curpos     += TAG_Int(&input[curpos], (int *)value.value);
      output->values.push_back(value);
      break;

    case TAG_LONG:
      value.value = (void *)new long long;
      curpos     += TAG_Long(&input[curpos], (long long *)value.value);
      output->values.push_back(value);
      break;

    case TAG_FLOAT:
      value.value = (void *)new float;
      curpos     += TAG_Float(&input[curpos], (float *)value.value);
      output->values.push_back(value);
      break;

    case TAG_DOUBLE:
      value.value = (void *)new double;
      curpos     += TAG_Double(&input[curpos], (double *)value.value);
      output->values.push_back(value);
      break;

    case TAG_BYTE_ARRAY:
      curpos += TAG_Byte_Array(&input[curpos], &bytearray);
      //Special handling with lightmaps
      if(bytearray.length == 0)
      {
        //If zero sized lightmap, generate a new empty lightmap
        if(bytearray.name == "BlockLight" || bytearray.name == "SkyLight")
        {
          bytearray.length = 16*16*128/2;
          bytearray.data   = new uint8[bytearray.length];
          memset(bytearray.data, 0, bytearray.length);
        }
      }
      output->byte_arrays.push_back(bytearray);
      break;

    case TAG_STRING:
      value.value = (void *)new std::string;
      curpos     += TAG_String(&input[curpos], (std::string *)value.value);
      output->values.push_back(value);
      break;

    case TAG_LIST:
      curpos += TAG_List(&input[curpos], &list);
      output->lists.push_back(list);
      break;

    case TAG_COMPOUND:
      if(start)
        curpos += TAG_Compound(&input[curpos], output);
      else
      {
        curpos += TAG_Compound(&input[curpos], (NBT_struct *)&compound);
        output->compounds.push_back(compound);
      }
      break;
    }

    if(start)
      break;
  }

  return curpos;
}
/*
   template <typename customType>
   bool get_NBT_value(NBT_struct *input, std::string TAG, customType *value)
   {
   for(unsigned i=0;i<input->values.size();i++)
   {
    if(input->values[i].name==TAG)
    {
   *value=*(customType*)input->values[i].value;
      return true;
    }
   }

   for(unsigned j=0;j<input->compounds.size();j++)
   {
    return get_NBT_value(&input->compounds[j], TAG, value);
   }
   return false;
   }
 */

uint8 *get_NBT_pointer(NBT_struct *input, std::string TAG)
{
  uint8 *pointer;

  for(unsigned i = 0; i < input->byte_arrays.size(); i++)
  {
    if(input->byte_arrays[i].name == TAG)
      return input->byte_arrays[i].data;
  }

  for(unsigned j = 0; j < input->compounds.size(); j++)
  {
    pointer = get_NBT_pointer(&input->compounds[j], TAG);
    if(pointer != 0)
      return pointer;
  }

  return 0;
}

NBT_list *get_NBT_list(NBT_struct *input, std::string TAG)
{
  NBT_list *pointer;

  for(unsigned i = 0; i < input->lists.size(); i++)
  {
    if(input->lists[i].name == TAG)
      return &input->lists[i];
  }

  for(unsigned j = 0; j < input->compounds.size(); j++)
  {
    pointer = get_NBT_list(&input->compounds[j], TAG);
    if(pointer != 0)
      return pointer;
  }

  return 0;
}


int dumpNBT_string(uint8 *buffer, std::string name)
{
  int curpos = 0;
  putSint16(buffer, name.length());
  curpos += 2;

  for(unsigned int i = 0; i < name.length(); i++)
  {
    buffer[curpos] = name[i];
    curpos++;
  }

  return curpos;
}

int dumpNBT_value(NBT_value *input, uint8 *buffer)
{
  int curpos = 0;
  buffer[curpos] = input->type;
  curpos++;
  curpos        += dumpNBT_string(&buffer[curpos], input->name);

  switch(input->type)
  {
  case TAG_BYTE:
    buffer[curpos] = *(char *)input->value;
    curpos++;
    break;

  case TAG_SHORT:
    putSint16(&buffer[curpos], *(int *)input->value);
    curpos += 2;
    break;

  case TAG_INT:
    putSint32(&buffer[curpos], *(int *)input->value);
    curpos += 4;
    break;

  case TAG_LONG:
    putSint64(&buffer[curpos], *(long long *)input->value);
    curpos += 8;
    break;

  case TAG_FLOAT:
    putFloat(&buffer[curpos], *(float *)input->value);
    curpos += 4;
    break;

  case TAG_DOUBLE:
    putDouble(&buffer[curpos], *(double *)input->value);
    curpos += 8;
    break;

  case TAG_STRING:
    curpos += dumpNBT_string(&buffer[curpos], *(std::string *)input->value);
    break;
  }

  return curpos;
}

int dumpNBT_list(NBT_list *input, uint8 *buffer)
{
  int curpos = 0;
  buffer[curpos] = TAG_LIST;
  curpos++;
  curpos        += dumpNBT_string(&buffer[curpos], input->name);

  buffer[curpos] = input->tagId;
  curpos++;

  putSint32(&buffer[curpos], input->length);
  curpos += 4;
  //NBT_struct **structlist=(NBT_struct **)input->items;
  for(int i = 0; i < input->length; i++)
  {
    switch(input->tagId)
    {
    case TAG_BYTE:
      buffer[curpos] = *(char *)input->items[i];
      curpos++;
      break;

    case TAG_SHORT:
      putSint16(&buffer[curpos], *(int *)input->items[i]);
      curpos += 2;
      break;

    case TAG_INT:
      putSint32(&buffer[curpos], *(int *)input->items[i]);
      curpos += 4;
      break;

    case TAG_LONG:
      putSint64(&buffer[curpos], *(long long *)input->items[i]);
      curpos += 8;
      break;

    case TAG_FLOAT:
      putFloat(&buffer[curpos], *(float *)input->items[i]);
      curpos += 4;
      break;

    case TAG_DOUBLE:
      putDouble(&buffer[curpos], *(double *)input->items[i]);
      curpos += 8;
      break;

    case TAG_STRING:
      curpos += dumpNBT_string(&buffer[curpos], *(std::string *)input->items[i]);
      break;

    case TAG_BYTE_ARRAY:
      curpos += dumpNBT_byte_array((NBT_byte_array *)input->items[i], &buffer[curpos], true);
      break;

    case TAG_COMPOUND:
      curpos += dumpNBT_struct((NBT_struct *)input->items[i], &buffer[curpos], true);
      break;
    }
  }

  return curpos;
}

int dumpNBT_byte_array(NBT_byte_array *input, uint8 *buffer, bool list)
{
  int curpos = 0;

  if(!list)
  {
    buffer[curpos] = TAG_BYTE_ARRAY;
    curpos++;
    curpos        += dumpNBT_string(&buffer[curpos], input->name);
  }

  putSint32(&buffer[curpos], input->length);
  curpos += 4;
  memcpy(&buffer[curpos], input->data, input->length);
  curpos += input->length;

  return curpos;
}

int dumpNBT_struct(NBT_struct *input, uint8 *buffer, bool list)
{
  int curpos = 0;

  if(!list)
  {
    buffer[curpos] = TAG_COMPOUND;
    curpos++;
    curpos        += dumpNBT_string(&buffer[curpos], input->name);
  }

  //Dump all values
  for(unsigned int i = 0; i < input->values.size(); i++)
  {
    curpos += dumpNBT_value(&input->values[i], &buffer[curpos]);
  }

  //Dump byte arrays
  for(unsigned int i = 0; i < input->byte_arrays.size(); i++)
  {
    curpos += dumpNBT_byte_array(&input->byte_arrays[i], &buffer[curpos]);
  }

  //Dump lists
  for(unsigned int i = 0; i < input->lists.size(); i++)
  {
    curpos += dumpNBT_list(&input->lists[i], &buffer[curpos]);
  }

  //Dump compounds
  for(unsigned int i = 0; i < input->compounds.size(); i++)
  {
    curpos += dumpNBT_struct(&input->compounds[i], &buffer[curpos]);
  }

  buffer[curpos] = 0x00; //TAG_END
  curpos++;

  //Total size
  return curpos;
}

bool freeNBT_list(NBT_list *input)
{
  //Free lists
  if(!input->length)
    return false;


  switch(input->tagId)
  {
    case TAG_BYTE:
      for(int j = 0; j < input->length; j++)
      {
        delete (char *)input->items[j];
      }
      delete [] (char **)input->items;
      break;

  case TAG_SHORT:
    for(int j = 0; j < input->length; j++)
    {
      delete (int *)input->items[j];
    }
    delete[] (int **)input->items;
    break;

  case TAG_INT:
    for(int j = 0; j < input->length; j++)
    {
      delete (int *)input->items[j];
    }
    delete[] (int **)input->items;
    break;

  case TAG_LONG:
    for(int j = 0; j < input->length; j++)
    {
      delete (long long *)input->items[j];
    }
    delete[] (long long **)input->items;
    break;

  case TAG_FLOAT:
    for(int j = 0; j < input->length; j++)
    {
      delete (float *)input->items[j];
    }
    delete[] (float **)input->items;
    break;

  case TAG_DOUBLE:
    for(int j = 0; j < input->length; j++)
    {
      delete (double *)input->items[j];
    }
    delete[] (double **)input->items;
    break;

  case TAG_STRING:
    for(int j = 0; j < input->length; j++)
    {
      delete (std::string *)input->items[j];
    }
    delete[] (std::string **)input->items;
    break;

  case TAG_COMPOUND:
    for(int j = 0; j < input->length; j++)
    {
      freeNBT_struct((NBT_struct *)input->items[j]);
      delete (NBT_struct *)input->items[j];
    }
    delete[] (NBT_struct **)input->items;
    break;

  case TAG_BYTE_ARRAY:
    for(int j = 0; j < input->length; j++)
    {
      NBT_byte_array *temparray = (NBT_byte_array *)input->items[j];
      delete[] temparray->data;
      delete temparray;
    }
    delete[] (NBT_byte_array **)input->items;
    break;
  }

  return true;
}

bool freeNBT_struct(NBT_struct *input)
{
  //Free all values
  for(unsigned int i = 0; i < input->values.size(); i++)
  {
    switch(input->values[i].type)
    {
    case TAG_BYTE:
      delete (char *)input->values[i].value;
      break;

    case TAG_SHORT:
      delete (int *)input->values[i].value;
      break;

    case TAG_INT:
      delete (int *)input->values[i].value;
      break;

    case TAG_LONG:
      delete (long long *)input->values[i].value;
      break;

    case TAG_FLOAT:
      delete (float *)input->values[i].value;
      break;

    case TAG_DOUBLE:
      delete (double *)input->values[i].value;
      break;

    case TAG_STRING:
      std::string *temp_string = (std::string *)input->values[i].value;
      temp_string->clear();
      delete (std::string *)input->values[i].value;
      break;
    }
  }

  input->values.clear();

  //Free byte arrays
  for(unsigned int i = 0; i < input->byte_arrays.size(); i++)
  {
    delete[] input->byte_arrays[i].data;
  }

  for(unsigned int i = 0; i < input->lists.size(); i++)
  {
    freeNBT_list(&input->lists[i]);
  }

  input->byte_arrays.clear();



  input->lists.clear();

  //Free compounds
  for(unsigned int i = 0; i < input->compounds.size(); i++)
  {
    freeNBT_struct(&input->compounds[i]);
  }

  input->compounds.clear();

  return true;
}
