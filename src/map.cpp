/*
   Copyright (c) 2011, The Mineserver Project
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
#include <direct.h>
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
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
#include "mineserver.h"
#include "tree.h"
#include "furnaceManager.h"
#include "mcregion.h"

// Copy Construtor
Map::Map(const Map& oldmap)
  :
  chunks(oldmap.chunks),
  mapLastused(oldmap.mapLastused),
  mapChanged(oldmap.mapChanged),
  mapLightRegen(oldmap.mapLightRegen),
  items(oldmap.items),
  mapTime(oldmap.mapTime),
  mapSeed(oldmap.mapSeed)
{
}

Map::Map()
  :
  chunks(441) // buckets!
{
  std::fill(emitLight, emitLight + 256, 0);

  emitLight[0x0A] = 15; // Lava
  emitLight[0x0B] = 15; // Stationary Lava
  emitLight[0x27] = 1;  // Brown mushroom
  emitLight[0x32] = 14; // Torch
  emitLight[0x33] = 15; // Fire
  emitLight[0x3E] = 14; // Lit furnace
  emitLight[0x4A] = 9;  // Redstone ore (Glowing)
  emitLight[0x4C] = 7;  // Redstone Torch (On)
  emitLight[0x59] = 15; // Lightstone
  emitLight[0x5A] = 11; // Portal
  emitLight[0x5B] = 15; // Jack-O-Lantern

  std::fill(stopLight, stopLight + 256, 16);

  stopLight[0x00] = 0; // Empty
  stopLight[0x06] = 0; // Sapling
  stopLight[0x08] = 3; // Water
  stopLight[0x09] = 3; // Stationary water
  stopLight[0x12] = 3; // Leaves
  stopLight[0x14] = 0; // Glass
  stopLight[0x25] = 0; // Yellow flower
  stopLight[0x26] = 0; // Red rose
  stopLight[0x27] = 0; // Brown mushroom
  stopLight[0x28] = 0; // Red mushroom
  stopLight[0x32] = 0; // Torch
  stopLight[0x33] = 0; // Fire
  stopLight[0x34] = 0; // Mob spawner
  stopLight[0x35] = 0; // Wooden stairs
  stopLight[0x37] = 0; // Redstone wire
  stopLight[0x40] = 0; // Wooden door
  stopLight[0x41] = 0; // Ladder
  stopLight[0x42] = 0; // Minecart track
  stopLight[0x43] = 0; // Cobblestone stairs
  stopLight[0x47] = 0; // Iron door
  stopLight[0x4b] = 0; // Redstone Torch (Off)
  stopLight[0x4C] = 0; // Redstone Torch (On)
  stopLight[0x4e] = 0; // Snow
  stopLight[0x4f] = 3; // Ice
  stopLight[0x55] = 0; // Fence
  stopLight[0x5A] = 0; // Portal
  stopLight[0x5B] = 0; // Jack-O-Lantern
  stopLight[BLOCK_SIGN_POST] = 0; // Sign post
  stopLight[BLOCK_WALL_SIGN] = 0; // Wall sign
}

Map::~Map()
{

  // louisdx: This destructor is doing WAY too much. All this should be in a separate function.
  // A destructor must never throw, and this destructor does tons of non-exception-safe stuff.

  for (ChunkMap::const_iterator it = chunks.begin(); it != chunks.end(); )
  {
    releaseMap((it++)->first);
  }

  // Free item memory
  for (std::map<uint32_t, spawnedItem*>::const_iterator it = items.begin(); it != items.end(); ++it)
  {
    delete it->second;
  }

  items.clear();
  std::string infile = mapDirectory + "/level.dat";

  NBT_Value* root = NBT_Value::LoadFromFile(infile);
  if (root != NULL)
  {
    NBT_Value& data = *((*root)["Data"]);

    //Get time from the map
    data.Insert("Time", new NBT_Value(mapTime));

    NBT_Value* trees = ((*root)["Trees"]);

    if (trees)
    {
      std::vector<NBT_Value*>* tree_vec = trees->GetList();

      tree_vec->clear();

      for (std::list<sTree>::iterator iter = saplings.begin(); iter != saplings.end(); ++iter)
      {
        //(*trees)[i] = (*iter)
        NBT_Value* tree = new NBT_Value(NBT_Value::TAG_COMPOUND);
        tree->Insert("X", new NBT_Value((int32_t)(*iter).x));
        tree->Insert("Y", new NBT_Value((int32_t)(*iter).y));
        tree->Insert("Z", new NBT_Value((int32_t)(*iter).z));
        tree->Insert("plantedTime", new NBT_Value((int32_t)(*iter).plantedTime));
        tree->Insert("plantedBy", new NBT_Value((int32_t)(*iter).plantedBy));
        tree_vec->push_back(tree);
      }
    }

    root->SaveToFile(infile);

    delete root;
  }
}

void Map::addSapling(User* user, int x, int y, int z)
{
  LOG(INFO, "Map", "Place sapling " + dtos(x) + " " + dtos(y) + " " + dtos(z));

  saplings.push_back(sTree(x, y, z, mapTime, user->UID));
}

void Map::checkGenTrees()
{
  std::list<sTree>::iterator iter = saplings.begin();

  static uint8_t light;
  static uint8_t skylight;

  static uint8_t blocktype;
  static uint8_t meta;

  while (iter != saplings.end())
  {
    getLight(iter->x, iter->y + 1, iter->z, &light, &skylight);
    if (light > 9 || skylight > 3)
    {
      //Check above blocks
      uint8_t i = 1;
      for (i = 1; i < MAX_TRUNK; i++)
      {
        if (!getBlock(iter->x, iter->y + i, iter->z, &blocktype, &meta) || blocktype != BLOCK_AIR)
        {
          break;
        }
      }
      if (i >= MIN_TREE_SPACE)
      {
        //If there is enough space
        if (rand() % 50 == 0)
        {
          LOG(INFO, "Map", "Grow tree!");

          Tree tree((*iter).x, (*iter).y, (*iter).z, 0);
          saplings.erase(iter++);  // alternatively, i = items.erase(i);
        }
      }
      else
      {
        iter++;
      }
    }
    else
    {
      iter++;
    }
  }
}

void Map::init(int number)
{
  m_number = number;
  const char* key = "map.storage.nbt.directories"; // Prefix for worlds config
  std::list<std::string> tmp = Mineserver::get()->config()->mData(key)->keys();

  if (number < int(tmp.size()))
  {
    std::list<std::string>::const_iterator it = tmp.begin();
    std::advance(it, number);
    mapDirectory = *it;
  }

  LOG2(INFO, "World " + mapDirectory);

  if (mapDirectory == "Not found!")
  {
    LOG2(WARNING, "mapdir not defined");
    exit(EXIT_FAILURE);
  }

  std::string infile = mapDirectory + "/level.dat";

  struct stat stFileInfo;
  if (stat(mapDirectory.c_str(), &stFileInfo) != 0)
  {
    LOG(WARNING, "Map", "Warning: Map directory not found, creating it now.");

#ifdef WIN32
    if (_mkdir(mapDirectory.c_str()) == -1)
#else
    if (mkdir(mapDirectory.c_str(), 0755) == -1)
#endif
    {
      LOG(EMERG, "Map", "Error: Could not create map directory.");

      exit(EXIT_FAILURE);
    }
  }

  if (stat((infile).c_str(), &stFileInfo) != 0)
  {
    LOG(WARNING, "Map", "Warning: level.dat not found, creating it now.");

    NBT_Value level(NBT_Value::TAG_COMPOUND);
    level.Insert("Data", new NBT_Value(NBT_Value::TAG_COMPOUND));
    level["Data"]->Insert("Time", new NBT_Value((int64_t)0));
    level["Data"]->Insert("SpawnX", new NBT_Value((int32_t)0));
    level["Data"]->Insert("SpawnY", new NBT_Value((int32_t)120));
    level["Data"]->Insert("SpawnZ", new NBT_Value((int32_t)0));
    level["Data"]->Insert("RandomSeed", new NBT_Value((int64_t)(rand() * 65535)));
    level["Data"]->Insert("version", new NBT_Value((int32_t)19132));
    level["Data"]->Insert("LevelName", new NBT_Value(std::string("Mineserver world")));

    level.Insert("Trees", new NBT_Value(NBT_Value::TAG_LIST));

    level.SaveToFile(infile);

    if (stat(infile.c_str(), &stFileInfo) != 0)
    {
      LOG(EMERG, "Map", "Error: Could not create level.dat");
      exit(EXIT_FAILURE);
    }
  }

  NBT_Value* root = NBT_Value::LoadFromFile(infile);
  NBT_Value& data = *((*root)["Data"]);

  spawnPos.x() = (int32_t) * data["SpawnX"];
  spawnPos.y() = (int32_t) * data["SpawnY"];
  spawnPos.z() = (int32_t) * data["SpawnZ"];

  //Get time from the map
  mapTime      = (int64_t) * data["Time"];
  mapSeed      = (int64_t) * data["RandomSeed"];


  //Check for McRegion format!
  int32_t version = (int32_t) * data["version"];

  //Not in McRegion format?
  if (version != 19132)
  {
    LOG(EMERG, "Map", "Error: map not in McRegion format, converting..(old mapdata is not removed!)");
    LOG(EMERG, "Map", "THIS MIGHT TAKE A WHILE");
    LOG(EMERG, "Map", "every dot is converted subfolder and | is root level folder");

    //Do the actual conversion
    convertMap(mapDirectory);

    //Add version info to tell we are using McRegion format and levelname
    (*root)["Data"]->Insert("version", new NBT_Value((int32_t)19132));
    (*root)["Data"]->Insert("LevelName", new NBT_Value(std::string("Mineserver world")));

    //Save level.dat back with the new info
    root->SaveToFile(infile);
  }

  /////////////
  // Basic tree handling

  // Get list of saplings from map:
  NBT_Value* trees = ((*root)["Trees"]);

  if (!trees || trees->GetListType() != NBT_Value::TAG_COMPOUND)
  {
    LOG(INFO, "Map", "No Trees in level.dat, creating..");
    root->Insert("Trees", new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND));
    trees = ((*root)["Trees"]);
    root->SaveToFile(infile);
  }

  std::vector<NBT_Value*>* tree_list = trees->GetList();

  for (std::vector<NBT_Value*>::iterator iter = (*tree_list).begin(); iter != (*tree_list).end(); ++iter)
  {
    NBT_Value& tree = *(*iter);
    int32_t x = (int32_t) * tree["X"];
    int32_t y = (int32_t) * tree["Y"];
    int32_t z = (int32_t) * tree["Z"];
    int32_t plantedTime = (int32_t) * tree["plantedTime"];
    int32_t plantedBy = (int32_t) * tree["plantedBy"];
    saplings.push_back(sTree(x, y, z, plantedTime, plantedBy));
    LOG(INFO, "Map", "sapling: " + dtos(x) + " " + dtos(y) + " " + dtos(z));
  }

  /////////////////

  // Init mapgenerator
  Mineserver::get()->mapGen(m_number)->re_init((int32_t)mapSeed);

  delete root;
}

sChunk* Map::getMapData(int x, int z,  bool generate)
{
  const ChunkMap::const_iterator it = chunks.find(Coords(x, z));

  if (it != chunks.end()) return it->second;

  return generate == false ? NULL : loadMap(x, z, true);
}

bool Map::saveWholeMap()
{
  for (ChunkMap::const_iterator it = chunks.begin(); it != chunks.end(); ++it)
  {
    saveMap(it->first);
  }

  /////////////////////
  // Save map details

  std::string infile = mapDirectory + "/level.dat";

  NBT_Value* root = NBT_Value::LoadFromFile(infile);
  if (root != NULL)
  {
    NBT_Value& data = *((*root)["Data"]);

    //Get time from the map
    *data["Time"] = mapTime;
    *data["SpawnX"] = spawnPos.x();
    *data["SpawnY"] = spawnPos.y();
    *data["SpawnZ"] = spawnPos.z();

    NBT_Value* trees = ((*root)["Trees"]);

    if (trees)
    {
      std::vector<NBT_Value*>* tree_vec = trees->GetList();

      tree_vec->clear();

      for (std::list<sTree>::iterator iter = saplings.begin(); iter != saplings.end(); ++iter)
      {
        //(*trees)[i] = (*iter)
        NBT_Value* tree = new NBT_Value(NBT_Value::TAG_COMPOUND);
        tree->Insert("X", new NBT_Value((int32_t)(*iter).x));
        tree->Insert("Y", new NBT_Value((int32_t)(*iter).y));
        tree->Insert("Z", new NBT_Value((int32_t)(*iter).z));
        tree->Insert("plantedTime", new NBT_Value((int32_t)(*iter).plantedTime));
        tree->Insert("plantedBy", new NBT_Value((int32_t)(*iter).plantedBy));
        tree_vec->push_back(tree);
      }
    }
    root->SaveToFile(infile);

    delete root;
  }

  return true;
}

//#define PRINT_LIGHTGEN_TIME

bool Map::generateLight(int x, int z, sChunk* chunk)
{
  if (chunk == NULL)
  {
    const ChunkMap::const_iterator it = chunks.find(Coords(x, z));

    if (it == chunks.end())
    {
      LOGLF("Loading chunk failed (generateLight)");
      return false;
    }
    else
    {
      chunk = it->second;
    }
  }

#ifdef PRINT_LIGHTGEN_TIME
#ifdef WIN32
  DWORD t_begin, t_end;
  t_begin = timeGetTime();
#else
  clock_t t_begin, t_end;
  t_begin = clock();
#endif
#endif

  uint8_t* blocks     = chunk->blocks;
  uint8_t* skylight   = chunk->skylight;
  uint8_t* blocklight = chunk->blocklight;
  uint8_t* heightmap  = chunk->heightmap;

  int highest_y = 0;

  // Clear lightmaps
  memset(skylight, 0, 16 * 16 * 128 / 2);
  memset(blocklight, 0, 16 * 16 * 128 / 2);

  // Sky light
  int light = 0;
  bool foundheight = false;
  for (int block_x = 0; block_x < 16; block_x++)
  {
    for (int block_z = 0; block_z < 16; block_z++)
    {
      light = 15;
      foundheight = false;
      int32_t blockx_blockz = (block_z << 7) + (block_x << 11);

      for (int block_y = 127; block_y > 0; block_y--)
      {
        int index      = block_y + blockx_blockz;
        int absolute_x = x * 16 + block_x;
        int absolute_z = z * 16 + block_z;
        uint8_t block    = blocks[index];

        light -= stopLight[block];
        if (light < 0)
        {
          light = 0;
        }

        // Calculate heightmap while looping this
        if ((block != BLOCK_AIR) && (foundheight == false))
        {
          heightmap[block_z + (block_x << 4)] = ((block_y == 127) ? block_y : block_y + 1);
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
      int32_t blockx_blockz = (block_z << 7) + (block_x << 11);
      for (int block_y = highest_y; block_y >= 0; block_y--)
      {
        int index      = block_y + blockx_blockz;
        int absolute_x = x * 16 + block_x;
        int absolute_z = z * 16 + block_z;
        uint8_t block    = blocks[index];

        // If light emitting block
        if (emitLight[block] > 0)
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
      for (int block_y = heightmap[block_z + (block_x << 4)]; block_y >= 0; block_y--)
      {
        int absolute_x = x * 16 + block_x;
        int absolute_z = z * 16 + block_z;
        uint8_t skylight_s, blocklight_s;

        getLight(absolute_x, block_y, absolute_z, &skylight_s, &blocklight_s, chunk);

        if (skylight_s)
        {
          spreadLight(absolute_x, block_y, absolute_z, skylight_s, 0);
        }

        if (blocklight_s)
        {
          spreadLight(absolute_x, block_y, absolute_z, blocklight_s, 1);
        }
      }
    }
  }
#ifdef PRINT_LIGHTGEN_TIME
#ifdef WIN32
  t_end = timeGetTime();
  Mineserver::get()->logger()->log("Lightgen: " + dtos(t_end - t_begin) + "ms");
#else
  t_end = clock();
  Mineserver::get()->logger()->log("Lightgen: " + dtos((t_end - t_begin) / (CLOCKS_PER_SEC / 1000))) + "ms");
#endif
#endif


  return true;
}

void Map::spreadLight(int x, int y, int z, int light_value, uint8_t type /* 0: sky, 1: block */)
{
  if ((y < 0) || (y > 127))
  {
    //LOGLF("Invalid y value (spreadLight)");
    // For sky style maps or maps with holes, this spams.
    return;
  }

  // If no light, stop!
  if (light_value < 1)
  {
    return;
  }

  for (int direction = 0; direction < 6; direction++)
  {
    // Going too high
    if ((y == 127) && (direction == 2))
    {
      //Skip this direction
      direction++;
    }
    // going negative
    if ((y == 0) && (direction == 3))
    {
      //Skip this direction
      direction++;
    }

    int x_toset = x;
    int y_toset = y;
    int z_toset = z;

    switch (direction)
    {
    case 0:
      x_toset++;
      break;
    case 1:
      x_toset--;
      break;
    case 2:
      y_toset++;
      break;
    case 3:
      y_toset--;
      break;
    case 4:
      z_toset++;
      break;
    case 5:
      z_toset--;
      break;
    }

    sChunk * chunk = getMapData(blockToChunk(x_toset), blockToChunk(z_toset), false);

    if (chunk != NULL)
    {
      uint8_t block, meta, skylightCurrent, blocklightCurrent;

      getBlock(x_toset, y_toset, z_toset, &block, &meta, false, chunk);

      getLight(x_toset, y_toset, z_toset, &skylightCurrent, &blocklightCurrent, chunk);

      const int lightNew = std::max(0, light_value - int(stopLight[block]) - 1);

      if (lightNew > (type == 0 ? skylightCurrent : blocklightCurrent))
      {
        if      (type == 0) setLight(x_toset, y_toset, z_toset, lightNew, 0, 1, chunk);
        else if (type == 1) setLight(x_toset, y_toset, z_toset, 0, lightNew, 2, chunk);
        spreadLight(x_toset, y_toset, z_toset, lightNew, type);
      }
    }
    else
    {
      //LOGLF("Loading chunk failed (spreadLight)");
      return;
    }
  }
}

