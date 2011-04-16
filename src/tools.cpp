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

#ifdef WIN32
#include <conio.h>
#include <WinSock2.h>
#else
#include <netinet/in.h>
#include <pwd.h>
#endif

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <string>
#include <cctype>

#include "tools.h"
// zomg
#include "logger.h"
#include "mineserver.h"

void putSint64(uint8_t* buf, int64_t value)
{
  uint64_t nval = ntohll(value);
  memcpy(buf, &nval, 8);
}

void putSint32(uint8_t* buf, int32_t value)
{
  uint32_t nval = htonl(value);
  memcpy(buf, &nval, 4);
}

void putSint16(uint8_t* buf, int16_t value)
{
  short value2 = htons(value);
  memcpy(buf, &value2, 2);
}

void putFloat(uint8_t* buf, float value)
{
  uint32_t nval;
  memcpy(&nval, &value, 4);
  nval = htonl(nval);
  memcpy(buf, &nval, 4);
}

void putDouble(uint8_t* buf, double value)
{
  uint64_t nval;
  memcpy(&nval, &value, 8);
  nval = ntohll(nval);
  memcpy(buf, &nval, 8);
}

double getDouble(uint8_t* buf)
{
  double val;
  uint64_t ival;
  memcpy(&ival, buf, 8);
  ival = ntohll(ival);
  memcpy(&val, &ival, 8);
  return val;
}

float getFloat(uint8_t* buf)
{
  float val;
  int ival = ntohl(*reinterpret_cast<const int32_t*>(buf));
  memcpy(&val, &ival, 4);
  return val;
}

int64_t getSint64(uint8_t* buf)
{
  int64_t val;
  memcpy(&val, buf, 8);
  val = ntohll(val);
  return val;
}

int32_t getSint32(uint8_t* buf)
{
  int val = ntohl(*reinterpret_cast<const int32_t*>(buf));
  return val;
}

int32_t getSint16(uint8_t* buf)
{
  short val = ntohs(*reinterpret_cast<const int16_t*>(buf));

  return val;
}

std::string base36_encode(int value)
{
  std::string output;
  my_itoa((int)abs(value), output, 36);
  if (value < 0)
  {
    output.insert(output.begin(), '-');
  }

  return output;
}

void my_itoa(int value, std::string& buf, int base)
{
  std::string hexarray("0123456789abcdefghijklmnopqrstuvwxyz");
  int i = 30;
  buf = "";

  if (!value)
  {
    buf = "0";
  }

  for (; value && i; --i, value /= base)
  {
    buf.insert(buf.begin(), (char)hexarray[value % base]);
  }
}

std::string strToLower(std::string temp)
{
  const int len = temp.length();

  for (int i = 0; i != len; ++i)
  {
    temp[i] = std::tolower(temp[i]);
  }

  return temp;
}

std::string dtos(double n)
{
  std::ostringstream result;
  result << n;
  return result.str();
}

std::string hash(std::string value)
{
  // Hash the player's name along with a secret to generate a unique hash for this player
  // Uses the DJB2 algorithm
  unsigned long hash = 5381;
  int c;

  char* cvalue = const_cast<char*>(value.c_str());

  while ((c = *cvalue++))
  {
    hash = ((hash << 5) + hash) + c;  /* hash * 33 + c */
  }

  std::ostringstream hashString;
  hashString << hash;

  return hashString.str();
}

std::string pathExpandUser(const std::string& path)
{
  std::string out;
  std::string user;
  size_t pos;

  // The behaviour of this function used to be undefined when path == "".
  // Please examine it and ensure it behaves as intended in that case.

  if (path.empty() || path[0] != '~')
  {
    return path;
  }

  // current config system supports only unix-style paths

  pos = path.find('/');

  if (pos == std::string::npos)
  {
    pos = path.length();
  }

  user = path.substr(1, pos - 1); // OK, we checked that path is non-empty.

  if (!user.empty())
  {
#ifdef WIN32
    LOG2(ERROR, "~user notation is not implemented for this platform :P");
    return path;
#else
    struct passwd* entry = getpwnam(user.c_str());
    if (!entry)
    {
      LOG2(ERROR, user + ": no such user!");
      return path;
    }

    out += entry->pw_dir;
#endif
  }
  else
  {
#ifdef WIN32
#define ENV_HOME "APPDATA"
#else
#define ENV_HOME "HOME"
#endif
    out.assign(getenv(ENV_HOME));
  }

  out += pos < path.length() ? path.substr(pos) : "/";

  return out;
}
