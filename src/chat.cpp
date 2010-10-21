#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <deque>
#include <fstream>

#include "constants.h"

#include "DisplaySocket.h"
#include "StatusHandler.h"
#include "tri_logger.hpp"

#include "tools.h"
#include "map.h"
#include "user.h"
#include "chat.h"

extern StatusHandler h;

Chat::Chat() 
{
  // Read admins to deque
  std::ifstream ifs( ADMINFILE );
  std::string temp;

  while( getline( ifs, temp ) ) {
    // If not commentline
    if(temp.substr(0,1) != "#") {
        admins.push_back( temp );
        //TRI_LOG_STR("Admin: " + temp);
    }
  }
  ifs.close();
}

bool Chat::sendUserlist( User *user ) {
      this->sendMsg(user, COLOR_BLUE + "Playerlist", USER);
      for(int i=0;i<Users.size();i++)
      {
          this->sendMsg(user, Users[i].nick, USER);
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
        this->sendMsg(user, msg.substr(1), ALL);
    } 
    else if(msg.substr(0,1) == "/")
    {
        if(msg.substr(1,7) == "players") {
            this->sendUserlist(user);
        }
        if(msg.substr(1,5) == "about") {
            this->sendMsg(user, COLOR_DARK_MAGENTA + "SERVER:" + COLOR_RED + " Mineserver v." + VERSION, USER);
        }
        if(user->admin) {
            if(msg.substr(1,4) == "smsg") {
                this->sendMsg(user, msg.substr(5), ALL);
            }
            if(msg.substr(1,4) == "kick") {
                this->sendMsg(user, "POTKITAANPOTKITAAN", ALL);
            }

            if(msg.substr(1,3) == "ctp") { // x y z
                msg = msg.substr(5);
                TRI_LOG(msg);
                float x = atof(msg.substr(0, msg.find(' ')).c_str());
                msg = msg.substr(msg.find(' ')+1);
                TRI_LOG(msg);
                float y = atof(msg.substr(0, msg.find(' ')).c_str());
                msg = msg.substr(msg.find(' ')+1);
                TRI_LOG(msg);
                float z = atof(msg.c_str());
                user->teleport(x,z,y);
            }
        }
    } 
    // Normal message
    else {
    
        //Send message to others
        msg = timeStamp + " <"+user->nick+"> "+msg;

        TRI_LOG(msg);

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