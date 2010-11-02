#ifdef WIN32
  #define _CRTDBG_MAP_ALLOC

  #include <crtdbg.h>
  #include <conio.h>
  #include <direct.h>
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
#include "config.h"


extern ListenSocket<DisplaySocket> l;
extern StatusHandler h;

Map &Map::get()
{
  static Map instance;
  return instance;
}

void Map::initMap()
{  
  this->mapDirectory=Conf::get().value("mapdir");
  if(this->mapDirectory=="Not found!")
  {
    std::cout << "Error, mapdir not defined!" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::string infile=mapDirectory+"/level.dat";

  struct stat stFileInfo; 
  int intStat; 
  // Attempt to get the file attributes 
  intStat = stat(infile.c_str(),&stFileInfo); 

  if(intStat != 0)
  {
    std::cout << "Error, map not found!" << std::endl;
    exit(EXIT_FAILURE);
  }
  //Read gzipped map file
  gzFile mapfile=gzopen(infile.c_str(),"rb");        
  uint8 uncompressedData[200000];
  int uncompressedSize=gzread(mapfile,&uncompressedData[0],200000);
  gzclose(mapfile);

  //Save level data
  TAG_Compound(&uncompressedData[0], &levelInfo,true);

  if(!get_NBT_value(&levelInfo, "SpawnX", &spawnPos.x) ||
     !get_NBT_value(&levelInfo, "SpawnY", &spawnPos.y) ||
     !get_NBT_value(&levelInfo, "SpawnZ", &spawnPos.z))
  {
    std::cout << "Error, spawn pos not found from " << infile << "!" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::cout << "Spawn: (" << spawnPos.x << "," << spawnPos.y << "," << spawnPos.z << ")" << std::endl;
  
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
      //Update last used time
      mapLastused[x][z]=(int)time(0);
      //Data in memory
      return &maps[x][z];
    }
  }
  return 0;
}


bool Map::saveWholeMap()
{
  for (std::map<int, std::map<int, NBT_struct> >::const_iterator it = maps.begin(); it != maps.end(); ++it)
  {
    for (std::map<int, NBT_struct>::const_iterator it2 = maps[it->first].begin(); it2 != maps[it->first].end(); ++it2)
    {
      saveMap(it->first, it2->first);
    }
  }
  return true;
}

bool Map::generateLightMaps(int x, int z)
{
  if(!loadMap(x,z)) return false;
  uint8 *skylight=maps[x][z].skylight;
  uint8 *blocklight=maps[x][z].blocklight;
  uint8 *blocks=maps[x][z].blocks;

  //Clear lightmaps
  memset(blocklight, 0, 16*16*128/2);
  memset(skylight, 0, 16*16*128/2);

  //Skylight

  //First set sunlight for all blocks until hit ground
  for(int block_x=0;block_x<16;block_x++)
  {
    for(int block_z=0;block_z<16;block_z++)
    {
      for(int block_y=127;block_y>0;block_y--)
      {
        int index=block_y + (block_z * 128) + (block_x * 128 * 16);
        int absolute_x=x*16+block_x;
        int absolute_z=z*16+block_z;
        uint8 block=blocks[index];
        setBlockLight(absolute_x,block_y,absolute_z, 0, 15, 2);
        if(stopLight[block]==-16)
        {
          break;
        }
      }
    }
  }

  //Loop again and now spread the light
  for(int block_x=0;block_x<16;block_x++)
  {
    for(int block_z=0;block_z<16;block_z++)
    {
      for(int block_y=127;block_y>=0;block_y--)
      {
        int index=block_y + (block_z * 128) + (block_x * 128 * 16);
        int absolute_x=x*16+block_x;
        int absolute_z=z*16+block_z;
        uint8 block=blocks[index];
        //getBlock(absolute_x,block_y,absolute_z,&block,&meta);
        if(stopLight[block]==-16)
        {
          setBlockLight(absolute_x,block_y,absolute_z, 15+stopLight[block], 0, 1);
          break;
        }
        else
        {
          setBlockLight(absolute_x,block_y,absolute_z, 15, 0, 1);
          lightmapStep(absolute_x,block_y,absolute_z,15+stopLight[block]);
        }
      }
    }
  }

  //Blocklight
  for(uint8 block_x=0;block_x<16;block_x++)
  {
    for(uint8 block_z=0;block_z<16;block_z++)
    {
      for(int block_y=127;block_y>=0;block_y--)
      {
        int index=block_y + (block_z * 128) + (block_x * 128 * 16);

        //If light emitting block
        if(emitLight[blocks[index]])
        {
          int absolute_x=x*16+block_x;
          int absolute_z=z*16+block_z;          
          blocklightmapStep(absolute_x,block_y,absolute_z,emitLight[blocks[index]]);          
        }
      }
    }
  }
  return true;
}

