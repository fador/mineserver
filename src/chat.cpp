#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <deque>
#include <fstream>
#include "DisplaySocket.h"
#include "StatusHandler.h"
#include "tri_logger.hpp"

#include "map.h"
#include "user.h"
#include "chat.h"

extern StatusHandler h;

Chat::Chat() 
{
  // Read admins to deque
  std::ifstream ifs( "admin.txt" );
  std::string temp;

  while( getline( ifs, temp ) ) {
    admins.push_back( temp );
    TRI_LOG_STR("Admin: " + temp);
  }
}

bool Chat::handleMsg( User *user, std::string msg ) {
    // Timestamp
    time_t rawTime = time(NULL);
    struct tm* Tm = localtime(&rawTime);
    
    std::string timeStamp (asctime(Tm));
    
    timeStamp = timeStamp.substr(11,5);

    // Admincommands
    if(msg.substr(0,1) == "/")
    {
        if(user->admin) {
            TRI_LOG_STR(user->nick + " adminkomento!");
            TRI_LOG(msg);
            this->sendMsg(user, msg.substr(1), ALL);
        } else {
            TRI_LOG_STR(user->nick + " ei ole admin");
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
    delete [] tmpArray;
    
    return true;
}