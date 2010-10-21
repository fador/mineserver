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
    uint32 i;
    uint32 x,y;
    this->mapDirectory="testmap";
}


void Map::freeMap()
{
    uint32 i;
    uint32 x,y;
}

//Send chunk to user
void Map::sendToUser(User *user, int x, int z)
{
    bool dataFromMemory=false;
    storedMap *mapData;
    std::map<int, std::map<int, storedMap> >::iterator iter;
    iter = maps.find(x);
    if (iter != maps.end() )
    {
       std::map<int, storedMap>::iterator iter2;
       iter2 = maps[x].find(z);
       if (iter2 != maps[x].end() )
       {
         //Data in memory
         dataFromMemory=true;
         mapData=&maps[x][z];
       }
    }
    uint8 data4[18+81920];
    uint8 mapdata[81920]={0};    
    int mapposx=x;
    int mapposz=z;

    //Generate map file name
    int modulox=(mapposx-15);
    while(modulox<0) modulox+=64;
    int moduloz=(mapposz-14);
    while(moduloz<0) moduloz+=64;
    modulox%=64;
    moduloz%=64;
    std::string infile=mapDirectory+"/"+base36_encode(modulox)+"/"+base36_encode(moduloz)+"/c."+base36_encode(mapposx-15)+"."+base36_encode(mapposz-14)+".dat";

    struct stat stFileInfo; 
    bool blnReturn; 
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


      if(dataFromMemory)
      {
        memcpy(&mapdata[0],mapData->blocks, 32768);
        memcpy(&mapdata[32768],mapData->metadata, 16384);
        memcpy(&mapdata[32768+16384],mapData->blocklight, 16384);
        memcpy(&mapdata[32768+16384+16384],mapData->skylight, 16384);
        std::cout << "Taking data from memory!" << std::endl;
      }
      else
      {
        //Read gzipped map file
        gzFile mapfile=gzopen(infile.c_str(),"rb");        
        uint8 uncompressedData[200000];
        int uncompressedSize=gzread(mapfile,&uncompressedData[0],200000);
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

        //Save this map data to map manager
        int pointerPos=0;
        storedMap newMapStruct;
        uint8 *newMapData=new uint8 [uncompressedSize];
        memcpy(newMapData,&uncompressedData[0],uncompressedSize);
        newMapStruct.datasize=uncompressedSize;
        newMapStruct.fullData=newMapData;
        newMapStruct.x=x;
        newMapStruct.z=z;
        readTag(&uncompressedData[0],uncompressedSize, NULL, &outlen, "Blocks", &pointerPos);
        newMapStruct.blocks=&newMapData[pointerPos];
        readTag(&uncompressedData[0],uncompressedSize, NULL, &outlen, "Data", &pointerPos);
        newMapStruct.metadata=&newMapData[pointerPos];
        readTag(&uncompressedData[0],uncompressedSize, NULL, &outlen, "BlockLight", &pointerPos);
        newMapStruct.blocklight=&newMapData[pointerPos];
        readTag(&uncompressedData[0],uncompressedSize, NULL, &outlen, "SkyLight", &pointerPos);
        newMapStruct.skylight=&newMapData[pointerPos];
        maps[x][z]=newMapStruct;
      }


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


