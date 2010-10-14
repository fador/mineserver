#ifdef WIN32

    #define _CRTDBG_MAP_ALLOC
    #include <stdlib.h>
    #include <crtdbg.h>
    #include <conio.h>

#endif

#include <SocketHandler.h>
#include <ListenSocket.h>
#include <iostream>
#include <deque>
#include "zlib/zlib.h"
#include "DisplaySocket.h"
#include "StatusHandler.h"
#include "map.h"
#include "user.h"

typedef std::map<SOCKET,Socket *> socket_m;

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

std::deque<unsigned char> buffer;

void DisplaySocket::OnAccept()
{

  buffer.clear();
  //Send data to new user


  /*
  //Handshake
  char data2[19]={0x02, 0x00,0x10,0x32 ,0x65 ,0x36 ,0x36 ,0x66 ,0x31 ,0x64 ,0x63 ,0x30 ,0x33 ,0x32 ,0x61 ,0x62,0x35 ,0x66 ,0x30};    
  Send(std::string((char *)&data2[0], 19));

  char data3[5]={0x1e, 0x01, 0x02, 0x03, 0x04};
  Send(std::string((char *)&data3[0], 5));
  */
}
/*
std::string ToHex(unsigned int value)
{
  std::ostringstream oss;
  if(!(oss<<std::hex<<std::setw(2)<<std::setfill('0')<<value)) return 0;
  return oss.str();
}*/


