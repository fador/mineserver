#ifdef WIN32
  #define _CRTDBG_MAP_ALLOC

  #include <crtdbg.h>
  #include <conio.h>
  #include <direct.h>
  #include <winsock2.h>
#endif
#include <string.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <algorithm>

#include "logger.h"

#include <sys/stat.h>

#include "tools.h"
#include "map.h"
#include <zlib.h>
#include "user.h"
#include "nbt.h"
#include "config.h"

Map &Map::get()
{
  static Map instance;
  return instance;
}

void Map::posToId(int x, int z, uint32 *id)
{
  uint8 *id_pointer=reinterpret_cast<uint8 *>(id);
  putSint16(&id_pointer[0],x);
  putSint16(&id_pointer[2],z);  
}

void Map::idToPos(uint32 id, int *x, int *z)
{
  uint8 *id_pointer=reinterpret_cast<uint8 *>(&id);
  *x=getSint16(&id_pointer[0]);
  *z=getSint16(&id_pointer[2]);
}

void Map::initMap()
{
#ifdef MSDBG
  printf("initMap()\n");
#endif

  this->mapDirectory = Conf::get().value("mapdir");
  if(this->mapDirectory == "Not found!")
  {
    std::cout << "Error, mapdir not defined!" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::string infile=mapDirectory+"/level.dat";

  struct stat stFileInfo;
  if(stat(infile.c_str(), &stFileInfo) != 0)
  {
    std::cout << "Error, map not found!" << std::endl;
    exit(EXIT_FAILURE);
  }

  // Read gzipped map file
  gzFile mapfile=gzopen(infile.c_str(), "rb");
  uint8 uncompressedData[200000];
  gzread(mapfile, &uncompressedData[0], 200000);
  gzclose(mapfile);

  // Save level data
  TAG_Compound(&uncompressedData[0], &levelInfo, true);

  if(!get_NBT_value(&levelInfo, "SpawnX", &spawnPos.x) ||
     !get_NBT_value(&levelInfo, "SpawnY", &spawnPos.y) ||
     !get_NBT_value(&levelInfo, "SpawnZ", &spawnPos.z))
  {
    std::cout << "Error, spawn pos not found from " << infile << "!" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::cout << "Spawn: (" << spawnPos.x << "," << spawnPos.y << "," << spawnPos.z << ")" << std::endl;
}

void Map::freeMap() {}

NBT_struct *Map::getMapData(int x, int z)
{
#ifdef MSDBG
  printf("getMapData(x=%d, z=%d)\n", x, z);
#endif

  uint32 mapId;
  Map::posToId(x, z, &mapId);
#ifdef MSDBG
  printf("Getting data for chunk %u\n", mapId);
#endif
  if (!maps.count(mapId) && !loadMap(x, z))
    return 0;

  // Update last used time
  mapLastused[mapId] = (int)time(0);

  // Data in memory
  return &maps[mapId];
}

bool Map::saveWholeMap()
{
#ifdef MSDBG
  printf("saveWholeMap()\n");
#endif

  for (std::map<uint32, NBT_struct>::const_iterator it = maps.begin(); it != maps.end(); ++it)
    saveMap(maps[it->first].x, maps[it->first].z);
  return true;
}

bool Map::generateLightMaps(int x, int z)
{
#ifdef MSDBG
  printf("generateLightMaps(x=%d, z=%d)\n", x, z);
#endif

  uint32 mapId;
  Map::posToId(x, z, &mapId);

  uint8 *skylight = maps[mapId].skylight;
  uint8 *blocklight = maps[mapId].blocklight;
  uint8 *blocks = maps[mapId].blocks;

  // Clear lightmaps
  memset(blocklight, 0, 16*16*128/2);
  memset(skylight, 0, 16*16*128/2);

  // Skylight

  // First set sunlight for all blocks until hit ground
  for(int block_x=0;block_x<16;block_x++)
  {
    for(int block_z=0;block_z<16;block_z++)
    {
      for(int block_y=127;block_y>0;block_y--)
      {
        int index = block_y + (block_z * 128) + (block_x * 128 * 16);
        int absolute_x = x*16+block_x;
        int absolute_z = z*16+block_z;
        uint8 block = blocks[index];

        setBlockLight(absolute_x, block_y, absolute_z, 0, 15, 2);

        if(stopLight[block] == -16)
        {
          break;
        }
      }
    }
  }

  // Loop again and now spread the light
  for(int block_x=0;block_x<16;block_x++)
  {
    for(int block_z=0;block_z<16;block_z++)
    {
      for(int block_y=127;block_y>=0;block_y--)
      {
        int index = block_y + (block_z * 128) + (block_x * 128 * 16);
        int absolute_x = x*16+block_x;
        int absolute_z = z*16+block_z;
        uint8 block = blocks[index];

        if(stopLight[block] == -16)
        {
          setBlockLight(absolute_x, block_y, absolute_z, 15+stopLight[block], 0, 1);
          break;
        }
        else
        {
          setBlockLight(absolute_x, block_y, absolute_z, 15, 0, 1);
          lightmapStep(absolute_x, block_y, absolute_z, 15+stopLight[block]);
        }
      }
    }
  }

  // Blocklight
  for(uint8 block_x=0;block_x<16;block_x++)
  {
    for(uint8 block_z=0;block_z<16;block_z++)
    {
      for(int block_y=127;block_y>=0;block_y--)
      {
        int index=block_y + (block_z * 128) + (block_x * 128 * 16);

        // If light emitting block
        if(emitLight[blocks[index]])
        {
          int absolute_x = x*16+block_x;
          int absolute_z = z*16+block_z;
          blocklightmapStep(absolute_x, block_y, absolute_z, emitLight[blocks[index]]);
        }
      }
    }
  }

  return true;
}

bool Map::blocklightmapStep(int x, int y, int z, int light)
{
#ifdef MSDBG
  printf("blocklightmapStep(x=%d, y=%d, z=%d, light=%d)\n", x, y, z, light);
#endif

  uint8 block, meta;

  // If no light, stop!
  if(light < 1) return false;

  for(uint8 i=0;i<6;i++)
  {
    // Going too high
    if((y == 127) && (i == 2))
    {
      i++;
    }
    // going negative
    else if((y == 0) && (i == 3))
    {
      i++;
    }

    int x_local = x;
    int y_local = y;
    int z_local = z;

    switch(i)
    {
      case 0: x_local++; break;
      case 1: x_local--; break;
      case 2: y_local++; break;
      case 3: y_local--; break;
      case 4: z_local++; break;
      case 5: z_local--; break;
    }

    if(getBlock(x_local, y_local, z_local, &block, &meta))
    {
      uint8 blocklight, skylight;

      getBlockLight(x_local, y_local, z_local, &blocklight, &skylight);

      if(blocklight<light+stopLight[block]-1)
      {
        setBlockLight(x_local, y_local, z_local, light+stopLight[block]-1, 15, 1);

        if(stopLight[block]!=-16)
          blocklightmapStep(x_local, y_local, z_local, light+stopLight[block]-1);
      }
    }
  }

  return true;
}

bool Map::lightmapStep(int x, int y, int z, int light)
{
#ifdef MSDBG
  printf("lightmapStep(x=%d, y=%d, z=%d, light=%d)\n", x, y, z, light);
#endif

  uint8 block, meta;

  // If no light, stop!
  if(light < 1)
    return false;

  for(uint8 i=0;i<6;i++)
  {
    // Going too high
    if(y==127 && i==2)
    {
      i++;
    }
    // going negative
    else if(y==0 && i==3)
    {
      i++;
    }

    int x_local = x;
    int y_local = y;
    int z_local = z;

    switch(i)
    {
      case 0: x_local++; break;
      case 1: x_local--; break;
      case 2: y_local++; break;
      case 3: y_local--; break;
      case 4: z_local++; break;
      case 5: z_local--; break;
    }

    //printf("getBlock(%d, %d, %d) (lightmapStep)\n", x_local, y_local, z_local);
    if(getBlock(x_local, y_local, z_local, &block, &meta))
    {
      uint8 blocklight, skylight;

      getBlockLight(x_local, y_local, z_local, &blocklight, &skylight);

      if(skylight<light+stopLight[block]-1)
      {
        setBlockLight(x_local, y_local, z_local, 0, light+stopLight[block]-1, 2);

        if(stopLight[block]!=-16)
          lightmapStep(x_local, y_local, z_local, light+stopLight[block]-1);
      }
    }
  }

  return true;
}

bool Map::getBlock(int x, int y, int z, uint8 *type, uint8 *meta){
#ifdef MSDBG
  printf("getBlock(x=%d, y=%d, z=%d)\n", x, y, z);
#endif

  if ((y < 0) || (y > 127))
  {
    LOG("Invalid y value (getBlock)");
    return false;
  }

  int chunk_x = ((x<0) ? (((x+1)/16)-1) : (x/16));
  int chunk_z = ((z<0) ? (((z+1)/16)-1) : (z/16));

  uint32 mapId;
  Map::posToId(chunk_x, chunk_z, &mapId);

  NBT_struct *chunk = getMapData(chunk_x, chunk_z);

  if(!chunk)
  {
    LOG("Loading chunk failed (getBlock)");
    return false;
  }

  int chunk_block_x = ((x<0) ? (15+((x+1)%16)) : (x%16));
  int chunk_block_z = ((z<0) ? (15+((z+1)%16)) : (z%16));

  uint8 *blocks = chunk->blocks;
  uint8 *metapointer = chunk->data;
  int index = y + (chunk_block_z * 128) + (chunk_block_x * 128 * 16);
  *type = blocks[index];
  uint8 metadata = metapointer[(index)>>1];

  if(y%2)
  {
    metadata&=0xf0;
    metadata>>=4;
  }
  else
  {
    metadata&=0x0f;
  }

  *meta = metadata;
  mapLastused[mapId] = (int)time(0);

  return true;
}

bool Map::getBlockLight(int x, int y, int z, uint8 *blocklight, uint8 *skylight)
{
#ifdef MSDBG
  printf("getBlockLight(x=%d, y=%d, z=%d)\n", x, y, z);
#endif

  if ((y < 0) || (y > 127))
  {
    LOG("Invalid y value (getBlockLight)");
    return false;
  }

  // Map chunk pos from block pos
  int chunk_x = ((x<0) ? (((x+1)/16)-1) : (x/16));
  int chunk_z = ((z<0) ? (((z+1)/16)-1) : (z/16));

  NBT_struct *chunk = getMapData(chunk_x, chunk_z);

  if(!chunk)
  {
    LOG("Loading chunk failed (getBlockLight)");
    return false;
  }

  // Which block inside the chunk
  int chunk_block_x = ((x<0) ? (15+((x+1)%16)) : (x%16));
  int chunk_block_z = ((z<0) ? (15+((z+1)%16)) : (z%16));

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
#ifdef MSDBG
  printf("setBlockLight(x=%d, y=%d, z=%d, %u, %u)\n", x, z, blocklight, skylight, setLight);
#endif

  if ((y < 0) || (y > 127))
  {
    LOG("Invalid y value (setBlockLight)");
    return false;
  }

  int chunk_x = ((x<0) ? (((x+1)/16)-1) : (x/16));
  int chunk_z = ((z<0) ? (((z+1)/16)-1) : (z/16));

  NBT_struct *chunk = getMapData(chunk_x, chunk_z);

  if(!chunk)
  {
    LOG("Loading chunk failed (setBlockLight)");
    return false;
  }

  int chunk_block_x = ((x<0) ? (15+((x+1)%16)) : (x%16));
  int chunk_block_z = ((z<0) ? (15+((z+1)%16)) : (z%16));

  uint8 *blocklightpointer = chunk->blocklight;
  uint8 *skylightpointer = chunk->skylight;
  int index = y + (chunk_block_z * 128) + (chunk_block_x * 128 * 16);
  char skylight_local = skylightpointer[index>>1];
  char blocklight_local = blocklightpointer[index>>1];

  if(y % 2)
  {
    if(setLight & 0x6) // 2 or 4
    {
      skylight_local &= 0x0f;
      skylight_local |= skylight<<4;
    }

    if(setLight & 0x5) // 1 or 4
    {
      blocklight_local &= 0x0f;
      blocklight_local |= blocklight<<4;
    }
  }
  else
  {
    if(setLight & 0x6) // 2 or 4
    {
      skylight_local &= 0xf0;
      skylight_local |= skylight;
    }

    if(setLight & 0x5) // 1 or 4
    {
      blocklight_local &= 0xf0;
      blocklight_local |= blocklight;
    }
  }

  if(setLight & 0x6) // 2 or 4
  {
    skylightpointer[index>>1] = skylight_local;
  }

  if(setLight & 0x5) // 1 or 4
  {
    blocklightpointer[index>>1] = blocklight_local;
  }

  return true;
}

bool Map::setBlock(int x, int y, int z, char type, char meta)
{
#ifdef MSDBG
  printf("setBlock(x=%d, y=%d, z=%d, type=%d, char=%d)\n", x, y, z, type, meta);
#endif

  if ((y < 0) || (y > 127))
  {
    LOG("Invalid y value (setBlock)");
    return false;
  }

  // Map chunk pos from block pos
  int chunk_x = ((x<0) ? (((x+1)/16)-1) : (x/16));
  int chunk_z = ((z<0) ? (((z+1)/16)-1) : (z/16));

  uint32 mapId;
  Map::posToId(chunk_x, chunk_z, &mapId);

  NBT_struct *chunk = getMapData(chunk_x, chunk_z);

  if(!chunk)
  {
    LOG("Loading chunk failed (setBlock)");
    return false;
  }

  // Which block inside the chunk
  int chunk_block_x = ((x<0) ? (15+((x+1)%16)) : (x%16));
  int chunk_block_z = ((z<0) ? (15+((z+1)%16)) : (z%16));

  uint8 *blocks = chunk->blocks;
  uint8 *metapointer = chunk->data;
  int index = y + (chunk_block_z * 128) + (chunk_block_x * 128 * 16);
  blocks[index] = type;
  char metadata = metapointer[index>>1];

  if(y%2)
  {
    metadata &= 0x0f;
    metadata |= meta<<4;
  }
  else
  {
    metadata &= 0xf0;
    metadata |= meta;
  }
  metapointer[index >> 1] = metadata;

  mapChanged[mapId] = 1;
  mapLastused[mapId] = (int)time(0);

  return true;
}

bool Map::sendBlockChange(int x, int y, int z, char type, char meta)
{
#ifdef MSDBG
  printf("sendBlockChange(x=%d, y=%d, z=%d, type=%d, meta=%d)\n", x, y, z, type, meta);
#endif

    uint8 curpos=0;
    uint8 changeArray[12];
    changeArray[0]=0x35; // Block change package
    curpos=1;
    putSint32(&changeArray[curpos], x);
    curpos+=4;
    changeArray[curpos]=y;
    curpos++;
    putSint32(&changeArray[curpos], z);
    curpos+=4;
    changeArray[10]=type;  // Replace block with
    changeArray[11]=meta;  // Metadata

    // TODO: only send to users in range
    for(unsigned int i=0;i<Users.size();i++)
    {
      bufferevent_write(Users[i]->buf_ev, &changeArray[0], 12);
    }

    return true;
}

bool Map::sendPickupSpawn(spawnedItem item)
{
  uint8 curpos=0;
  uint8 changeArray[23];
  changeArray[curpos]=0x15; // Pickup Spawn
  curpos++;
  putSint32(&changeArray[curpos], item.EID);
  curpos+=4;
  putSint16(&changeArray[curpos], item.item);
  curpos+=2;
  changeArray[curpos]=item.count;
  curpos++;

  putSint32(&changeArray[curpos], item.x);
  curpos+=4;
  putSint32(&changeArray[curpos], item.y);
  curpos+=4;
  putSint32(&changeArray[curpos], item.z);
  curpos+=4;
  changeArray[curpos]=0; // Rotation
  curpos++;
  changeArray[curpos]=0; // Pitch
  curpos++;
  changeArray[curpos]=0; // Roll
  curpos++;

  // TODO: only send to users in range
  for(unsigned int i=0;i<Users.size();i++)
  {
    bufferevent_write(Users[i]->buf_ev, &changeArray[0], 23);
  }

  return true;
}

bool Map::loadMap(int x, int z)
{
#ifdef MSDBG
  printf("loadMap(x=%d, z=%d)\n", x, z);
#endif

  uint32 mapId;
  Map::posToId(x, z, &mapId);

  // Generate map file name

  int mapposx=x;
  int modulox=(mapposx);
  while(modulox<0) modulox+=64;
  modulox%=64;

  int mapposz=z;
  int moduloz=(mapposz);
  while(moduloz<0) moduloz+=64;
  moduloz%=64;

  std::string infile=mapDirectory+"/"+base36_encode(modulox)+"/"+base36_encode(moduloz)+"/c."+base36_encode(mapposx)+"."+base36_encode(mapposz)+".dat";

  struct stat stFileInfo;
  if(stat(infile.c_str(), &stFileInfo) != 0)
  {
    LOG("File not found: " + infile);
    return false;
  }

  // Read gzipped map file
  gzFile mapfile=gzopen(infile.c_str(), "rb");
  uint8 uncompressedData[200000];
  gzread(mapfile, &uncompressedData[0], 200000);
  gzclose(mapfile);

  // Save this map data to map manager
  NBT_struct newMapStruct;
  TAG_Compound(&uncompressedData[0], &newMapStruct, true);

  maps[mapId] = newMapStruct;

  maps[mapId].x = x;
  maps[mapId].z = z;

  maps[mapId].blocks = get_NBT_pointer(&maps[mapId], "Blocks");
  maps[mapId].data = get_NBT_pointer(&maps[mapId], "Data");
  maps[mapId].blocklight = get_NBT_pointer(&maps[mapId], "BlockLight");
  maps[mapId].skylight = get_NBT_pointer(&maps[mapId], "SkyLight");
  // Check if the items were not found
  if(maps[mapId].blocks      == 0 ||
     maps[mapId].data        == 0 ||
     maps[mapId].blocklight  == 0 ||
     maps[mapId].skylight    == 0)
  {
    LOG("Error in map data");
    return false;
  }

  // Update last used time
  mapLastused[mapId] = (int)time(0);

  // Not changed
  mapChanged[mapId] = 0;

  return true;
}

bool Map::saveMap(int x, int z)
{
#ifdef MSDBG
  printf("saveMap(x=%d, z=%d)\n", x, z);
#endif

  uint32 mapId;
  Map::posToId(x, z, &mapId);

  if(!mapChanged[mapId])
    return true;

  if (!maps.count(mapId))
    return false;

  // Recalculate light maps
  generateLightMaps(x, z);

  // Generate map file name

  int mapposx=x;
  int modulox=(mapposx);
  while(modulox<0) modulox+=64;
  modulox%=64;

  int mapposz=z;
  int moduloz=(mapposz);
  while(moduloz<0) moduloz+=64;
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
  int dumpsize=dumpNBT_struct(&maps[mapId].compounds[0], &uncompressedData[0]);
  gzFile mapfile2=gzopen(outfile.c_str(), "wb");
  gzwrite(mapfile2, &uncompressedData[0], dumpsize);
  gzclose(mapfile2);

  // Set "not changed"
  mapChanged[mapId] = 0;

  return true;
}

bool Map::releaseMap(int x, int z)
{
  // save first
  saveMap(x, z);

  uint32 mapId;
  Map::posToId(x, z, &mapId);

  mapChanged.erase(mapId);
  mapLastused.erase(mapId);
  if (maps.count(mapId))
    freeNBT_struct(&maps[mapId]);

  return maps.erase(mapId)?true:false;
}

// Send chunk to user
void Map::sendToUser(User *user, int x, int z)
{
#ifdef MSDBG
  printf("sendToUser(x=%d, z=%d)\n", x, z);
#endif

  uint32 mapId;
  Map::posToId(x, z, &mapId);

  uint8 data4[18+81920];
  uint8 mapdata[81920]={0};
  int mapposx=x;
  int mapposz=z;

  if(loadMap(x, z))
  {
    // Pre chunk
    data4[0]=0x32;
    putSint32(&data4[1], mapposx);
    putSint32(&data4[5], mapposz);
    data4[9]=1; // Init chunk
    bufferevent_write(user->buf_ev, (uint8 *)&data4[0], 10);

    // Chunk
    data4[0]=0x33;

    data4[11]=15;  // Size_x
    data4[12]=127; // Size_y
    data4[13]=15;  // Size_z

    memcpy(&mapdata[0], maps[mapId].blocks, 32768);
    memcpy(&mapdata[32768], maps[mapId].data, 16384);
    memcpy(&mapdata[32768+16384], maps[mapId].blocklight, 16384);
    memcpy(&mapdata[32768+16384+16384], maps[mapId].skylight, 16384);

    putSint32(&data4[1], mapposx*16);
    data4[5]=0;
    data4[6]=0;
    putSint32(&data4[7], mapposz*16);

    uLongf written=81920;

    // Compress data with zlib deflate
    compress((uint8 *)&data4[18], &written, (uint8 *)&mapdata[0], 81920);

    putSint32(&data4[14], written);
    bufferevent_write(user->buf_ev, (uint8 *)&data4[0], 18+written);
  }
}
