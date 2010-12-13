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
#include "worldgen/mapgen.h"
#include "user.h"
#include "nbt.h"
#include "config.h"
#include "permissions.h"
#include "chat.h"


Map* Map::_instance = NULL;

void Map::addSapling(User* user, int x, int y, int z)
{
  Screen::get()->log("Place sapling " + dtos(x) + " " + dtos(y) + " " + dtos(z));

  saplings.push_back( sTree(x,y,z,mapTime,user->UID) );
}

void Map::checkGenTrees()
{
  std::list<sTree>::iterator iter = saplings.begin();

  while (iter != saplings.end())
  {
    if(rand() % 50 == 0)
    {
      Screen::get()->log("Grow tree!");

      sint32 x = (*iter).x;
      sint32 y = (*iter).y;
      sint32 z = (*iter).z;

      // grow tree!
      setBlock( x, y, z, BLOCK_LOG, 0);
      setBlock( x, y+1, z, BLOCK_LOG, 0);
      setBlock( x, y+2, z, BLOCK_LOG, 0);
      setBlock( x, y+3, z, BLOCK_LEAVES, 0);

      sendBlockChange( x, y, z, BLOCK_LOG, 0);
      sendBlockChange( x, y+1, z, BLOCK_LOG, 0);
      sendBlockChange( x, y+2, z, BLOCK_LOG, 0);
      sendBlockChange( x, y+3, z, BLOCK_LEAVES, 0);

        saplings.erase(iter++);  // alternatively, i = items.erase(i);
    }
    else
    {
        //other_code_involving(*i);
        ++iter;
    }
  }

}

