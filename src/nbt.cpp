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
#endif

#include <zlib.h>

#include "tools.h"
#include "logger.h"
#include "mineserver.h"
#include "nbt.h"
#include "constants.h"



//NBT level file reading
//More info: http://www.minecraft.net/docs/NBT.txt

NBT_Value::NBT_Value(eTAG_Type type, eTAG_Type listType) : m_type(type)
{
  memset(&m_value, 0, sizeof(m_value));
  if (type == TAG_LIST)
  {
    m_value.listVal.type = listType;
  }
}

NBT_Value::NBT_Value(int8_t value) : m_type(TAG_BYTE)
{
  m_value.byteVal = value;
}

NBT_Value::NBT_Value(int16_t value) : m_type(TAG_SHORT)
{
  m_value.shortVal = value;
}

NBT_Value::NBT_Value(int32_t value) : m_type(TAG_INT)
{
  m_value.intVal = value;
}

NBT_Value::NBT_Value(int64_t value) : m_type(TAG_LONG)
{
  m_value.longVal = value;
}

NBT_Value::NBT_Value(float value) : m_type(TAG_FLOAT)
{
  m_value.floatVal = value;
}

NBT_Value::NBT_Value(double value) : m_type(TAG_DOUBLE)
{
  m_value.doubleVal = value;
}

NBT_Value::NBT_Value(uint8_t* buf, int32_t len) : m_type(TAG_BYTE_ARRAY)
{
  m_value.byteArrayVal = new std::vector<uint8_t>(buf, buf + len);
}

NBT_Value::NBT_Value(std::vector<uint8_t> const& bytes) : m_type(TAG_BYTE_ARRAY)
{
  m_value.byteArrayVal = new std::vector<uint8_t>(bytes);
}

NBT_Value::NBT_Value(const std::string& str) : m_type(TAG_STRING)
{
  m_value.stringVal = new std::string(str);
}

NBT_Value::NBT_Value(eTAG_Type type, uint8_t** buf, int& remaining) : m_type(type)
{
  switch (m_type)
  {
  case TAG_BYTE:
    remaining--;
    if (remaining >= 0)
    {
      m_value.byteVal = **buf;
      (*buf)++;
    }
    break;
  case TAG_SHORT:
    remaining -= 2;
    if (remaining >= 0)
    {
      m_value.shortVal = getSint16(*buf);
      *buf += 2;
    }
    break;
  case TAG_INT:
    remaining -= 4;
    if (remaining >= 0)
    {
      m_value.intVal = getSint32(*buf);
      *buf += 4;
    }
    break;
  case TAG_LONG:
    remaining -= 8;
    if (remaining >= 0)
    {
      m_value.longVal = getSint64(*buf);
      *buf += 8;
    }
    break;
  case TAG_FLOAT:
    remaining -= 4;
    if (remaining >= 0)
    {
      m_value.floatVal = getFloat(*buf);
      *buf += 4;
    }
    break;
  case TAG_DOUBLE:
    remaining -= 8;
    if (remaining >= 0)
    {
      m_value.doubleVal = getDouble(*buf);
      *buf += 8;
    }
    break;
  case TAG_BYTE_ARRAY:
    remaining -= 4;
    if (remaining >= 0)
    {
      int32_t bufLen = getSint32(*buf);
      remaining -= bufLen;
      *buf += 4;
      if (remaining >= 0)
      {
        m_value.byteArrayVal = new std::vector<uint8_t>();
        m_value.byteArrayVal->assign(*buf, (*buf) + bufLen);
        *buf += bufLen;
      }
    }
    break;
  case TAG_STRING:
    remaining -= 2;
    if (remaining >= 0)
    {
      int16_t stringLen = getSint16(*buf);
      remaining -= stringLen;
      *buf += 2;
      if (remaining >= 0)
      {
        m_value.stringVal = new std::string((char*)*buf, stringLen);
        *buf += stringLen;
      }
    }
    break;
  case TAG_LIST:
    remaining -= 5;
    if (remaining >= 0)
    {
      int8_t type = **buf;
      (*buf)++;
      m_value.listVal.type = (eTAG_Type)type;
      int32_t count = getSint32(*buf);
      *buf += 4;
      m_value.listVal.data = new std::vector<NBT_Value*>();
      if (count)
      {
        m_value.listVal.data->resize(count);
      }

      for (int i = 0; i < count; i++)
      {
        (*m_value.listVal.data)[i] = new NBT_Value((eTAG_Type)type, buf, remaining);
      }
    }
    break;
  case TAG_COMPOUND:
    m_value.compoundVal = new std::map<std::string, NBT_Value*>();
    while (remaining > 0)
    {
      remaining--;
      int8_t type = **buf;
      (*buf)++;
      if (type == TAG_END)
      {
        break;
      }

      remaining -= 2;
      if (remaining <= 0)
      {
        break;
      }

      int16_t stringLen = getSint16(*buf);
      *buf += 2;

      remaining -= stringLen;

      if (remaining <= 0)
      {
        break;
      }

      std::string key((char*)*buf, stringLen);
      *buf += stringLen;

      (*m_value.compoundVal)[key] = new NBT_Value((eTAG_Type)type, buf, remaining);
    }
    break;
  case TAG_END:
    break;
  }
}

