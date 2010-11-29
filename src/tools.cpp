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

#ifdef WIN32
  #include <conio.h>
  #include <WinSock2.h>
#else
#include <netinet/in.h>
#endif

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <string.h>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <string>
#include <cctype>

#include "tools.h"

void putSint64(uint8 *buf, sint64 value)
{
  uint64 nval = ntohll(value);
  memcpy(buf, &nval, 8);
}

void putSint32(uint8 *buf, sint32 value)
{
  uint32 nval = htonl(value);
  memcpy(buf, &nval, 4);
}

void putSint16(uint8 *buf, short value)
{
  short value2=htons(value);
  memcpy(buf, &value2, 2);
}

void putFloat(uint8 *buf, float value)
{
  uint32 nval;
  memcpy(&nval, &value, 4);
  nval = htonl(nval);
  memcpy(buf, &nval, 4);
}

void putDouble(uint8 *buf, double value)
{
  uint64 nval;
  memcpy(&nval, &value, 8);
  nval = ntohll(nval);
  memcpy(buf, &nval, 8);
}

double getDouble(uint8 *buf)
{
  double val;
  uint64 ival = *reinterpret_cast<const sint64*>(buf);
  ival = ntohll(ival);
  memcpy(&val, &ival, 8);
  return val;
}

float getFloat(uint8 *buf)
{
  float val;
  int ival = ntohl(*reinterpret_cast<const sint32*>(buf));
  memcpy(&val, &ival, 4);
  return val;
}

sint64 getSint64(uint8 *buf)
{
  sint64 val;
  val = *reinterpret_cast<const sint64*>(buf);
  val = ntohll(val);
  return val;
}

sint32 getSint32(uint8 *buf)
{
  int val = ntohl(*reinterpret_cast<const sint32*>(buf));
  return val;
}

sint32 getSint16(uint8 *buf)
{
  short val = ntohs(*reinterpret_cast<const sint16*>(buf));

  return val;
}

std::string base36_encode(int value)
{
  std::string output;
  my_itoa((int)abs(value), output, 36);
  if(value < 0)
    output.insert (output.begin(), '-');

  return output;
}

void my_itoa(int value, std::string &buf, int base)
{
  std::string hexarray("0123456789abcdefghijklmnopqrstuvwxyz");
  int i = 30;
  buf = "";

  if(!value)
    buf = "0";

  for(; value && i; --i, value /= base)
  {
    buf.insert(buf.begin(), (char)hexarray[value % base]);
  }
}

std::string strToLower(std::string temp)
{
  const int len = temp.length();

  for(int i = 0; i != len; ++i)
  {
    temp[i] = std::tolower(temp[i]);
  }

  return temp;
}

std::string dtos( double n )
{
  std::ostringstream result;
  result << n;
  return result.str();
}
