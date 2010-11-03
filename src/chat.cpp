#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <deque>
#include <fstream>
#include <vector>
#include <ctime>
#ifdef WIN32
  #include <winsock2.h>
#endif

#include "logger.h"
#include "constants.h"

#include "tools.h"
#include "map.h"
#include "user.h"
#include "chat.h"
#include "config.h"


Chat &Chat::get()
{
  static Chat instance;
  return instance;
}


bool Chat::checkMotd(std::string motdFile)
{
  //
  // Create motdfile is it doesn't exist
  //
  std::ifstream ifs(motdFile.c_str());

  // If file does not exist
  if(ifs.fail())
  {
    std::cout << "> Warning: " << motdFile << " not found. Creating..." << std::endl;

    std::ofstream motdofs(motdFile.c_str());
    motdofs << DEFAULTMOTDFILE << std::endl;
    motdofs.close();
  }

  ifs.close();

  return true;
}

bool Chat::loadAdmins(std::string adminFile)
{
  // Read admins to deque
  std::ifstream ifs(adminFile.c_str());

  // If file does not exist
  if(ifs.fail())
  {
    std::cout << "> Warning: " << adminFile << " not found. Creating..." << std::endl;

    std::ofstream adminofs(adminFile.c_str());
    adminofs << DEFAULTADMINFILE << std::endl;
    adminofs.close();

    return true;
  }

  std::string temp;

  admins.clear();

  while(getline(ifs, temp))
  {
    // If not commentline
    if(temp[0] != COMMENTPREFIX)
    {
        admins.push_back(temp);
    }
  }
  ifs.close();

  std::cout << "Loaded admins from " << adminFile << std::endl;

  return true;
}

bool Chat::sendUserlist(User *user) {
  this->sendMsg(user, COLOR_BLUE + "[ Players online ]", USER);

  for(unsigned int i=0;i<Users.size();i++)
  {
    this->sendMsg(user, "> " + Users[i]->nick, USER);
  }

  return true;
}

std::deque<std::string> Chat::parseCmd(std::string cmd)
{
  int del;
  std::deque<std::string> temp;

  while(cmd.length() > 0)
  {
    while(cmd[0] == ' ')
      cmd = cmd.substr(1);

    del = cmd.find(' ');

    if(del > -1)
    {
      temp.push_back(cmd.substr(0, del));
      cmd = cmd.substr(del+1);
    }
    else
    {
      temp.push_back(cmd);
      break;
    }
  }

  if(temp.empty())
    temp.push_back("empty");

  return temp;
}

