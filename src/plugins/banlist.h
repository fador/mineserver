#include <string>
#include <vector>

#include "../mineserver.h"
#include "../plugin.h"
#include "../screen.h"

#ifndef _BANLIST_H
#define _BANLIST_H

class Banlist
{
public:
  Banlist(Mineserver* mineserver) : m_mineserver(mineserver) {}
  bool getBan(const std::string user);
  void setBan(const std::string user, bool banned);
private:
  Mineserver* m_mineserver;
  std::vector<std::string> m_banlist;
};

#endif
