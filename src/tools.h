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

#ifndef _TOOLS_H
#define _TOOLS_H

#include <stdint.h>
#include <string>

#ifdef WIN32
  #include <winsock2.h>
#else
  #include <arpa/inet.h>
#endif

typedef uint8_t uint8;
typedef int8_t sint8;
typedef uint16_t uint16;
typedef int16_t sint16;
typedef uint32_t uint32;
typedef int32_t sint32;
typedef uint64_t uint64;
typedef int64_t sint64;

void putSint64(uint8 *buf, sint64 value);
void putSint32(uint8 *buf, sint32 value);
void putSint16(uint8 *buf, short value);
void putDouble(uint8 *buf, double value);
void putFloat(uint8 *buf, float value);

sint64 getSint64(uint8 *buf);
double getDouble(uint8 *buf);
float  getFloat(uint8 *buf);
sint32 getSint32(uint8 *buf);
sint32 getSint16(uint8 *buf);

void my_itoa(int value, std::string &buf, int base);
std::string base36_encode(int value);
std::string strToLower(std::string temp);

std::string dtos(double n);

inline uint64 ntohll(uint64 v)
{
  if(htons(1) == 1) // check if already big-endian
        return v;
  return (uint64)ntohl(v & 0x00000000ffffffff) << 32 | (uint64)ntohl( (v >> 32) & 0x00000000ffffffff);
}

//Converts block-coordinates to chunk coordinate
inline sint32 blockToChunk(sint32 value)
{
  return value>>4;//(value < 0) ? (((value+1)/16)-1) : (value/16);
}

//Converts absolute block-coordinates to chunk-block-coordinates
inline sint32 blockToChunkBlock(sint32 value)
{
  return value&15;//(value < 0) ? (15+((value+1)%16)) : (value%16);
}

#endif
