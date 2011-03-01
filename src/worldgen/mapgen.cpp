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

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <cmath>
#include <ctime>

// libnoise
#ifdef LIBNOISE
#include <libnoise/noise.h>
#else
#include <noise/noise.h>
#endif

#include "cavegen.h"
#include "mapgen.h"

#include "../mineserver.h"
#include "../config.h"
#include "../constants.h"
#include "../logger.h"
#include "../map.h"
#include "../nbt.h"
#include "../tree.h"
#include "../tools.h"

int g_seed;
int f_seed;

inline int fastrand()
{
  f_seed = (214013 * f_seed + 2531011);
  return (f_seed >> 16) & 0x7FFF;
}

MapGen::MapGen()
  : blocks(16 * 16 * 128, 0),
    blockdata(16 * 16 * 128 / 2, 0),
    skylight(16 * 16 * 128 / 2, 0),
    blocklight(16 * 16 * 128 / 2, 0),
    heightmap(16 * 16, 0)
{
}

void MapGen::init(int seed)
{
  cave.init(seed + 7);
  f_seed = seed; // used for fastrand and can change
  g_seed = seed; // used for height map, cannot change

  ridgedMultiNoise.SetSeed(seed);
  ridgedMultiNoise.SetOctaveCount(6);
  ridgedMultiNoise.SetFrequency(1.0 / 180.0);
  ridgedMultiNoise.SetLacunarity(2.0);

  //###### TREE GEN #####
  treenoise.SetSeed(seed + 2);
  treenoise.SetOctaveCount(6);
  treenoise.SetFrequency(1.0 / 180.0);
  treenoise.SetLacunarity(2.0);

  //###### END TREE GEN #######
  seaLevel = Mineserver::get()->config()->iData("mapgen.sea.level");
  addTrees = Mineserver::get()->config()->bData("mapgen.trees.enabled");
  expandBeaches = Mineserver::get()->config()->bData("mapgen.beaches.expand");
  beachExtent = Mineserver::get()->config()->iData("mapgen.beaches.extent");
  beachHeight = Mineserver::get()->config()->iData("mapgen.beaches.height");

  addOre = Mineserver::get()->config()->bData("mapgen.addore");
  addCaves = Mineserver::get()->config()->bData("mapgen.caves.enabled");

  winterEnabled = Mineserver::get()->config()->bData("mapgen.winter.enabled");

}

void MapGen::re_init(int seed)
{
  cave.init(seed + 7);
  ridgedMultiNoise.SetSeed(seed);
  treenoise.SetSeed(seed + 2);
}


void MapGen::generateFlatgrass(int x, int z, int map)
{
  sChunk* chunk = Mineserver::get()->map(map)->chunks.getChunk(x, z);
  Block top = BLOCK_GRASS;
  if (winterEnabled)
  {
    top = BLOCK_SNOW;
  }

  for (int bX = 0; bX < 16; bX++)
  {
    for (int bZ = 0; bZ < 16; bZ++)
    {
      heightmap[(bZ << 4) + bX] = 64;
      for (int bY = 0; bY < 128; bY++)
      {
        if (bY == 0)
        {
          chunk->blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_BEDROCK;
        }
        else if (bY < 64)
        {
          chunk->blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_DIRT;
        }
        else if (bY == 64)
        {
          chunk->blocks[bY + (bZ * 128 + (bX * 128 * 16))] = top;
        }
        else
        {
          chunk->blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_AIR;
        }
      }
    }
  }
}