void Map::init()
{
#ifdef _DEBUG
  printf("Map::init()\n");
#endif

  mapDirectory = Conf::get()->sValue("map_directory");
  if(mapDirectory == "Not found!")
  {
    std::cout << "Error, mapdir not defined!" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::string infile = mapDirectory+"/level.dat";

  struct stat stFileInfo;
  if(stat(mapDirectory.c_str(), &stFileInfo) != 0)
  {
    Screen::get()->log("Warning: Map directory not found, creating it now.");

#ifdef WIN32
    if(_mkdir(mapDirectory.c_str()) == -1)
#else
    if(mkdir(mapDirectory.c_str(), 0755) == -1)
#endif
    {
      Screen::get()->log("Error: Could not create map directory.");
      exit(EXIT_FAILURE);
    }
  }

  if(stat((infile).c_str(), &stFileInfo) != 0)
  {
    Screen::get()->log("Warning: level.dat not found, creating it now.");

    NBT_Value level(NBT_Value::TAG_COMPOUND);
    level.Insert("Data", new NBT_Value(NBT_Value::TAG_COMPOUND));
    level["Data"]->Insert("Time", new NBT_Value((sint64)0));
    level["Data"]->Insert("SpawnX", new NBT_Value((sint32)0));
    level["Data"]->Insert("SpawnY", new NBT_Value((sint32)80));
    level["Data"]->Insert("SpawnZ", new NBT_Value((sint32)0));
    level["Data"]->Insert("RandomSeed", new NBT_Value((sint64)(rand()*65535)));

    level.Insert("Trees", new NBT_Value(NBT_Value::TAG_LIST));

    level.SaveToFile(infile);

    if (stat(infile.c_str(), &stFileInfo) != 0)
    {
      Screen::get()->log("Error: Could not create level.dat");
      exit(EXIT_FAILURE);
    }
  }

  NBT_Value* root = NBT_Value::LoadFromFile(infile);
  NBT_Value& data = *((*root)["Data"]);

  spawnPos.x() = (sint32)*data["SpawnX"];
  spawnPos.y() = (sint32)*data["SpawnY"];
  spawnPos.z() = (sint32)*data["SpawnZ"];

  //Get time from the map
  mapTime      = (sint64)*data["Time"];
  mapSeed      = (sint64)*data["RandomSeed"];

  /////////////
  // Basic tree handling

  // Get list of saplings from map:
  NBT_Value* trees = ((*root)["Trees"]);

  if(!trees || trees->GetListType() != NBT_Value::TAG_COMPOUND)
  {

    Screen::get()->log("No Trees in level.dat, creating..");
    root->Insert("Trees", new NBT_Value(NBT_Value::TAG_LIST,NBT_Value::TAG_COMPOUND));
    trees = ((*root)["Trees"]);
    root->SaveToFile(infile);
  }

  trees->Print();

  std::vector<NBT_Value*>* tree_list = trees->GetList();

  Screen::get()->log(dtos((*tree_list).size()) + " saplings");

  for(std::vector<NBT_Value*>::iterator iter = (*tree_list).begin(); iter != (*tree_list).end(); ++iter)
  {
    NBT_Value& tree = *(*iter);
    sint32 x = (sint32)*tree["X"];
    sint32 y = (sint32)*tree["Y"];
    sint32 z = (sint32)*tree["Z"];
    sint32 plantedTime = (sint32)*tree["plantedTime"];
    sint32 plantedBy = (sint32)*tree["plantedBy"];
    saplings.push_back( sTree(x,y,z,plantedTime,plantedBy) );
    Screen::get()->log("sapling: " + dtos(x) + " " + dtos(y) + " " + dtos(z));
  }

  /////////////////

  // Init mapgenerator
  MapGen::get()->init(mapSeed);

  delete root;
#ifdef _DEBUG
  Screen::get()->log("Spawn: (" + spawnPos.x() + "," + spawnPos.y() + "," + spawnPos.z() + ")");
#endif
}

void Map::free()
{
   if (_instance)
   {
      delete _instance;
      _instance = 0;
   }
}

sChunk* Map::getMapData(int x, int z, bool generate)
{
#ifdef _DEBUG2
  printf("getMapData(x=%d, z=%d, generate=%d)\n", x, z, generate);
#endif

  sChunk* chunk = chunks.GetChunk(x,z);

  if(chunk != NULL || generate == false)
	  return chunk;

  // TODO: mapLastUsed[]
  return loadMap(x,z, generate);

}

bool Map::saveWholeMap()
{
#ifdef _DEBUG
  printf("saveWholeMap()\n");
#endif

  for(std::map<uint32, sChunk>::const_iterator it = maps.begin(); it != maps.end(); ++it)
  {
    saveMap(maps[it->first].x, maps[it->first].z);
  }

  /////////////////////
  // Save map details

  std::string infile = mapDirectory+"/level.dat";

  NBT_Value* root = NBT_Value::LoadFromFile(infile);
  if(root != NULL)
  {
    NBT_Value& data = *((*root)["Data"]);

    //Get time from the map
    *data["Time"] = mapTime;
    NBT_Value* trees = ((*root)["Trees"]);

    if(trees)
    {
      std::vector<NBT_Value*>* tree_vec = trees->GetList();

      tree_vec->clear();

      for(std::list<sTree>::iterator iter = saplings.begin(); iter != saplings.end(); ++iter)
      {
        //(*trees)[i] = (*iter)
        NBT_Value* tree = new NBT_Value(NBT_Value::TAG_COMPOUND);
        tree->Insert("X", new NBT_Value( (sint32)(*iter).x));
        tree->Insert("Y", new NBT_Value( (sint32)(*iter).y));
        tree->Insert("Z", new NBT_Value( (sint32)(*iter).z));
        tree->Insert("plantedTime", new NBT_Value( (sint32)(*iter).plantedTime));
        tree->Insert("plantedBy", new NBT_Value( (sint32)(*iter).plantedBy));
        tree_vec->push_back(tree);
      }
    }
    root->SaveToFile(infile);

    delete root;
  }

  return true;
}

bool Map::generateLight(int x, int z)
{
#ifdef _DEBUG2
  printf("generateLight(x=%d, z=%d)\n", x, z);
#endif

  sChunk* chunk = chunks.GetChunk(x,z);
  if(chunk == NULL)
  {
    LOG("Loading chunk failed (generateLight)");
    return false;
  }

  return generateLight(x, z, chunk);
}

//#define PRINT_LIGHTGEN_TIME

bool Map::generateLight(int x, int z, sChunk* chunk)
{
#ifdef _DEBUG2
  printf("generateLight(x=%d, z=%d, chunk=%p)\n", x, z, chunk);
#endif
  #ifdef PRINT_LIGHTGEN_TIME
  #ifdef WIN32
     DWORD t_begin,t_end;
     t_begin = timeGetTime();
  #else
    clock_t t_begin,t_end;
    t_begin = clock();
  #endif
  #endif

  uint8* blocks     = chunk->blocks;
  uint8* skylight   = chunk->skylight;
  uint8* blocklight = chunk->blocklight;
  uint8* heightmap  = chunk->heightmap;

  int highest_y = 0;

  // Clear lightmaps
  memset(skylight, 0, 16*16*128/2);
  memset(blocklight, 0, 16*16*128/2);

  // Sky light
  int light = 0;
  bool foundheight = false;
  for(int block_x = 0; block_x < 16; block_x++)
  {
    for(int block_z = 0; block_z < 16; block_z++)
    {
      light = 15;
      foundheight = false;
      sint32 blockx_blockz=(block_z << 7) + (block_x << 11);

      for(int block_y = 127; block_y > 0; block_y--)
      {
        int index      = block_y + blockx_blockz;
        int absolute_x = x*16+block_x;
        int absolute_z = z*16+block_z;
        uint8 block    = blocks[index];

        light -= stopLight[block];
        if (light < 0)
        {
          light = 0;
        }

        // Calculate heightmap while looping this
        if ((block != BLOCK_AIR) && (foundheight == false))
        {
          heightmap[block_z+(block_x<<4)] = ((block_y == 127) ? block_y : block_y + 1);
          foundheight = true;
        }

        if (light < 1)
        {
          if (block_y > highest_y)
          {
            highest_y = block_y;
          }

          break;
        }

        setLight(absolute_x, block_y, absolute_z, light, 0, 1, chunk);
      }
    }
  }

  // Block light
  for (int block_x = 0; block_x < 16; block_x++)
  {
    for (int block_z = 0; block_z < 16; block_z++)
    {
      sint32 blockx_blockz=(block_z << 7) + (block_x << 11);
      for (int block_y = highest_y; block_y >= 0; block_y--)
      {
        int index      = block_y + blockx_blockz;
        int absolute_x = x*16+block_x;
        int absolute_z = z*16+block_z;
        uint8 block    = blocks[index];

        // If light emitting block
        if(emitLight[block] > 0)
        {
          setLight(absolute_x, block_y, absolute_z, 0, emitLight[block], 2, chunk);
        }
      }
    }
  }

  // Spread light
  for (int block_x = 0; block_x < 16; block_x++)
  {
    for (int block_z = 0; block_z < 16; block_z++)
    {
      for (int block_y = heightmap[block_z+(block_x<<4)]; block_y >= 0; block_y--)
      {
        int absolute_x = x*16+block_x;
        int absolute_z = z*16+block_z;
        uint8 skylight_s, blocklight_s;

        getLight(absolute_x, block_y, absolute_z, &skylight_s, &blocklight_s, chunk);

        if (skylight_s || blocklight_s)
        {
          spreadLight(absolute_x, block_y, absolute_z, skylight_s, blocklight_s);
        }
      }
    }
  }
  #ifdef PRINT_LIGHTGEN_TIME
  #ifdef WIN32
    t_end = timeGetTime ();
    Screen::get()->log("Lightgen: " + dtos(t_end-t_begin) + "ms");
  #else
    t_end = clock();
    Screen::get()->log("Lightgen: " + dtos((t_end-t_begin)/(CLOCKS_PER_SEC/1000))) + "ms");
  #endif
  #endif


  return true;
}

