#ifndef _CONFIG_H
#define _CONFIG_H

#include <string>
#include <list>

#include "config/parser.h"
#include "config/node.h"

class Config
{
public:
  Config();
  ~Config();

  bool load(std::string file);
  void dump();

  ConfigNode* root();

  int iData(std::string name);
  long lData(std::string name);
  float fData(std::string name);
  double dData(std::string name);
  std::string sData(std::string name);
  bool bData(std::string name);
  ConfigNode* mData(std::string name);

  bool has(std::string name);
  int type(std::string name);
  std::list<std::string>* keys(int type=CONFIG_NODE_UNDEFINED);

private:
  ConfigParser* m_parser;
  ConfigNode* m_root;
};

#endif
