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


#ifndef _BLOCKS_PLANT_H
#define _BLOCKS_PLANT_H

#include "basic.h"

#include "tr1.h"
#include TR1INCLUDE(memory)


// 10000 == 100%
enum { SEEDS_CHANCE = 1000 };

class User;

class PlantBlock
{
public:
  PlantBlock(int x, int y, int z, int map, int count) : x(x), y(y), z(z), map(map), count(count) { }
  int x, y, z, map, count;
};

typedef std::tr1::shared_ptr<PlantBlock> PlantBlockPtr;

struct PlantBlockFinder
{
  PlantBlockFinder(int x, int y, int z, int map) : x(x), y(y), z(z), map(map) { }
  inline bool operator()(const PlantBlockPtr & t) const { return t->x == x && t->y == y && t->z == z && t->map == map; }
  int x, y, z, map;
};

/** BlockPlant deals specifically with plant block functionality
@see BlockBasic
*/
class BlockPlant: public BlockBasic
{
public:
  int grass_timeout;
  int crop_timeout;
  int cactus_timeout;
  int reed_timeout;
  int cactus_max;
  int reed_max;

  inline bool affectedBlock(int block) const
  {
    return block == BLOCK_YELLOW_FLOWER || block == BLOCK_RED_ROSE || block == BLOCK_BROWN_MUSHROOM || block == BLOCK_RED_MUSHROOM || block == BLOCK_CROPS || block == BLOCK_CACTUS || block == BLOCK_REED || block == BLOCK_SAPLING || block == BLOCK_DIRT || block == BLOCK_GRASS || block == BLOCK_SOIL || block == ITEM_REED || block == ITEM_WOODEN_HOE || block == ITEM_STONE_HOE || block == ITEM_IRON_HOE || block == ITEM_DIAMOND_HOE || block == ITEM_GOLD_HOE || block == ITEM_SEEDS;
  }

  BlockPlant();
  void onStartedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction);
  void onDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction);
  void onStoppedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction);
  bool onBroken(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction);
  void onNeighbourBroken(User* user, int16_t oldblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction);
  bool onPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction);
  void onNeighbourPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction);
  void onReplace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction);
  void timer200();
  void addBlocks(int x, int y, int z, int map);
  void addBlock(PlantBlockPtr p2);
  void addBlock(int x, int y, int z, int map);
  void remBlock(PlantBlockPtr p2);
  void remBlock(int x, int y, int z, int map);
  bool isPlant(int num);
};


#endif