bool Map::spreadLight(int x, int y, int z, int skylight, int blocklight)
{
#ifdef _DEBUG2
  printf("spreadLight(x=%d, y=%d, z=%d, skylight=%d, blocklight=%d)\n", x, y, z, skylight, blocklight);
#endif

  if((y < 0) || (y > 127))
  {
    LOG("Invalid y value (spreadLight)");
    return false;
  }

  int chunk_x = blockToChunk(x);
  int chunk_z = blockToChunk(z);

  sChunk* chunk = getMapData(chunk_x, chunk_z, false);

  if(!chunk)
  {
    LOG("Loading chunk failed (spreadLight)");
    return false;
  }

  return spreadLight(x, y, z, skylight, blocklight, chunk);
}

bool Map::spreadLight(int x, int y, int z, int skylight, int blocklight, sChunk* chunk)
{
#ifdef _DEBUG2
  printf("spreadLight(x=%d, y=%d, z=%d, skylight=%d, blocklight=%d, chunk=%p)\n", x, y, z, skylight, blocklight, chunk);
#endif

  uint8 block, meta;

  // If no light, stop!
  if((skylight < 1) && (blocklight < 1))
    return false;

  for(int i = 0; i < 6; i++)
  {
    // Going too high
    if((y == 127) && (i == 2))
      i++;
    // going negative
    if((y == 0) && (i == 3))
      i++;

    int x_toset = x;
    int y_toset = y;
    int z_toset = z;

    switch(i)
    {
      case 0: x_toset++; break;
      case 1: x_toset--; break;
      case 2: y_toset++; break;
      case 3: y_toset--; break;
      case 4: z_toset++; break;
      case 5: z_toset--; break;
    }

    if (getBlock(x_toset, y_toset, z_toset, &block, &meta, false))
    {
      uint8 skylightCurrent, blocklightCurrent;
      int skylightNew, blocklightNew;
      bool spread = false;

      skylightNew = skylight-stopLight[block]-1;
      if (skylightNew < 0)
      {
        skylightNew = 0;
      }

      blocklightNew = blocklight-stopLight[block]-1;
      if (blocklightNew < 0)
      {
        blocklightNew = 0;
      }

      getLight(x_toset, y_toset, z_toset, &skylightCurrent, &blocklightCurrent, chunk);

      if (skylightNew > skylightCurrent)
      {
        skylightCurrent = skylightNew;
        spread = true;
      }

      if (blocklightNew > blocklightCurrent)
      {
        blocklightCurrent = blocklightNew;
        spread = true;
      }

      if (spread)
      {
        setLight(x_toset, y_toset, z_toset, skylightCurrent, blocklightCurrent, 4, chunk);
        spreadLight(x_toset, y_toset, z_toset, skylightCurrent, blocklightCurrent, chunk);
      }
    }
  }

  return true;
}

