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
#include "zlib/zlib.h"
#include "DisplaySocket.h"
#include "StatusHandler.h"
#include "map.h"
#include "user.h"
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
     //Create user for the socket
     addUser(GetSocket(),generateEID());
     user=&Users[Users.size()-1];
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

      std::cout << "Player login v." <<version<<" : " << player <<":" << passwd << std::endl;
      if(version==2)
      {
        user->logged=1;
        user->changeNick(player, chat.admins);
        // Send motd
        std::ifstream ifs( "motd.txt" );
        std::string temp;

        while( getline( ifs, temp ) ) {
            chat.sendMsg(user, temp, USER);
        }
      }
      else
      {

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
      if(user->buffer.size()<curpos+len)
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
      if(user->buffer.size()<curpos+len)
      {
        user->waitForData=true;
        return;
      }
      
      //Read message
      std::string msg;
      for(i=0;i<len;i++) msg += user->buffer[curpos+i];
      
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

      if(user->buffer.size()<6+2*items)
      {
        user->waitForData=true;
        return;
      }

      for(i=0;i<items;i++)
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

      std::cout << "Got items type " << type << std::endl;
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



      //If block broken
      if(user->buffer[0]==3)
      {              
        uint8 changeArray[12];
        changeArray[0]=0x35; //Block change package

        //Use location from user package
        for(i=0;i<9;i++)
        {
          changeArray[i+1]=user->buffer[i+1];
        }

        changeArray[10]=0; //Replace block with
        changeArray[11]=0;  //Metadata
        user->sendAll(&changeArray[0], 12);
      }

      user->buffer.erase(user->buffer.begin(), user->buffer.begin()+11);
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

      for(i=0;i<2;i++) tmpShortArray[i]=user->buffer[curpos+i]; 
      int blockID=getSint16(&tmpShortArray[0]);
      curpos+=2;

      for(i=0;i<4;i++) tmpIntArray[i]=user->buffer[curpos+i]; 
      int x=getSint32(&tmpIntArray[0]);
      curpos+=4;

      int y=user->buffer[curpos];
      curpos++;

      for(i=0;i<4;i++) tmpIntArray[i]=user->buffer[curpos+i];
      int z=getSint32(&tmpIntArray[0]);
      curpos+=4;

      int direction=user->buffer[curpos];


      user->buffer.erase(user->buffer.begin(), user->buffer.begin()+12);

      uint8 changeArray[12];
      changeArray[0]=0x35; //Block change package
      curpos=1;
      switch(direction)        
      {
        case 0:
          y--;
          break;
        case 1:
          y++;
          break;
        case 2:
          z--;
          break;
        case 3:
          z++;
          break;
        case 4:
          x--;
          break;
        case 5:
          x++;
          break;
      }
      putSint32(&changeArray[curpos],x);
      curpos+=4;
      changeArray[curpos]=y;
      curpos++;
      putSint32(&changeArray[curpos],z);
      curpos+=4;
      changeArray[10]=blockID; //Replace block with
      changeArray[11]=0;       //Metadata
      user->sendAll(&changeArray[0], 12);

    }
    else if(user->action==0x10) //Holding change
    {
      if(user->buffer.size()<6)
      {
        user->waitForData=true;
        return;
      }
      std::cout << "Player now holding: " << getUint16(&user->buffer[4]) << std::endl;
      user->buffer.erase(user->buffer.begin(), user->buffer.begin()+6);
    }
    else if(user->action==0x12) //Arm Animation
    {
      if(user->buffer.size()<5)
      {
        user->waitForData=true;
        return;
      }

      user->buffer.erase(user->buffer.begin(), user->buffer.begin()+5);
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
      if(user->buffer.size()<curpos+len)
      {
        user->waitForData=true;
        return;
      }
      
      //Read message
      std::string msg;
      for(i=0;i<len;i++) msg += user->buffer[curpos+i];

      curpos+=len;
      user->buffer.erase(user->buffer.begin(), user->buffer.begin()+curpos);

    }
    else
    {
      printf("Unknown action: 0x%x\n", user->action);
    }

    if(user->logged)
    {
      user->logged=false;
      uint8 data4[18+81920];
      uint8 mapdata[81920]={0};
      for(i=0;i<81920;i++) mapdata[i]=0;
      int mapposx,mapposz;
      for(mapposx=-5;mapposx<=5;mapposx++)
      {
        for(mapposz=-5;mapposz<=5;mapposz++)
        {
          //Pre chunk
          data4[0]=0x32;
          putSint32(&data4[1], mapposx);
          putSint32(&data4[5], mapposz);
          data4[9]=1; //Init chunk
          h.SendSock(GetSocket(), (uint8 *)&data4[0], 10);
        }
      }

      //Chunk
      data4[0]=0x33;
      
      data4[11]=15; //Size_x
      data4[12]=127; //Size_y
      data4[13]=15; //Size_z


      for(mapposx=-5;mapposx<=5;mapposx++)
      {
        for(mapposz=-5;mapposz<=5;mapposz++)
        {   

          //Generate map file name
          int modulox=(mapposx-15);
          while(modulox<0) modulox+=64;
          int moduloz=(mapposz-14);
          while(moduloz<0) moduloz+=64;
          modulox%=64;
          moduloz%=64;
          std::string infile="testmap/"+base36_encode(modulox)+"/"+base36_encode(moduloz)+"/c."+base36_encode(mapposx-15)+"."+base36_encode(mapposz-14)+".dat";

          //Read gzipped map file
          gzFile mapfile=gzopen(infile.c_str(),"rb");
          uint8 uncompressedData[100000];
          int uncompressedSize=gzread(mapfile,&uncompressedData[0],100000);
          gzclose(mapfile);

          //std::cout << "File: " << infile << std::endl;
          int outlen=81920;
          //std::cout << "Blocks: ";
          readTag(&uncompressedData[0],uncompressedSize, &mapdata[0], &outlen, "Blocks");
          //std::cout << "Data: ";
          readTag(&uncompressedData[0],uncompressedSize, &mapdata[32768], &outlen, "Data");
          //std::cout << "BlockLight: ";
          readTag(&uncompressedData[0],uncompressedSize, &mapdata[32768+16384], &outlen, "BlockLight");
          //std::cout << "SkyLight: ";
          readTag(&uncompressedData[0],uncompressedSize, &mapdata[32768+16384+16384], &outlen, "SkyLight");


          putSint32(&data4[1], mapposx*16);
          data4[5]=0;
          data4[6]=0;
          putSint32(&data4[7], mapposz*16);
        
          uLongf written=81920;
        
          //Compress data with zlib deflate
          compress((uint8 *)&data4[18], &written, (uint8 *)&mapdata[0],81920);
        
          putSint32(&data4[14], written);
          h.SendSock(GetSocket(), (uint8 *)&data4[0], 18+written);
          //std::cout << "Sent chunk " << written << " bytes" << std::endl; 
        }          
      }
      //Send "On Ground" signal
      char data6[2]={0x0A, 0x01};
      h.SendSock(GetSocket(), (char *)&data6[0], 2);

      //Teleport player
      user->teleport(0,70,0); 

      //Spawn this user to others
      user->spawnUser(0,70*32,0);

      //Spawn other users for connected user
      user->spawnOthers();

    }
  } //End while



  //Send to player which is sending this data
  //h.SendSock(GetSocket(), (void*)&data[0], data.size());

  //Add new player
  //addUser(GetSocket(), nick);
  
  //Send some data to all
  //h.SendAll(std::string(tmp, i));

}