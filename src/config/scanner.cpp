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

#include <string>
#include <fstream>
#include <sys/stat.h>

#include "scanner.h"

ConfigScanner::ConfigScanner() : m_pos(0)
{
}

ConfigScanner::~ConfigScanner()
{
}

bool ConfigScanner::read(const std::string& file)
{
  struct stat tmp;
  if (stat(file.c_str(), &tmp) != 0)
  {
    return false;
  }

  std::ifstream handle;
  handle.open(file.c_str(), std::ios_base::binary);

  if (handle.bad())
  {
    return false;
  }

  m_data.clear();

  handle.seekg(0, std::ios::end);
  int size = handle.tellg();
  handle.seekg(0, std::ios::beg);

  char* buf = new char [size];
  handle.read(buf, size);

  handle.close();

  m_data.assign(buf, size);

  delete [] buf;

  return true;
}

int ConfigScanner::size()
{
  return m_data.length();
}

int ConfigScanner::left()
{
  return m_data.length() - m_pos;
}

int ConfigScanner::move(int len)
{
  if ((unsigned int)(m_pos + len) > m_data.length())
  {
    len = m_data.length() - m_pos;
  }

  if ((m_pos + len) < 0)
  {
    len = (0 - m_pos);
  }

  m_pos += len;

  return len;
}

char ConfigScanner::get()
{
  return *(m_data.substr(m_pos, 1).c_str());
}

char ConfigScanner::at(int offset)
{
  if ((unsigned int)offset >= m_data.length())
  {
    return -1;
  }

  return *(m_data.substr(offset, 1).c_str());
}

int ConfigScanner::pos()
{
  return m_pos;
}