bool Map::getBlock(int x, int y, int z, uint8* type, uint8* meta, bool generate)
{
#ifdef _DEBUG2
  printf("getBlock(x=%d, y=%d, z=%d, type=%p, meta=%p, generate=%d)\n", x, y, z, type, meta, generate);
#endif

  if((y < 0) || (y > 127))
  {
    printf("(%i, %i, %i) ", x, y, z);
    LOG("Invalid y value (getBlock)");
    return false;
  }

  int chunk_x = blockToChunk(x);
  int chunk_z = blockToChunk(z);

  sChunk* chunk = getMapData(chunk_x, chunk_z, generate);

  if(!chunk)
  {
    if(generate)
    {
     LOG("Loading chunk failed (getBlock)");
    }

    return false;
  }

  return getBlock(x, y, z, type, meta, generate, chunk);
}

bool Map::getBlock(int x, int y, int z, uint8* type, uint8* meta, bool generate, sChunk* chunk)
{
#ifdef _DEBUG2
  printf("getBlock(x=%d, y=%d, z=%d, type=%p, meta=%p, generate=%d)\n", x, y, z, type, meta, generate);
#endif

  int chunk_block_x  = blockToChunkBlock(x);
  int chunk_block_z  = blockToChunkBlock(z);

  uint8* blocks      = chunk->blocks;
  uint8* metapointer = chunk->data;
  int index          = y + (chunk_block_z << 7) + (chunk_block_x << 11);
  *type              = blocks[index];
  uint8 metadata     = metapointer[(index)>>1];

  if(y & 1)
  {
    metadata  &= 0xf0;
    metadata >>= 4;
  }
  else
  {
    metadata &= 0x0f;
  }

  *meta              = metadata;
  // TODO: mapLastused[mapId] = (int)time(0);

  return true;
}

bool Map::getLight(int x, int y, int z, uint8* skylight, uint8* blocklight)
{
#ifdef _DEBUG2
  printf("getLight(x=%d, y=%d, z=%d, skylight=%p, blocklight=%p)\n", x, y, z, skylight, blocklight);
#endif

  if((y < 0) || (y > 127))
  {
    LOG("Invalid y value (getLight)");
    return false;
  }

  // Map chunk pos from block pos
  int chunk_x = blockToChunk(x);
  int chunk_z = blockToChunk(z);

  sChunk* chunk = getMapData(chunk_x, chunk_z, false);

  if(!chunk)
  {
    LOG("Loading chunk failed (getLight)");
    return false;
  }

  return getLight(x, y, z, skylight, blocklight, chunk);
}

