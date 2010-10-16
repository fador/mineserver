
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <deque>
#include <SocketHandler.h>
#include <ListenSocket.h>
#include "DisplaySocket.h"
#include "StatusHandler.h"

#include "map.h"
#include "user.h"
    

    extern ListenSocket<DisplaySocket> l;
    extern StatusHandler h;

    std::vector<User> Users;


    User::User(SOCKET sock, uint32 EID)
    {

      this->sock=sock;
      this->UID=EID;
      
      
      //Send signal to create an entity
      uint8 entityData[5];
      entityData[0]=0x1e; //Initialize entity;
      putSint32(&entityData[1], EID);
      this->sendOthers(&entityData[0],5);

      //sendOthers(sock, 
      //Send login to all
      //h.SendAll(std::string((char *)&data[0],8+nick.size()));        
    }

    bool User::changeNick(std::string nick)
    {
      this->nick=nick;

      return true;
    }

    User::~User()
    {
      //Send signal to everyone that the entity is destroyed
      uint8 entityData[5];
      entityData[0]=0x1d; //Destroy entity;
      putSint32(&entityData[1], this->UID);
      this->sendOthers(&entityData[0],5);
    }

    bool User::updatePos(double x, double y, double z, double stance)
    {
      unsigned int i;
      for(i=0;i<(int)Users.size();i++)
      {
        if(Users[i].sock==sock)
        {
          Users[i].pos.x=x;
          Users[i].pos.y=y;
          Users[i].pos.z=z;
          Users[i].pos.stance=stance;
          return true;
        }
      }
      return false;
    }

    bool User::updateLook(float yaw, float pitch)
    {
      unsigned int i;
      for(i=0;i<(int)Users.size();i++)
      {
        if(Users[i].sock==sock)
        {
          Users[i].pos.yaw=yaw;
          Users[i].pos.pitch=pitch;
          return true;
        }
      }
      return false;
    }

    bool User::sendOthers(uint8* data,uint32 len)
    {
      unsigned int i;
      for(i=0;i<(int)Users.size();i++)
      {
        if(Users[i].sock!=this->sock)
        {
          h.SendSock(Users[i].sock, data,len);
        }
      }
      return true;
    }



    bool addUser(SOCKET sock,uint32 EID)
    {
        User newuser(sock,EID);
        Users.push_back(newuser);

        return true;
    }

    bool remUser(SOCKET sock)
    {
        unsigned int i;
        for(i=0;i<(int)Users.size();i++)
        {
            if(Users[i].sock==sock)
            {
                Users.erase(Users.begin()+i);
                //Send quit to all
                //h.SendAll(std::string((char *)&data[0], 7));
                //std::cout << "Send remove UID: " << UID << std::endl;
                return true;
            }
        }
        return false;
    }

    bool isUser(SOCKET sock)
    {
        uint8 i;
        for(i=0;i<Users.size();i++)
        {
            if(Users[i].sock==sock)
                return true;
        }
        return false;
    }

    //Generate random and unique entity ID
    uint32 generateEID()
    {
      uint32 EID;
      bool finished=false;
      srand ( (uint32)time(NULL) );
              
      while(!finished)
      {
        finished=true;
        EID=rand() % 0xffffff;

        for(uint8 i=0;i<Users.size();i++)
        {
          if(Users[i].UID==EID)
          {
            finished=false;
          }
        }
      }
      return EID;
    }

