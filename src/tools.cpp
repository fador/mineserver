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

#if defined(linux)
#include <unistd.h>
#include <libgen.h>
#include <wordexp.h>  // for wordexp
#include <sys/stat.h> // for mkdir
#include <climits>
#elif defined(WIN32)
#include <direct.h>
#define _WINSOCKAPI_ //Stops windows.h from including winsock.h
                     //Fixes errors I was having
#include <ShlObj.h>
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <cctype>

#include "tools.h"
#include "logger.h"
#include "mineserver.h"
#include "config.h"

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

std::string my_itoa(int value, int base)
{
  std::string ret;
  my_itoa(value, ret, base);
  return ret;
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

bool fileExists(const std::string& filename)
{
  std::ifstream i(filename.c_str());
  return i;
}

bool makeDirectory(const std::string& path)
{
#ifdef WIN32
  return _mkdir(path.c_str()) != -1;
#else
  return mkdir(path.c_str(), 0755) != -1;
#endif
}

std::string canonicalizePath(const std::string& pathname)
{
  // We assume that pathname is already a path name, with no file component!

#if defined(linux)

  wordexp_t exp_result;

  wordexp(pathname.c_str(), &exp_result, 0);

  char * d = strdup(exp_result.we_wordv[0]);

  wordfree(&exp_result);

  char * rp = new char[PATH_MAX];
  std::memset(rp, 0, PATH_MAX);
  realpath(d, rp);

  std::string res(rp);

  free(d);
  delete[] rp;

  return res;

#elif defined(WIN32)

  return pathname;

#else
  // Why is this else? what the fuck are we compiling on? - Justasic
  return pathname;

#endif

}

std::string relativeToAbsolute(const std::string &path)
{
  /// This is a very crude way to check if the path is relative.
  /// We must replace this by a more portable "pathIsRelative()" check.
  std::string pathname = path;
  if (!pathname.empty() && pathname[0] != PATH_SEPARATOR && pathname[0] != '~')
    pathname = ServerInstance->config()->config_path + PATH_SEPARATOR + pathname;

  pathname = canonicalizePath(pathname);

  return pathname;
}

std::string getHomeDir()
{

#if defined(linux)

  return canonicalizePath("~/.mineserver");

#elif defined(WIN32)

char szPath[MAX_PATH];
if(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath) == 0)
{
  std::string temp = szPath;
  temp.append("\\Mineserver");
  return temp;
}
else
{
  return "%APPDATA%\\Mineserver"; // This doesn't seem to work.
}

#else

  return "[ERROR: getHomeDir() not implemented]";

#endif

}

std::string pathOfExecutable()
{
  const size_t dest_len = 4096;
  char path[dest_len];
  std::memset(path, 0, dest_len);

#if defined(linux)

  if (readlink ("/proc/self/exe", path, dest_len) != -1)
  {
    dirname(path);
  }

  return std::string(path);

#elif defined(WIN32)

  if (0 == GetModuleFileName(NULL, path, dest_len))
  {
    return "";
  }

  return pathOfFile(path).first;

#else

  return "";

#endif

}

std::pair<std::string, std::string> pathOfFile(const std::string& filename)
{

#if defined(linux)

  char * a = strdup(filename.c_str());
  char * b = strdup(filename.c_str());
  char * d = dirname(a);

  std::string res_p(d), res_f(basename(b));

  free(a);
  free(b);

  return std::make_pair(canonicalizePath(res_p), res_f);

#elif defined(WIN32)

  const size_t dest_len = 4096;
  char path[dest_len], *pPart;
  std::memset(path, 0, dest_len);

  GetFullPathName(filename.c_str(), dest_len, path, &pPart);

  const size_t diff = pPart - path;

  if (diff > 0 && diff != size_t(-1))
    return std::make_pair(std::string(path, diff - 1), std::string(pPart));
  else
    return std::make_pair(std::string(path), "");

#else

  return std::make_pair("[ERROR IN PATH RECONSTRUCTION: Unknown platform, please implement.]", "");

#endif

}
