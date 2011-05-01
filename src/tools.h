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

#ifndef _TOOLS_H
#define _TOOLS_H

#include <stdint.h>
#include <cstdlib>
#include <string>

#ifdef WIN32
#include <Winsock2.h>
#else
#include <arpa/inet.h>
#endif

void putSint64(uint8_t* buf, int64_t value);
void putSint32(uint8_t* buf, int32_t value);
void putSint16(uint8_t* buf, short value);
void putDouble(uint8_t* buf, double value);
void putFloat(uint8_t* buf, float value);

int64_t getSint64(uint8_t* buf);
double getDouble(uint8_t* buf);
float  getFloat(uint8_t* buf);
int32_t getSint32(uint8_t* buf);
int32_t getSint16(uint8_t* buf);

void my_itoa(int value, std::string& buf, int base);
std::string base36_encode(int value);
std::string strToLower(std::string temp);

std::string dtos(double n);
std::string hash(std::string value);

#ifdef WIN32
#define PATH_SEPARATOR  '\\'
#else
#define PATH_SEPARATOR  '/'
#endif

std::string pathExpandUser(const std::string& path);

#ifndef WIN32
int kbhit();
#endif

inline uint64_t ntohll(uint64_t v)
{
	return (uint64_t)ntohl(v & 0x00000000ffffffff) << 32 | (uint64_t)ntohl((v >> 32) & 0x00000000ffffffff);
}

// host wstring to net wstring
inline std::wstring hsttonst(std::wstring v)
{
	std::wstring outBuffer = L"";
	for(int strItr=0;strItr<v.length();strItr++)
	{
		outBuffer+=(wchar_t)htons(v[strItr]);
	}

	return outBuffer;
}

// net wstring to host wstring
inline std::wstring nsttohst(std::wstring v)
{
	std::wstring outBuffer = L"";
	for(int strItr=0;strItr<v.length();strItr++)
	{
		outBuffer+=(wchar_t)ntohs(v[strItr]);
	}

	return outBuffer;
}

// std::wstring -> std::string
inline std::string wstos(const std::wstring &in)
{
	std::string out;
	out.assign(in.begin(), in.end());
	return out;
}

// std::string -> std::wstring
inline std::wstring stows(const std::string &in)
{
	std::wstring out;
	out.assign(in.begin(), in.end());
	return out;
}

//Converts block-coordinates to chunk coordinate
inline int32_t blockToChunk(int32_t value)
{
  return value >> 4; //(value < 0) ? (((value+1)/16)-1) : (value/16);
}

//Converts absolute block-coordinates to chunk-block-coordinates
inline int32_t blockToChunkBlock(int32_t value)
{
  return value & 15; //(value < 0) ? (15+((value+1)%16)) : (value%16);
}

inline int8_t angleToByte(float angle)
{
  return (int8_t)((angle / 360.f) * 256);
}

inline int getRandInt(int min, int max)
{
  return (rand() % ((max - min) + 1) + min);
}

inline double BetterRand()
{
  return (rand() * (1.0 / (RAND_MAX + 1.0)));
}

inline int getBetterRandInt(int min, int max)
{
  return (BetterRand() * (max - min)) + min;
}

#endif
