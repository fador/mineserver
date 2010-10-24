#ifdef WIN32

    #define _CRTDBG_MAP_ALLOC
    #include <stdlib.h>
    #include <crtdbg.h>
    #include <conio.h>

#endif

#include <SocketHandler.h>
#include <ListenSocket.h>
#include <iostream>
#include <fstream>
#include <deque>
#include <fstream>

#include "logger.h"
#include "constants.h"

#include "tools.h"
#include "zlib/zlib.h"
#include "DisplaySocket.h"
#include "StatusHandler.h"
#include "user.h"
#include "map.h"
#include "chat.h"
#include "nbt.h"


typedef std::map<SOCKET,Socket *> socket_m;

Chat chat;

// the constant TCP_BUFSIZE_READ is the maximum size of the standard input
// buffer of TcpSocket
#define RSIZE TCP_BUFSIZE_READ

DisplaySocket::DisplaySocket(ISocketHandler& h) : TcpSocket(h)
{
    
}

extern ListenSocket<DisplaySocket> l;
extern StatusHandler h;

void DisplaySocket::OnDisconnect()
{  
  remUser(GetSocket());
}



void DisplaySocket::OnAccept()
{
  //Create user for the socket
  addUser(GetSocket(),generateEID());
}
/*
std::string ToHex(unsigned int value)
{
  std::ostringstream oss;
  if(!(oss<<std::hex<<std::setw(2)<<std::setfill('0')<<value)) return 0;
  return oss.str();
}*/