bool Map::getBlock(int x, int y, int z, uint8_t* type, uint8_t* meta, bool generate)
{
  if ((y < 0) || (y > 127))
  {
    std::ostringstream str;
    str << "Invalid y value (" << x << ", " << y << ", " << z << ")";
    LOG2(DEBUG, str.str());
    return false;
  }

  int chunk_x = blockToChunk(x);
  int chunk_z = blockToChunk(z);

  sChunk* chunk = getMapData(chunk_x, chunk_z, generate);

  if (!chunk)
  {
    if (generate)
    {
      LOGLF("Loading chunk failed (getBlock)");
    }

    return false;
  }

  return getBlock(x, y, z, type, meta, generate, chunk);
}

bool Map::getBlock(int x, int y, int z, uint8_t* type, uint8_t* meta, bool generate, sChunk* chunk)
{
  int chunk_block_x  = blockToChunkBlock(x);
  int chunk_block_z  = blockToChunkBlock(z);

  uint8_t* blocks      = chunk->blocks;
  uint8_t* metapointer = chunk->data;
  int index            = y + (chunk_block_z << 7) + (chunk_block_x << 11);
  *type                = blocks[index];
  uint8_t metadata     = metapointer[(index) >> 1];

  if (y & 1)
  {
    metadata  &= 0xf0;
    metadata >>= 4;
  }
  else
  {
    metadata &= 0x0f;
  }

  *meta              = metadata;
  chunk->lastused    = (int)time(0);

  return true;
}

