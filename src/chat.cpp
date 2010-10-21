#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <deque>
#include <fstream>

#include "logger.h"
#include "constants.h"

#include "DisplaySocket.h"
#include "StatusHandler.h"

#include "tools.h"
#include "map.h"
#include "user.h"
#include "chat.h"

extern StatusHandler h;

Chat::Chat() 
{
  // Read admins to deque
  std::ifstream ifs( ADMINFILE.c_str() );
  std::string temp;

  while( getline( ifs, temp ) ) {
    // If not commentline
    if(temp.substr(0,1) != "#") {
        admins.push_back( temp );
        LOG("Admin: " + temp);
    }
  }
  ifs.close();
}

bool Chat::sendUserlist( User *user ) {
      this->sendMsg(user, COLOR_BLUE + "[ Players online ]", USER);
      for(int i=0;i<Users.size();i++)
      {
          this->sendMsg(user, "> " + Users[i].nick, USER);
      }

      return true;
}

bool Chat::handleMsg( User *user, std::string msg ) {
    // Timestamp
    time_t rawTime = time(NULL);
    struct tm* Tm = localtime(&rawTime);
    
    std::string timeStamp (asctime(Tm));
    
    timeStamp = timeStamp.substr(11,5);

    //
    // Chat commands
    //
    
    // Servermsg (Admin-only)
    if(msg.substr(0,1) == "%" && user->admin) 
    {
        // Decorate server message
        msg = COLOR_RED + "[!]" + COLOR_MAGENTA + msg.substr(1);
        this->sendMsg(user, msg, ALL);
    } 
    else if(msg.substr(0,1) == "/")
    {
        // Playerlist
        if(msg.substr(1,7) == "players") 
        {
          this->sendUserlist(user);
        }
        // About server
        if(msg.substr(1,5) == "about") 
        {
          this->sendMsg(user, COLOR_DARK_MAGENTA + "SERVER:" + COLOR_RED + " Mineserver v." + VERSION, USER);
        }
        //
        // Admin commands
        //
        if(user->admin) 
        {
          // Kick user
          if(msg.substr(1,4) == "kick") 
          {
            this->sendMsg(user, "Kick!", ALL);
          }
          
          // Teleport to coordinates
          if(msg.substr(1,3) == "ctp") 
          {
              msg = msg.substr(5);
              LOG(user->nick + " teleport to: " + msg);
              float x = atof(msg.substr(0, msg.find(' ')).c_str());
              msg = msg.substr(msg.find(' ')+1);
              float y = atof(msg.substr(0, msg.find(' ')).c_str());
              msg = msg.substr(msg.find(' ')+1);
              float z = atof(msg.c_str());
              user->teleport(x,z,y);
          }
          
          // Teleport to user
          if(msg.substr(1,2) == "tp") 
          {
              msg = msg.substr(3);
              LOG(user->nick + " teleport to: " + msg);
              
              // Get coordinates
              for(unsigned int i=0;i<Users.size();i++)
              {
                if(Users[i].nick == msg)
                {
                  double x = Users[i].pos.x;
                  double y = Users[i].pos.y;
                  double z = Users[i].pos.z;
                  user->teleport(x,z,y);
                  break;
                }
              }
          }
        }
    } 
    // Normal message
    else {
        
        if(user->admin) 
        {
          msg = timeStamp + " <"+ COLOR_DARK_MAGENTA + user->nick + COLOR_WHITE + "> " + msg;
        } else 
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
    tmpArray[0]=0x03;
    tmpArray[1]=0;
    tmpArray[2]=msg.size()&0xff;      
    for(int i=0;i<msg.size();i++) tmpArray[i+3]=msg[i]; 

    if(action == ALL) user->sendAll(&tmpArray[0],msg.size()+3);
    if(action == USER) h.SendSock(user->sock, &tmpArray[0], msg.size()+3);
    if(action == OTHERS)  user->sendOthers(&tmpArray[0], msg.size()+3);
    delete [] tmpArray;
    
    return true;
}