void MapGen::generateChunk(int x, int z, int map)
{
  NBT_Value* main = new NBT_Value(NBT_Value::TAG_COMPOUND);
  NBT_Value* val = new NBT_Value(NBT_Value::TAG_COMPOUND);

  val->Insert("Blocks", new NBT_Value(blocks));
  val->Insert("Data", new NBT_Value(blockdata));
  val->Insert("SkyLight", new NBT_Value(skylight));
  val->Insert("BlockLight", new NBT_Value(blocklight));
  val->Insert("HeightMap", new NBT_Value(heightmap));
  val->Insert("Entities", new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND));
  val->Insert("TileEntities", new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND));
  val->Insert("LastUpdate", new NBT_Value((int64_t)time(NULL)));
  val->Insert("xPos", new NBT_Value(x));
  val->Insert("zPos", new NBT_Value(z));
  val->Insert("TerrainPopulated", new NBT_Value((int8_t)1));

  main->Insert("Level", val);

  /*  uint32_t chunkid;
  Mineserver::get()->map()->posToId(x, z, &chunkid);

  Mineserver::get()->map()->maps[chunkid].x = x;
  Mineserver::get()->map()->maps[chunkid].z = z; */

  std::vector<uint8_t> *t_blocks = (*val)["Blocks"]->GetByteArray();
  std::vector<uint8_t> *t_data = (*val)["Data"]->GetByteArray();
  std::vector<uint8_t> *t_blocklight = (*val)["BlockLight"]->GetByteArray();
  std::vector<uint8_t> *t_skylight = (*val)["SkyLight"]->GetByteArray();
  std::vector<uint8_t> *heightmap = (*val)["HeightMap"]->GetByteArray();

  sChunk* chunk = new sChunk();
  chunk->blocks = &((*t_blocks)[0]);
  chunk->data = &((*t_data)[0]);
  chunk->blocklight = &((*t_blocklight)[0]);
  chunk->skylight = &((*t_skylight)[0]);
  chunk->heightmap = &((*heightmap)[0]);
  chunk->nbt = main;
  chunk->x = x;
  chunk->z = z;
  Mineserver::get()->map(map)->chunks.linkChunk(chunk, x, z);
  if (Mineserver::get()->config()->bData("mapgen.flatgrass"))
  {
    generateFlatgrass(x, z, map);
  }
  else
  {
    generateWithNoise(x, z, map);
  }


  // Update last used time
  //Mineserver::get()->map()->mapLastused[chunkid] = (int)time(0);

  // Not changed
  chunk->changed = Mineserver::get()->config()->bData("map.save_unchanged_chunks");

  //Mineserver::get()->map()->maps[chunkid].nbt = main;

  if (addOre)
  {
    AddOre(x, z, map, BLOCK_COAL_ORE);
    AddOre(x, z, map, BLOCK_IRON_ORE);
    AddOre(x, z, map, BLOCK_GOLD_ORE);
    AddOre(x, z, map, BLOCK_DIAMOND_ORE);
    AddOre(x, z, map, BLOCK_REDSTONE_ORE);
    AddOre(x, z, map, BLOCK_LAPIS_ORE);
  }

  AddOre(x, z, map, BLOCK_GRAVEL);

  // Add trees
  if (addTrees)
  {
    AddTrees(x, z, map);  // add trees will make a *kind-of* forest of 16*16 chunks
  }

  if (expandBeaches)
  {
    ExpandBeaches(x, z, map);
  }

}

//#define PRINT_MAPGEN_TIME


void MapGen::AddTrees(int x, int z, int map)
{
  int xBlockpos = x << 4;
  int zBlockpos = z << 4;

  int blockX, blockZ;
  uint8_t blockY;

  uint8_t block;
  uint8_t meta;

  uint8_t un = fastrand() % 4 + 2;
  uint8_t vn = fastrand() % 4 + 2;

  float uFactor = (16 / (float)un);   //relational to literal
  float vFactor = (16 / (float)vn);
  for (uint8_t u = 0; u < un; u++)
  {
    //u for x and v for z iteration
    for (uint8_t v = 0; v < vn; v++)
    {
      uint8_t a = u * uFactor;
      uint8_t b = v * vFactor;

      blockX = a + xBlockpos;
      blockZ = b + zBlockpos;
      blockY = heightmap[(a << 4) + b] + 1;

      Mineserver::get()->map(map)->getBlock(blockX, blockY, blockZ, &block, &meta);

      // No trees on water
      if (block != BLOCK_WATER && block != BLOCK_STATIONARY_WATER && block != BLOCK_SAND)
      {
        if (abs(treenoise.GetValue(blockX, 0, blockZ)) >= 0.9)
        {
          Tree tree(blockX, blockY, blockZ, map);
        }
      }
    }
  }
}

