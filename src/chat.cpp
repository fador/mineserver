#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <deque>
#include "DisplaySocket.h"
#include "StatusHandler.h"
#include "tri_logger.hpp"

#include "map.h"
#include "user.h"
#include "chat.h"

bool Chat::handleMsg( User *user, std::string msg ) {
    // Timestamp
    time_t rawTime = time(NULL);
    struct tm* Tm = localtime(&rawTime);
    
    std::string timeStamp (asctime(Tm));
    
    timeStamp = timeStamp.substr(11,5);

    
    //Send message to others
    msg = timeStamp + " <"+user->nick+"> "+msg;

    TRI_LOG(msg);

    uint8 *tmpArray = new uint8 [msg.size()+3];
    tmpArray[0]=0x03;
    tmpArray[1]=0;
    tmpArray[2]=msg.size()&0xff;      
    for(int i=0;i<msg.size();i++) tmpArray[i+3]=msg[i]; 

    user->sendAll(&tmpArray[0],msg.size()+3);
    delete [] tmpArray;

    return true;
}