NBT_Value::~NBT_Value()
{
  cleanup();
}

NBT_Value* NBT_Value::operator[](const std::string& index)
{
  if (m_type != TAG_COMPOUND)
  {
    return NULL;
  }

  if (!m_value.compoundVal->count(index))
  {
    return NULL;
  }

  return (*m_value.compoundVal)[index];
}

NBT_Value* NBT_Value::operator[](const char* index)
{
  if (m_type != TAG_COMPOUND)
  {
    return NULL;
  }

  std::string stdIndex(index, strlen(index));

  if (!m_value.compoundVal->count(stdIndex))
  {
    return NULL;
  }

  return (*m_value.compoundVal)[stdIndex];
}

void NBT_Value::Insert(const std::string& str, NBT_Value* val)
{
  if (m_type != NBT_Value::TAG_COMPOUND)
  {
    return;
  }

  if (m_value.compoundVal == 0)
  {
    m_value.compoundVal = new std::map<std::string, NBT_Value*>();
  }

  if ((*m_value.compoundVal)[str] != 0)
  {
    delete(*m_value.compoundVal)[str];
  }

  (*m_value.compoundVal)[str] = val;
}

NBT_Value::operator int8_t()
{
  if (!this || m_type != TAG_BYTE)
  {
    return 0;
  }

  return m_value.byteVal;
}

NBT_Value::operator int16_t()
{
  if (!this || m_type != TAG_SHORT)
  {
    return 0;
  }

  return m_value.shortVal;
}

NBT_Value::operator int32_t()
{
  if (!this || m_type != TAG_INT)
  {
    return 0;
  }

  return m_value.intVal;
}

NBT_Value::operator int64_t()
{
  if (!this || m_type != TAG_LONG)
  {
    return 0;
  }

  return m_value.longVal;
}

NBT_Value::operator float()
{
  if (!this || m_type != TAG_FLOAT)
  {
    return 0;
  }

  return m_value.floatVal;
}

NBT_Value::operator double()
{
  if (!this || m_type != TAG_DOUBLE)
  {
    return 0;
  }

  return m_value.doubleVal;
}

NBT_Value& NBT_Value::operator =(int8_t val)
{
  cleanup();
  m_type = TAG_BYTE;
  m_value.byteVal = val;
  return *this;
}

NBT_Value& NBT_Value::operator =(int16_t val)
{
  cleanup();
  m_type = TAG_SHORT;
  m_value.shortVal = val;
  return *this;
}

NBT_Value& NBT_Value::operator =(int32_t val)
{
  cleanup();
  m_type = TAG_INT;
  m_value.intVal = val;
  return *this;
}

