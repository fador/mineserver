/**
 * Compile this with the following command if you're using linux (and gcc):
 * $ g++ -shared -o banlist.so banlist.cpp
 * (add -DDEBIAN if you're not on Debian)
 *
 * On Windows under MSVS you'll have to link the DLL with the .lib from the
 * mineserver binary. With MingW I'm not sure how to make it work...
 *
 * Then put it in the same directory as your mineserver binary and issue the
 * command `/load banlist ./banlist.so`. Of course replace .so with .dll if
 * you're running Windows.
 *
 * Right now it doesn't do much, but it does demonstrate all the basics needed
 * to write your own plugins. Make sure you check out banlist.h as well.
 */

#include <string>
#include <vector>

#include "../../mineserver.h"
#include "../../plugin.h"
#include "../../screen.h"

#include "banlist.h"

extern "C" void banlist_init(Mineserver* mineserver)
{
  mineserver->screen()->log("banlist_init called");

  if (mineserver->plugin()->hasPointer("banlist"))
  {
    mineserver->screen()->log("banlist is already loaded!");
    return;
  }

  mineserver->plugin()->setPointer("banlist", new Banlist(mineserver));
}

extern "C" void banlist_shutdown(Mineserver* mineserver)
{
  mineserver->screen()->log("banlist_shutdown called");

  if (!mineserver->plugin()->hasPointer("banlist"))
  {
    mineserver->screen()->log("banlist is not loaded!");
    return;
  }

  Banlist* banlist = (Banlist*)mineserver->plugin()->getPointer("banlist");
  mineserver->plugin()->remPointer("banlist");
  delete banlist;
}

Banlist::Banlist(Mineserver* mineserver) : m_mineserver(mineserver)
{
  if (m_mineserver->plugin()->hasHook("LoginPre"))
  {
    (static_cast<Hook2<bool,User*,std::string*>*>(m_mineserver->plugin()->getHook("LoginPre")))->addCallback(&Banlist::callbackLoginPre);
  }
  else
  {
    Mineserver::get()->screen()->log("Banlist: Can't find the LoginPre hook, banlist will not be operational.");
  }
}

Banlist::~Banlist()
{
  if (m_mineserver->plugin()->hasHook("LoginPre"))
  {
    (static_cast<Hook2<bool,User*,std::string*>*>(m_mineserver->plugin()->getHook("LoginPre")))->remCallback(&Banlist::callbackLoginPre);
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

bool Banlist::callbackLoginPre(User* user, std::string* reason)
{
  Banlist* banlist = static_cast<Banlist*>(Mineserver::get()->plugin()->getPointer("banlist"));
  Mineserver::get()->screen()->log("Banlist: Checking if user "+user->nick+" is banned");
  if (banlist->getBan(user->nick))
  {
    Mineserver::get()->screen()->log("Banlist: They are!");
    reason->assign("You've been banned!");
    return false;
  }
  else
  {
    Mineserver::get()->screen()->log("Banlist: They're not!");
    return true;
  }
}
