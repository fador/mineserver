
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
      this->action=0;
      this->waitForData=false;
      this->sock=sock;
      this->UID=EID;
      
      
      //Send signal to create an entity
      uint8 entityData[5];
      entityData[0]=0x1e; //Initialize entity;
      putSint32(&entityData[1], EID);
      this->sendOthers(&entityData[0],5);

      for(int i=0;i<Users.size();i++)
      {
        putSint32(&entityData[1], Users[i].UID);        
        h.SendSock(this->sock, (uint8 *)&entityData[0], 5);

        uint8 teleportData[19];
        teleportData[0]=0x22; //Teleport
        putSint32(&teleportData[1],Users[i].UID);
        putSint32(&teleportData[5],(int)Users[i].pos.x);
        putSint32(&teleportData[9],(int)Users[i].pos.y);
        putSint32(&teleportData[13],(int)Users[i].pos.z);
        teleportData[17]=(char)Users[i].pos.yaw;
        teleportData[18]=(char)Users[i].pos.pitch;

        h.SendSock(this->sock, (uint8 *)&teleportData[0], 19);
      }
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
          //Do we send relative or absolute move values
          if(abs(x-Users[i].pos.x)<127
            && abs(y-Users[i].pos.y)<127
            && abs(z-Users[i].pos.z)<127)
          {
            uint8 movedata[8];
            movedata[0]=0x1f; //Relative move
            putUint32(&movedata[1],this->UID);
            movedata[5]=(char)(x-Users[i].pos.x);
            movedata[6]=(char)(y-Users[i].pos.y);
            movedata[7]=(char)(z-Users[i].pos.z);
            this->sendOthers(&movedata[0],8);
          }
          else
          {
            uint8 teleportData[19];
            teleportData[0]=0x22; //Teleport
            putSint32(&teleportData[1],Users[i].UID);
            putSint32(&teleportData[5],(int)Users[i].pos.x);
            putSint32(&teleportData[9],(int)Users[i].pos.y);
            putSint32(&teleportData[13],(int)Users[i].pos.z);
            teleportData[17]=(char)Users[i].pos.yaw;
            teleportData[18]=(char)Users[i].pos.pitch;
            this->sendOthers(&teleportData[0],19);
          }
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
          uint8 lookdata[6];
          putUint32(&lookdata[0],this->UID);
          lookdata[4]=(char)(yaw);
          lookdata[5]=(char)(pitch);          
          this->sendOthers(&lookdata[0],7);

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

    bool User::sendAll(uint8* data,uint32 len)
    {
      unsigned int i;
      for(i=0;i<(int)Users.size();i++)
      {
        if(Users[i].sock)
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