NBT_Value& NBT_Value::operator =(int64_t val)
{
  cleanup();
  m_type = TAG_LONG;
  m_value.longVal = val;
  return *this;
}

NBT_Value& NBT_Value::operator =(float val)
{
  cleanup();
  m_type = TAG_FLOAT;
  m_value.floatVal = val;
  return *this;
}

NBT_Value& NBT_Value::operator =(double val)
{
  cleanup();
  m_type = TAG_DOUBLE;
  m_value.doubleVal = val;
  return *this;
}

std::vector<uint8_t> *NBT_Value::GetByteArray()
{
  if (m_type != TAG_BYTE_ARRAY)
  {
    return NULL;
  }
  if (m_value.byteArrayVal == NULL)
  {
    m_value.byteArrayVal = new std::vector<uint8_t>();
  }
  return m_value.byteArrayVal;
}


std::string* NBT_Value::GetString()
{
  if (m_type != TAG_STRING)
  {
    return NULL;
  }
  if (m_value.stringVal == NULL)
  {
    m_value.stringVal = new std::string();
  }
  return m_value.stringVal;
}

NBT_Value::eTAG_Type NBT_Value::GetListType()
{
  if (m_type != TAG_LIST)
  {
    return TAG_END;
  }
  return m_value.listVal.type;
}

std::vector<NBT_Value*> *NBT_Value::GetList()
{
  if (m_type != TAG_LIST)
  {
    return NULL;
  }
  if (m_value.listVal.data == NULL)
  {
    m_value.listVal.data = new std::vector<NBT_Value*>();
  }
  return m_value.listVal.data;
}


void NBT_Value::SetType(eTAG_Type type, eTAG_Type listType)
{
  cleanup();
  m_type = type;

  if (m_type == TAG_LIST)
  {
    m_value.listVal.type = listType;
  }
}

NBT_Value::eTAG_Type NBT_Value::GetType()
{
  return m_type;
}

void NBT_Value::cleanup()
{
  if (m_type == TAG_STRING)
  {
    delete m_value.stringVal;
  }
  if (m_type == TAG_BYTE_ARRAY)
  {
    delete m_value.byteArrayVal;
  }
  if (m_type == TAG_LIST)
  {
    if (m_value.listVal.data != NULL)
    {
      std::vector<NBT_Value*>::iterator iter = m_value.listVal.data->begin(), end = m_value.listVal.data->end();
      for (; iter != end ; iter++)
      {
        delete *iter;
      }
      delete m_value.listVal.data;
    }
  }
  if (m_type == TAG_COMPOUND)
  {
    if (m_value.compoundVal != NULL)
    {
      std::map<std::string, NBT_Value*>::iterator iter = m_value.compoundVal->begin(), end = m_value.compoundVal->end();
      for (; iter != end ; iter++)
      {
        delete iter->second;
      }

      delete m_value.compoundVal;
    }
  }

  memset(&m_value, 0, sizeof(m_value));
  m_type = TAG_END;
}

