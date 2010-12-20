#include <string>
#include <vector>

#include "../mineserver.h"
#include "../plugin.h"
#include "../screen.h"

#include "banlist.h"

extern "C" void banlist_init(Mineserver* mineserver)
{
  mineserver->screen()->log("banlist_init called");

  mineserver->plugin()->setPointer("banlist", new Banlist(mineserver));
}

extern "C" void banlist_shutdown(Mineserver* mineserver)
{
  mineserver->screen()->log("banlist_shutdown called");

  if (mineserver->plugin()->hasPointer("banlist"))
  {
    Banlist* banlist = (Banlist*)mineserver->plugin()->getPointer("banlist");
    mineserver->plugin()->remPointer("banlist");
    delete banlist;
  }
}

bool Banlist::getBan(const std::string user)
{
  std::vector<std::string>::iterator it_a = m_banlist.begin();
  std::vector<std::string>::iterator it_b = m_banlist.end();
  for (;it_a!=it_b;++it_a)
  {
    if (*it_a == user)
    {
      return true;
    }
  }

  return false;
}

void Banlist::setBan(const std::string user, bool banned)
{
  std::vector<std::string>::iterator it_a = m_banlist.begin();
  std::vector<std::string>::iterator it_b = m_banlist.end();
  for (;it_a!=it_b;++it_a)
  {
    if (*it_a == user)
    {
      if (banned)
      {
        return;
      }
      else
      {
        m_banlist.erase(it_a);
        return;
      }
    }
  }

  if (banned)
  {
    m_banlist.push_back(user);
  }
}