bool Chat::handleMsg(User *user, std::string msg)
{
  // Timestamp
  time_t rawTime = time(NULL);
  struct tm* Tm = localtime(&rawTime);

  std::string timeStamp (asctime(Tm));

  timeStamp = timeStamp.substr(11, 5);

  //
  // Chat commands
  //

  // Servermsg (Admin-only)
  if(msg[0] == SERVERMSGPREFIX && user->admin)
  {
    // Decorate server message
    msg = COLOR_RED + "[!] " + COLOR_GREEN + msg.substr(1);
    this->sendMsg(user, msg, ALL);
  }

  // Command
  else if(msg[0] == CHATCMDPREFIX)
  {
    std::deque<std::string> cmd = this->parseCmd(msg.substr(1));

    // Playerlist
    if(cmd[0] == "players")
    {
      this->sendUserlist(user);
    }

    // About server
    else if(cmd[0] == "about")
    {
      this->sendMsg(user, COLOR_DARK_MAGENTA + "SERVER:" + COLOR_RED + " Mineserver v." + VERSION, USER);
    }

    //
    // Admin commands
    //

    // Save map
    else if((cmd[0] == "save") && user->admin)
    {
      Map::get().saveWholeMap();
      this->sendMsg(user, COLOR_DARK_MAGENTA + "SERVER:" + COLOR_RED + " Saved map to disc", USER);
    }

    // Kick user
    else if((cmd[0] == "kick") && user->admin)
    {
      cmd.pop_front();

      if(!cmd.empty())
      {
        LOG("Kicking: " + cmd[0]);

        // Get coordinates
        User* tUser = getUserByNick(cmd[0]);

        if(tUser != false)
        {
          cmd.pop_front();
          std::string kickMsg = DEFAULTKICKMSG;

          if(!cmd.empty())
          {
            kickMsg = "";

            while(!cmd.empty())
            {
              kickMsg += cmd[0] + " ";
              cmd.pop_front();
            }
          }

          tUser->kick(kickMsg);

          LOG("Kicked!");
        }
        else
        {
          this->sendMsg(user, COLOR_DARK_MAGENTA + "Error!" + COLOR_RED + " User " + cmd[0] + " not found (See /players)", USER);
        }
      }
      else
      {
        this->sendMsg(user, COLOR_DARK_MAGENTA + "Error!" + COLOR_RED + " Usage: /kick user [reason]", USER);
      }
    }

    // Teleport to coordinates
    else if((cmd[0] == "ctp") && user->admin)
    {
      cmd.pop_front();

      if(cmd.size() > 2)
      {
        LOG(user->nick + " teleport to: " + cmd[0] + " " + cmd[1] + " " + cmd[2]);

        double x = atof(cmd[0].c_str());
        double y = atof(cmd[1].c_str());
        double z = atof(cmd[2].c_str());

        user->teleport(x, y, z);
      }
    }

    // Teleport to user
    else if((cmd[0] == "tp") && user->admin && (cmd.size() == 2))
    {
      cmd.pop_front();

      LOG(user->nick + " teleport to: " + cmd[0]);

      // Get coordinates
      User* tUser = getUserByNick(cmd[0]);

      if(tUser != false)
      {
        user->teleport(tUser->pos.x, tUser->pos.y+2, tUser->pos.z);
      }
      else
      {
        this->sendMsg(user, COLOR_DARK_MAGENTA + "Error!" + COLOR_RED + " User " + cmd[0] + " not found (See /players)", USER);
      }
    }

    // Teleport param1 to param2
    else if((cmd[0] == "tp") && user->admin && (cmd.size() == 3))
    {
      cmd.pop_front();

      LOG(user->nick + ": teleport " + cmd[0] + " to " + cmd[1]);

      // Get coordinates
      User* whatUser = getUserByNick(cmd[0]);
      User* toUser = getUserByNick(cmd[1]);

      if(whatUser != false && toUser != false)
      {
        whatUser->teleport(toUser->pos.x, toUser->pos.y+2, toUser->pos.z);
        this->sendMsg(user, COLOR_MAGENTA + "Teleported!", USER);
      }
      else
      {
        this->sendMsg(user, COLOR_DARK_MAGENTA + "Error!" + COLOR_RED + " User " + cmd[0] + " or " + cmd[1] + " not found (See /players)", USER);
      }
    }

    // Reload admins and configuration
    else if((cmd[0] == "reload") && user->admin)
    {
      cmd.pop_front();

      // Load admins
      this->loadAdmins(ADMINFILE);

      // Load config
      Conf::get().load(CONFIGFILE);

      this->sendMsg(user, COLOR_DARK_MAGENTA + "SERVER:" + COLOR_RED + " Reloaded admins and config", USER);

      // Note: Motd is loaded whenever new user joins
    }

    // Spawn items
    else if((cmd[0] == "give") && user->admin)
    {
      cmd.pop_front();

      User* tUser;
      int itemId;
      char itemCount = 1;

      if(cmd.size() > 1)
      {
        tUser = getUserByNick(cmd[0]);
        itemId = atoi(cmd[1].c_str());

        if(cmd.size() > 2)
          itemCount = atoi(cmd[2].c_str());
      }
      // Invalid parameters
      else
      {
        this->sendMsg(user, COLOR_DARK_MAGENTA + "Error!" + COLOR_RED + " Too few parameters", USER);
        return true;
      }

      if(tUser){
        spawnedItem item;

        item.EID = generateEID();
        item.item = itemId;
        item.count = itemCount;
        item.x = tUser->pos.x*32;
        item.y = tUser->pos.y*32;
        item.z = tUser->pos.z*32;

        Map::get().sendPickupSpawn(item);

        this->sendMsg(user, COLOR_RED + "Spawned some items!", USER);
      }
      else
      {
        this->sendMsg(user, COLOR_DARK_MAGENTA + "Error!" + COLOR_RED + " User " + cmd[0] + " not found (See /players)", USER);
      }
    }
  }

  // Normal message
  else
  {
    if(user->admin)
    {
      msg = timeStamp + " <"+ COLOR_DARK_MAGENTA + user->nick + COLOR_WHITE + "> " + msg;
    }
    else
    {
      msg = timeStamp + " <"+ user->nick + "> " + msg;
    }

    LOG(msg);

    this->sendMsg(user, msg, ALL);
  }

  return true;
}

bool Chat::sendMsg(User *user, std::string msg, int action = ALL)
{
  uint8 *tmpArray = new uint8 [msg.size()+3];

  tmpArray[0] = 0x03;
  tmpArray[1] = 0;
  tmpArray[2] = msg.size()&0xff;

  for(unsigned int i=0;i<msg.size();i++)
    tmpArray[i+3] = msg[i];

  if(action == ALL)
    user->sendAll(&tmpArray[0], msg.size()+3);

  if(action == USER)
    bufferevent_write(user->buf_ev, &tmpArray[0], msg.size()+3);

  if(action == OTHERS)
    user->sendOthers(&tmpArray[0], msg.size()+3);

  delete [] tmpArray;

  return true;
}