NBT_Value* NBT_Value::LoadFromFile(const std::string& filename)
{
  FILE* fp = fopen(filename.c_str(), "rb");
  if (fp == NULL)
  {
    return NULL;
  }
  fseek(fp, -4, SEEK_END);
  uint32_t uncompressedSize = 0;
  fread(&uncompressedSize, 4, 1, fp);
  fclose(fp);

  //Do endian testing!
  int32_t endiantestint = 1;
  int8_t* endiantestchar = (int8_t*)&endiantestint;
  if (*endiantestchar != 1)
  {
    //Swap order
    int uncompressedSizeOld = uncompressedSize;
    uint8_t* newpointer = reinterpret_cast<uint8_t*>(&uncompressedSize);
    uint8_t* oldpointer = reinterpret_cast<uint8_t*>(&uncompressedSizeOld);
    newpointer[0] = oldpointer[3];
    newpointer[1] = oldpointer[2];
    newpointer[2] = oldpointer[1];
    newpointer[3] = oldpointer[0];
  }

  if (uncompressedSize == 0)
  {
    Mineserver::get()->logger()->log(LogType::LOG_WARNING, "NBT", "Unable to determine uncompressed size of " + filename);
    uncompressedSize = ALLOCATE_NBTFILE;
  }

  uint8_t* uncompressedData = new uint8_t[uncompressedSize];
  gzFile nbtFile = gzopen(filename.c_str(), "rb");
  if (nbtFile == NULL)
  {
    delete[] uncompressedData;
    return NULL;
  }
  gzread(nbtFile, uncompressedData, uncompressedSize);
  gzclose(nbtFile);

  uint8_t* ptr = uncompressedData + 3; // Jump blank compound
  int remaining = uncompressedSize;

  NBT_Value* root = new NBT_Value(TAG_COMPOUND, &ptr, remaining);

  delete[] uncompressedData;

  return root;
}

void NBT_Value::SaveToFile(const std::string& filename)
{
  std::vector<uint8_t> buffer;

  // Blank compound tag
  buffer.push_back(TAG_COMPOUND);
  buffer.push_back(0);
  buffer.push_back(0);

  Write(buffer);

  buffer.push_back(0);
  buffer.push_back(0);
  buffer.push_back(0);

  gzFile nbtFile = gzopen(filename.c_str(), "wb");
  gzwrite(nbtFile, &buffer[0], buffer.size());
  gzclose(nbtFile);
}

void NBT_Value::Write(std::vector<uint8_t> &buffer)
{
  int storeAt = buffer.size();;
  switch (m_type)
  {
  case TAG_BYTE:
    buffer.push_back(m_value.byteVal);
    break;
  case TAG_SHORT:
    buffer.resize(storeAt + 2);
    putSint16(&buffer[storeAt], m_value.shortVal);
    break;
  case TAG_INT:
    buffer.resize(storeAt + 4);
    putSint32(&buffer[storeAt], m_value.intVal);
    break;
  case TAG_LONG:
    buffer.resize(storeAt + 8);
    putSint64(&buffer[storeAt], m_value.longVal);
    break;
  case TAG_FLOAT:
    buffer.resize(storeAt + 4);
    putFloat(&buffer[storeAt], m_value.floatVal);
    break;
  case TAG_DOUBLE:
    buffer.resize(storeAt + 8);
    putDouble(&buffer[storeAt], m_value.doubleVal);
    break;
  case TAG_BYTE_ARRAY:
  {
    int arraySize = m_value.byteArrayVal ? m_value.byteArrayVal->size() : 0;
    buffer.resize(storeAt + 4 + arraySize);
    putSint32(&buffer[storeAt], arraySize);
    storeAt += 4;
    if (arraySize)
    {
      memcpy(&buffer[storeAt], &(*m_value.byteArrayVal)[0], arraySize);
    }
    break;
  }
  case TAG_STRING:
  {
    int stringLen = m_value.stringVal ? m_value.stringVal->size() : 0;
    buffer.resize(storeAt + 2 + stringLen);
    putSint16(&buffer[storeAt], (int16_t)stringLen);
    storeAt += 2;
    if (stringLen > 0)
    {
      memcpy(&buffer[storeAt], m_value.stringVal->c_str(), stringLen);
    }
    break;
  }
  case TAG_LIST:
  {
    buffer.resize(storeAt + 5);
    int listCount = m_value.listVal.data ? m_value.listVal.data->size() : 0;
    buffer[storeAt] = m_value.listVal.type;
    storeAt++;
    putSint32(&buffer[storeAt], listCount);
    for (int i = 0; i < listCount; i++)
    {
      (*m_value.listVal.data)[i]->Write(buffer);
    }
    break;
  }
  case TAG_COMPOUND:
  {
    int compoundCount = m_value.compoundVal ? m_value.compoundVal->size() : 0;
    if (compoundCount)
    {
      std::map<std::string, NBT_Value*>::iterator iter = m_value.compoundVal->begin(), end = m_value.compoundVal->end();
      for (; iter != end; iter++)
      {
        const std::string& key = iter->first;
        int keySize = key.size();
        NBT_Value* val = iter->second;
        int curPos = buffer.size();
        buffer.resize(curPos + 3 + keySize);
        buffer[curPos] = (uint8_t)val->GetType();
        curPos++;
        putSint16(&buffer[curPos], keySize);
        curPos += 2;
        if (keySize)
        {
          memcpy(&buffer[curPos], key.c_str(), keySize);
        }
        val->Write(buffer);
      }
    }
    buffer.push_back(TAG_END);
    break;
  }
  case TAG_END:
    break; //for completeness
  }
}

