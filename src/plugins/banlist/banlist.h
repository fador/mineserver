#include <string>
#include <vector>

#include "../../mineserver.h"
#include "../../user.h"

#ifndef _PLUGIN_BANLIST_H
#define _PLUGIN_BANLIST_H

#define PLUGIN_BANLIST_VERSION 0.1

class P_Banlist
{
public:
  P_Banlist(Mineserver* mineserver);
  ~P_Banlist();
  bool getBan(const std::string user);
  void setBan(const std::string user, bool banned);
  static bool callbackLoginPre(User* user, std::string* reason);
private:
  Mineserver* m_mineserver;
  std::vector<std::string> m_banlist;
};

#endif