bool Map::getLight(int x, int y, int z, uint8* skylight, uint8* blocklight, sChunk* chunk)
{
#ifdef _DEBUG2
  printf("getLight(x=%d, y=%d, z=%d, skylight=%p, blocklight=%p, chunk=%p)\n", x, y, z, skylight, blocklight, chunk);
#endif

  // Which block inside the chunk
  int chunk_block_x = blockToChunkBlock(x);
  int chunk_block_z = blockToChunkBlock(z);

  uint8* blocklightPtr = chunk->blocklight;
  uint8* skylightPtr   = chunk->skylight;
  int index            = y + (chunk_block_z << 7) + (chunk_block_x << 11);
  *blocklight = blocklightPtr[(index)>>1];
  *skylight   = skylightPtr[(index)>>1];

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

bool Map::setLight(int x, int y, int z, int skylight, int blocklight, int type)
{
#ifdef _DEBUG2
  printf("setLight(x=%d, y=%d, z=%d, skylight=%d, blocklight=%d, type=%d)\n", x, y, z, skylight, blocklight, type);
#endif

  if ((y < 0) || (y > 127))
  {
    LOG("Invalid y value (setLight)");
    return false;
  }

  int chunk_x = blockToChunk(x);
  int chunk_z = blockToChunk(z);

  sChunk* chunk = getMapData(chunk_x, chunk_z, false);

  if (!chunk)
  {
    LOG("Loading chunk failed (setLight)");
    return false;
  }

  return setLight(x, y, z, skylight, blocklight, type, chunk);
}

bool Map::setLight(int x, int y, int z, int skylight, int blocklight, int type, sChunk* chunk)
{
#ifdef _DEBUG2
  printf("setLight(x=%d, y=%d, z=%d, skylight=%d, blocklight=%d, type=%d, chunk=%p)\n", x, y, z, skylight, blocklight, type, chunk);
#endif

  int chunk_block_x        = blockToChunkBlock(x);
  int chunk_block_z        = blockToChunkBlock(z);

  uint8* blocklightPtr     = chunk->blocklight;
  uint8* skylightPtr       = chunk->skylight;
  int index                = y + (chunk_block_z << 7) + (chunk_block_x << 11);
  char skylight_local      = skylightPtr[index>>1];
  char blocklight_local    = blocklightPtr[index>>1];

  if (y & 1)
  {
    if (type & 0x5) // 1 or 4
    {
      skylight_local &= 0x0f;
      skylight_local |= skylight<<4;
    }

    if (type & 0x6) // 2 or 4
    {
      blocklight_local &= 0x0f;
      blocklight_local |= blocklight<<4;
    }
  }
  else
  {
    if (type & 0x5) // 1 or 4
    {
      skylight_local &= 0xf0;
      skylight_local |= skylight;
    }

    if (type & 0x6) // 2 or 4
    {
      blocklight_local &= 0xf0;
      blocklight_local |= blocklight;
    }
  }

  if (type & 0x5) // 1 or 4
  {
    skylightPtr[index>>1] = skylight_local;
  }

  if (type & 0x6) // 2 or 4
  {
    blocklightPtr[index>>1] = blocklight_local;
  }

  return true;
}

bool Map::setBlock(int x, int y, int z, char type, char meta)
{
#ifdef _DEBUG2
  printf("setBlock(x=%d, y=%d, z=%d, type=%d, char=%d)\n", x, y, z, type, meta);
#endif

  if((y < 0) || (y > 127))
  {
    LOG("Invalid y value (setBlock)");
    return false;
  }

  // Map chunk pos from block pos
  int chunk_x = blockToChunk(x);
  int chunk_z = blockToChunk(z);

  //uint32 mapId;
  //Map::posToId(chunk_x, chunk_z, &mapId);

  sChunk* chunk = getMapData(chunk_x, chunk_z, true);

  if(!chunk)
  {
    LOG("Loading chunk failed (setBlock)");
    return false;
  }

  // Which block inside the chunk
  int chunk_block_x  = blockToChunkBlock(x);
  int chunk_block_z  = blockToChunkBlock(z);

  uint8* blocks      = chunk->blocks;
  uint8* metapointer = chunk->data;
  int index          = y + (chunk_block_z << 7) + (chunk_block_x << 11);
  blocks[index] = type;
  char metadata      = metapointer[index>>1];

  if(y & 1)
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

//  mapChanged[mapId]       = true;
//  mapLightRegen[mapId]    = true;
//  mapLastused[mapId]      = (int)time(0);

  return true;
}

bool Map::sendBlockChange(int x, int y, int z, char type, char meta)
{
#ifdef _DEBUG
  printf("sendBlockChange(x=%d, y=%d, z=%d, type=%d, meta=%d)\n", x, y, z, type, meta);
#endif

  Packet pkt;
  pkt << PACKET_BLOCK_CHANGE << (sint32)x << (sint8)y << (sint32)z << (sint8)type << (sint8)meta;

  sChunk* chunk = chunks.GetChunk(blockToChunk(x), blockToChunk(z));
  if(chunk == NULL)
	  return false;

  chunk->sendPacket(pkt);

  return true;
}

bool Map::sendPickupSpawn(spawnedItem item)
{
  //Push to global item storage
  spawnedItem* storedItem = new spawnedItem;
  *storedItem     = item;
  items[item.EID] = storedItem;

  //Push to local item storage
  int chunk_x = blockToChunk(item.pos.x()/32);
  int chunk_z = blockToChunk(item.pos.z()/32);

  sChunk* chunk = chunks.GetChunk(chunk_x, chunk_z);
  if(chunk == NULL)
	  return false;

  chunk->items.push_back(storedItem);

  Packet pkt;
  pkt << PACKET_PICKUP_SPAWN << (sint32)item.EID << (sint16)item.item << (sint8)item.count
      << (sint32)item.pos.x() << (sint32)item.pos.y() << (sint32)item.pos.z()
      << (sint8)0 << (sint8)0 << (sint8)0;

  chunk->sendPacket(pkt);

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

   sendPickupSpawn(item);
}

sChunk*  Map::loadMap(int x, int z, bool generate)
{
#ifdef _DEBUG2
  printf("loadMap(x=%d, z=%d, generate=%d)\n", x, z, generate);
#endif

  sChunk* chunk = chunks.GetChunk(x,z);
  if(chunk != NULL)
	  return chunk;

  // Generate map file name

  int mapposx = x;
  int modulox = mapposx & 0x3F;

  int mapposz = z;
  int moduloz = mapposz & 0x3F;

  std::string infile = mapDirectory+"/"+base36_encode(modulox)+"/"+base36_encode(moduloz)+"/c."+
                       base36_encode(mapposx)+"."+base36_encode(mapposz)+".dat";

  struct stat stFileInfo;
  if (stat(infile.c_str(), &stFileInfo) != 0)
  {
    // If generate (false only for lightmapgenerator)
    if(generate)
    {
      MapGen::get()->generateChunk(x,z);
      generateLight(x, z);

	  //mapLightRegen[mapId] = false;

      //If we generated spawn pos, make sure the position is not underground!
      if(x == blockToChunk(spawnPos.x()) &&
         z == blockToChunk(spawnPos.z()))
      {
        uint8 block,meta;
        bool foundAir=false;
        if(getBlock(spawnPos.x(),spawnPos.y(),spawnPos.z(), &block, &meta,false) && block != BLOCK_AIR)
        {
          uint8 new_y;
          for(new_y = spawnPos.y(); new_y < 128 ; new_y++)
          {
            if(getBlock(spawnPos.x(),new_y,spawnPos.z(), &block, &meta,false) && block == BLOCK_AIR)
            {
              foundAir=true;
              break;
            }
          }
          if(foundAir)
          {
            spawnPos.y() = new_y;

            std::string infile = mapDirectory+"/level.dat";

            NBT_Value* root = NBT_Value::LoadFromFile(infile);
            if(root != NULL)
            {
              NBT_Value& data = *((*root)["Data"]);
              *data["SpawnX"] = (sint32)spawnPos.x();
              *data["SpawnY"] = (sint32)spawnPos.y();
              *data["SpawnZ"] = (sint32)spawnPos.z();

              root->SaveToFile(infile);

              delete root;
            }
          }
        }

      }

	  return chunks.GetChunk(x,z);
    }
    else
    {
      return NULL;
    }
  }

  chunk = new sChunk();

  chunk->nbt = NBT_Value::LoadFromFile(infile.c_str());


  if(chunk->nbt == NULL)
  {
    LOG("Error in loading map (unable to load file)");
    return false;
  }

  NBT_Value& level = *(*chunk->nbt)["Level"];

  chunk->x = (sint32)(*level["xPos"]);
  chunk->z = (sint32)(*level["zPos"]);

  if(chunk->x != x || chunk->z != z)
  {
    LOG("Error in loading map (incorrect chunk)");
	delete chunk->nbt;
	delete chunk;
    return NULL;
  }

  std::vector<uint8>* blocks = level["Blocks"]->GetByteArray();
  std::vector<uint8>* data = level["Data"]->GetByteArray();
  std::vector<uint8>* blocklight = level["BlockLight"]->GetByteArray();
  std::vector<uint8>* skylight = level["SkyLight"]->GetByteArray();
  std::vector<uint8>* heightmap = level["HeightMap"]->GetByteArray();

  if(blocks == 0 || data == 0 || blocklight == 0 || skylight == 0 || heightmap == 0)
  {
    LOG("Error in loading map (chunk missing data)");
	delete chunk->nbt;
	delete chunk;
    return NULL;
  }

  size_t fullLen = (16 * 128 * 16);
  size_t halfLen = fullLen >> 1;

  if(blocks->size() != fullLen ||
    data->size() != halfLen ||
    blocklight->size() != halfLen ||
    skylight->size() != halfLen)
  {
    LOG("Error in loading map (corrupt?)");
	delete chunk->nbt;
	delete chunk;
    return NULL;
  }

  chunk->blocks = &((*blocks)[0]);
  chunk->data = &((*data)[0]);
  chunk->blocklight = &((*blocklight)[0]);
  chunk->skylight = &((*skylight)[0]);
  chunk->heightmap = &((*heightmap)[0]);

  chunks.LinkChunk(chunk, x, z);

  // Update last used time
  //mapLastused[mapId] = (int)time(0);

  // Not changed
//  mapChanged[mapId]    = false;
//  mapLightRegen[mapId] = false;

  return chunk;
}

bool Map::saveMap(int x, int z)
{
#ifdef _DEBUG
  printf("saveMap(x=%d, z=%d)\n", x, z);
#endif

//  uint32 mapId;
//  Map::posToId(x, z, &mapId);

// TODO: Implement this in hash map
//  if(!mapChanged[mapId])
//    return true;

//  if(!maps.count(mapId))
//    return false;

  sChunk*  chunk = chunks.GetChunk(x, z);

  // Recalculate light maps
//  if(mapLightRegen[mapId])
  {
    generateLight(x, z, chunk);
  }

  // Generate map file name

  int mapposx = x;
  int modulox = mapposx & 0x3F;

  int mapposz = z;
  int moduloz = mapposz & 0x3F;

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

  chunk->nbt->SaveToFile(outfile);

  // Set "not changed"
  //mapChanged[mapId] = false;
  //mapLightRegen[mapId]= false;

  return true;
}

bool Map::releaseMap(int x, int z)
{
  // save first
  saveMap(x, z);

  sChunk* chunk = chunks.GetChunk(x,z);
  if(chunk == NULL)
	  return false;
  chunks.UnlinkChunk(x, z);
  delete chunk->nbt;
  delete chunk;
  
  return true;

  //uint32 mapId;
  //Map::posToId(x, z, &mapId);

  //mapChanged.erase(mapId);
  //mapLastused.erase(mapId);
  //if(maps.count(mapId))
  //{
//    delete maps[mapId].nbt;
  //}

//  return maps.erase(mapId) ? true : false;
}

// Send chunk to user
void Map::sendToUser(User* user, int x, int z)
{
#ifdef _DEBUG
  printf("sendToUser(x=%d, z=%d)\n", x, z);
#endif

//  uint32 mapId;
//  Map::posToId(x, z, &mapId);
  sChunk* chunk = loadMap(x, z);
  if(chunk == NULL)
	  return;

  uint8* data4   = new uint8[18+81920];
  uint8* mapdata = new uint8[81920];
  sint32 mapposx    = x;
  sint32 mapposz    = z;

    //Regenerate lighting if needed
//    if(mapLightRegen[mapId])
    {
      generateLight(x, z, chunk);
  //    mapLightRegen[mapId] = false;
    }
    // Pre chunk
    user->buffer << (sint8)PACKET_PRE_CHUNK << mapposx << mapposz << (sint8)1;

    // Chunk
    user->buffer << (sint8)PACKET_MAP_CHUNK << (sint32)(mapposx * 16) << (sint16)0 << (sint32)(mapposz * 16)
      << (sint8)15 << (sint8)127 << (sint8)15;

  memcpy(&mapdata[0], chunk->blocks, 32768);
  memcpy(&mapdata[32768], chunk->data, 16384);
  memcpy(&mapdata[32768+16384], chunk->blocklight, 16384);
  memcpy(&mapdata[32768+16384+16384], chunk->skylight, 16384);

  uLongf written = 81920;
  Bytef* buffer = new Bytef[written];

  // Compress data with zlib deflate
  compress(buffer, &written, &mapdata[0], 81920);

  user->buffer << (sint32)written;
  user->buffer.addToWrite(buffer, written);

  //Get list of chests,furnaces etc on the chunk
  NBT_Value* entityList = (*(*chunk->nbt)["Level"])["TileEntities"];

  //Verify the type
  if(entityList && entityList->GetType() == NBT_Value::TAG_LIST && entityList->GetListType() == NBT_Value::TAG_COMPOUND)
  {
    std::vector<NBT_Value*>* entities = entityList->GetList();
    std::vector<NBT_Value*>::iterator iter = entities->begin(), end = entities->end();

    uint8* compressedData = new uint8[ALLOCATE_NBTFILE];

    for( ; iter != end ; iter++)
    {
      std::vector<uint8> buffer;
      NBT_Value* idVal = (**iter)["id"];
      if(idVal == NULL)
        continue;
      std::string* id = idVal->GetString();
      if(id && (*id=="Chest" || *id=="Furnace" || *id=="Sign"))
      {
        if((**iter)["x"]->GetType() != NBT_Value::TAG_INT ||
          (**iter)["y"]->GetType() != NBT_Value::TAG_INT ||
          (**iter)["z"]->GetType() != NBT_Value::TAG_INT)
        {
          continue;
        }


          if(*id == "Chest")
          {
            NBT_Value* lockData = (**iter)["Lockdata"];
            if(lockData != NULL)
            {
              if((*lockData)["locked"] != NULL)
              {
                sint8 locked = *(*lockData)["locked"];
                std::string chestowner = *(*lockData)["player"]->GetString();
                // If locked
                if (locked == 1)
                {
                  // Check permission to access
                  if(!(chestowner == user->nick || IS_ADMIN(user->permissions)))
                  {
                    Chat::get()->sendMsg(user, MC_COLOR_BLUE + "Chest is locked.", Chat::USER);
                    continue;
                  }
                }
              }
            }
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
            Screen::get()->log("Error in deflate: " + dtos(state));
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

  delete[] data4;
  delete[] mapdata;
}

void Map::setComplexEntity(User* user, sint32 x, sint32 y, sint32 z, NBT_Value* entity)
{
//  uint32 mapId;

  int chunk_x = blockToChunk(x);
  int chunk_z = blockToChunk(z);

  sChunk* chunk = loadMap(chunk_x, chunk_z);
  if(chunk == NULL)
	  return;

  std::string player = "";
  if(user != NULL)
  {
    player = user->nick;
  }
  sint8 locked = Conf::get()->bValue("chests_locked_by_default")?1:0;

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

  NBT_Value* entityList = (*(*chunk->nbt)["Level"])["TileEntities"];

  if(!entityList)
  {
    entityList = new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND);
    chunk->nbt->Insert("TileEntities", entityList);
  }

  if(entityList->GetType() == NBT_Value::TAG_LIST)
  {
    if(entityList->GetListType() != NBT_Value::TAG_COMPOUND)
    {
      entityList->SetType(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND);
    }

    std::vector<NBT_Value*>* entities = entityList->GetList();
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
        NBT_Value* nbtLockdata = (**iter)["Lockdata"];

        if(nbtLockdata != NULL)
        {
          player = *(*nbtLockdata)["player"]->GetString();
          locked = *(*nbtLockdata)["locked"];

          if(locked == 1)
          {
            if(!(user != NULL && player != user->nick))
            {
              return;
            }
          }
        }


        // Replace entity
        delete *iter;
        *iter = entity;
        done = true;
        break;
      }
    }

    if(!done)
    {
      if(user != NULL)
      {
        NBT_Value* nbtLockdata = new NBT_Value(NBT_Value::TAG_COMPOUND);
        nbtLockdata->Insert("player", new NBT_Value(player));
        nbtLockdata->Insert("locked", new NBT_Value(locked));
        entity->Insert("Lockdata", nbtLockdata);
      }

      // Add new entity
      entityList->GetList()->push_back(entity);
    }
  }
  else
  {
    LOG("TileEntities list type not valid");
    return;
  }

  // TODO: mapChanged[mapId] = true;

  std::vector<uint8> buffer;
  buffer.push_back(NBT_Value::TAG_COMPOUND);
  buffer.push_back(0);
  buffer.push_back(0);
  entity->Write(buffer);

  uint8* compressedData = new uint8[ALLOCATE_NBTFILE];

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
    Screen::get()->log("Error in deflate: " + dtos(state));
  }

  deflateEnd(&zstream2);

  Packet pkt;
  pkt << (sint8)PACKET_COMPLEX_ENTITIES
    << x << (sint16)y << z << (sint16)zstream2.total_out;
  pkt.addToWrite(compressedData, zstream2.total_out);

  chunk->sendPacket(pkt);

  delete [] compressedData;

  //User::sendAll((uint8*)pkt.getWrite(), pkt.getWriteLen());
}
