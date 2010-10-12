
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <SocketHandler.h>
#include <ListenSocket.h>
#include "DisplaySocket.h"
#include "StatusHandler.h"

#include "map.h"
#include "user.h"
    

extern ListenSocket<DisplaySocket> l;
extern StatusHandler h;

    std::vector<MyUser> Users;

    bool addUser(int UID, std::string nick)
    {
        MyUser newuser={UID, nick};
        
        Users.push_back(newuser);

        //Lähetä tieto kaikille
        //h.SendAll(std::string((char *)&data[0],8+nick.size()));
        return true;
    }

    bool remUser(int UID)
    {
        unsigned int i;
        for(i=0;i<(int)Users.size();i++)
        {
            if(Users[i].UID==UID)
            {
                Users.erase(Users.begin()+i);
                //Lähetä tieto kaikille
                //h.SendAll(std::string((char *)&data[0], 7));
                //std::cout << "Send remove UID: " << UID << std::endl;
                return true;
            }
        }
        return false;
    }

    bool isUser(int UID)
    {
        uint8 i;
        for(i=0;i<Users.size();i++)
        {
            if(Users[i].UID==UID)
                return true;
        }
        return false;
    }