bool Map::getLight(int x, int y, int z, uint8_t* skylight, uint8_t* blocklight)
{
  if ((y < 0) || (y > 127))
  {
    LOGLF("Invalid y value (getLight)");
    return false;
  }

  // Map chunk pos from block pos
  int chunk_x = blockToChunk(x);
  int chunk_z = blockToChunk(z);

  sChunk* chunk = getMapData(chunk_x, chunk_z, false);

  if (!chunk)
  {
    LOGLF("Loading chunk failed (getLight)");
    return false;
  }

  return getLight(x, y, z, skylight, blocklight, chunk);
}

bool Map::getLight(int x, int y, int z, uint8_t* skylight, uint8_t* blocklight, sChunk* chunk)
{
  // Which block inside the chunk
  int chunk_block_x = blockToChunkBlock(x);
  int chunk_block_z = blockToChunkBlock(z);

  uint8_t* blocklightPtr = chunk->blocklight;
  uint8_t* skylightPtr   = chunk->skylight;
  int index            = y + (chunk_block_z << 7) + (chunk_block_x << 11);
  *blocklight = blocklightPtr[(index) >> 1];
  *skylight   = skylightPtr[(index) >> 1];

  if (y % 2)
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

  if ((y < 0) || (y > 127))
  {
    LOGLF("Invalid y value (setLight)");
    return false;
  }

  int chunk_x = blockToChunk(x);
  int chunk_z = blockToChunk(z);

  sChunk* chunk = getMapData(chunk_x, chunk_z, false);

  if (!chunk)
  {
    LOGLF("Loading chunk failed (setLight)");
    return false;
  }

  return setLight(x, y, z, skylight, blocklight, type, chunk);
}

