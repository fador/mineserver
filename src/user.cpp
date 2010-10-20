
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <deque>
#include <SocketHandler.h>
#include <ListenSocket.h>
#include <algorithm>
#include <sys/stat.h> 
#include "tri_logger.hpp"
#include "DisplaySocket.h"
#include "StatusHandler.h"

#include "tools.h"
#include "map.h"
#include "user.h"
#include "nbt.h"
#include "zlib/zlib.h"
#include "chat.h"

    
    extern Chat chat;
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
    }

    bool User::changeNick(std::string nick, std::deque<std::string> admins)
    {
      this->nick=nick;
      
      // Check adminstatus
      for(int i = 0; i < admins.size(); i++) {
        if(admins[i] == nick) {
            this->admin=true;
            //TRI_LOG_STR(nick + " admin");
        }
      }      
      
      return true;
    }

    User::~User()
    {
      //chat.sendMsg(this, nick+" disconnected!", ALL);
      //Send signal to everyone that the entity is destroyed
      uint8 entityData[5];
      entityData[0]=0x1d; //Destroy entity;
      putSint32(&entityData[1], this->UID);
      this->sendOthers(&entityData[0],5);
    }

    bool User::updatePos(double x, double y, double z, double stance)
    {
         
        //Do we send relative or absolute move values
        if(0)//abs(x-this->pos.x)<127
          //&& abs(y-this->pos.y)<127
          //&& abs(z-this->pos.z)<127)
        {
          uint8 movedata[8];
          movedata[0]=0x1f; //Relative move
          putUint32(&movedata[1],this->UID);
          movedata[5]=(char)(x-this->pos.x);
          movedata[6]=(char)(y-this->pos.y);
          movedata[7]=(char)(z-this->pos.z);
          this->sendOthers(&movedata[0],8);
        }
        else
        {          
          this->pos.x=x;
          this->pos.y=y;
          this->pos.z=z;
          this->pos.stance=stance;
          uint8 teleportData[19];
          teleportData[0]=0x22; //Teleport
          putSint32(&teleportData[1],this->UID);
          putSint32(&teleportData[5],(int)this->pos.x*32);
          putSint32(&teleportData[9],(int)this->pos.y*32);
          putSint32(&teleportData[13],(int)this->pos.z*32);
          teleportData[17]=(char)this->pos.yaw;
          teleportData[18]=(char)this->pos.pitch;
          this->sendOthers(&teleportData[0],19);
        }
        
        //Chunk position changed, check for map updates
        if((int)(x/16) != curChunk.x ||(int)(z/16) != curChunk.z)
        {
          curChunk.x=x/16;
          curChunk.z=z/16;

          for(int mapx=-viewDistance+curChunk.x;mapx<=viewDistance+curChunk.x;mapx++)
          {
            for(int mapz=-viewDistance+curChunk.z;mapz<=viewDistance+curChunk.z;mapz++)
            {
              addQueue(mapx,mapz);
            }
          }

          for(unsigned int i=0;i<mapKnown.size();i++)
          {
            //If client has map data more than viesDistance+1 chunks away, remove it
            if(mapKnown[i].x<curChunk.x-viewDistance-1 ||
               mapKnown[i].x>curChunk.x+viewDistance+1 ||
               mapKnown[i].z<curChunk.z-viewDistance-1 ||
               mapKnown[i].z>curChunk.z+viewDistance+1)
            {
              addRemoveQueue(mapKnown[i].x,mapKnown[i].z);              
            }
          }
        }

        this->pos.x=x;
        this->pos.y=y;
        this->pos.z=z;
        this->pos.stance=stance;
        return true;
    }

    bool User::updateLook(float yaw, float pitch)
    {
      
        uint8 lookdata[7];
        lookdata[0]=0x20;
        putUint32(&lookdata[1],this->UID);
        lookdata[5]=(char)(yaw);
        lookdata[6]=(char)(pitch);          
        this->sendOthers(&lookdata[0],7);
          
        this->pos.yaw=yaw;
        this->pos.pitch=pitch;
        return true;

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

    bool User::addQueue(int x, int z)
    {
      coord newMap={x,0,z};

      for(unsigned int i=0;i<mapQueue.size();i++)
      {
        //Check for duplicates
        if(mapQueue[i].x==newMap.x && mapQueue[i].z==newMap.z)
        {
          return false;
        }
      }
      for(unsigned int i=0;i<mapKnown.size();i++)
      {
        //Check for duplicates
        if(mapKnown[i].x==newMap.x && mapKnown[i].z==newMap.z)
        {
          return false;
        }
      }
      this->mapQueue.push_back(newMap);

      return true;
    }

    bool User::addRemoveQueue(int x, int z)
    {
      coord newMap={x,0,z};

      this->mapRemoveQueue.push_back(newMap);

      return true;
    }

    bool User::addKnown(int x, int z)
    {
      coord newMap={x,0,z};
      this->mapKnown.push_back(newMap);

      return true;
    }

    bool User::delKnown(int x, int z)
    {
      
      for(unsigned int i=0;i<mapKnown.size();i++)
      {
        if(mapKnown[i].x==x && mapKnown[i].z==z)
        {
          mapKnown.erase(mapKnown.begin()+i);
          return true;
        }
      }

      return false;
    }

    bool SortVect(const coord &first, const coord &second)
    {
        return first.x*first.x+first.z*first.z < second.x*second.x+second.z*second.z;
    }

    bool User::popMap()
    {
      //If map in queue, push it to client
      if(this->mapRemoveQueue.size())
      {
        uint8 preChunk[10];
        //Pre chunk
        preChunk[0]=0x32;
        putSint32(&preChunk[1], mapRemoveQueue[0].x);
        putSint32(&preChunk[5], mapRemoveQueue[0].z);
        preChunk[9]=0; //Unload chunk
        h.SendSock(this->sock, (uint8 *)&preChunk[0], 10);

        //Delete from known list
        delKnown(mapRemoveQueue[0].x, mapRemoveQueue[0].z);

        //Remove from queue
        mapRemoveQueue.erase(mapRemoveQueue.begin());

        return true;
      }

      return false;
    }


    bool User::pushMap()
    {
      //If map in queue, push it to client
      if(this->mapQueue.size())
      {
        //Sort by distance from center
        //sort(mapQueue.begin(),mapQueue.end(),SortVect);

        Map::getInstance().sendToUser(this,mapQueue[0].x, mapQueue[0].z);

        //Add this to known list
        addKnown(mapQueue[0].x, mapQueue[0].z);

        //Remove from queue
        mapQueue.erase(mapQueue.begin());
      }
      else
      {
        return false;
      }
      return true;
    }
    bool User::teleport(double x, double y, double z)
    {      
      uint8 teleportdata[42]={0};
      int curpos=0;
      teleportdata[curpos]=0x0d;
      curpos++;
      putDouble(&teleportdata[curpos],x); //X
      curpos+=8;
      putDouble(&teleportdata[curpos],y);  //Y
      curpos+=8;
      putDouble(&teleportdata[curpos],z); //Z
      curpos+=8;
      putDouble(&teleportdata[curpos], 0.0); //Stance
      curpos+=8;
      putFloat(&teleportdata[curpos], 0.0);
      curpos+=4;
      putFloat(&teleportdata[curpos], 0.0);
      curpos+=4;
      teleportdata[curpos] = 0; //On Ground
      h.SendSock(this->sock, (char *)&teleportdata[0], 42);

      return true;
    }

    bool User::spawnUser(int x, int y, int z)
    {
       uint8 entityData2[256];
      int curpos=0;
      entityData2[curpos]=0x14; //Named Entity Spawn
      curpos++;
      putSint32(&entityData2[curpos], this->UID);        
      curpos+=4;
      entityData2[curpos]=0;
      entityData2[curpos+1]=this->nick.size();
      curpos+=2;
      
      for(int j=0;j<this->nick.size();j++)
      {
        entityData2[curpos]=this->nick[j];
        curpos++;
      }
      
      putSint32(&entityData2[curpos],x);
      curpos+=4;
      putSint32(&entityData2[curpos],y);
      curpos+=4;
      putSint32(&entityData2[curpos],z);
      curpos+=4;
      entityData2[curpos]=0; //Rotation
      entityData2[curpos+1]=0; //Pitch
      curpos+=2;
      putSint16(&entityData2[curpos],0); //current item
      curpos+=2;
      this->sendOthers((uint8 *)&entityData2[0], curpos);
      return true;
    }

    bool User::spawnOthers()
    {

      for(int i=0;i<Users.size(); i++)
      {
        if(Users[i].UID!=this->UID && Users[i].nick != this->nick)
        {
          uint8 entityData2[256];
          int curpos=0;
          entityData2[curpos]=0x14; //Named Entity Spawn
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
      
          putSint32(&entityData2[curpos],Users[i].pos.x*32);
          curpos+=4;
          putSint32(&entityData2[curpos],Users[i].pos.y*32);
          curpos+=4;
          putSint32(&entityData2[curpos],Users[i].pos.z*32);
          curpos+=4;
          entityData2[curpos]=0; //Rotation
          entityData2[curpos+1]=0; //Pitch
          curpos+=2;
          putSint16(&entityData2[curpos],0); //current item
          curpos+=2;
          h.SendSock(this->sock,(uint8 *)&entityData2[0], curpos);
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
        for(unsigned int i=0;i<(int)Users.size();i++)
        {
            if(Users[i].sock==sock)
            {
                chat.sendMsg(&Users[i], Users[i].nick+" disconnected!", OTHERS);
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
        EID=rand()%0xffff;

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


