#ifdef WIN32
  #define _CRTDBG_MAP_ALLOC

  #include <crtdbg.h>
  #include <conio.h>
#endif

#include <cstdlib>
#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <sys/stat.h> 
#include <SocketHandler.h>
#include <ListenSocket.h>
#include "DisplaySocket.h"
#include "StatusHandler.h"


#include "map.h"
#include "tools.h"
#include "zlib/zlib.h"
#include "user.h"
#include "nbt.h"


extern ListenSocket<DisplaySocket> l;
extern StatusHandler h;

Map &Map::getInstance()
{
  static Map instance;
  return instance;
}

void Map::initMap()
{
    this->mapDirectory="testmap";
}


void Map::freeMap()
{

}

//Send chunk to user
void Map::sendToUser(User *user, int x, int z)
{
    bool dataFromMemory=false;
    std::map<int, std::map<int, NBT_struct> >::iterator iter;
    uint8 data4[18+81920];
    uint8 mapdata[81920]={0};    
    int mapposx=x;
    int mapposz=z;

    iter = maps.find(x);
    if (iter != maps.end() )
    {
      std::map<int, NBT_struct>::iterator iter2;
      iter2 = maps[x].find(z);
      if (iter2 != maps[x].end() )
      {
        //Data in memory
        dataFromMemory=true;
      }
    }


    //Generate map file name
    int modulox=(mapposx-15);
    while(modulox<0) modulox+=64;
    int moduloz=(mapposz-14);
    while(moduloz<0) moduloz+=64;
    modulox%=64;
    moduloz%=64;
    std::string infile=mapDirectory+"/"+base36_encode(modulox)+"/"+base36_encode(moduloz)+"/c."+base36_encode(mapposx-15)+"."+base36_encode(mapposz-14)+".dat";

    struct stat stFileInfo; 
    int intStat; 

    if(!dataFromMemory)
    {
      // Attempt to get the file attributes 
      intStat = stat(infile.c_str(),&stFileInfo); 
    }
    if(intStat != 0 && !dataFromMemory)
    { 
      std::cout << "File not found: " << infile << std::endl;
    }
    else
    {
      //Pre chunk
      data4[0]=0x32;
      putSint32(&data4[1], mapposx);
      putSint32(&data4[5], mapposz);
      data4[9]=1; //Init chunk
      h.SendSock(user->sock, (uint8 *)&data4[0], 10);


      //Chunk
      data4[0]=0x33;
      
      data4[11]=15; //Size_x
      data4[12]=127; //Size_y
      data4[13]=15; //Size_z

      if(!dataFromMemory)      
      {
        //Read gzipped map file
        gzFile mapfile=gzopen(infile.c_str(),"rb");        
        uint8 uncompressedData[200000];
        int uncompressedSize=gzread(mapfile,&uncompressedData[0],200000);
        gzclose(mapfile);

        int outlen=81920;

        //Save this map data to map manager
        NBT_struct newMapStruct;
        TAG_Compound(&uncompressedData[0], &newMapStruct,true);
        maps[x][z]=newMapStruct;
      }

      memcpy(&mapdata[0],get_NBT_pointer(&maps[x][z], "Blocks"), 32768);
      memcpy(&mapdata[32768],get_NBT_pointer(&maps[x][z], "Data"), 16384);
      memcpy(&mapdata[32768+16384],get_NBT_pointer(&maps[x][z], "BlockLight"), 16384);
      memcpy(&mapdata[32768+16384+16384],get_NBT_pointer(&maps[x][z], "SkyLight"), 16384);


      putSint32(&data4[1], mapposx*16);
      data4[5]=0;
      data4[6]=0;
      putSint32(&data4[7], mapposz*16);
        
      uLongf written=81920;
        
      //Compress data with zlib deflate
      compress((uint8 *)&data4[18], &written, (uint8 *)&mapdata[0],81920);
        
      putSint32(&data4[14], written);
      h.SendSock(user->sock, (uint8 *)&data4[0], 18+written);
    }
}