bool Map::setLight(int x, int y, int z, int skylight, int blocklight, int type, sChunk* chunk)
{
  //Make sure we are inside boundaries
  if ((y < 0) || (y > 127))
  {
    LOGLF("Invalid y value (setLight 2)");
    return false;
  }
  int chunk_block_x        = blockToChunkBlock(x);
  int chunk_block_z        = blockToChunkBlock(z);

  uint8_t* blocklightPtr     = chunk->blocklight;
  uint8_t* skylightPtr       = chunk->skylight;
  int index                = y + (chunk_block_z << 7) + (chunk_block_x << 11);
  char skylight_local      = skylightPtr[index >> 1];
  char blocklight_local    = blocklightPtr[index >> 1];

  if (y & 1)
  {
    if (type & 0x5) // 1 or 4
    {
      skylight_local &= 0x0f;
      skylight_local |= skylight << 4;
    }

    if (type & 0x6) // 2 or 4
    {
      blocklight_local &= 0x0f;
      blocklight_local |= blocklight << 4;
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
    skylightPtr[index >> 1] = skylight_local;
  }

  if (type & 0x6) // 2 or 4
  {
    blocklightPtr[index >> 1] = blocklight_local;
  }

  return true;
}

bool Map::setBlock(int x, int y, int z, char type, char meta)
{
  if ((y < 0) || (y > 127))
  {
    LOGLF("Invalid y value (setBlock)");
    return false;
  }

  // Map chunk pos from block pos
  int chunk_x = blockToChunk(x);
  int chunk_z = blockToChunk(z);

  sChunk* chunk = getMapData(chunk_x, chunk_z, true);

  if (!chunk)
  {
    LOGLF("Loading chunk failed (setBlock)");
    return false;
  }

  // Which block inside the chunk
  int chunk_block_x  = blockToChunkBlock(x);
  int chunk_block_z  = blockToChunkBlock(z);

  uint8_t* blocks      = chunk->blocks;
  uint8_t* metapointer = chunk->data;
  int index          = y + (chunk_block_z << 7) + (chunk_block_x << 11);
  blocks[index] = type;
  char metadata      = metapointer[index >> 1];

  if (y & 1)
  {
    metadata &= 0x0f;
    metadata |= meta << 4;
  }
  else
  {
    metadata &= 0xf0;
    metadata |= meta;
  }
  metapointer[index >> 1] = metadata;

  chunk->changed       = true;
  chunk->lightRegen    = true;
  chunk->lastused      = (int)time(NULL);

  if (type == BLOCK_AIR)
  {
    uint8_t temp_type = 0, temp_meta = 0;
    int8_t temp_y = y;
    while (getBlock(x, temp_y, z, &temp_type, &temp_meta, false) && (temp_type == BLOCK_AIR))
    {
      temp_y--;
    }
    // We've actually moved down past the last air block to the one beneath, so we need to go back up one
    temp_y++;

    std::map<uint32_t, spawnedItem*>::const_iterator it_a = items.begin(), it_b = items.end();
    for (; it_a != it_b; ++it_a)
    {
      if ((floor(static_cast<double>(it_a->second->pos.x()) / 32) == x) && (floor(static_cast<double>(it_a->second->pos.y()) / 32) == y + 1) && (floor(static_cast<double>(it_a->second->pos.z()) / 32) == z))
      {
        it_a->second->pos.y() = temp_y * 32;
      }
    }
  }

  return true;
}

bool Map::sendBlockChange(int x, int y, int z, char type, char meta)
{
  const ChunkMap::const_iterator it = chunks.find(Coords(blockToChunk(x), blockToChunk(z)));

  if (it == chunks.end())
  {
    return false;
  }

  Packet pkt;

  pkt << PACKET_BLOCK_CHANGE << (int32_t)x << (int8_t)y << (int32_t)z << (int8_t)type << (int8_t)meta;

  it->second->sendPacket(pkt);

  return true;
}

bool Map::sendNote(int x, int y, int z, char instrument, char pitch)
{
  const ChunkMap::const_iterator it = chunks.find(Coords(blockToChunk(x), blockToChunk(z)));

  if (it == chunks.end())
  {
    return false;
  }

  Packet pkt;

  pkt << PACKET_PLAY_NOTE << (int32_t)x << (int16_t)y << (int32_t)z << (int8_t)instrument << (int8_t)pitch;

  it->second->sendPacket(pkt);

  return true;
}

bool Map::sendPickupSpawn(spawnedItem item)
{
  //Push to global item storage
  spawnedItem* storedItem = new spawnedItem;
  *storedItem     = item;
  items[item.EID] = storedItem;

  //Push to local item storage
  int chunk_x = blockToChunk(item.pos.x() / 32);
  int chunk_z = blockToChunk(item.pos.z() / 32);

  const ChunkMap::const_iterator it = chunks.find(Coords(chunk_x, chunk_z));

  if (it == chunks.end())
  {
    return false;
  }

  it->second->items.push_back(storedItem);

  Packet pkt;

  pkt << PACKET_PICKUP_SPAWN << (int32_t)item.EID << (int16_t)item.item << (int8_t)item.count << (int16_t)item.health
      << (int32_t)item.pos.x() << (int32_t)item.pos.y() << (int32_t)item.pos.z()
      << (int8_t)0 << (int8_t)0 << (int8_t)0;

  it->second->sendPacket(pkt);

  return true;
}

void Map::createPickupSpawn(int x, int y, int z, int type, int count, int health, User* user)
{
  spawnedItem item;
  item.EID      = Mineserver::generateEID();
  item.health   = health;
  item.item     = type;
  item.count    = count;
  if (user != NULL)
  {
    item.spawnedBy = user->UID;
  }
  item.spawnedAt = time(NULL);

  // Check to see if the block we're being asked to put the pickup spawn in is air
  // If so, find the lowest air block at x,*,z that we can place it in and move it there
  // Effectively we get... Gravity!
  uint8_t temp_type = 0, temp_meta = 0;
  int8_t temp_y = y;
  if (getBlock(x, temp_y, z, &temp_type, &temp_meta, false) && (temp_type == BLOCK_AIR))
  {
    while (getBlock(x, temp_y, z, &temp_type, &temp_meta, false) && (temp_type == BLOCK_AIR))
    {
      temp_y--;
    }
    // We've actually moved down past the last air block, so we need to go back up one
    temp_y++;

    y = temp_y;
  }

  item.pos.x()  = x * 32;
  item.pos.y()  = y * 32;
  item.pos.z()  = z * 32;
  //Randomize spawn position a bit
  item.pos.x() += 5 + (rand() % 22);
  item.pos.z() += 5 + (rand() % 22);

  sendPickupSpawn(item);
}

bool Map::sendProjectileSpawn(User* user, int8_t projID)
{
  if (!projID)
  {
    return false;
  }

  Packet  pkt;
  int32_t EID = Mineserver::generateEID();
  float   tempMult = 1.f - abs(user->pos.pitch / 90.f);

  // Spawn projectile on player location
  // TODO: add about 0.5-1 units in forward direction here
  vec pos = vec(user->pos.x * 32, (user->pos.y + 1.5) * 32, user->pos.z * 32);
  vec vel = vec(
              sinf(-(user->pos.yaw / 360.f) * 2.f * M_PI) * tempMult * 32768.f,
              -(user->pos.pitch / 90.f) * 32768.f,
              cosf(-(user->pos.yaw / 360.f) * 2.f * M_PI) * tempMult * 32768.f
            );
  pkt << (int8_t)PACKET_ENTITY << (int32_t)EID
      << (int8_t)PACKET_ADD_OBJECT << (int32_t)EID << (int8_t)projID << (int32_t)pos.x() << (int32_t)pos.y() << (int32_t)pos.z()
      << (int8_t)PACKET_ENTITY_VELOCITY << (int32_t)EID << (int16_t)vel.x() << (int16_t)vel.y() << (int16_t)vel.z();

  user->sendAll((uint8_t*)pkt.getWrite(), pkt.getWriteLen());

  return true;
}

sChunk* Map::loadMap(int x, int z, bool generate)
{
  const ChunkMap::const_iterator it = chunks.find(Coords(x, z));

  // Case 1: We already have the chunk.
  if (it != chunks.end())
  {
    return it->second;
  }

  // Case 2: We don't have the chunk but it's on file. Try to open region file.
  RegionFile* newRegion = new RegionFile;
  if (!newRegion->openFile(mapDirectory, x, z))
  {
    std::cout << "Error loading file" << std::endl;
    delete newRegion;
    return NULL;
  }

  //Allocate memory for uncompressed chunk data
  uint8_t* chunkPointer =  new uint8_t[ALLOCATE_NBTFILE*10];
  uint32_t chunkLen = 0;

  // Try to load chunk. If fails, Case 3: Generate a new chunk.
  if (!newRegion->readChunk(chunkPointer, &chunkLen, x, z))
  {
    // If generate (false only for lightmapgenerator)
    if (generate)
    {
      // Re-seed! We share map gens with other maps
      Mineserver::get()->mapGen(m_number)->init((int32_t)mapSeed);
      Mineserver::get()->mapGen(m_number)->generateChunk(x, z, m_number);
      generateLight(x, z);
      //If we generated spawn pos, make sure the position is not underground!
      if (x == blockToChunk(spawnPos.x()) && z == blockToChunk(spawnPos.z()))
      {
        uint8_t block, meta;
        bool foundLand = false;
        if (getBlock(spawnPos.x(), spawnPos.y(), spawnPos.z(), &block, &meta, false) && block == BLOCK_AIR)
        {
          uint8_t new_y;
          for (new_y = spawnPos.y(); new_y > 30; new_y--)
          {
            if (getBlock(spawnPos.x(), new_y, spawnPos.z(), &block, &meta, false) && block != BLOCK_AIR)
            {
              foundLand = true;
              break;
            }
          }
          if (foundLand)
          {
            //Store new spawn position to level.dat
            spawnPos.y() = new_y + 1;
            std::string infile = mapDirectory + "/level.dat";
            NBT_Value* root = NBT_Value::LoadFromFile(infile);
            if (root != NULL)
            {
              NBT_Value& data = *((*root)["Data"]);
              *data["SpawnX"] = (int32_t)spawnPos.x();
              *data["SpawnY"] = (int32_t)spawnPos.y();
              *data["SpawnZ"] = (int32_t)spawnPos.z();

              root->SaveToFile(infile);

              delete root;
            }
          }
        }
      }
      delete newRegion;
      delete [] chunkPointer;
      return getChunk(x, z);
    }
    else
    {
      delete newRegion;
      delete [] chunkPointer;
      return NULL;
    }
  }

  delete newRegion;

  sChunk* chunk = new sChunk();

  //Load NBT from memory
  chunk->nbt = NBT_Value::LoadFromMemory(chunkPointer, chunkLen);

  delete [] chunkPointer;


  if (chunk->nbt == NULL)
  {
    LOGLF("Error in loading map (unable to load file)");
    delete chunk;
    Mineserver::get()->mapGen(m_number)->init((int32_t)mapSeed);
    Mineserver::get()->mapGen(m_number)->generateChunk(x, z, m_number);
    generateLight(x, z);
    return getChunk(x, z);
  }

  NBT_Value* level = (*chunk->nbt)["Level"];

  if (level == NULL)
  {
    LOGLF("Error in loading map (unable to find Level)");
    delete chunk;
    Mineserver::get()->mapGen(m_number)->init((int32_t)mapSeed);
    Mineserver::get()->mapGen(m_number)->generateChunk(x, z, m_number);
    generateLight(x, z);
    return getChunk(x, z);
  }

  NBT_Value* xPos = (*level)["xPos"];
  NBT_Value* zPos = (*level)["zPos"];

  if (xPos && zPos)
  {
    chunk->x = *xPos;
    chunk->z = *zPos;
  }
  else
  {
    LOG(WARNING, "Map", "incorrect chunk (missing xPos or zPos) regenerating");
    delete chunk;
    Mineserver::get()->mapGen(m_number)->init((int32_t)mapSeed);
    Mineserver::get()->mapGen(m_number)->generateChunk(x, z, m_number);
    generateLight(x, z);
    return getChunk(x, z);
  }

  NBT_Value* nbt_blocks     = (*level)["Blocks"];
  NBT_Value* nbt_data       = (*level)["Data"];
  NBT_Value* nbt_blocklight = (*level)["BlockLight"];
  NBT_Value* nbt_skylight   = (*level)["SkyLight"];
  NBT_Value* nbt_heightmap  = (*level)["HeightMap"];

  if (!nbt_blocks || !nbt_data || !nbt_blocklight || !nbt_skylight || !nbt_heightmap)
  {
    LOGLF("Error in loading map (chunk missing data)");
    delete chunk;
    return NULL;
  }

  std::vector<uint8_t>* blocks = nbt_blocks->GetByteArray();
  std::vector<uint8_t>* data = nbt_data->GetByteArray();
  std::vector<uint8_t>* blocklight = nbt_blocklight->GetByteArray();
  std::vector<uint8_t>* skylight = nbt_skylight->GetByteArray();
  std::vector<uint8_t>* heightmap = nbt_heightmap->GetByteArray();

  if (!blocks || !data || !blocklight || !skylight || !heightmap)
  {
    LOGLF("Error in loading map (chunk missing data)");
    delete chunk;
    return NULL;
  }

  size_t fullLen = (16 * 128 * 16);
  size_t halfLen = fullLen >> 1;
  //Check that there is all the data we need
  if (blocks->size()     != fullLen ||
      data->size()       != halfLen ||
      blocklight->size() != halfLen ||
      skylight->size()   != halfLen)
  {
    LOGLF("Error in loading map (corrupt?)");
    delete chunk;
    return NULL;
  }

  chunk->blocks     = &((*blocks)[0]);
  chunk->data       = &((*data)[0]);
  chunk->blocklight = &((*blocklight)[0]);
  chunk->skylight   = &((*skylight)[0]);
  chunk->heightmap  = &((*heightmap)[0]);

  chunks.insert(ChunkMap::value_type(ChunkMap::key_type(x, z), chunk));

  // Update last used time
  chunk->lastused = time(NULL);

  // Not changed
  chunk->changed    = false;
  chunk->lightRegen = false;

  //Get list of chests,furnaces etc on the chunk
  NBT_Value* entityList = (*level)["TileEntities"];

  //Verify the type
  if (entityList && entityList->GetType() == NBT_Value::TAG_LIST && entityList->GetListType() == NBT_Value::TAG_COMPOUND)
  {
    std::vector<NBT_Value*>* const entities = entityList->GetList();

    for (std::vector<NBT_Value*>::iterator iter = entities->begin(); iter != entities->end() ; ++iter)
    {
      std::vector<uint8_t> buffer;
      NBT_Value* idVal = (**iter)["id"];
      if (idVal == NULL)
      {
        continue;
      }

      //Get TileEntity ID
      std::string* id = idVal->GetString();
      if (id == NULL)
      {
        continue;
      }

      //Check that x,y and z are present and in correct format
      if ((**iter)["x"]->GetType() != NBT_Value::TAG_INT ||
          (**iter)["y"]->GetType() != NBT_Value::TAG_INT ||
          (**iter)["z"]->GetType() != NBT_Value::TAG_INT)
      {
        continue;
      }

      int32_t entityX = *(**iter)["x"];
      int32_t entityY = *(**iter)["y"];
      int32_t entityZ = *(**iter)["z"];

      //Extract sign data
      if ((*id == "Sign"))
      {
        signData* newSign = new signData;
        newSign->x = entityX;
        newSign->y = entityY;
        newSign->z = entityZ;
        newSign->text1 = *(**iter)["Text1"]->GetString();
        newSign->text2 = *(**iter)["Text2"]->GetString();
        newSign->text3 = *(**iter)["Text3"]->GetString();
        newSign->text4 = *(**iter)["Text4"]->GetString();

        chunk->signs.push_back(newSign);
      }
      //Extract chest data
      else if ((*id == "Chest"))
      {
        NBT_Value* chestItems = (**iter)["Items"];

        if (chestItems->GetType() == NBT_Value::TAG_LIST)
        {
          if (chestItems->GetListType() != NBT_Value::TAG_COMPOUND)
          {
            continue;
          }

          std::vector<NBT_Value*>* itemList = chestItems->GetList();
          std::vector<NBT_Value*>::iterator itemIterator = itemList->begin(), itemIteratorEnd = itemList->end();

          chestData* newChest = new chestData;
          newChest->x = entityX;
          newChest->y = entityY;
          newChest->z = entityZ;

          //Loop items
          for (; itemIterator != itemIteratorEnd; itemIterator++)
          {
            //Check that all info exists and is the right type
            if ((**itemIterator)["Count"]  == NULL || (**itemIterator)["Slot"] == NULL ||
                (**itemIterator)["Damage"] == NULL || (**itemIterator)["id"] == NULL   ||

                (**itemIterator)["Count"]->GetType() != NBT_Value::TAG_BYTE ||
                (**itemIterator)["Slot"]->GetType() != NBT_Value::TAG_BYTE ||
                (**itemIterator)["Damage"]->GetType() != NBT_Value::TAG_SHORT ||
                (**itemIterator)["id"]->GetType() != NBT_Value::TAG_SHORT)
            {
              continue;
            }
            newChest->items[(int8_t) *(**itemIterator)["Slot"]].setCount((int8_t)   *(**itemIterator)["Count"]);
            newChest->items[(int8_t) *(**itemIterator)["Slot"]].setHealth((int16_t) *(**itemIterator)["Damage"]);
            newChest->items[(int8_t) *(**itemIterator)["Slot"]].setType((int16_t)   *(**itemIterator)["id"]);
          }
          //Push to our chest storage at chunk
          chunk->chests.push_back(newChest);
        }
      }

      //Next, furnace data
      else if ((*id == "Furnace"))
      {
        NBT_Value* chestItems = (**iter)["Items"];

        if (chestItems->GetType() == NBT_Value::TAG_LIST)
        {
          if (chestItems->GetListType() != NBT_Value::TAG_COMPOUND)
          {
            continue;
          }

          std::vector<NBT_Value*>* itemList = chestItems->GetList();
          std::vector<NBT_Value*>::iterator itemIterator = itemList->begin(), itemIteratorEnd = itemList->end();

          if ((**iter)["BurnTime"] == NULL || (**iter)["CookTime"] == NULL)
          {
            //Skip
            continue;
          }

          furnaceData* newFurnace = new furnaceData;
          newFurnace->x = entityX;
          newFurnace->y = entityY;
          newFurnace->z = entityZ;
          newFurnace->map = m_number;
          newFurnace->burnTime = (int16_t) * (**iter)["BurnTime"];
          newFurnace->cookTime = (int16_t) * (**iter)["CookTime"];

          //Loop through all items
          for (; itemIterator != itemIteratorEnd; itemIterator++)
          {
            //Check that all info exists and is the right type
            if ((**itemIterator)["Count"] == NULL || (**itemIterator)["Slot"] == NULL ||
                (**itemIterator)["Damage"] == NULL || (**itemIterator)["id"] == NULL ||

                (**itemIterator)["Count"]->GetType()  != NBT_Value::TAG_BYTE  ||
                (**itemIterator)["Slot"]->GetType()   != NBT_Value::TAG_BYTE  ||
                (**itemIterator)["Damage"]->GetType() != NBT_Value::TAG_SHORT ||
                (**itemIterator)["id"]->GetType()     != NBT_Value::TAG_SHORT ||
                (int8_t) *(**itemIterator)["Slot"] > 3 || (int8_t) *(**itemIterator)["Slot"] < 0)
            {
              //Skip
              continue;
            }
            newFurnace->items[(int8_t) *(**itemIterator)["Slot"]].setCount((int8_t)   *(**itemIterator)["Count"]);
            newFurnace->items[(int8_t) *(**itemIterator)["Slot"]].setHealth((int16_t) *(**itemIterator)["Damage"]);
            newFurnace->items[(int8_t) *(**itemIterator)["Slot"]].setType((int16_t)   *(**itemIterator)["id"]);
          }

          //Push to our furnace storage at chunk and check for possible activity
          chunk->furnaces.push_back(newFurnace);
          Mineserver::get()->furnaceManager()->handleActivity(newFurnace);
        }
      }

      //Delete list item
      delete(*iter);
      (*iter) = NULL;
    }

    //Clear the list
    entityList->GetList()->clear();
  }

  return chunk;
}

bool Map::saveMap(int x, int z)
{
  sChunk* chunk = getChunk(x, z);

  if (!chunk->changed)
  {
    return true;
  }

  // Recalculate light maps
  if (chunk->lightRegen)
  {
    generateLight(x, z, chunk);
  }

  //Create directory for region files
  struct stat stFileInfo;
  std::string regionDir = mapDirectory + "/region";
  if (stat(regionDir.c_str(), &stFileInfo) != 0)
  {
#ifdef WIN32
    if (_mkdir(std::string(regionDir).c_str()) == -1)
#else
    if (mkdir(std::string(regionDir).c_str(), 0755) == -1)
#endif
    {
      LOG(EMERG, "Map", "Error: Could not create map/region directory.");

      exit(EXIT_FAILURE);
    }
  }

  //Store tile entities separately because they are stored in our own arrays with chunk data
  NBT_Value* entityList = (*(*chunk->nbt)["Level"])["TileEntities"];

  if (!entityList)
  {
    entityList = new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND);
    chunk->nbt->Insert("TileEntities", entityList);
  }

  //Save signs
  for (uint32_t i = 0; i < chunk->signs.size(); i++)
  {
    NBT_Value* val = new NBT_Value(NBT_Value::TAG_COMPOUND);
    val->Insert("id", new NBT_Value(std::string("Sign")));
    val->Insert("x", new NBT_Value((int32_t)chunk->signs[i]->x));
    val->Insert("y", new NBT_Value((int32_t)chunk->signs[i]->y));
    val->Insert("z", new NBT_Value((int32_t)chunk->signs[i]->z));
    val->Insert("Text1", new NBT_Value(chunk->signs[i]->text1));
    val->Insert("Text2", new NBT_Value(chunk->signs[i]->text2));
    val->Insert("Text3", new NBT_Value(chunk->signs[i]->text3));
    val->Insert("Text4", new NBT_Value(chunk->signs[i]->text4));

    entityList->GetList()->push_back(val);
  }

  //Save chests
  for (uint32_t i = 0; i < chunk->chests.size(); i++)
  {
    NBT_Value* val = new NBT_Value(NBT_Value::TAG_COMPOUND);
    val->Insert("id", new NBT_Value(std::string("Chest")));
    val->Insert("x", new NBT_Value((int32_t)chunk->chests[i]->x));
    val->Insert("y", new NBT_Value((int32_t)chunk->chests[i]->y));
    val->Insert("z", new NBT_Value((int32_t)chunk->chests[i]->z));
    NBT_Value* nbtInv = new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND);
    for (uint32_t slot = 0; slot < 27; slot++)
    {
      if (chunk->chests[i]->items[slot].getCount() && chunk->chests[i]->items[slot].getType() != -1)
      {
        NBT_Value* val = new NBT_Value(NBT_Value::TAG_COMPOUND);
        val->Insert("Count", new NBT_Value((int8_t)chunk->chests[i]->items[slot].getCount()));
        val->Insert("Slot", new NBT_Value((int8_t)slot));
        val->Insert("Damage", new NBT_Value((int16_t)chunk->chests[i]->items[slot].getHealth()));
        val->Insert("id", new NBT_Value((int16_t)chunk->chests[i]->items[slot].getType()));
        nbtInv->GetList()->push_back(val);
      }
    }
    val->Insert("Items", nbtInv);

    entityList->GetList()->push_back(val);
  }

  //Save furnaces
  for (uint32_t i = 0; i < chunk->furnaces.size(); i++)
  {
    NBT_Value* val = new NBT_Value(NBT_Value::TAG_COMPOUND);
    val->Insert("id", new NBT_Value(std::string("Furnace")));
    val->Insert("x", new NBT_Value((int32_t)chunk->furnaces[i]->x));
    val->Insert("y", new NBT_Value((int32_t)chunk->furnaces[i]->y));
    val->Insert("z", new NBT_Value((int32_t)chunk->furnaces[i]->z));
    val->Insert("BurnTime", new NBT_Value((int16_t)chunk->furnaces[i]->burnTime));
    val->Insert("CookTime", new NBT_Value((int16_t)chunk->furnaces[i]->cookTime));
    NBT_Value* nbtInv = new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND);

    for (uint32_t slot = 0; slot < 3; slot++)
    {
      //Store only non-null info
      if (chunk->furnaces[i]->items[slot].getCount() && chunk->furnaces[i]->items[slot].getType() != 0 && chunk->furnaces[i]->items[slot].getType() != -1)
      {
        NBT_Value* val = new NBT_Value(NBT_Value::TAG_COMPOUND);
        val->Insert("Count", new NBT_Value((int8_t)chunk->furnaces[i]->items[slot].getCount()));
        val->Insert("Slot", new NBT_Value((int8_t)slot));
        val->Insert("Damage", new NBT_Value((int16_t)chunk->furnaces[i]->items[slot].getHealth()));
        val->Insert("id", new NBT_Value((int16_t)chunk->furnaces[i]->items[slot].getType()));
        nbtInv->GetList()->push_back(val);
      }
    }
    val->Insert("Items", nbtInv);

    entityList->GetList()->push_back(val);
  }

  //Allocate memory for NBT and save (+deflate) it
  uint8_t* buffer = new uint8_t[ALLOCATE_NBTFILE];
  uint32_t len;
  chunk->nbt->SaveToMemory(buffer, &len);

  //Open regionfile and write chunk
  RegionFile newRegion;
  newRegion.openFile(mapDirectory, x, z);
  newRegion.writeChunk(buffer, len, x, z);

  delete [] buffer;

  // Set "not changed"
  chunk->changed    = false;
  chunk->lightRegen = false;

  return true;
}