void MapGen::generateWithNoise(int x, int z, int map)
{
  // Debug..
#ifdef PRINT_MAPGEN_TIME
#ifdef WIN32
  DWORD t_begin, t_end;
  t_begin = timeGetTime();
#else
  struct timeval start, end;
  gettimeofday(&start, NULL);
#endif
#endif
  sChunk* chunk = Mineserver::get()->map(map)->chunks.getChunk(x, z);

  // Winterland
  Block topBlock = BLOCK_GRASS;
  if (winterEnabled)
  {
    topBlock = BLOCK_SNOW;
  }

  // Populate blocks in chunk
  int32_t currentHeight;
  int32_t ymax;
  uint8_t* curBlock;

  double xBlockpos = x << 4;
  double zBlockpos = z << 4;
  for (int bX = 0; bX < 16; bX++)
  {
    for (int bZ = 0; bZ < 16; bZ++)
    {
      heightmap[(bZ << 4) + bX] = ymax = currentHeight = (uint8_t)((ridgedMultiNoise.GetValue(xBlockpos + bX, 0, zBlockpos + bZ) * 15) + 64);

      int32_t stoneHeight = (int32_t)(currentHeight * 0.94);
      int32_t bYbX = ((bZ << 7) + (bX << 11));

      if (ymax < seaLevel)
      {
        ymax = seaLevel;
      }

      for (int bY = 0; bY <= ymax; bY++)
      {
        curBlock = &(chunk->blocks[bYbX++]);

        // Place bedrock
        if (bY == 0)
        {
          *curBlock = BLOCK_BEDROCK;
          continue;
        }

        if (bY < currentHeight)
        {
          if (bY < stoneHeight)
          {
            *curBlock = BLOCK_STONE;
            // Add caves
            if (addCaves)
            {
              cave.AddCaves(*curBlock, xBlockpos + bX, bY, zBlockpos + bZ);
            }
          }
          else
          {
            *curBlock = BLOCK_DIRT;
          }
        }
        else if (currentHeight == bY)
        {
          if (bY == seaLevel || bY == seaLevel - 1 || bY == seaLevel - 2)
          {
            *curBlock = BLOCK_SAND;  // FF
          }
          else if (bY < seaLevel - 1)
          {
            *curBlock = BLOCK_GRAVEL;  // FF
          }
          else
          {
            *curBlock = topBlock;  // FF
          }
        }
        else
        {
          if (bY <= seaLevel)
          {
            *curBlock = BLOCK_WATER;  // FF
          }
          else
          {
            *curBlock = BLOCK_AIR;  // FF
          }
        }
      }
    }
  }

#ifdef PRINT_MAPGEN_TIME
#ifdef WIN32
  t_end = timeGetTime();
  Mineserver::get()->logger()->log("Mapgen: " + dtos(t_end - t_begin) + "ms");
#else
  gettimeofday(&end, NULL);
  Mineserver::get()->logger()->log("Mapgen: " + dtos(end.tv_usec - start.tv_usec));
#endif
#endif
}

void MapGen::ExpandBeaches(int x, int z, int map)
{
  sChunk* chunk = Mineserver::get()->map(map)->chunks.getChunk(blockToChunk(x), blockToChunk(z));
  int beachExtentSqr = (beachExtent + 1) * (beachExtent + 1);
  int xBlockpos = x << 4;
  int zBlockpos = z << 4;

  int blockX, blockZ, h;
  uint8_t block = 0;
  uint8_t meta = 0;

  for (int bX = 0; bX < 16; bX++)
  {
    for (int bZ = 0; bZ < 16; bZ++)
    {
      blockX = xBlockpos + bX;
      blockZ = zBlockpos + bZ;

      h = heightmap[(bZ << 4) + bX];

      if (h < 0)
      {
        continue;
      }

      bool found = false;
      for (int dx = -beachExtent; !found && dx <= beachExtent; dx++)
      {
        for (int dz = -beachExtent; !found && dz <= beachExtent; dz++)
        {
          for (int dh = -beachHeight; !found && dh <= 0; dh++)
          {
            if (dx * dx + dz * dz + dh * dh > beachExtentSqr)
            {
              continue;
            }

            int xx = bX + dx;
            int zz = bZ + dz;
            int hh = h + dh;
            if (xx < 0 || xx >= 15 || zz < 0 || zz >= 15 || hh < 0 || hh >= 127)
            {
              continue;
            }

            //ToDo: add getBlock!!
            if (block == BLOCK_WATER || block == BLOCK_STATIONARY_WATER)
            {
              found = true;
              break;
            }
          }
        }
      }
      if (found)
      {
        Mineserver::get()->map(map)->sendBlockChange(blockX, h, blockZ, BLOCK_SAND, 0);
        Mineserver::get()->map(map)->setBlock(blockX, h, blockZ, BLOCK_SAND, 0);

        Mineserver::get()->map(map)->getBlock(blockX, h - 1, blockZ, &block, &meta);

        if (h > 0 && block == BLOCK_DIRT)
        {
          Mineserver::get()->map(map)->sendBlockChange(blockX, h - 1, blockZ, BLOCK_SAND, 0);
          Mineserver::get()->map(map)->setBlock(blockX, h - 1, blockZ, BLOCK_SAND, 0);
        }
      }
    }
  }
}

