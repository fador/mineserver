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
#else
  #include <netinet/in.h>
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
#include "mapgen.h"

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

  NBT_Value *root = NBT_Value::LoadFromFile(infile);

  NBT_Value &data = *((*root)["Data"]);

  spawnPos.x() = (sint32)*data["SpawnX"];
  spawnPos.y() = (sint32)*data["SpawnY"];
  spawnPos.z() = (sint32)*data["SpawnZ"];


  root->SaveToFile("test.nbt");

  delete root;

  std::cout << "Spawn: (" << spawnPos.x() << "," << spawnPos.y() << "," << spawnPos.z() << ")"<<
  std::endl;
}

void Map::freeMap()
{
}

sChunk *Map::getMapData(int x, int z, bool generate)
{
#ifdef MSDBG
  printf("getMapData(x=%d, z=%d)\n", x, z);
#endif

  uint32 mapId;
  Map::posToId(x, z, &mapId);
#ifdef MSDBG
  printf("Getting data for chunk %u\n", mapId);
#endif
  if(!maps.count(mapId) && (!generate || !loadMap(x, z, generate)))
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

  for(std::map<uint32, sChunk>::const_iterator it = maps.begin(); it != maps.end(); ++it)
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

  uint8 highest_y = 0;

  uint8 *skylight   = maps[mapId].skylight;
  uint8 *blocklight = maps[mapId].blocklight;
  uint8 *blocks     = maps[mapId].blocks;
  uint8 *heightmap  = maps[mapId].heightmap;

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
        {
          //Calculate heightmap while looping this
          heightmap[block_z+(block_x<<4)] = ((block_y==127)?block_y:block_y+1);
          if(block_y>highest_y)
          {
            highest_y=block_y;
          }
          break;
        }
      }
    }
  }

  // Loop again and now spread the light
  for(int block_x = 0; block_x < 16; block_x++)
  {
    for(int block_z = 0; block_z < 16; block_z++)
    {
      //Start from highest pos of the chunk, might still mess lighting
      // if neighboring chunks are higher..
      for(int block_y = highest_y; block_y >= 0; block_y--)
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
      //Start searching from first block pos
      for(int block_y = heightmap[block_z+(block_x<<4)]; block_y >= 0; block_y--)
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

    if(getBlock(x_local, y_local, z_local, &block, &meta, false))
    {
      uint8 blocklight, skylight;

      getBlockLight(x_local, y_local, z_local, &blocklight, &skylight);

      if(blocklight < light+stopLight[block]-1)
      {
        setBlockLight(x_local, y_local, z_local, light+stopLight[block]-1, 0, 1);

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
    if(getBlock(x_local, y_local, z_local, &block, &meta, false))
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

bool Map::getBlock(int x, int y, int z, uint8 *type, uint8 *meta, bool generate)
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

  sChunk *chunk = getMapData(chunk_x, chunk_z, generate);

  if(!chunk)
  {
    if(generate)
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

  sChunk *chunk = getMapData(chunk_x, chunk_z, false);

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

  sChunk *chunk = getMapData(chunk_x, chunk_z, false);

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

  sChunk *chunk = getMapData(chunk_x, chunk_z);

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

  Packet pkt;
  pkt << PACKET_BLOCK_CHANGE << (sint32)x << (sint8)y << (sint32)z << (sint8)type << (sint8)meta;


  // TODO: only send to users in range
  for(unsigned int i = 0; i < Users.size(); i++)
  {
    Users[i]->buffer.addToWrite(pkt.getWrite(), pkt.getWriteLen());
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

  Packet pkt;
  pkt << PACKET_PICKUP_SPAWN << (sint32)item.EID << (sint16)item.item << (sint8)item.count
    << (sint32)item.pos.x() << (sint32)item.pos.y() << (sint32)item.pos.z()
    << (sint8)0 << (sint8)0 << (sint8)0;

  // TODO: only send to users in range
  for(unsigned int i = 0; i < Users.size(); i++)
  {
    Users[i]->buffer.addToWrite(pkt.getWrite(), pkt.getWriteLen());
  }

  return true;
}

void Map::createPickupSpawn(int x, int y, int z, int type, int count)
{
   spawnedItem item;
   item.EID      = generateEID();
   item.health   = 0;
   item.item     = type;
   item.count    = count;

   item.pos.x()  = x*32;
   item.pos.y()  = y*32;
   item.pos.z()  = z*32;
   //Randomize spawn position a bit
   item.pos.x() += 5+(rand()%22);
   item.pos.z() += 5+(rand()%22);

   this->sendPickupSpawn(item);
}

bool Map::loadMap(int x, int z, bool generate)
{
#ifdef MSDBG
  printf("loadMap(x=%d, z=%d)\n", x, z);
#endif

  uint32 mapId;
  Map::posToId(x, z, &mapId);

  if(maps.count(mapId))
    return true;

  // Generate map file name

  int mapposx = x;
  int modulox = (mapposx) & 0x3F;

  int mapposz = z;
  int moduloz = (mapposz) & 0x3F;

  std::string infile = mapDirectory+"/"+base36_encode(modulox)+"/"+base36_encode(moduloz)+"/c."+
                       base36_encode(mapposx)+"."+base36_encode(mapposz)+".dat";

  struct stat stFileInfo;
  if(stat(infile.c_str(), &stFileInfo) != 0)
  {
    //std::cout << "Mappos: " << x << "," << z << std::endl;
    
    // If generate (false only for lightmapgenerator)
    if(generate)
    {    
      MapGen mapgen((int)time(NULL));
      mapgen.generateChunk(x,z);
      generateLightMaps(x, z);
      return true;
    } 
    else 
    {
      return false;
    }
  }
  else 
  {
    maps[mapId].nbt = NBT_Value::LoadFromFile(infile.c_str());
  }
  
  
  if(maps[mapId].nbt == NULL)
  {
    LOG("Error in loading map (unable to load file)");
    return false;
  }

  NBT_Value &level = *(*maps[mapId].nbt)["Level"];

  maps[mapId].x = (sint32)(*level["xPos"]);
  maps[mapId].z = (sint32)(*level["zPos"]);

  if(maps[mapId].x != x || maps[mapId].z != z)
  {
    LOG("Error in loading map (incorrect chunk)");
    return false;
  }

  std::vector<uint8> *blocks = level["Blocks"]->GetByteArray();
  std::vector<uint8> *data = level["Data"]->GetByteArray();
  std::vector<uint8> *blocklight = level["BlockLight"]->GetByteArray();
  std::vector<uint8> *skylight = level["SkyLight"]->GetByteArray();
  std::vector<uint8> *heightmap = level["HeightMap"]->GetByteArray();

  if(blocks == 0 || data == 0 || blocklight == 0 || skylight == 0 || heightmap == 0)
  {
    LOG("Error in loading map (chunk missing data)");
    return false;
  }

  size_t fullLen = (16 * 128 * 16);
  size_t halfLen = fullLen >> 1;

  if(blocks->size() != fullLen ||
    data->size() != halfLen ||
    blocklight->size() != halfLen ||
    skylight->size() != halfLen)
  {
    LOG("Error in loading map (corrupt?)");
    return false;
  }

  maps[mapId].blocks = &((*blocks)[0]);
  maps[mapId].data = &((*data)[0]);
  maps[mapId].blocklight = &((*blocklight)[0]);
  maps[mapId].skylight = &((*skylight)[0]);
  maps[mapId].heightmap = &((*heightmap)[0]);

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

  maps[mapId].nbt->SaveToFile(outfile);

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
  {
    delete maps[mapId].nbt;
  }

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
  sint32 mapposx    = x;
  sint32 mapposz    = z;

  if(loadMap(x, z))
  {
    // Pre chunk
  user->buffer << (sint8)PACKET_PRE_CHUNK << mapposx << mapposz << (sint8)1;

    // Chunk
  user->buffer << (sint8)PACKET_MAP_CHUNK << (sint32)(mapposx * 16) << (sint16)0 << (sint32)(mapposz * 16) 
      << (sint8)15 << (sint8)127 << (sint8)15;

    memcpy(&mapdata[0], maps[mapId].blocks, 32768);
    memcpy(&mapdata[32768], maps[mapId].data, 16384);
    memcpy(&mapdata[32768+16384], maps[mapId].blocklight, 16384);
    memcpy(&mapdata[32768+16384+16384], maps[mapId].skylight, 16384);

    uLongf written = 81920;
    Bytef *buffer = new Bytef[written];

    // Compress data with zlib deflate
    compress(buffer, &written, &mapdata[0], 81920);

    user->buffer << (sint32)written;
    user->buffer.addToWrite(buffer, written);

    //Get list of chests,furnaces etc on the chunk
    NBT_Value *entityList = (*(*maps[mapId].nbt)["Level"])["TileEntities"];

    //Verify the type
    if(entityList && entityList->GetType() == NBT_Value::TAG_LIST && entityList->GetListType() == NBT_Value::TAG_COMPOUND)
    {
      std::vector<NBT_Value*> *entities = entityList->GetList();
      std::vector<NBT_Value*>::iterator iter = entities->begin(), end = entities->end();

      uint8 *compressedData = new uint8[ALLOCATE_NBTFILE];

      for( ; iter != end ; iter++)
      {
        std::vector<uint8> buffer;
        NBT_Value *idVal = (**iter)["id"];
        if(idVal == NULL)
          continue;
        std::string *id = idVal->GetString();
        if(id && (*id=="Chest" || *id=="Furnace" || *id=="Sign"))
        {
          if((**iter)["x"]->GetType() != NBT_Value::TAG_INT ||
            (**iter)["y"]->GetType() != NBT_Value::TAG_INT ||
            (**iter)["z"]->GetType() != NBT_Value::TAG_INT)
          {
            continue;
          }

            buffer.push_back(NBT_Value::TAG_COMPOUND);
            buffer.push_back(0);
            buffer.push_back(0);
            (*iter)->Write(buffer);
            buffer.push_back(0);
            buffer.push_back(0);


          z_stream zstream2;
          zstream2.zalloc = Z_NULL;
          zstream2.zfree = Z_NULL;
          zstream2.opaque = Z_NULL;
          zstream2.next_out=compressedData;
          zstream2.next_in=&buffer[0];
          zstream2.avail_in=buffer.size();
          zstream2.avail_out=ALLOCATE_NBTFILE;
          zstream2.total_out=0;
          zstream2.total_in=0;
          deflateInit2(&zstream2, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15+MAX_WBITS, 8,
                     Z_DEFAULT_STRATEGY);

          //Gzip the data
          if(int state=deflate(&zstream2,Z_FULL_FLUSH)!=Z_OK)
          {
            std::cout << "Error in deflate: " << state << std::endl;            
          }

          sint32 entityX = *(**iter)["x"];
          sint32 entityY = *(**iter)["y"];
          sint32 entityZ = *(**iter)["z"];

          // !!!! Complex Entity packet! !!!!
          user->buffer << (sint8)PACKET_COMPLEX_ENTITIES 
            << (sint32)entityX << (sint16)entityY << (sint32)entityZ << (sint16)zstream2.total_out;
          user->buffer.addToWrite(compressedData, zstream2.total_out);

          deflateEnd(&zstream2);
        }
      }
      delete [] compressedData;
    }
    delete [] buffer;
  }


  delete[] data4;
  delete[] mapdata;
}


void Map::setComplexEntity(sint32 x, sint32 y, sint32 z, NBT_Value *entity)
{
  uint32 mapId;
  
  int block_x = blockToChunk(x);
  int block_z = blockToChunk(z);

  if(!loadMap(block_x, block_z))
    return;

  Map::posToId(block_x, block_z, &mapId);

  if(entity->GetType() != NBT_Value::TAG_COMPOUND)
  {
    LOG("Complex Entity wasn't TAG_COMPOUND");
    return;
  }

  if((*entity)["x"] == NULL || (sint32)*(*entity)["x"] != x  ||
     (*entity)["y"] == NULL || (sint32)*(*entity)["y"] != y  ||
     (*entity)["z"] == NULL || (sint32)*(*entity)["z"] != z)
  {
    LOG("Invalid Complex Entity");
    return;
  }

  NBT_Value *entityList = (*(*maps[mapId].nbt)["Level"])["TileEntities"];

  if(!entityList)
  {
    entityList = new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND);
    maps[mapId].nbt->Insert("TileEntities", entityList);
  }

  if(entityList->GetType() == NBT_Value::TAG_LIST)
  {
   if(entityList->GetListType() != NBT_Value::TAG_COMPOUND)
     entityList->SetType(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND);

   std::vector<NBT_Value*> *entities = entityList->GetList();
   std::vector<NBT_Value*>::iterator iter = entities->begin(), end = entities->end();

   bool done = false;

   for( ; iter != end; iter++ )
   {
     if((**iter)["x"] == NULL || (**iter)["y"] == NULL || (**iter)["z"] == NULL ||
      (**iter)["x"]->GetType() != NBT_Value::TAG_INT ||
      (**iter)["y"]->GetType() != NBT_Value::TAG_INT ||
      (**iter)["z"]->GetType() != NBT_Value::TAG_INT)
    {
      continue;
    }

    if((sint32)(*(**iter)["x"]) == x && (sint32)(*(**iter)["y"]) == y && (sint32)(*(**iter)["z"]) == z)
    {
      // Replace entity
      delete *iter;
      *iter = entity;
      done = true;
      break;
    }
   }

   if(!done)
   {
    // Add new entity
    entityList->GetList()->push_back(entity);
   }
  }
  else
  {
    LOG("TileEntities list type not valid");
    return;
  }

  mapChanged[mapId] = true;

  std::vector<uint8> buffer;
  buffer.push_back(NBT_Value::TAG_COMPOUND);
  buffer.push_back(0);
  buffer.push_back(0);
  entity->Write(buffer);

    uint8 *compressedData = new uint8[ALLOCATE_NBTFILE];

  z_stream zstream2;
  zstream2.zalloc = Z_NULL;
  zstream2.zfree = Z_NULL;
  zstream2.opaque = Z_NULL;
  zstream2.next_out=compressedData;
  zstream2.next_in=&buffer[0];
  zstream2.avail_in=buffer.size();
  zstream2.avail_out=ALLOCATE_NBTFILE;
  zstream2.total_out=0;
  zstream2.total_in=0;
  deflateInit2(&zstream2, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15+MAX_WBITS, 8,
             Z_DEFAULT_STRATEGY);

  //Gzip the data
  if(int state=deflate(&zstream2,Z_FULL_FLUSH)!=Z_OK)
  {
    std::cout << "Error in deflate: " << state << std::endl;            
  }

  deflateEnd(&zstream2);


  Packet pkt;
  pkt << (sint8)PACKET_COMPLEX_ENTITIES 
    << x << (sint16)y << z << (sint16)zstream2.total_out;
  pkt.addToWrite(compressedData, zstream2.total_out);


  User::sendAll((uint8*)pkt.getWrite(), pkt.getWriteLen());
}
