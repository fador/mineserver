#include <string>

#ifndef _CONFIG_SCANNER_H
#define _CONFIG_SCANNER_H

class ConfigScanner
{
public:
  ConfigScanner();
  ~ConfigScanner();
  bool read(std::string file);
  int size();
  int left();
  int move(int len);
  char get();
  int get(std::string* str, int len);
  char at(int pos);
  int pos();
private:
  std::string m_data;
  int m_pos;
};

#endif
