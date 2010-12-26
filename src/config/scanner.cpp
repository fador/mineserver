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

bool ConfigScanner::read(std::string file)
{
  struct stat tmp;
  if(stat(file.c_str(), &tmp) != 0)
  {
    return false;
  }

  std::ifstream handle;
  handle.open(file.c_str());

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