void NBT_Value::Dump(std::string& data, const std::string& name, int tabs)
{
  std::string tabPrefix = "";
  for (int i = 0; i < tabs; i++)
  {
    tabPrefix += "  ";
  }

  switch (m_type)
  {
  case TAG_END:
    data += tabPrefix + "TAG_End(\"" + name + "\")\n";
    break;
  case TAG_BYTE:
    data += tabPrefix + "TAG_Byte(\"" + name + "\"): " + dtos((int)m_value.byteVal) + "\n";
    break;
  case TAG_SHORT:
    data += tabPrefix + "TAG_Short(\"" + name + "\"): " + dtos(m_value.shortVal) + "\n";
    break;
  case TAG_INT:
    data += tabPrefix + "TAG_Int(\"" + name + "\"): " + dtos(m_value.intVal) + "\n";
    break;
  case TAG_LONG:
    data += tabPrefix + "TAG_Long(\"" + name + "\"): " + dtos(m_value.longVal) + "\n";
    break;
  case TAG_FLOAT:
    data += tabPrefix + "TAG_Float(\"" + name + "\"): " + dtos(m_value.floatVal) + "\n";
    break;
  case TAG_DOUBLE:
    data += tabPrefix + "TAG_Double(\"" + name + "\"): " + dtos(m_value.doubleVal) + "\n";
    break;
  case TAG_BYTE_ARRAY:
    data += tabPrefix + "TAG_Byte_Array(\"" + name + "\"): \n";
    if (m_value.byteArrayVal != NULL)
    {
      data += tabPrefix + dtos(m_value.byteArrayVal->size()) + " bytes\n";
    }
    else
    {
      data += tabPrefix + "0 bytes\n";
    }
    break;
  case TAG_STRING:
    data += tabPrefix + "TAG_String(\"" + name + "\"): \n";
    if (m_value.stringVal != NULL)
    {
      data += tabPrefix + *m_value.stringVal + "\n";
    }
    else
    {
      data += tabPrefix + "\n";
    }
    break;
  case TAG_LIST:
    data += tabPrefix + "TAG_List(\"" + name + "\"): Type " + dtos(m_value.listVal.type) + "\n";
    if (m_value.listVal.data != NULL)
    {
      std::vector<NBT_Value*>::iterator iter = m_value.listVal.data->begin(), end = m_value.listVal.data->end();
      for (; iter != end ; iter++)
      {
        (*iter)->Dump(data, std::string(""), tabs + 1);
      }
    }
    break;
  case TAG_COMPOUND:
    data += tabPrefix + "TAG_Compound(\"" + name + "\"):\n";
    if (m_value.compoundVal != NULL)
    {
      std::map<std::string, NBT_Value*>::iterator iter = m_value.compoundVal->begin(), end = m_value.compoundVal->end();
      for (; iter != end; iter++)
      {
        iter->second->Dump(data, iter->first, tabs + 1);
      }
    }
    break;
  default:
    data += tabPrefix + "Invalid TAG:" + dtos(m_type) + "\n";
  }
}