void DisplaySocket::OnRead()
{
  uint32 i=0;
  // OnRead of TcpSocket ei kunactually reads the data from the socket
  // and moves it to the input buffer (ibuf)
  TcpSocket::OnRead();
  // get number of bytes in input buffer
  size_t n = ibuf.GetLength();
  char tmp[RSIZE]; // <--- tmp's here
  ibuf.Read(tmp,n);

  User *user=0;

  for(i=0;i<Users.size();i++)
  {
      if(Users[i].sock==GetSocket())
      {
          user=&Users[i];
      }
  }
  if(!user) //Must have user!
  {
    this->Close();
    return;
  }

  //Push data to buffer
  for(uint32 i=0;i<n;i++)
  {
      user->buffer.push_back(tmp[i]);
  }

  while(user->buffer.size()>0)
  {
    // If not waiting more data
    if(!user->waitForData)
    {
      user->action=user->buffer.front();
      user->buffer.pop_front();
      //printf("Action: 0x%x\n", user->action);
    }
    else
    {
       user->waitForData=false;
    }

    //Login package
    if(user->action==0x00)
    {  
      //Ping
    }
    else if(user->action==0x01) //Login request
    {    
      //Check that we have enought data in the buffer
      if(user->buffer.size()<12)
      {
        user->waitForData=true;
        return;
      }
      uint32 curpos=0;
      
      //Client protocol version
      uint8 tmpIntArray[4] = {0};
      for(i=0;i<4;i++) tmpIntArray[i]=user->buffer[curpos+i]; 
      int version = getUint32(&tmpIntArray[0]);     
      curpos+=4;

      //Player name length
      uint8 tmpShortArray[2] = {0};
      for(i=0;i<2;i++) tmpShortArray[i]=user->buffer[curpos+i]; 
      int len = getUint16(&tmpShortArray[0]);     
      curpos+=2;

      //Check for data
      if(user->buffer.size()<curpos+len+2)
      {
        user->waitForData=true;
        return;
      }

      std::string player;
      //Read player name
      for(int pos=0;pos<len;pos++)
      {
        player+=user->buffer[curpos+pos];
      }
      curpos+=len;

    
      //Password length
      for(i=0;i<2;i++) tmpShortArray[i]=user->buffer[curpos+i]; 
      len = getUint16(&tmpShortArray[0]);     
      curpos+=2;
    
      std::string passwd;
      //Check for data
      if(user->buffer.size()<curpos+len)
      {
        user->waitForData=true;
        return;
      }

      //Read password
      for(int pos=0;pos<len;pos++)
      {
        passwd += user->buffer[curpos+pos];
      }
      curpos+=len;

      //Package completely received, remove from buffer
      user->buffer.erase(user->buffer.begin(), user->buffer.begin()+curpos);

      std::cout << "Player " << user->UID << " login v." <<version<<" : " << player <<":" << passwd << std::endl;
      if(version==2)
      {        
        user->logged=1;
        user->changeNick(player, chat.admins);
       
        // Send motd
        std::ifstream ifs( MOTDFILE.c_str() );
        std::string temp;

        while( getline( ifs, temp ) ) {
          // If not commentline
          if(temp.substr(0,1) != "#") {
            chat.sendMsg(user, temp, USER);
          }
        }
        ifs.close();

        chat.sendMsg(user, player+" connected!", USER);

      }
      else
      {
        this->Close();
      }
    
      //Login OK package
      char data[9]={0x01, 0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00};
      putSint32((uint8 *)&data[1],user->UID);
      h.SendSock(GetSocket(), (char *)&data[0], 9);


      //Send server time (after dawn)
      uint8 data3[9]={0x04, 0x00, 0x00, 0x00,0x00,0x00,0x00,0x0e,0x00};
      h.SendSock(GetSocket(), (char *)&data3[0], 9);

      //Inventory
      uint8 data4[7+36*5];
      data4[0]=0x05;
      putSint32(&data4[1],-1);
      data4[5]=0;
      data4[6]=36;
      for(i=0;i<36;i++)
      {
        if(i<10)
          putSint16(&data4[7+i*5], 0x115); //Diamond shovel
        else if(i<20)
          putSint16(&data4[7+i*5], 50); //Torch
        else
          putSint16(&data4[7+i*5], 1); //Stone

        data4[7+2+i*5]=1; //Count
        putSint16(&data4[7+3+i*5], 0);
      }
      h.SendSock(GetSocket(), (char *)&data4[0], 7+36*5);

      data4[6]=4;
      putSint32(&data4[1],-2);
      h.SendSock(GetSocket(), (char *)&data4[0], 7+4*5);

      putSint32(&data4[1],-3);
      h.SendSock(GetSocket(), (char *)&data4[0], 7+4*5);
      


            
      //Send "On Ground" signal
      char data6[2]={0x0A, 0x01};
      h.SendSock(GetSocket(), (char *)&data6[0], 2);
    }
    else if(user->action==0x02) //Handshake
    {
      if(user->buffer.size()<3)
      {
        user->waitForData=true;
        return;
      }
      int curpos=0;

      //Player name length
      uint8 tmpShortArray[2] = {0};
      for(i=0;i<2;i++) tmpShortArray[i]=user->buffer[curpos+i]; 
      int len = getSint16(&tmpShortArray[0]);     
      curpos+=2;

      //Check for data
      if(user->buffer.size()<(unsigned int)curpos+len)
      {
        user->waitForData=true;
        return;
      }

      //Read player name
      std::string player;
      for(int pos=0;pos<len;pos++)
      {
         player += user->buffer[curpos+pos];
      }
      curpos+=len;

      //Remove package from buffer
      user->buffer.erase(user->buffer.begin(), user->buffer.begin()+curpos);
      std::cout << "Handshake player: " << player << std::endl;

      //char data[9]={0x01, 0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00};    
      //h.SendSock(GetSocket(), (char *)&data[0], 9); 
      //char data2[19]={0x02, 0x00,0x10,0x32 ,0x65 ,0x36 ,0x36 ,0x66 ,0x31 ,0x64 ,0x63 ,0x30 ,0x33 ,0x32 ,0x61 ,0x62,0x35 ,0x66 ,0x30};    
      //h.SendSock(GetSocket(), (char *)&data2[0], 19);

      //Send handshake package
      char data2[4]={0x02, 0x00,0x01,'-'};    
      h.SendSock(GetSocket(), (char *)&data2[0], 4);

      //user->logged=1;
      //user->changeNick(player);
      //char data3[5]={0x1e, 0x01, 0x02, 0x03, 0x04};
      //h.SendSock(GetSocket(), (char *)&data3[0], 5);
    } 
    // 
    // CHATMESSAGE
    //
    else if(user->action==0x03)
    {
      // Wait for length-short. HEHE
      if(user->buffer.size()<2)
      {
        user->waitForData=true;
        return;
      }
      
      int curpos=0;
      
      uint8 tmpLenArray[2] = {0};
      for(i=0;i<2;i++) tmpLenArray[i]=user->buffer[curpos+i]; 
      short len = getSint16(&tmpLenArray[0]); 
      curpos+=2;
      
      // Wait for whole message
      if(user->buffer.size()<(unsigned int)curpos+len)
      {
        user->waitForData=true;
        return;
      }
      
      //Read message
      std::string msg;
      for(i=0;i<(unsigned int)len;i++) msg += user->buffer[curpos+i];
      
      curpos += len;
      
      // Remove from buffer
      user->buffer.erase(user->buffer.begin(), user->buffer.begin()+curpos);

      chat.handleMsg( user, msg );

    }
    else if(user->action==0x05) //Inventory change
    {

      if(user->buffer.size()<14)
      {
        user->waitForData=true;
        return;
      }

      int curpos=0;

      //Read inventory type (-1,-2 or -3)
      uint8 tmpIntArray[4] = {0};
      for(i=0;i<4;i++) tmpIntArray[i]=user->buffer[curpos+i]; 
      int type = getSint32(&tmpIntArray[0]);
      curpos+=4;

      uint8 tmpShortArray[2] = {0};
      for(i=0;i<2;i++) tmpShortArray[i]=user->buffer[curpos+i]; 
      int count = getSint16(&tmpShortArray[0]);     
      curpos+=2;

      int items=0;

      switch(type)
      {
        //Main inventory
        case -1:
          items=36;
        break;

        //Equipped armour
        case -2:
          items=4;
        break;

        //Crafting slots
        case -3:
          items=4;
        break;
      }

      if(user->buffer.size()<(unsigned int)6+2*items)
      {
        user->waitForData=true;
        return;
      }

      for(i=0;i<(unsigned int)items;i++)
      {
        int j = 0;
        for(j=0;j<2;j++) tmpShortArray[j]=user->buffer[curpos+j]; 
        int item_id = getSint16(&tmpShortArray[0]);     
        curpos+=2;

        if(user->buffer.size()<curpos+(items-i-1)*2)

        {
          user->waitForData = true;
          return;
        }
        
        if(item_id!=-1)
        {
          if(user->buffer.size()-curpos<(items-i-1)*2+3)
          {
            user->waitForData=true;
            return;
          }
          uint8 numberOfItems=user->buffer[curpos];
          curpos++;
          
          for(j=0;j<2;j++) tmpShortArray[j]=user->buffer[curpos+j]; 
          int health = getSint16(&tmpShortArray[0]);     
          curpos+=2;
        }
      }

      //std::cout << "Got items type " << type << std::endl;
      //Package completely received, remove from buffer
      user->buffer.erase(user->buffer.begin(), user->buffer.begin()+curpos);

    }
    else if(user->action==0x0a) //"On Ground" of "Flying" package
    {
      if(user->buffer.size()<1)
      {
        user->waitForData=true;
        return;
      }      
      user->buffer.erase(user->buffer.begin(), user->buffer.begin()+1);
    }
    else if(user->action==0x0b) // PLayer position
    {
      if(user->buffer.size()<33)
      {
        user->waitForData=true;
        return;
      }
      int curpos=0;
      double x,y,stance,z;
      uint8 doublearray[8];

      //Read double X
      for(i=0;i<8;i++) doublearray[i]=user->buffer[curpos+i];
      x=getDouble(&doublearray[0]);    
      curpos+=8;

      //Read double Y
      for(i=0;i<8;i++) doublearray[i]=user->buffer[curpos+i];
      y=getDouble(&doublearray[0]);
      curpos+=8;

      //Read double stance
      for(i=0;i<8;i++) doublearray[i]=user->buffer[curpos+i];
      stance=getDouble(&doublearray[0]);
      curpos+=8;

      //Read double Z
      for(i=0;i<8;i++) doublearray[i]=user->buffer[curpos+i];
      z=getDouble(&doublearray[0]);
      curpos+=8;

      user->updatePos(x, y, z, stance);

      //Skip others
      user->buffer.erase(user->buffer.begin(), user->buffer.begin()+33);
    }
    else if(user->action==0x0c) //Player Look
    {
      if(user->buffer.size()<9)
      {
        user->waitForData=true;
        return;
      }

      float yaw,pitch;
      uint8 onground;
      uint8 tmpFloatArray[4] = {0};
      int curpos=0;
      
      for(i=0;i<4;i++) tmpFloatArray[i]=user->buffer[curpos+i]; 
      yaw = getFloat(&tmpFloatArray[0]);     
      curpos+=4;
      
      for(i=0;i<4;i++) tmpFloatArray[i]=user->buffer[curpos+i]; 
      pitch = getFloat(&tmpFloatArray[0]);     
      curpos+=4;
      
      user->updateLook(yaw, pitch);

      onground = user->buffer[curpos];
      curpos++;

      user->buffer.erase(user->buffer.begin(), user->buffer.begin()+9);
    }
    else if(user->action==0x0d) //Player Position & Look
    {
      if(user->buffer.size()<41)
      {
        user->waitForData=true;
        return;
      }
      int curpos=0;

      double x,y,stance,z;
      uint8 doublearray[8];

      //Read double X
      for(i=0;i<8;i++) doublearray[i]=user->buffer[curpos+i];
      x=getDouble(&doublearray[0]);    
      curpos+=8;

      //Read double Y
      for(i=0;i<8;i++) doublearray[i]=user->buffer[curpos+i];
      y=getDouble(&doublearray[0]);
      curpos+=8;

      //Read double stance
      for(i=0;i<8;i++) doublearray[i]=user->buffer[curpos+i];
      stance=getDouble(&doublearray[0]);
      curpos+=8;

      //Read double Z
      for(i=0;i<8;i++) doublearray[i]=user->buffer[curpos+i];
      z=getDouble(&doublearray[0]);
      curpos+=8;


      float yaw,pitch;
      uint8 tmpFloatArray[4] = {0};
      
      for(i=0;i<4;i++) tmpFloatArray[i]=user->buffer[curpos+i]; 
      yaw = getFloat(&tmpFloatArray[0]);     
      curpos+=4;
      
      for(i=0;i<4;i++) tmpFloatArray[i]=user->buffer[curpos+i]; 
      pitch = getFloat(&tmpFloatArray[0]);     
      curpos+=4;

      //Update user data
      user->updatePos(x, y, z, stance);
      user->updateLook(yaw, pitch);
      user->buffer.erase(user->buffer.begin(), user->buffer.begin()+41);
    }
    else if(user->action==0x0e) //Player Digging
    {
      if(user->buffer.size()<11)
      {
        user->waitForData=true;
        return;
      }
      uint8 tmpIntArray[4];
      int curpos=0;
      char status=user->buffer[curpos];
      curpos++;

      for(i=0;i<4;i++) tmpIntArray[i]=user->buffer[curpos+i];      
      int x=getSint32(&tmpIntArray[0]);
      curpos+=4;

      char y=user->buffer[curpos];
      curpos++;

      for(i=0;i<4;i++) tmpIntArray[i]=user->buffer[curpos+i];
      int z=getSint32(&tmpIntArray[0]);
      curpos+=4;
      user->buffer.erase(user->buffer.begin(), user->buffer.begin()+11);
      //If block broken
      if(status==3)
      {
        char block; char meta;
        Map::get().getBlock(x,y,z, block, meta);

        Map::get().sendBlockChange(x,y,z,0,0);
        Map::get().setBlock(x,y,z,0,0);

        char topblock; char topmeta;        
        if(Map::get().getBlock(x,y+1,z, topblock, topmeta) && topblock==0x4e) //If snow on top, destroy it
        {
          Map::get().sendBlockChange(x,y+1,z,0, 0);
          Map::get().setBlock(x,y+1,z,0,0);
        }

        if(block!=0x4e)
        {
          spawnedItem item;
          item.EID=generateEID();
          item.item=block;
          item.x=x*32;
          item.y=y*32;
          item.z=z*32;
          item.x+=(rand()%32);
          item.z+=(rand()%32);
          Map::get().sendPickupSpawn(item);
        }
      }
    }
    else if(user->action==0x0f) //Player Block Placement
    {
      if(user->buffer.size()<12)
      {
        user->waitForData=true;
        return;
      }
      int curpos=0;
      uint8 tmpShortArray[2];
      uint8 tmpIntArray[4];
      int orig_x,orig_y,orig_z;
      bool change=false;

      for(i=0;i<2;i++) tmpShortArray[i]=user->buffer[curpos+i]; 
      int blockID=getSint16(&tmpShortArray[0]);
      curpos+=2;

      for(i=0;i<4;i++) tmpIntArray[i]=user->buffer[curpos+i];      
      int x=orig_x=getSint32(&tmpIntArray[0]);
      curpos+=4;

      int y=orig_y=user->buffer[curpos];
      curpos++;

      for(i=0;i<4;i++) tmpIntArray[i]=user->buffer[curpos+i];
      int z=orig_z=getSint32(&tmpIntArray[0]);
      curpos+=4;

      int direction=user->buffer[curpos];


      user->buffer.erase(user->buffer.begin(), user->buffer.begin()+12);



      char block;
      char metadata;
      Map::get().getBlock(x,y,z, block, metadata);

      switch(direction)        
      {
        case 0: y--; break;
        case 1: y++; break;
        case 2: z--; break;
        case 3: z++; break;
        case 4: x--; break;
        case 5: x++; break;
      }
      
      char block_direction;
      char metadata_direction;
      Map::get().getBlock(x,y,z, block_direction, metadata_direction);


      //If placing normal block and current block is empty
      if(blockID<0xff && blockID!=-1 && block_direction==0x00)
      {
        change=true;
      }

      if(block==0x4e || block==50) //If snow or torch, overwrite
      {
        change=true;
        x=orig_x;
        y=orig_y;
        z=orig_z;
      }

      //Door status change
      if((block==0x40)|| (block==0x47))
      {
        change=true;

        blockID=block;

        //Toggle door state
        if(metadata&0x4)
        {
          metadata&=~0x4;
        }
        else
        {
          metadata|=0x4;
        }

        char metadata2,block2;

        int modifier=(metadata&0x8)?1:-1;
                
        x=orig_x;
        y=orig_y;
        z=orig_z;

        Map::get().getBlock(x,y+modifier,z, block2, metadata2);
        if(block2==block)
        {
          if(metadata2&0x4)
          {
            metadata2&=~0x4;
          }
          else
          {
            metadata2|=0x4;
          }

          Map::get().setBlock(x,y+modifier,z, block2, metadata2);
          Map::get().sendBlockChange(x,y+modifier,z,blockID, metadata2);
        }

      }

      if(change)
      {
        Map::get().setBlock(x,y,z, blockID, metadata);
        Map::get().sendBlockChange(x,y,z,blockID, metadata);
      }

    }
    else if(user->action==0x10) //Holding change
    {
      if(user->buffer.size()<6)
      {
        user->waitForData=true;
        return;
      }
      int itemID=getUint16(&user->buffer[4]);      
      user->buffer.erase(user->buffer.begin(), user->buffer.begin()+6);

      //Send holding change to others
      uint8 holdingPackage[7];
      holdingPackage[0]=0x10;
      putSint32(&holdingPackage[1], user->UID);
      putSint16(&holdingPackage[5], itemID);
      user->sendOthers(&holdingPackage[0],7);

    }
    else if(user->action==0x12) //Arm Animation
    {
      if(user->buffer.size()<5)
      {
        user->waitForData=true;
        return;
      }
      char forward=user->buffer[4];
      user->buffer.erase(user->buffer.begin(), user->buffer.begin()+5);

      uint8 animationPackage[6];
      animationPackage[0]=0x12;
      putSint32(&animationPackage[1],user->UID);
      animationPackage[5]=forward;
      user->sendOthers(&animationPackage[0], 6);
    }
    else if(user->action==0x15) //Pickup Spawn
    {
      if(user->buffer.size()<22)
      {
        user->waitForData=true;
        return;
      }
      
      user->buffer.erase(user->buffer.begin(), user->buffer.begin()+22);
    }
    else if(user->action==0xff) //Quit message
    {
      if(user->buffer.size()<2)
      {
        user->waitForData=true;
        return;
      }
      int curpos=0;
      uint8 shortArray[2];
      for(i=0;i<2;i++) shortArray[i]=user->buffer[i];
      int len=getSint16(&shortArray[0]);

      curpos+=2;
      // Wait for whole message
      if(user->buffer.size()<(unsigned int)curpos+len)
      {
        user->waitForData=true;
        return;
      }
      
      //Read message
      std::string msg;
      for(i=0;i<(unsigned int)len;i++) msg += user->buffer[curpos+i];

      curpos+=len;
      user->buffer.erase(user->buffer.begin(), user->buffer.begin()+curpos);
      this->SetCloseAndDelete();

    }
    else
    {
      printf("Unknown action: 0x%x\n", user->action);
      this->SetCloseAndDelete();
    }

  } //End while



  //Send to player which is sending this data
  //h.SendSock(GetSocket(), (void*)&data[0], data.size());

  //Add new player
  //addUser(GetSocket(), nick);
  
  //Send some data to all
  //h.SendAll(std::string(tmp, i));

}