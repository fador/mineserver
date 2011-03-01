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

#ifndef _MAP_H_
#define _MAP_H_

#include <map>
#include <list>
#include <ctime>

#include "vec.h"
#include "chunkmap.h"

class User;

struct sTree
{
  int32_t x, y, z;
  int64_t plantedTime;
  uint32_t plantedBy;

  sTree(int32_t _x, int32_t _y, int32_t _z, int64_t _plantedTime, uint32_t _plantedBy) :
    x(_x), y(_y), z(_z),
    plantedTime(_plantedTime), plantedBy(_plantedBy) {}
};

class Map
{
public:
  Map();
  Map(const Map& oldmap);
  ~Map();

  std::string mapDirectory;

  // List of saplings ready to grow
  std::list<sTree> saplings;
  void addSapling(User* user, int x, int y, int z);
  void checkGenTrees();

  // Map spawn position
  vec spawnPos;

  // Map number
  int m_number;

  // How blocks affect light
  int stopLight[256];

  // Blocks that emit light
  int emitLight[256];

  // Store all maps here
  std::map<uint32_t, sChunk> maps;

  // Store chunks here (remove maps)
  ChunkMap chunks;

  // Store the time map chunk has been last used
  std::map<uint32_t, int> mapLastused;

  // Store if map has been modified
  std::map<uint32_t, bool> mapChanged;

  // Do we need light regeneration
  std::map<uint32_t, bool> mapLightRegen;

  // Store item pointers for each chunk
  //std::map<uint32, std::vector<spawnedItem *> > mapItems;

  //All spawned items on map
  std::map<uint32_t, spawnedItem*> items;

  //  void posToId(int x, int z, uint32_t *id);
  //  void idToPos(uint32_t id, int *x, int *z);

  void init(int number);
  void sendToUser(User* user, int x, int z, bool login = false);

  //Time in the map
  int64_t mapTime;

  // Map seed
  int64_t mapSeed;

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
  bool getLight(int x, int y, int z, uint8_t* blocklight, uint8_t* skylight);
  bool getLight(int x, int y, int z, uint8_t* blocklight, uint8_t* skylight, sChunk* chunk);
  bool setLight(int x, int y, int z, int blocklight, int skylight, int setLight);
  bool setLight(int x, int y, int z, int blocklight, int skylight, int setLight, sChunk* chunk);
  bool spreadLight(int x, int y, int z, int skylight, int blocklight);
  bool spreadLight(int x, int y, int z, int skylight, int blocklight, sChunk* chunk);

  // Block value/meta get/set
  bool getBlock(int x, int y, int z, uint8_t* type, uint8_t* meta, bool generate = true);
  bool getBlock(int x, int y, int z, uint8_t* type, uint8_t* meta, bool generate, sChunk* chunk);
  bool getBlock(vec pos, uint8_t* type, uint8_t* meta)
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
  bool sendNote(int x, int y, int z, char instrument, char pitch);
  bool sendNote(vec pos, char instrument, char pitch)
  {
    return sendNote(pos.x(), pos.y(), pos.z(), instrument, pitch);
  }

  bool sendPickupSpawn(spawnedItem item);
  void createPickupSpawn(int x, int y, int z, int type, int count, int health, User* user);

  bool sendProjectileSpawn(User* user, int8_t projID);
};

#endif
