#include <string>
#include <vector>

#include "../../mineserver.h"
#include "../../plugin.h"
#include "../../screen.h"
#include "../../user.h"

#ifndef _BANLIST_H
#define _BANLIST_H

class Banlist
{
public:
  Banlist(Mineserver* mineserver);
  ~Banlist();
  bool getBan(const std::string user);
  void setBan(const std::string user, bool banned);
  static bool callbackLoginPre(User* user, std::string* reason);
private:
  Mineserver* m_mineserver;
  std::vector<std::string> m_banlist;
};

#endif
