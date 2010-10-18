
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
      this->logged=false;
      this->admin=false;
      
      
      
      //Send signal to create an entity
      uint8 entityData[5];
      entityData[0]=0x1e; //Initialize entity;
      putSint32(&entityData[1], EID);
      this->sendOthers(&entityData[0],5);
      
      uint8 entityData2[256];  

      int curpos=0;
      entityData2[curpos]=0x14; //Named Entity Spawn
      curpos++;
      putSint32(&entityData2[curpos], EID);        
      curpos+=4;
      entityData2[curpos]=0;
      entityData2[curpos+1]=nick.size();
      curpos+=2;
      for(int j=0;j<nick.size();j++)
      {
        entityData2[curpos]=nick[j];
        curpos++;
      }
      putSint32(&entityData2[curpos],0);
      curpos+=4;
      putSint32(&entityData2[curpos],65*32);
      curpos+=4;
      putSint32(&entityData2[curpos],0);
      curpos+=4;
      entityData2[curpos]=10; //Rotation
      entityData2[curpos+1]=0; //Pitch
      curpos+=2;
      putSint16(&entityData2[curpos],0); //current item
      curpos+=2;
      this->sendOthers(&entityData2[0],curpos);

      /*
      for(int i=0;i<Users.size();i++)
      {
        //Initialize entity
        putSint32(&entityData[1], Users[i].UID);
        h.SendSock(sock, &entityData[0],5);

        int curpos=0;
        entityData2[0]=0x14; //Named Entity Spawn
        curpos++;
        putSint32(&entityData2[curpos], Users[i].UID);        
        curpos+=4;
        entityData2[curpos]=0;
        entityData2[curpos+1]=Users[i].nick.size();
        curpos+=2;
        for(int j=0;j<Users[i].nick.size();j++)
        {
          entityData2[curpos]=Users[i].nick[j];
          curpos++;
        }
        putSint32(&entityData2[curpos],(int)Users[i].pos.x);
        curpos+=4;
        putSint32(&entityData2[curpos],(int)Users[i].pos.y);
        curpos+=4;
        putSint32(&entityData2[curpos],(int)Users[i].pos.z);
        curpos+=4;
        entityData2[curpos]=10; //Rotation
        entityData2[curpos+1]=(char)Users[i].pos.yaw;
        curpos+=2;
        putSint16(&entityData2[curpos],1);
        curpos+=2;

        h.SendSock(sock, (uint8 *)&entityData2[0], curpos);
      }
      */
      
      
      
      
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
          
          uint8 lookdata[7];
          lookdata[0]=0x20;
          putUint32(&lookdata[1],this->UID);
          lookdata[5]=(char)(yaw);
          lookdata[6]=(char)(pitch);          
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
      srand ( time(NULL) );
              
      while(!finished)
      {
        finished=true;
        EID=rand() & 0xffffff;

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

