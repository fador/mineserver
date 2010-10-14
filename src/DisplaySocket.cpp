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
char action;
bool waitForData=false;

void DisplaySocket::OnRead()
{
  uint32 i=0;
  // OnRead of TcpSocket actually reads the data from the socket
  // and moves it to the input buffer (ibuf)
  TcpSocket::OnRead();
  // get number of bytes in input buffer
  size_t n = ibuf.GetLength();
  char tmp[RSIZE]; // <--- tmp's here
  ibuf.Read(tmp,n);

  for(int i=0;i<n;i++) buffer.push_back(tmp[i]);


  if(!waitForData)
  {
    action=buffer.front();
    buffer.pop_front();
  }
  else
  {
     waitForData=false;
  }

  //printf("Action: 0x%x\n", action);
  //Login package
  if(action==0x00)
  {   
    //Ping
  }
  else if(action==0x01)
  {    
    if(buffer.size()<12)
    {
      waitForData=true;
      return;
    }
    int curpos=0;
    int version=(buffer[0]<<24)+(buffer[1]<<16)+(buffer[2]<<8)+buffer[3];
    curpos+=4;
    int len=(buffer[curpos]<<8)+buffer[curpos+1];
    curpos+=2;
    
    std::string player;

    if(buffer.size()<curpos+len+2)
    {
      waitForData=true;
      return;
    }
    for(int pos=0;pos<len;pos++)
    {
      player.append(1,buffer[curpos+pos]);
    }
    curpos+=len;
    std::string passwd;

    len=(buffer[curpos]<<8)+buffer[curpos+1];
    curpos+=2;
    
    if(buffer.size()<curpos+len)
    {
      waitForData=true;
      return;
    }
    for(int pos=0;pos<len;pos++)
    {
      passwd.append(1,buffer[curpos+pos]);
    }
    curpos+=len;

    buffer.erase(buffer.begin(), buffer.begin()+curpos);

    std::cout << "Player login v." <<version<<" : " << player <<":" << passwd << std::endl;
    if(version==2)
    {
      logged=1;      
    }
    
    //Login
    char data[9]={0x01, 0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00};    
    h.SendSock(GetSocket(), (char *)&data[0], 9);   

   

  }
  else if(action==0x02)
  {
    if(buffer.size()<3)
    {
      waitForData=true;
      return;
    }
    int curpos=0;
    int len=(buffer[curpos]<<8)+buffer[curpos+1];
    curpos+=2;
    if(buffer.size()<curpos+len)
    {
      waitForData=true;
      return;
    }
    std::string player;
    for(int pos=0;pos<len;pos++)
    {
       player.append(1,buffer[curpos+pos]);
    }
    curpos+=len;

    buffer.erase(buffer.begin(), buffer.begin()+curpos);
    std::cout << "Handshake player: " << player << std::endl;

    //char data[9]={0x01, 0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00};    
    //h.SendSock(GetSocket(), (char *)&data[0], 9); 
    //char data2[19]={0x02, 0x00,0x10,0x32 ,0x65 ,0x36 ,0x36 ,0x66 ,0x31 ,0x64 ,0x63 ,0x30 ,0x33 ,0x32 ,0x61 ,0x62,0x35 ,0x66 ,0x30};    
    //h.SendSock(GetSocket(), (char *)&data2[0], 19);
    char data2[4]={0x02, 0x00,0x01,'-'};    
    h.SendSock(GetSocket(), (char *)&data2[0], 4);

    //char data3[5]={0x1e, 0x01, 0x02, 0x03, 0x04};
    //h.SendSock(GetSocket(), (char *)&data3[0], 5);
  }
  else if(action==0x0a)
  {
    if(buffer[0]==1)
    {
      std::cout << "On ground!" << std::endl;
    }
    buffer.pop_front();
  }
  else if(action==0x0b) // PLayer position
  {
    if(buffer.size()<34)
    {
      waitForData=true;
      return;
    }
    int curpos=0;
    double x,y,stance,z;
    float rotation,pitch;
    uint8 doublearray[8];

    for(i=0;i<8;i++) doublearray[i]=buffer[curpos+i];
    x=getDouble(&doublearray[0]);    
    curpos+=8;

    for(i=0;i<8;i++) doublearray[i]=buffer[curpos+i];
    y=getDouble(&doublearray[0]);
    curpos+=8;

    for(i=0;i<8;i++) doublearray[i]=buffer[curpos+i];
    z=getDouble(&doublearray[0]);
    curpos+=8;

    std::cout << "X: " << x << " Y: " << y << " Z: " << z << std::endl;
    buffer.erase(buffer.begin(), buffer.begin()+34);

    //Player at ground
    if(y==60.00f) //Just for this test
    {

      //Server time
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
        putShort(&data4[7+i*5], 1);
        data4[7+2+i*5]=1;
        putShort(&data4[7+3+i*5], 0);
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
      char data5[9]={0x03, 0x00, 0x06, 'J','o','i','n','e','d'};
      h.SendSock(GetSocket(), (char *)&data5[0], 9);
      
      //Send "On Ground" signal
      char data6[2]={0x0A, 0x01};
      h.SendSock(GetSocket(), (char *)&data6[0], 2);

      //Kick player out
      //char dataQuit[9]={0xff, 0x00, 0x06, 'G','r','o','u','n','d'};
      //h.SendSock(GetSocket(), (char *)&dataQuit[0], 9);
    }
  }
  else if(action==0x0d)
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

    for(mapposx=-2;mapposx<=1;mapposx++)
    {
      for(mapposz=-2;mapposz<=1;mapposz++)
      {
        //Pre chunk
        data4[0]=0x32;
        putSint32(&data4[1], mapposx);
        putSint32(&data4[5], mapposz);
        data4[9]=1; //Init chunk
        h.SendSock(GetSocket(), (uint8 *)&data4[0], 10);
      }
    }
    for(mapposx=-2;mapposx<=1;mapposx++)
    {
      for(mapposz=-2;mapposz<=1;mapposz++)
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

  //Send to player which is sending this data
  //h.SendSock(GetSocket(), (void*)&data[0], data.size());

  //Add new player
  //addUser(GetSocket(), nick);
  
  //Send some data to all
  //h.SendAll(std::string(tmp, i));

}