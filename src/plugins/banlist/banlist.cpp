/**
 * Compile this with the following command:
 *
 * $ g++ -DDEBIAN -shared -o banlist.so banlist.cpp
 *
 * Then put it in the same directory as your mineserver binary and issue the
 * command `/load banlist ./banlist.so`. Of course replace .so with .dll if
 * you're running windows.
 *
 * Right now it doesn't do much, but it does demonstrate all the basics needed
 * to write your own plugins.
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
  mineserver->plugin()->hookLogin.addCallback(&Banlist::callbackLogin);
}

bool Banlist::callbackLogin(User* user, bool* kick, std::string* reason)
{
  if (getBan(user->nick))
  {
    *kick = true;
    reason->assign("You're banned!");
    return false;
  }

  return true;
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
