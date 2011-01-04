#include <string>

#include "../../mineserver.h"
#include "../../user.h"

#ifndef _PLUGIN_COMMAND_H
#define _PLUGIN_COMMAND_H

#define PLUGIN_COMMAND_VERSION 0.1

class P_Command
{
public:
  P_Command(Mineserver* mineserver);
  ~P_Command();
  static bool callbackChatPre(User* user, time_t time, std::string data);
private:
  Mineserver* m_mineserver;
};

#endif
