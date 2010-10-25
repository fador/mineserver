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

#include "logger.h"

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

Map &Map::get()
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

NBT_struct *Map::getMapData(int x, int z)
{
  std::map<int, std::map<int, NBT_struct> >::iterator iter;
  iter = maps.find(x);
  if (iter != maps.end() )
  {
    std::map<int, NBT_struct>::iterator iter2;
    iter2 = maps[x].find(z);
    if (iter2 != maps[x].end() )
    {
      //Data in memory
      return &maps[x][z];
    }
  }
  return 0;
}

bool Map::getBlock(int x, int y, int z, char &type, char &meta)
{
  int chunk_x=((x<0)?(x/16)-1:x/16);
  int chunk_z=((z<0)?(z/16)-1:z/16);
  NBT_struct *chunk=getMapData(chunk_x, chunk_z);
  if(!chunk || y<0 || y>127)
  {
    return false;
  }

  int chunk_block_x=x-(chunk_x*16);
  int chunk_block_z=z-(chunk_z*16);
  uint8 *blocks=chunk->blocks;
  uint8 *metapointer=chunk->data;
  int index=y + (chunk_block_z * 128) + (chunk_block_x * 128 * 16);
  type=blocks[index];
  char metadata=metapointer[(index)>>1];
  
  if(y%2)
  {
    metadata&=0xf0;
    metadata>>=4;
  }
  else
  {
    metadata&=0x0f;
  }
  meta=metadata;

  return true;
}

bool Map::setBlock(int x, int y, int z, char type, char meta)
{
  int chunk_x=((x<0)?(x/16)-1:x/16);
  int chunk_z=((z<0)?(z/16)-1:z/16);
  NBT_struct *chunk=getMapData(chunk_x, chunk_z);
  if(!chunk || y<0 || y>127)
  {
    return false;
  }

  int chunk_block_x=x-(chunk_x*16);
  int chunk_block_z=z-(chunk_z*16);
  uint8 *blocks=chunk->blocks;
  uint8 *metapointer=chunk->data;
  int index=y + (chunk_block_z * 128) + (chunk_block_x * 128 * 16);
  blocks[index]=type;
  char metadata=metapointer[index>>1];
  if(y%2)
  {
    metadata&=0x0f;
    metadata|=meta<<4;
  }
  else
  {
    metadata&=0xf0;
    metadata|=meta;
  }
  metapointer[index>>1]=metadata;

  return true;
}

bool Map::sendBlockChange(int x, int y, int z, char type, char meta)
{
    uint8 curpos=0;
    uint8 changeArray[12];
    changeArray[0]=0x35; //Block change package
    curpos=1;
    putSint32(&changeArray[curpos],x);
    curpos+=4;
    changeArray[curpos]=y;
    curpos++;
    putSint32(&changeArray[curpos],z);
    curpos+=4;
    changeArray[10]=type;   //Replace block with
    changeArray[11]=meta;  //Metadata

    //TODO: only send to users in range
    for(unsigned int i=0;i<Users.size();i++)
    {
      h.SendSock(Users[i].sock,&changeArray[0], 12);
    }

    return true;
}

bool Map::sendPickupSpawn(spawnedItem item)
{
  uint8 curpos=0;
  uint8 changeArray[23];
  changeArray[curpos]=0x15; //Pickup Spawn
  curpos++;
  putSint32(&changeArray[curpos],item.EID);
  curpos+=4;
  putSint16(&changeArray[curpos],item.item);
  curpos+=2;
  changeArray[curpos]=item.count;
  curpos++;

  putSint32(&changeArray[curpos],item.x);
  curpos+=4;
  putSint32(&changeArray[curpos],item.y);
  curpos+=4;
  putSint32(&changeArray[curpos],item.z);
  curpos+=4;
  changeArray[curpos]=0; //Rotation
  curpos++;
  changeArray[curpos]=0; //Pitch
  curpos++;
  changeArray[curpos]=0; //Roll
  curpos++;

  //TODO: only send to users in range
  for(unsigned int i=0;i<Users.size();i++)
  {
    h.SendSock(Users[i].sock,&changeArray[0], 23);
  }

  return true;
}

bool Map::loadMap(int x, int z)
{
  //Update last used time
  mapLastused[x][z]=time(0);

  if(getMapData(x,z)!=0)
  {
    return true;
  }

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
  int intStat; 
  // Attempt to get the file attributes 
  intStat = stat(infile.c_str(),&stFileInfo); 

  if(intStat != 0)
  {
    LOG("File not found: " + infile);
    return false;
  }
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

  return true;
}

bool Map::saveMap(int x, int z)
{
  int mapposx=x;
  int mapposz=z;
  //Generate map file name
  int modulox=(mapposx-15);
  while(modulox<0) modulox+=64;
  int moduloz=(mapposz-14);
  while(moduloz<0) moduloz+=64;
  modulox%=64;
  moduloz%=64;
  std::string outfile=mapDirectory+"/"+base36_encode(modulox)+"/"+base36_encode(moduloz)+"/c."+base36_encode(mapposx-15)+"."+base36_encode(mapposz-14)+".dat";

  uint8 uncompressedData[200000];
  int dumpsize=dumpNBT_struct(&maps[x][z].compounds[0], &uncompressedData[0]);
  gzFile mapfile2=gzopen(outfile.c_str(),"wb");        
  gzwrite(mapfile2,&uncompressedData[0],dumpsize);
  gzclose(mapfile2);

  return true;
}


bool Map::releaseMap(int x, int z)
{

  return true;
}

//Send chunk to user
void Map::sendToUser(User *user, int x, int z)
{
    bool dataFromMemory=false;    
    uint8 data4[18+81920];
    uint8 mapdata[81920]={0};    
    int mapposx=x;
    int mapposz=z;

    if(loadMap(x,z))
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

      maps[x][z].blocks=get_NBT_pointer(&maps[x][z], "Blocks");
      maps[x][z].data=get_NBT_pointer(&maps[x][z], "Data");
      maps[x][z].blocklight=get_NBT_pointer(&maps[x][z], "BlockLight");
      maps[x][z].skylight=get_NBT_pointer(&maps[x][z], "SkyLight");

      //Check if the items were not found
      if(maps[x][z].blocks == 0     ||
         maps[x][z].data == 0       ||
         maps[x][z].blocklight == 0 ||
         maps[x][z].skylight == 0)
      {
        LOG("Error in map data");
        return;
      }

      memcpy(&mapdata[0],maps[x][z].blocks, 32768);
      memcpy(&mapdata[32768],maps[x][z].data, 16384);
      memcpy(&mapdata[32768+16384],maps[x][z].blocklight, 16384);
      memcpy(&mapdata[32768+16384+16384],maps[x][z].skylight, 16384);


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