bool Map::blocklightmapStep(int x, int y, int z, int light)
{
  uint8 block,meta;

  //If no light, stop!
  if(light<1) return false;

  for(uint8 i=0;i<6;i++)
  {
    int x_local=x;
    int y_local=y;
    int z_local=z;
    switch(i)
    {
      case 0: x_local++; break;
      case 1: x_local--; break;
      case 2: y_local++; break;
      case 3: y_local--; break;
      case 4: z_local++; break;
      case 5: z_local--; break;
    }

    if(getBlock(x_local,y_local,z_local,&block,&meta))
    {
      uint8 blocklight,skylight;
      getBlockLight(x_local,y_local,z_local, &blocklight, &skylight);
      if(blocklight<light+stopLight[block]-1)
      {
        setBlockLight(x_local,y_local,z_local, light+stopLight[block]-1,15,1);
        if(stopLight[block]!=-16)
        {
          blocklightmapStep(x_local,y_local,z_local, light+stopLight[block]-1);
        }
      }
    }
  }
  return true;
}

bool Map::lightmapStep(int x, int y, int z, int light)
{
  uint8 block,meta;

  //If no light, stop!
  if(light<1) return false;

  for(uint8 i=0;i<6;i++)
  {
    int x_local=x;
    int y_local=y;
    int z_local=z;
    switch(i)
    {
      case 0: x_local++; break;
      case 1: x_local--; break;
      case 2: y_local++; break;
      case 3: y_local--; break;
      case 4: z_local++; break;
      case 5: z_local--; break;
    }

    if(getBlock(x_local,y_local,z_local,&block,&meta))
    {
      uint8 blocklight,skylight;
      getBlockLight(x_local,y_local,z_local, &blocklight, &skylight);
      if(skylight<light+stopLight[block]-1)
      {
        setBlockLight(x_local,y_local,z_local,0, light+stopLight[block]-1,2);
        if(stopLight[block]!=-16)
        {
          lightmapStep(x_local,y_local,z_local, light+stopLight[block]-1);
        }
      }
    }
  }
  return true;
}


bool Map::getBlock(int x, int y, int z, uint8 *type, uint8 *meta){

  int chunk_x=((x<0)?((x+1)/16)-1:x/16);
  int chunk_z=((z<0)?((z+1)/16)-1:z/16);
  if(!loadMap(chunk_x,chunk_z))
  {
    LOG("Loadmap failed");
    return false;
  }
  NBT_struct *chunk=getMapData(chunk_x, chunk_z);
  if(!chunk || y<0 || y>127)
  {
    //LOG("chunk failed");
    return false;
  }

  int chunk_block_x=((x<0)?15+((x+1)%16):(x%16));
  int chunk_block_z=((z<0)?15+((z+1)%16):(z%16));

  uint8 *blocks=chunk->blocks;
  uint8 *metapointer=chunk->data;
  int index=y + (chunk_block_z * 128) + (chunk_block_x * 128 * 16);
  *type=blocks[index];
  uint8 metadata=metapointer[(index)>>1];
  
  if(y%2)
  {
    metadata&=0xf0;
    metadata>>=4;
  }
  else
  {
    metadata&=0x0f;
  }
  *meta=metadata;
  mapLastused[chunk_x][chunk_z]=(int)time(0);

  return true;
}

