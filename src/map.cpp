/*
   Copyright (c) 2010, The Mineserver Project
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
 * Neither the name of the The Mineserver Project nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef WIN32
  #define _CRTDBG_MAP_ALLOC
//  #define ZLIB_WINAPI

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

#include <zlib.h>
#include <sys/stat.h>

#include "logger.h"
#include "tools.h"
#include "map.h"

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
  uint8 *id_pointer = reinterpret_cast<uint8 *>(id);
  putSint16(&id_pointer[0], x);
  putSint16(&id_pointer[2], z);
}

void Map::idToPos(uint32 id, int *x, int *z)
{
  uint8 *id_pointer = reinterpret_cast<uint8 *>(&id);
  *x = getSint16(&id_pointer[0]);
  *z = getSint16(&id_pointer[2]);
}

void Map::initMap()
{
#ifdef MSDBG
  printf("initMap()\n");
#endif

  this->mapDirectory = Conf::get().sValue("mapdir");
  if(this->mapDirectory == "Not found!")
  {
    std::cout << "Error, mapdir not defined!" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::string infile = mapDirectory+"/level.dat";

  struct stat stFileInfo;
  if(stat(infile.c_str(), &stFileInfo) != 0)
  {
    std::cout << "Error, map not found!" << std::endl;
    exit(EXIT_FAILURE);
  }

  // Read gzipped map file
  gzFile mapfile          = gzopen(infile.c_str(), "rb");
  uint8 *uncompressedData = new uint8[ALLOCATE_NBTFILE];
  gzread(mapfile, uncompressedData, ALLOCATE_NBTFILE);
  gzclose(mapfile);

  // Save level data
  TAG_Compound(uncompressedData, &levelInfo, true);

  delete[] uncompressedData;

  if(!get_NBT_value(&levelInfo, "SpawnX", &spawnPos.x()) ||
     !get_NBT_value(&levelInfo, "SpawnY", &spawnPos.y()) ||
     !get_NBT_value(&levelInfo, "SpawnZ", &spawnPos.z()))
  {
    std::cout << "Error, spawn pos not found from " << infile << "!" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::cout << "Spawn: (" << spawnPos.x() << "," << spawnPos.y() << "," << spawnPos.z() << ")"<<
  std::endl;
}

void Map::freeMap()
{
}

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
  if(!maps.count(mapId) && !loadMap(x, z))
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

  for(std::map<uint32, NBT_struct>::const_iterator it = maps.begin(); it != maps.end(); ++it)
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

  uint8 *skylight   = maps[mapId].skylight;
  uint8 *blocklight = maps[mapId].blocklight;
  uint8 *blocks     = maps[mapId].blocks;

  // Clear lightmaps
  memset(blocklight, 0, 16*16*128/2);
  memset(skylight, 0, 16*16*128/2);

  // Skylight

  // First set sunlight for all blocks until hit ground
  for(int block_x = 0; block_x < 16; block_x++)
  {
    for(int block_z = 0; block_z < 16; block_z++)
    {
      for(int block_y = 127; block_y > 0; block_y--)
      {
        int index      = block_y + (block_z * 128) + (block_x * 128 * 16);
        int absolute_x = x*16+block_x;
        int absolute_z = z*16+block_z;
        uint8 block    = blocks[index];

        setBlockLight(absolute_x, block_y, absolute_z, 0, 15, 2);

        if(stopLight[block] == -16)
          break;
      }
    }
  }

  // Loop again and now spread the light
  for(int block_x = 0; block_x < 16; block_x++)
  {
    for(int block_z = 0; block_z < 16; block_z++)
    {
      for(int block_y = 127; block_y >= 0; block_y--)
      {
        int index      = block_y + (block_z * 128) + (block_x * 128 * 16);
        int absolute_x = x*16+block_x;
        int absolute_z = z*16+block_z;
        uint8 block    = blocks[index];

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
  for(uint8 block_x = 0; block_x < 16; block_x++)
  {
    for(uint8 block_z = 0; block_z < 16; block_z++)
    {
      for(int block_y = 127; block_y >= 0; block_y--)
      {
        int index = block_y + (block_z * 128) + (block_x * 128 * 16);

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
  if(light < 1)
    return false;

  for(uint8 i = 0; i < 6; i++)
  {
    // Going too high
    if((y == 127) && (i == 2))
      i++;
    // going negative
    else if((y == 0) && (i == 3))
      i++;

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

      if(blocklight < light+stopLight[block]-1)
      {
        setBlockLight(x_local, y_local, z_local, light+stopLight[block]-1, 15, 1);

        if(stopLight[block] != -16)
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

  for(uint8 i = 0; i < 6; i++)
  {
    // Going too high
    if(y == 127 && i == 2)
      i++;
    // going negative
    else if(y == 0 && i == 3)
      i++;

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

      if(skylight < light+stopLight[block]-1)
      {
        setBlockLight(x_local, y_local, z_local, 0, light+stopLight[block]-1, 2);

        if(stopLight[block] != -16)
          lightmapStep(x_local, y_local, z_local, light+stopLight[block]-1);
      }
    }
  }

  return true;
}

bool Map::getBlock(int x, int y, int z, uint8 *type, uint8 *meta)
{
#ifdef MSDBG
  printf("getBlock(x=%d, y=%d, z=%d)\n", x, y, z);
#endif

  if((y < 0) || (y > 127))
  {
    printf("(%i, %i, %i) ", x, y, z);
    LOG("Invalid y value (getBlock)");
    return false;
  }

  int chunk_x = blockToChunk(x);
  int chunk_z = blockToChunk(z);

  uint32 mapId;
  Map::posToId(chunk_x, chunk_z, &mapId);

  NBT_struct *chunk = getMapData(chunk_x, chunk_z);

  if(!chunk)
  {
    LOG("Loading chunk failed (getBlock)");
    return false;
  }

  int chunk_block_x  = blockToChunkBlock(x);
  int chunk_block_z  = blockToChunkBlock(z);

  uint8 *blocks      = chunk->blocks;
  uint8 *metapointer = chunk->data;
  int index          = y + (chunk_block_z * 128) + (chunk_block_x * 128 * 16);
  *type = blocks[index];
  uint8 metadata     = metapointer[(index)>>1];

  if(y%2)
  {
    metadata  &= 0xf0;
    metadata >>= 4;
  }
  else
    metadata &= 0x0f;

  *meta              = metadata;
  mapLastused[mapId] = (int)time(0);

  return true;
}

bool Map::getBlockLight(int x, int y, int z, uint8 *blocklight, uint8 *skylight)
{
#ifdef MSDBG
  printf("getBlockLight(x=%d, y=%d, z=%d)\n", x, y, z);
#endif

  if((y < 0) || (y > 127))
  {
    LOG("Invalid y value (getBlockLight)");
    return false;
  }

  // Map chunk pos from block pos
  int chunk_x       = blockToChunk(x);
  int chunk_z       = blockToChunk(z);

  NBT_struct *chunk = getMapData(chunk_x, chunk_z);

  if(!chunk)
  {
    LOG("Loading chunk failed (getBlockLight)");
    return false;
  }

  // Which block inside the chunk
  int chunk_block_x        = blockToChunkBlock(x);
  int chunk_block_z        = blockToChunkBlock(z);

  uint8 *blocklightpointer = chunk->blocklight;
  uint8 *skylightpointer   = chunk->skylight;
  int index                = y + (chunk_block_z * 128) + (chunk_block_x * 128 * 16);
  *blocklight = blocklightpointer[(index)>>1];
  *skylight   = skylightpointer[(index)>>1];

  if(y%2)
  {
    *blocklight  &= 0xf0;
    *blocklight >>= 4;

    *skylight    &= 0xf0;
    *skylight   >>= 4;
  }
  else
  {
    *blocklight &= 0x0f;
    *skylight   &= 0x0f;
  }

  return true;
}

bool Map::setBlockLight(int x, int y, int z, uint8 blocklight, uint8 skylight, uint8 setLight)
{
#ifdef MSDBG
  printf("setBlockLight(x=%d, y=%d, z=%d, %u, %u)\n", x, z, blocklight, skylight, setLight);
#endif

  if((y < 0) || (y > 127))
  {
    LOG("Invalid y value (setBlockLight)");
    return false;
  }

  int chunk_x       = blockToChunk(x);
  int chunk_z       = blockToChunk(z);

  NBT_struct *chunk = getMapData(chunk_x, chunk_z);

  if(!chunk)
  {
    LOG("Loading chunk failed (setBlockLight)");
    return false;
  }

  int chunk_block_x        = blockToChunkBlock(x);
  int chunk_block_z        = blockToChunkBlock(z);

  uint8 *blocklightpointer = chunk->blocklight;
  uint8 *skylightpointer   = chunk->skylight;
  int index                = y + (chunk_block_z * 128) + (chunk_block_x * 128 * 16);
  char skylight_local      = skylightpointer[index>>1];
  char blocklight_local    = blocklightpointer[index>>1];

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

    skylightpointer[index>>1] = skylight_local;

  if(setLight & 0x5) // 1 or 4

    blocklightpointer[index>>1] = blocklight_local;

  return true;
}

bool Map::setBlock(int x, int y, int z, char type, char meta)
{
#ifdef MSDBG
  printf("setBlock(x=%d, y=%d, z=%d, type=%d, char=%d)\n", x, y, z, type, meta);
#endif

  if((y < 0) || (y > 127))
  {
    LOG("Invalid y value (setBlock)");
    return false;
  }

  // Map chunk pos from block pos
  int chunk_x = ((x < 0) ? (((x+1)/16)-1) : (x/16));
  int chunk_z = ((z < 0) ? (((z+1)/16)-1) : (z/16));

  uint32 mapId;
  Map::posToId(chunk_x, chunk_z, &mapId);

  NBT_struct *chunk = getMapData(chunk_x, chunk_z);

  if(!chunk)
  {
    LOG("Loading chunk failed (setBlock)");
    return false;
  }

  // Which block inside the chunk
  int chunk_block_x  = ((x < 0) ? (15+((x+1)%16)) : (x%16));
  int chunk_block_z  = ((z < 0) ? (15+((z+1)%16)) : (z%16));

  uint8 *blocks      = chunk->blocks;
  uint8 *metapointer = chunk->data;
  int index          = y + (chunk_block_z * 128) + (chunk_block_x * 128 * 16);
  blocks[index] = type;
  char metadata      = metapointer[index>>1];

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

  mapChanged[mapId]       = 1;
  mapLastused[mapId]      = (int)time(0);

  return true;
}

bool Map::sendBlockChange(int x, int y, int z, char type, char meta)
{
#ifdef MSDBG
  printf("sendBlockChange(x=%d, y=%d, z=%d, type=%d, meta=%d)\n", x, y, z, type, meta);
#endif

  uint8 curpos = 0;
  uint8 changeArray[12];
  changeArray[0]      = 0x35; // Block change package
  curpos              = 1;
  putSint32(&changeArray[curpos], x);
  curpos             += 4;
  changeArray[curpos] = y;
  curpos++;
  putSint32(&changeArray[curpos], z);
  curpos         += 4;
  changeArray[10] = type;  // Replace block with
  changeArray[11] = meta;  // Metadata

  // TODO: only send to users in range
  for(unsigned int i = 0; i < Users.size(); i++)
  {
    bufferevent_write(Users[i]->buf_ev, &changeArray[0], 12);
  }

  return true;
}

bool Map::sendPickupSpawn(spawnedItem item)
{
  //Push to global item storage
  spawnedItem *storedItem = new spawnedItem;
  *storedItem     = item;
  items[item.EID] = storedItem;

  //Push to local item storage
  int chunk_x = blockToChunk(item.pos.x()/32);
  int chunk_z = blockToChunk(item.pos.z()/32);
  uint32 chunkHash;
  posToId(chunk_x, chunk_z, &chunkHash);
  mapItems[chunkHash].push_back(storedItem);


  uint8 curpos = 0;
  uint8 changeArray[23];
  changeArray[curpos] = 0x15; // Pickup Spawn
  curpos++;
  putSint32(&changeArray[curpos], item.EID);
  curpos             += 4;
  putSint16(&changeArray[curpos], item.item);
  curpos             += 2;
  changeArray[curpos] = item.count;
  curpos++;

  putSint32(&changeArray[curpos], item.pos.x());
  curpos             += 4;
  putSint32(&changeArray[curpos], item.pos.y());
  curpos             += 4;
  putSint32(&changeArray[curpos], item.pos.z());
  curpos             += 4;
  changeArray[curpos] = 0; // Rotation
  curpos++;
  changeArray[curpos] = 0; // Pitch
  curpos++;
  changeArray[curpos] = 0; // Roll
  curpos++;

  // TODO: only send to users in range
  for(unsigned int i = 0; i < Users.size(); i++)
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

  int mapposx = x;
  int modulox = (mapposx);
  while(modulox < 0)
    modulox += 64;
  modulox %= 64;

  int mapposz = z;
  int moduloz = (mapposz);
  while(moduloz < 0)
    moduloz += 64;
  moduloz %= 64;

  std::string infile = mapDirectory+"/"+base36_encode(modulox)+"/"+base36_encode(moduloz)+"/c."+
                       base36_encode(mapposx)+"."+base36_encode(mapposz)+".dat";

  struct stat stFileInfo;
  if(stat(infile.c_str(), &stFileInfo) != 0)
  {
    LOG("File not found: " + infile);
    return false;
  }

  // Read gzipped map file
  gzFile mapfile          = gzopen(infile.c_str(), "rb");
  uint8 *uncompressedData = new uint8[ALLOCATE_NBTFILE];
  gzread(mapfile, uncompressedData, ALLOCATE_NBTFILE);
  gzclose(mapfile);

  // Save this map data to map manager
  NBT_struct newMapStruct;
  TAG_Compound(uncompressedData, &newMapStruct, true);

  delete[] uncompressedData;

  maps[mapId]            = newMapStruct;

  maps[mapId].x          = x;
  maps[mapId].z          = z;

  maps[mapId].blocks     = get_NBT_pointer(&maps[mapId], "Blocks");
  maps[mapId].data       = get_NBT_pointer(&maps[mapId], "Data");
  maps[mapId].blocklight = get_NBT_pointer(&maps[mapId], "BlockLight");
  maps[mapId].skylight   = get_NBT_pointer(&maps[mapId], "SkyLight");
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

  if(!maps.count(mapId))
    return false;

  // Recalculate light maps
  generateLightMaps(x, z);

  // Generate map file name

  int mapposx = x;
  int modulox = (mapposx);
  while(modulox < 0)
    modulox += 64;
  modulox %= 64;

  int mapposz = z;
  int moduloz = (mapposz);
  while(moduloz < 0)
    moduloz += 64;
  moduloz %= 64;

  std::string outfile = mapDirectory+"/"+base36_encode(modulox)+"/"+base36_encode(moduloz)+"/c."+
                        base36_encode(mapposx)+"."+base36_encode(mapposz)+".dat";

  // Try to create parent directories if necessary
  struct stat stFileInfo;
  if(stat(outfile.c_str(), &stFileInfo) != 0)
  {
    std::string outdir_a = mapDirectory+"/"+base36_encode(modulox);
    std::string outdir_b = mapDirectory+"/"+base36_encode(modulox)+"/"+base36_encode(moduloz);

    if(stat(outdir_b.c_str(), &stFileInfo) != 0)
    {
      if(stat(outdir_a.c_str(), &stFileInfo) != 0)
      {
#ifdef WIN32
        if(_mkdir(outdir_a.c_str()) == -1)
#else
        if(mkdir(outdir_a.c_str(), 0755) == -1)
#endif

          return false;
      }

#ifdef WIN32
      if(_mkdir(outdir_b.c_str()) == -1)
#else
      if(mkdir(outdir_b.c_str(), 0755) == -1)
#endif

        return false;
    }
  }

  uint8 *uncompressedData = new uint8[ALLOCATE_NBTFILE];
  int dumpsize            = dumpNBT_struct(&maps[mapId], uncompressedData);
  gzFile mapfile2         = gzopen(outfile.c_str(), "wb");
  gzwrite(mapfile2, uncompressedData, dumpsize);
  gzclose(mapfile2);

  delete[] uncompressedData;

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
  if(maps.count(mapId))
    freeNBT_struct(&maps[mapId]);

  return maps.erase(mapId) ? true : false;
}

// Send chunk to user
void Map::sendToUser(User *user, int x, int z)
{
#ifdef MSDBG
  printf("sendToUser(x=%d, z=%d)\n", x, z);
#endif

  uint32 mapId;
  Map::posToId(x, z, &mapId);

  uint8 *data4   = new uint8[18+81920];
  uint8 *mapdata = new uint8[81920];
  int mapposx    = x;
  int mapposz    = z;

  if(loadMap(x, z))
  {
    // Pre chunk
    data4[0] = 0x32;
    putSint32(&data4[1], mapposx);
    putSint32(&data4[5], mapposz);
    data4[9] = 1; // Init chunk
    bufferevent_write(user->buf_ev, (uint8 *)&data4[0], 10);

    // Chunk
    data4[0]  = 0x33;

    data4[11] = 15;  // Size_x
    data4[12] = 127; // Size_y
    data4[13] = 15;  // Size_z

    memcpy(&mapdata[0], maps[mapId].blocks, 32768);
    memcpy(&mapdata[32768], maps[mapId].data, 16384);
    memcpy(&mapdata[32768+16384], maps[mapId].blocklight, 16384);
    memcpy(&mapdata[32768+16384+16384], maps[mapId].skylight, 16384);

    putSint32(&data4[1], mapposx*16);
    data4[5] = 0;
    data4[6] = 0;
    putSint32(&data4[7], mapposz*16);

    uLongf written = 81920;

    // Compress data with zlib deflate
    compress(&data4[18], &written, &mapdata[0], 81920);

    putSint32(&data4[14], written);
    bufferevent_write(user->buf_ev, &data4[0], 18+written);

    //Get list of chests,furnaces etc on the chunk
    NBT_list *entitylist = get_NBT_list(&maps[mapId], "TileEntities");

    //Verify the type
    if(entitylist && entitylist->tagId==TAG_COMPOUND)
    {
      uint8 *structdump = new uint8 [ALLOCATE_NBTFILE];
      uint8 *packetData = new uint8[ALLOCATE_NBTFILE];

      NBT_struct **tempstruct=(NBT_struct **)entitylist->items;
      //Loop through every item
      for(sint32 i=0;i<entitylist->length;i++)
      {
        std::string id;
        //Just send chest,sign and furnace data for now
        if(get_NBT_value(tempstruct[i], "id", &id) && (id=="Chest" || id=="Furnace" || id=="Sign"))
        {
          //Get position
          int entity_x,entity_y,entity_z;
          if(!get_NBT_value(tempstruct[i], "x", &entity_x) ||
             !get_NBT_value(tempstruct[i], "y", &entity_y) ||
             !get_NBT_value(tempstruct[i], "z", &entity_z))
          {
            continue;
          }
          //Dump the struct data to NBT format
          int dumped=dumpNBT_struct(tempstruct[i], structdump);
          written=ALLOCATE_NBTFILE;
          z_stream zstream2;
          zstream2.zalloc = Z_NULL;
          zstream2.zfree = Z_NULL;
          zstream2.opaque = Z_NULL;
          zstream2.next_out=&packetData[13];
          zstream2.next_in=structdump;
          zstream2.avail_in=dumped;
          zstream2.avail_out=written;
          zstream2.total_out=0;
          zstream2.total_in=0;
          deflateInit2(&zstream2, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15+MAX_WBITS, 8,
                               Z_DEFAULT_STRATEGY);
   
          //Gzip the data
          if(int state=deflate(&zstream2,Z_FULL_FLUSH)!=Z_OK)
          {
            std::cout << "Error in deflate: " << state << std::endl;
            deflateEnd(&zstream2);
          }
          else
          {
            written=zstream2.total_out;
    
            // !!!! Complex Entity packet! !!!!
            packetData[0] = 0x3b; //Complex Entities
            putSint32(&packetData[1],entity_x); //X-pos
            putSint16(&packetData[5],entity_y); //Y-pos
            putSint32(&packetData[7],entity_z); //Z-pos
            putSint16(&packetData[11], (sint16)written); //Size
            bufferevent_write(user->buf_ev, (uint8 *)&packetData[0], 13+written);
          }
        }
      }

      delete [] packetData;
      delete [] structdump;
    }
  }

  delete[] data4;
  delete[] mapdata;
}