void MapGen::AddOre(int x, int z, int map, uint8_t type)
{
  sChunk* chunk = Mineserver::get()->map(map)->chunks.getChunk(x, z);

  int blockX, blockY, blockZ;
  uint8_t block;

  // Parameters for deposits
  int count, startHeight, minDepoSize, maxDepoSize;

  switch (type)
  {
  case BLOCK_COAL_ORE:
    count = fastrand() % 10 + 20; // 20-30 coal deposits
    startHeight = 90;
    minDepoSize = 3;
    maxDepoSize = 7;
    break;
  case BLOCK_IRON_ORE:
    count = fastrand() % 8 + 10; // 10-18 iron deposits
    startHeight = 60;
    minDepoSize = 2;
    maxDepoSize = 5;
    break;
  case BLOCK_GOLD_ORE:
    count = fastrand() % 4 + 5; // 4-9 gold deposits
    startHeight = 32;
    minDepoSize = 2;
    maxDepoSize = 4;
    break;
  case BLOCK_DIAMOND_ORE:
    count = fastrand() % 1 + 2; // 1-3 diamond deposits
    startHeight = 17;
    minDepoSize = 1;
    maxDepoSize = 2;
    break;
  case BLOCK_REDSTONE_ORE:
    count = fastrand() % 5 + 5; // 5-10 redstone deposits
    startHeight = 25;
    minDepoSize = 2;
    maxDepoSize = 4;
    break;
  case BLOCK_LAPIS_ORE:
    count = fastrand() % 1 + 2; // 1-3 lapis lazuli deposits
    startHeight = 17;
    minDepoSize = 1;
    maxDepoSize = 2;
    break;
  case BLOCK_GRAVEL:
    count = fastrand() % 10 + 20; // 20-30 gravel deposits
    startHeight = 90;
    minDepoSize = 4;
    maxDepoSize = 10;
    break;
  default:
    return;
  }

  int i = 0;
  while (i < count)
  {
    blockX = fastrand() % 8 + 4;
    blockZ = fastrand() % 8 + 4;

    blockY = heightmap[(blockZ << 4) + blockX];
    blockY -= 5;

    // Check that startheight is not higher than height at that column
    if (blockY > startHeight)
    {
      blockY = startHeight;
    }

    //blockX += xBlockpos;
    //blockZ += zBlockpos;

    // Calculate Y
    blockY = fastrand() % blockY;

    i++;

    block = chunk->blocks[blockY + ((blockZ << 7) + (blockX << 11))];
    // No ore in caves
    if (block == BLOCK_AIR)
    {
      continue;
    }

    AddDeposit(blockX, blockY, blockZ, map, type, minDepoSize, maxDepoSize, chunk);

  }
}

void MapGen::AddDeposit(int x, int y, int z, int map, uint8_t block, int minDepoSize, int maxDepoSize, sChunk* chunk)
{
  int depoSize = fastrand() % (maxDepoSize - minDepoSize) + minDepoSize;
  for (int i = 0; i < depoSize; i++)
  {
    if (chunk->blocks[y + ((z << 7) + (x << 11))] != BLOCK_GRASS ||
        chunk->blocks[y + ((z << 7) + (x << 11))] != BLOCK_SNOW)
    {
      chunk->blocks[y + ((z << 7) + (x << 11))] = block;
    }

    z = z + ((fastrand() % 2) - 1);
    x = x + ((fastrand() % 2) - 1);
    y = y + ((fastrand() % 2) - 1);

    // If over chunk borders
    if (z < 0 || z > 15 || x < 0 || x > 15 || y < 1)
    {
      break;
    }
  }
}
