#ifndef _CONFIG_NODE_H
#define _CONFIG_NODE_H

#include <string>
#include <map>
#include <list>

#define CONFIG_NODE_UNDEFINED 0
#define CONFIG_NODE_LIST 1
#define CONFIG_NODE_BOOLEAN 2
#define CONFIG_NODE_NUMBER 3
#define CONFIG_NODE_STRING 4

class ConfigNode
{
public:
  ConfigNode();
  ~ConfigNode();
  bool data(bool* ptr);
  bool data(int* ptr);
  bool data(long* ptr);
  bool data(float* ptr);
  bool data(double* ptr);
  bool data(std::string* ptr);
  bool bData();
  int iData();
  long lData();
  float fData();
  double dData();
  std::string sData();
  void setData(bool data);
  void setData(int data);
  void setData(long data);
  void setData(float data);
  void setData(double data);
  void setData(std::string& data);
  std::list<std::string>* keys(int type=CONFIG_NODE_UNDEFINED);
  int type();
  void setType(int type);
  bool has(std::string& key);
  ConfigNode* get(std::string& key, bool createMissing=true);
  bool set(std::string& key, ConfigNode* ptr, bool createMissing=true);
  bool add(ConfigNode* ptr);
  void clear();
  void dump(int indent);
  void dump() { dump(0); }
private:
  int m_type;
  int m_index;
  double m_nData;
  std::string m_sData;
  std::map<std::string, ConfigNode*> m_list;
};

#endif