bool Map::getBlockLight(int x, int y, int z, uint8 *blocklight, uint8 *skylight)
{

  int chunk_x=((x<0)?((x+1)/16)-1:x/16);
  int chunk_z=((z<0)?((z+1)/16)-1:z/16);
  if(!loadMap(chunk_x,chunk_z))
  {
    LOG("Loadmap failed");
    return false;
  }
  NBT_struct *chunk=getMapData(chunk_x, chunk_z);
  if(!chunk || y<0 || y>127)
  {
    LOG("chunk failed");
    return false;
  }

  int chunk_block_x=((x<0)?15+((x+1)%16):(x%16));
  int chunk_block_z=((z<0)?15+((z+1)%16):(z%16));

  uint8 *blocklightpointer=chunk->blocklight;
  uint8 *skylightpointer=chunk->skylight;

  int index=y + (chunk_block_z * 128) + (chunk_block_x * 128 * 16);
  *blocklight=blocklightpointer[(index)>>1];
  *skylight=skylightpointer[(index)>>1];
  
  if(y%2)
  {
    *blocklight&=0xf0;
    *blocklight>>=4;

    *skylight&=0xf0;
    *skylight>>=4;
  }
  else
  {
    *blocklight&=0x0f;
    *skylight&=0x0f;
  }

  return true;
}

bool Map::setBlockLight(int x, int y, int z, uint8 blocklight, uint8 skylight, uint8 setLight)
{

  int chunk_x=((x<0)?((x+1)/16)-1:x/16);
  int chunk_z=((z<0)?((z+1)/16)-1:z/16);
  if(!loadMap(chunk_x,chunk_z))
  {
    return false;
  }
  NBT_struct *chunk=getMapData(chunk_x, chunk_z);
  if(!chunk || y<0 || y>127)
  {
    return false;
  }

  int chunk_block_x=((x<0)?15+((x+1)%16):(x%16));
  int chunk_block_z=((z<0)?15+((z+1)%16):(z%16));
  uint8 *blocklightpointer=chunk->blocklight;
  uint8 *skylightpointer=chunk->skylight;
  int index=y + (chunk_block_z * 128) + (chunk_block_x * 128 * 16);

  char skylight_local=skylightpointer[index>>1];
  char blocklight_local=blocklightpointer[index>>1];
  if(y%2)
  {
    if(setLight&0x6) // 2 or 4
    {
      skylight_local&=0x0f;
      skylight_local|=skylight<<4;
    }
    if(setLight&0x5) //1 or 4
    {
      blocklight_local&=0x0f;
      blocklight_local|=blocklight<<4;
    }
  }
  else
  {
    if(setLight&0x6) // 2 or 4
    {
      skylight_local&=0xf0;
      skylight_local|=skylight;
    }
    if(setLight&0x5) //1 or 4
    {
      blocklight_local&=0xf0;
      blocklight_local|=blocklight;
    }
  }

  if(setLight&0x6) // 2 or 4
  {
    skylightpointer[index>>1]=skylight_local;
  }
  if(setLight&0x5) //1 or 4
  {
    blocklightpointer[index>>1]=blocklight_local;
  }

  return true;
}

bool Map::setBlock(int x, int y, int z, char type, char meta)
{

  int chunk_x=((x<0)?((x+1)/16)-1:x/16);
  int chunk_z=((z<0)?((z+1)/16)-1:z/16);
  if(!loadMap(chunk_x,chunk_z))
  {
    return false;
  }
  NBT_struct *chunk=getMapData(chunk_x, chunk_z);
  if(!chunk || y<0 || y>127)
  {
    return false;
  }

  int chunk_block_x=((x<0)?15+((x+1)%16):(x%16));
  int chunk_block_z=((z<0)?15+((z+1)%16):(z%16));
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

  mapChanged[chunk_x][chunk_z]=1;
  mapLastused[chunk_x][chunk_z]=(int)time(0);

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
  mapLastused[x][z]=(int)time(0);

  if(getMapData(x,z)!=0)
  {
    return true;
  }

  int mapposx=x;
  int mapposz=z;
  //Generate map file name
  int modulox=(mapposx);
  while(modulox<0) modulox+=64;
  int moduloz=(mapposz);
  while(moduloz<0) moduloz+=64;
  modulox%=64;
  moduloz%=64;
  std::string infile=mapDirectory+"/"+base36_encode(modulox)+"/"+base36_encode(moduloz)+"/c."+base36_encode(mapposx)+"."+base36_encode(mapposz)+".dat";

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

  
  maps[x][z].blocks=get_NBT_pointer(&maps[x][z], "Blocks");
  maps[x][z].data=get_NBT_pointer(&maps[x][z], "Data");
  maps[x][z].blocklight=get_NBT_pointer(&maps[x][z], "BlockLight");
  maps[x][z].skylight=get_NBT_pointer(&maps[x][z], "SkyLight");
  //Check if the items were not found
  if(maps[x][z].blocks      == 0 ||
     maps[x][z].data        == 0 ||
     maps[x][z].blocklight  == 0 ||
     maps[x][z].skylight    == 0)
  {
    LOG("Error in map data");
    return false;
  }

  //Update last used time
  mapLastused[x][z]=(int)time(0);

  //Not changed
  mapChanged[x][z]=0;

  return true;
}