bool Map::releaseMap(int x, int z)
{
  // save first
  saveMap(x, z);

  // free the memory allocated to the sChunk
  delete getChunk(x, z);

  // erase the chunk pointer from the collection
  chunks.erase(Coords(x, z));

  return true;
}

bool Map::sendMultiBlocks(std::vector<vec> & blocks)
{
  while (blocks.size() > 0)
  {
    std::vector<vec> toRem;
    int chunk_x = blockToChunk(blocks[0].x());
    int chunk_z = blockToChunk(blocks[0].z());
    for (size_t i = 0; i < blocks.size(); i++)
    {
      int t_chunk_x = blockToChunk(blocks[i].x());
      int t_chunk_z = blockToChunk(blocks[i].z());
      if (chunk_x == t_chunk_x && chunk_z == t_chunk_z)
      {
        for (int j = toRem.size() - 1; j >= 0; j--)
        {
          if (toRem[j].x() == blocks[i].x() &&
              toRem[j].y() == blocks[i].y() &&
              toRem[j].z() == blocks[i].z())
          {
            continue;
          }
        }
        toRem.push_back(blocks[i]);
      }
    }
    Packet packet, pC, pT, pM;
    int offsetx = chunk_x << 4, offsetz = chunk_z << 4;
    packet << (int8_t) PACKET_MULTI_BLOCK_CHANGE << (int32_t) chunk_x << (int32_t) chunk_z << (int16_t) toRem.size();
    for (size_t i = 0; i < toRem.size(); i++)
    {
      uint8_t block, meta;
      Mineserver::get()->map(m_number)->getBlock(toRem[i].x(), toRem[i].y(), toRem[i].z(), &block, &meta);
      // Sending packet a uint16_t makes it assume int...
      uint16_t coord = (((toRem[i].x() - offsetx) << 12) + ((toRem[i].z() - offsetz) << 8) + (toRem[i].y()));
      int16_t* coord2 = (int16_t*)&coord;
      pC << (int16_t)(*coord2);
      pT << (int8_t) block;
      pM << (int8_t) meta;
      for (int j = blocks.size() - 1; j >= 0; j--)
      {
        if (blocks[j].x() == toRem[i].x() &&
            blocks[j].y() == toRem[j].y() &&
            blocks[j].z() == toRem[i].z())
        {
          blocks.erase(blocks.begin() + j);
        }
      }
    }
    toRem.clear();

    const ChunkMap::const_iterator it = chunks.find(Coords(chunk_x, chunk_z));

    if (it == chunks.end())
    {
      //return false;
      continue;
    }

    it->second->sendPacket(packet);
    it->second->sendPacket(pC);
    it->second->sendPacket(pT);
    it->second->sendPacket(pM);
  }

  return true;
}

