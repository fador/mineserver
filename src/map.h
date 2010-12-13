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

#ifndef _MAP_H_
#define _MAP_H_

#include <map>
#include <list>
#include <ctime>
#include "nbt.h"
#include "user.h"
#include "vec.h"
#include "chunkmap.h"

struct sTree
{
  sint32 x,y,z;
  sint32 plantedTime;
  uint32 plantedBy;

  sTree(sint32 _x,sint32 _y, sint32 _z, sint32 _plantedTime, uint32 _plantedBy) :
    x(_x),y(_y),z(_z),
    plantedTime(_plantedTime),plantedBy(_plantedBy) {}

};

/*struct sChunk
{
  uint8 *blocks;
  uint8 *data;
  uint8 *blocklight;
  uint8 *skylight;
  uint8 *heightmap;
  sint32 x;
  sint32 z;
  NBT_Value *nbt;
};*/

struct spawnedItem
{
  int EID;
  sint16 item;
  char count;
  sint16 health;
  vec pos;
  time_t spawnedAt;
  uint32 spawnedBy;

  spawnedItem()
  {
    spawnedAt = time(0);
    spawnedBy = 0;
  }
};

class Map
{
private:

  Map()
  {
    for(int i = 0; i < 256; i++)
    {
      emitLight[i] = 0;
    }
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

    for(int i = 0; i < 256; i++)
    {
      stopLight[i] = 16;
    }
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

  ~Map()
  {
    // Free all memory
    for(std::map<uint32, sChunk>::const_iterator it = maps.begin(); it != maps.end(); it = maps.begin())
    {
      releaseMap(maps[it->first].x, maps[it->first].z);
    }

    //Free item memory
    for(std::map<uint32, spawnedItem *>::const_iterator it = items.begin(); it != items.end(); ++it)
    {
      delete items[it->first];
    }

    items.clear();
    std::string infile = mapDirectory+"/level.dat";

    NBT_Value* root = NBT_Value::LoadFromFile(infile);
    if(root != NULL)
    {
      NBT_Value &data = *((*root)["Data"]);

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


  }
  static Map* _instance;

public:

  std::string mapDirectory;

  // List of saplings ready to grow
  std::list<sTree> saplings;
  void addSapling(User* user, int x, int y, int z);
  void checkGenTrees();

  // Map spawn position
  vec spawnPos;

  // How blocks affect light
  int stopLight[256];

  // Blocks that emit light
  int emitLight[256];

  // Store all maps here
  std::map<uint32, sChunk> maps;

  // Store chunks here (remove maps)
  ChunkMap chunks;

  // Store the time map chunk has been last used
  std::map<uint32, int> mapLastused;

  // Store if map has been modified
  std::map<uint32, bool> mapChanged;

  // Do we need light regeneration
  std::map<uint32, bool> mapLightRegen;

  // Store item pointers for each chunk
  //std::map<uint32, std::vector<spawnedItem *> > mapItems;

  //All spawned items on map
  std::map<uint32, spawnedItem*> items;

//  void posToId(int x, int z, uint32 *id);
//  void idToPos(uint32 id, int *x, int *z);

  void init();
  void free();
  void sendToUser(User *user, int x, int z);

  //Time in the map
  sint64 mapTime;

  // Map seed
  sint64 mapSeed;

  // Get pointer to struct
  sChunk* getMapData(int x, int z, bool generate = true);

  // Load map chunk
  sChunk* loadMap(int x, int z, bool generate = true);

  // Save map chunk to disc
  bool saveMap(int x, int z);

  // Save whole map to disc (/save command)
  bool saveWholeMap();

  // Generate light maps for chunk
  bool generateLight(int x, int z);
  bool generateLight(int x, int z, sChunk* chunk);

  // Release/save map chunk
  bool releaseMap(int x, int z);

  // Light get/set
  bool getLight(int x, int y, int z, uint8* blocklight, uint8* skylight);
  bool getLight(int x, int y, int z, uint8* blocklight, uint8* skylight, sChunk* chunk);
  bool setLight(int x, int y, int z, int blocklight, int skylight, int setLight);
  bool setLight(int x, int y, int z, int blocklight, int skylight, int setLight, sChunk* chunk);
  bool spreadLight(int x, int y, int z, int skylight, int blocklight);
  bool spreadLight(int x, int y, int z, int skylight, int blocklight, sChunk* chunk);

  // Block value/meta get/set
  bool getBlock(int x, int y, int z, uint8* type, uint8* meta, bool generate = true);
  bool getBlock(int x, int y, int z, uint8* type, uint8* meta, bool generate, sChunk* chunk);
  bool getBlock(vec pos, uint8* type, uint8* meta)
  {
    return getBlock(pos.x(), pos.y(), pos.z(), type, meta);
  }
  bool setBlock(int x, int y, int z, char type, char meta);
  bool setBlock(vec pos, char type, char meta)
  {
    return setBlock(pos.x(), pos.y(), pos.z(), type, meta);
  }

  bool sendBlockChange(int x, int y, int z, char type, char meta);
  bool sendBlockChange(vec pos, char type, char meta)
  {
    return sendBlockChange(pos.x(), pos.y(), pos.z(), type, meta);
  }

  bool sendPickupSpawn(spawnedItem item);
  void createPickupSpawn(int x, int y, int z, int type, int count);

  void setComplexEntity(User* user, sint32 x, sint32 y, sint32 z, NBT_Value* entity);

  static Map* get()
  {
    if(!_instance) {
      _instance = new Map();
    }
    return _instance;
  }
};

#endif