int mapposx=-1;
int mapposz=-1;
int logged=0;
uint8 action;
bool waitForData=false;

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

  for(int i=0;i<n;i++) buffer.push_back(tmp[i]);

  while(buffer.size()>0)
  {
    if(!waitForData)
    {
      action=buffer.front();
      buffer.pop_front();
      printf("Action: 0x%x\n", action);
    }
    else
    {
       waitForData=false;
    }

    //Login package
    if(action==0x00)
    {  

      //Ping
    }
    else if(action==0x01) //Login request
    {    
      //Check that we have enought data in the buffer
      if(buffer.size()<12)
      {
        waitForData=true;
        return;
      }
      int curpos=0;
      //Client protocol version
      int version=getUint32(&buffer[curpos]);
      curpos+=4;

      //Player name length
      int len=getUint16(&buffer[curpos]);
      curpos+=2;
    
    

      //Check for data
      if(buffer.size()<curpos+len+2)
      {
        waitForData=true;
        return;
      }

      std::string player;
      //Read player name
      for(int pos=0;pos<len;pos++)
      {
        player.append(1,buffer[curpos+pos]);
      }
      curpos+=len;

    
      //Password length
      len=getUint16(&buffer[curpos]);
      curpos+=2;
    
      std::string passwd;
      //Check for data
      if(buffer.size()<curpos+len)
      {
        waitForData=true;
        return;
      }

      //Read password
      for(int pos=0;pos<len;pos++)
      {
        passwd.append(1,buffer[curpos+pos]);
      }
      curpos+=len;

      //Package completely received, remove from buffer
      buffer.erase(buffer.begin(), buffer.begin()+curpos);

      std::cout << "Player login v." <<version<<" : " << player <<":" << passwd << std::endl;
      if(version==2)
      {
        logged=1;      
      }
    
      //Login OK package
      char data[9]={0x01, 0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00};    
      h.SendSock(GetSocket(), (char *)&data[0], 9);
    }
    else if(action==0x02) //Handshake
    {
      if(buffer.size()<3)
      {
        waitForData=true;
        return;
      }
      int curpos=0;

      //Player name length
      int len=getUint16(&buffer[curpos]);
      curpos+=2;

      //Check for data
      if(buffer.size()<curpos+len)
      {
        waitForData=true;
        return;
      }

      //Read player name
      std::string player;
      for(int pos=0;pos<len;pos++)
      {
         player.append(1,buffer[curpos+pos]);
      }
      curpos+=len;

      //Remove package from buffer
      buffer.erase(buffer.begin(), buffer.begin()+curpos);
      std::cout << "Handshake player: " << player << std::endl;

      //char data[9]={0x01, 0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00};    
      //h.SendSock(GetSocket(), (char *)&data[0], 9); 
      //char data2[19]={0x02, 0x00,0x10,0x32 ,0x65 ,0x36 ,0x36 ,0x66 ,0x31 ,0x64 ,0x63 ,0x30 ,0x33 ,0x32 ,0x61 ,0x62,0x35 ,0x66 ,0x30};    
      //h.SendSock(GetSocket(), (char *)&data2[0], 19);

      //Send handshake package
      char data2[4]={0x02, 0x00,0x01,'-'};    
      h.SendSock(GetSocket(), (char *)&data2[0], 4);

      //char data3[5]={0x1e, 0x01, 0x02, 0x03, 0x04};
      //h.SendSock(GetSocket(), (char *)&data3[0], 5);
    }  
    else if(action==0x03) // Chatmessage
    {
      if(buffer.size()<2)
      {
        waitForData=true;
        return;
      }
      
      int curpos=0;
      
      uint8 shortarray[] = {};
      for(i=0;i<2;i++) shortarray[i]=buffer[curpos+i]; 
      short len = getSint16(&shortarray[0]); 
      curpos+=2;
      
      // Wait for whole message
      if(buffer.size()<curpos+len)
      {
        waitForData=true;
        return;
      }
      
      //Read message
      std::string msg;
      for(i=0;i<len;i++) msg+=buffer[curpos+i];
      
      // Debug
      std::cout << "Message received: " << msg << std::endl;
      
      // Remove from buffer
      buffer.erase(buffer.begin(), buffer.begin()+curpos+len);
    }
    else if(action==0x05) //Inventory change
    {

      if(buffer.size()<14)
      {
        waitForData=true;
        return;
      }

      int curpos=0;

      //Read inventory type (-1,-2 or -3)
      int type=getSint32(&buffer[curpos]);
      curpos+=4;

      int count=getSint16(&buffer[curpos]);
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

      if(buffer.size()<6+2*items)
      {
        waitForData=true;
        return;
      }

      for(i=0;i<items;i++)
      {
        int item_id=getSint16(&buffer[curpos]);
        curpos+=2;
        if(buffer.size()-curpos<items*2)
        {
          waitForData=true;
          return;
        }
        if(item_id!=-1)
        {
          if(buffer.size()-curpos<3)
          {
            waitForData=true;
            return;
          }
          uint8 numberOfItems=buffer[curpos];
          curpos++;
          int health=getSint16(&buffer[curpos]);
          curpos+=2;
        }
      }

      std::cout << "Got items" << std::endl;
      //Package completely received, remove from buffer
      buffer.erase(buffer.begin(), buffer.begin()+curpos);

    }
    else if(action==0x0a) //"On Ground" of "Flying" package
    {
      if(buffer.size()<1)
      {
        waitForData=true;
        return;
      }      
      buffer.erase(buffer.begin(), buffer.begin()+1);
    }
    else if(action==0x0b) // PLayer position
    {
      if(buffer.size()<33)
      {
        waitForData=true;
        return;
      }
      int curpos=0;
      double x,y,stance,z;
      float rotation,pitch;
      uint8 doublearray[8];

      //Read double X
      for(i=0;i<8;i++) doublearray[i]=buffer[curpos+i];
      x=getDouble(&doublearray[0]);    
      curpos+=8;

      //Read double Y
      for(i=0;i<8;i++) doublearray[i]=buffer[curpos+i];
      y=getDouble(&doublearray[0]);
      curpos+=8;

      //Read double Z
      for(i=0;i<8;i++) doublearray[i]=buffer[curpos+i];
      z=getDouble(&doublearray[0]);
      curpos+=8;

      std::cout << "X: " << x << " Y: " << y << " Z: " << z << std::endl;

      //Skip others
      buffer.erase(buffer.begin(), buffer.begin()+33);

      //Player at ground
      if(y==60.00f) //Just for this test
      {

        //Send server time (after dawn)
        uint8 data3[9]={0x04, 0x00, 0x00, 0x00,0x00,0x00,0x00,0x0e,0x00};
        h.SendSock(GetSocket(), (char *)&data3[0], 9);

        //Inventory (full of stone blocks)
        uint8 data4[7+36*5];
        data4[0]=0x05;
        putSint32(&data4[1],-1);
        data4[5]=0;
        data4[6]=36;
        for(i=0;i<36;i++)
        {
          putSint16(&data4[7+i*5], 1);
          data4[7+2+i*5]=1;
          putSint16(&data4[7+3+i*5], 0);
        }
        h.SendSock(GetSocket(), (char *)&data4[0], 7+36*5);

        data4[6]=4;
        putSint32(&data4[1],-2);
        h.SendSock(GetSocket(), (char *)&data4[0], 7+4*5);

        putSint32(&data4[1],-3);
        h.SendSock(GetSocket(), (char *)&data4[0], 7+4*5);
      
        //Setup spawn position (0,70,0)
        uint8 data2[13]={0};
        data2[0]=0x06;
        putSint32(&data2[1], 0); //X
        putSint32(&data2[5], 70);//Y
        putSint32(&data2[9], 0); //Z      
        h.SendSock(GetSocket(), (char *)&data2[0], 13); 

        //Add chat message
        //char data5[9]={0x03, 0x00, 0x06, 'J','o','i','n','e','d'};
        //h.SendSock(GetSocket(), (char *)&data5[0], 9);
      
        //Send "On Ground" signal
        char data6[2]={0x0A, 0x01};
        h.SendSock(GetSocket(), (char *)&data6[0], 2);

        //Kick player out
        //char dataQuit[9]={0xff, 0x00, 0x06, 'G','r','o','u','n','d'};
        //h.SendSock(GetSocket(), (char *)&dataQuit[0], 9);
      }
    }
    else if(action==0x0c) //Player Look
    {
      if(buffer.size()<9)
      {
        waitForData=true;
        return;
      }

      float yaw,pitch;
      uint8 onground;
      int curpos=0;
      yaw=getFloat(&buffer[curpos]);
      curpos+=4;
      pitch=getFloat(&buffer[curpos]);
      curpos+=4;
      onground=buffer[curpos];
      curpos++;

      buffer.erase(buffer.begin(), buffer.begin()+9);
    }
    else if(action==0x0d) //Player Position & Look
    {
      if(buffer.size()<41)
      {
        waitForData=true;
        return;
      }
      int curpos=0;
      double x,y,stance,z;
      float rotation,pitch;
      uint8 *doubleAddress=(uint8 *)&x;
      for(i=0;i<8;i++)
      {
        doubleAddress[7-i]=buffer[curpos+i];
      }
      std::cout << "X: " << x << std::endl;
      buffer.erase(buffer.begin(), buffer.begin()+41);
    }
    else if(action==0x10) //Holding change
    {
      std::cout << "Player now holding: " << getUint16(&buffer[4]) << std::endl;
      buffer.erase(buffer.begin(), buffer.begin()+6);
    }
    else
    {
      printf("Unknown action: 0x%x\n", action);
    }

    if(logged)
    {
      logged=false;
      uint8 data4[18+81920+1000];
      uint8 mapdata[81920]={0};
      for(i=0;i<81920;i++) mapdata[i]=0;

      for(mapposx=-1;mapposx<=1;mapposx++)
      {
        for(mapposz=-1;mapposz<=1;mapposz++)
        {
          //Pre chunk
          data4[0]=0x32;
          putSint32(&data4[1], mapposx);
          putSint32(&data4[5], mapposz);
          data4[9]=1; //Init chunk
          h.SendSock(GetSocket(), (uint8 *)&data4[0], 10);
        }
      }
      for(mapposx=-1;mapposx<=1;mapposx++)
      {
        for(mapposz=-1;mapposz<=1;mapposz++)
        {   
          //Chunk
          data4[0]=0x33;
          putSint32(&data4[1], mapposx);
          data4[5]=0;
          data4[6]=0;
          putSint32(&data4[7], mapposz);
          data4[11]=15; //Size_x
          data4[12]=127; //Size_y
          data4[13]=15; //Size_z


          int index=0;
        
          //Type array
          for(int mapx=0;mapx<16;mapx++)
          {
            for(int mapz=0;mapz<16;mapz++)
            {
              for(int mapy=0;mapy<128;mapy++)
              {
                if(mapy<1)
                {
                  mapdata[index]=7; //BedRock
                }
                else if(mapy<60)
                {
                  mapdata[index]=1; //Rock
                }              
                else
                {
                  mapdata[index]=0; //Empty
                }
                index++;
              }
            }
          }

          //metadata
          for(int i=0;i<16*16*128/2;i++)
          {
            mapdata[index]=0;
            index++;
          }


          //Block light
          for(int i=0;i<16*16*128/2;i++)
          {
            mapdata[index]=0xee;
            index++;
          }

          //Sky light
          for(int i=0;i<16*16*128/2;i++)
          {
            mapdata[index]=0x00;
            index++;
          }
        
        
          uLongf written=81920;
        
          //Compress data with zlib deflate
          compress((uint8 *)&data4[18], &written, (uint8 *)&mapdata[0],81920+1000);            
        
          putSint32(&data4[14], written);
          h.SendSock(GetSocket(), (uint8 *)&data4[0], 18+written);
          std::cout << "Sent chunk " << written << " bytes" << std::endl;
        
        }          
      }
    }
    if(waitForData) break;
  } //End while

  //Send to player which is sending this data
  //h.SendSock(GetSocket(), (void*)&data[0], data.size());

  //Add new player
  //addUser(GetSocket(), nick);
  
  //Send some data to all
  //h.SendAll(std::string(tmp, i));

}