// Send chunk to user
void Map::sendToUser(User* user, int x, int z, bool login)
{
  Packet* p;
  if (login)
  {
    p = &user->loginBuffer;
  }
  else
  {
    p = &user->buffer;
  }
  sChunk* chunk = loadMap(x, z);
  if (chunk == NULL)
  {
    return;
  }

  uint8_t* data4   = new uint8_t[18 + 81920];
  uint8_t* mapdata = new uint8_t[81920];
  int32_t mapposx    = x;
  int32_t mapposz    = z;

  //Regenerate lighting if needed
  if (chunk->lightRegen)
  {
    generateLight(x, z, chunk);
    chunk->lightRegen = false;
  }


  // Chunk
  (*p) << (int8_t)PACKET_MAP_CHUNK << (int32_t)(mapposx * 16) << (int16_t)0 << (int32_t)(mapposz * 16)
       << (int8_t)15 << (int8_t)127 << (int8_t)15;

  memcpy(&mapdata[0], chunk->blocks, 32768);
  memcpy(&mapdata[32768], chunk->data, 16384);
  memcpy(&mapdata[32768 + 16384], chunk->blocklight, 16384);
  memcpy(&mapdata[32768 + 16384 + 16384], chunk->skylight, 16384);

  uLongf written = 81920;
  Bytef* buffer = new Bytef[written];

  // Compress data with zlib deflate
  compress(buffer, &written, &mapdata[0], 81920);

  (*p) << (int32_t)written;
  (*p).addToWrite(buffer, written);

  //Push sign data to player
  for (uint32_t i = 0; i < chunk->signs.size(); i++)
  {
    (*p) << (int8_t)PACKET_SIGN << chunk->signs[i]->x << (int16_t)chunk->signs[i]->y << chunk->signs[i]->z;
    (*p) << chunk->signs[i]->text1 << chunk->signs[i]->text2 << chunk->signs[i]->text3 << chunk->signs[i]->text4;
  }


  delete [] buffer;

  delete[] data4;
  delete[] mapdata;
}