bool Map::saveMap(int x, int z)
{
  if(!mapChanged[x][z])
  {
    return true;
  }

  std::map<int, std::map<int, NBT_struct> >::iterator iter;
  iter = maps.find(x);
  if (iter != maps.end() )
  {
    std::map<int, NBT_struct>::iterator iter2;
    iter2 = maps[x].find(z);
    if (iter2 == maps[x].end() )
    {
      //Map not in memory!
      return false;
    }
  }
  else
  {
    //Map not in memory!
    return false;
  }

  //Recalculate light maps
  generateLightMaps(x,z);

  int mapposx=x;
  int mapposz=z;
  //Generate map file name
  int modulox=(mapposx);
  while(modulox<0) modulox+=64;
  int moduloz=(mapposz);
  while(moduloz<0) moduloz+=64;
  modulox%=64;
  moduloz%=64;
  std::string outfile=mapDirectory+"/"+base36_encode(modulox)+"/"+base36_encode(moduloz)+"/c."+base36_encode(mapposx)+"."+base36_encode(mapposz)+".dat";

  // Try to create parent directories if necessary
  struct stat stFileInfo;
  if (stat(outfile.c_str(), &stFileInfo) != 0)
  {
    std::string outdir_a = mapDirectory+"/"+base36_encode(modulox);
    std::string outdir_b = mapDirectory+"/"+base36_encode(modulox)+"/"+base36_encode(moduloz);

    if (stat(outdir_b.c_str(), &stFileInfo) != 0)
    {
      if (stat(outdir_a.c_str(), &stFileInfo) != 0)
      {
#ifdef WIN32
        if (mkdir(outdir_a.c_str()) == -1)
#else
        if (mkdir(outdir_a.c_str(), 0755) == -1)
#endif
        {
          return false;
        }
      }

#ifdef WIN32
      if (mkdir(outdir_b.c_str()) == -1)
#else
      if (mkdir(outdir_b.c_str(), 0755) == -1)
#endif
      {
        return false;
      }
    }
  }

  uint8 uncompressedData[200000];
  int dumpsize=dumpNBT_struct(&maps[x][z].compounds[0], &uncompressedData[0]);
  gzFile mapfile2=gzopen(outfile.c_str(),"wb");        
  gzwrite(mapfile2,&uncompressedData[0],dumpsize);
  gzclose(mapfile2);

  //Set "not changed"
  mapChanged[x][z]=0;
  return true;
}


bool Map::releaseMap(int x, int z)
{
  //save first
  saveMap(x,z);

  std::map<int, std::map<int, NBT_struct> >::iterator iter;
  iter = maps.find(x);
  std::map<int, NBT_struct>::iterator iter2;
  if (iter != maps.end() )
  {    
    iter2 = maps[x].find(z);
    if (iter2 == maps[x].end() )
    {
        mapChanged[x].erase(z);
      if(!mapChanged.count(x))
      {
        mapChanged.erase(x);
      }

      mapLastused[x].erase(z);
      if(!mapLastused.count(x))
      {
        mapLastused.erase(x);
      }
      //Map not in memory!
      return false;
    }
  }
  else
  {
    mapChanged[x].erase(z);
    if(!mapChanged.count(x))
    {
      mapChanged.erase(x);
    }

    mapLastused[x].erase(z);
    if(!mapLastused.count(x))
    {
      mapLastused.erase(x);
    }
    //Map not in memory!
    return false;
  }

  freeNBT_struct(&maps[x][z]);  
  //Erase from map
  maps[x].erase(z);
  if(!maps.count(x))
  {
    maps.erase(x);
  }
  mapChanged[x].erase(z);
  if(!mapChanged.count(x))
  {
    mapChanged.erase(x);
  }

  mapLastused[x].erase(z);
  if(!mapLastused.count(x))
  {
    mapLastused.erase(x);
  }
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


