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

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <cmath>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>


// libnoise
#ifdef LIBNOISE
#include <libnoise/noise.h>
#else
#include <noise/noise.h>
#endif

#include "nethergen.h"
#include "cavegen.h"

#include "mineserver.h"
#include "config.h"
#include "constants.h"
#include "logger.h"
#include "map.h"
#include "nbt.h"
#include "tree.h"

#include "tools.h"
#include "random.h"

int neth_seed;

NetherGen::NetherGen()
  : netherblocks(16 * 16 * 128, 0),
    blockdata(16 * 16 * 128 / 2, 0),
    skylight(16 * 16 * 128 / 2, 0),
    blocklight(16 * 16 * 128 / 2, 0),
    heightmap(16 * 16, 0)
{
}

inline int fastrand()
{
  neth_seed = (214013 * neth_seed + 2531011);
  return (neth_seed >> 16) & 0x7FFF;
}

void NetherGen::init(int seed)
{
  neth_seed = seed;

  Randomgen.SetSeed(seed);
  Randomgen.SetFrequency(0.1);
  Randomciel.SetSeed(seed);
  Randomciel.SetOctaveCount(6);
  Randomciel.SetFrequency(1.0 / 180.0);
  Randomciel.SetLacunarity(2.0);

  seaLevel = ServerInstance->config()->iData("mapgen.sea.level");
  addTrees = false;//ServerInstance->config()->bData("mapgen.trees.enabled");
  expandBeaches = false;//ServerInstance->config()->bData("mapgen.beaches.expand");
  beachExtent = false;//ServerInstance->config()->iData("mapgen.beaches.extent");
  beachHeight = false;//ServerInstance->config()->iData("mapgen.beaches.height");

  addOre = true;//ServerInstance->config()->bData("mapgen.caves.ore");
}

void NetherGen::re_init(int seed)
{
  Randomgen.SetSeed(seed);
}

void NetherGen::generateChunk(int x, int z, int map)
{
  NBT_Value* main = new NBT_Value(NBT_Value::TAG_COMPOUND);
  NBT_Value* val = new NBT_Value(NBT_Value::TAG_COMPOUND);

  generateWithNoise(x, z, map);

  val->Insert("Blocks", new NBT_Value(netherblocks));
  val->Insert("Data", new NBT_Value(blockdata));
  val->Insert("SkyLight", new NBT_Value(skylight));
  val->Insert("BlockLight", new NBT_Value(blocklight));
  val->Insert("HeightMap", new NBT_Value(heightmap));
  val->Insert("Entities", new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND));
  val->Insert("TileEntities", new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND));
  val->Insert("LastUpdate", new NBT_Value((int64_t)time(NULL)));
  val->Insert("xPos", new NBT_Value(x));
  val->Insert("zPos", new NBT_Value(z));
  val->Insert("TerrainPopulated", new NBT_Value((char)1));

  main->Insert("Level", val);

  /*  uint32_t chunkid;
  ServerInstance->map()->posToId(x, z, &chunkid);

  ServerInstance->map()->maps[chunkid].x = x;
  ServerInstance->map()->maps[chunkid].z = z; */

  std::vector<uint8_t> *t_blocks = (*val)["Blocks"]->GetByteArray();
  std::vector<uint8_t> *t_data = (*val)["Data"]->GetByteArray();
  std::vector<uint8_t> *t_blocklight = (*val)["BlockLight"]->GetByteArray();
  std::vector<uint8_t> *t_skylight = (*val)["SkyLight"]->GetByteArray();
  std::vector<int32_t> *heightmap = (*val)["HeightMap"]->GetIntArray();

  sChunk* chunk = new sChunk();
  chunk->blocks = &((*t_blocks)[0]);
  chunk->data = &((*t_data)[0]);
  chunk->blocklight = &((*t_blocklight)[0]);
  chunk->skylight = &((*t_skylight)[0]);
  chunk->heightmap = &((*heightmap)[0]);
  chunk->nbt = main;
  chunk->x = x;
  chunk->z = z;

  ServerInstance->map(map)->chunks.insert(ChunkMap::value_type(ChunkMap::key_type(x, z), chunk));

  // Update last used time
  //ServerInstance->map()->mapLastused[chunkid] = (int)time(0);

  // Not changed
  //ServerInstance->map()->mapChanged[chunkid] = ServerInstance->config()->bData("save_unchanged_chunks");

  //ServerInstance->map()->maps[chunkid].nbt = main;

  if (addOre)
  {
    AddOre(x, z, map, BLOCK_GLOWSTONE);
    AddOre(x, z, map, BLOCK_STATIONARY_LAVA);
  }

  // Add trees
  if (addTrees)
  {
    AddTrees(x, z, map,  fastrand() % 2 + 3);
  }

  if (expandBeaches)
  {
    ExpandBeaches(x, z, map);
  }

}

//#define PRINT_MAPGEN_TIME


void NetherGen::AddTrees(int x, int z, int map, uint16_t count)
{
  int xBlockpos = x << 4;
  int zBlockpos = z << 4;

  int blockX, blockY, blockZ;
  uint8_t block;
  uint8_t meta;

  for (uint16_t i = 0; i < count; i++)
  {
    blockX = fastrand() % 16;
    blockZ = fastrand() % 16;

    blockY = heightmap[(blockZ << 4) + blockX] + 1;

    blockX += xBlockpos;
    blockZ += zBlockpos;

    ServerInstance->map(map)->getBlock(blockX, blockY, blockZ, &block, &meta);
    // No trees on water
    if (block == BLOCK_WATER || block == BLOCK_STATIONARY_WATER)
    {
      continue;
    }

    Tree tree(blockX, blockY, blockZ, map);
  }
}

void NetherGen::generateWithNoise(int x, int z, int map)
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

  // Populate blocks in chunk
  int32_t currentHeight;
  int32_t ymax;
  uint16_t ciel;
  uint8_t* curBlock;
  netherblocks.assign(16 * 16 * 128, 0);

  double xBlockpos = x << 4;
  double zBlockpos = z << 4;
  for (int bX = 0; bX < 16; bX++)
  {
    for (int bZ = 0; bZ < 16; bZ++)
    {
      double ciel2 = (Randomciel.GetValue(xBlockpos + bX, 0 , zBlockpos + bZ) * 1.5);
      ciel = 128 - (uint16_t)(abs(ciel2 * ciel2 * ciel2 * ciel2 * ciel2 * ciel2)); // Cubed! Get some good stalagtites!
      heightmap[(bZ << 4) + bX] = ymax = currentHeight = (uint8_t)((Randomgen.GetValue(xBlockpos + bX, 0, zBlockpos + bZ) * 15) + 64);

      int32_t stoneHeight = (int32_t)(currentHeight * 0.94);
      int32_t bYbX = ((bZ << 7) + (bX << 11));

      if (ymax < seaLevel)
      {
        ymax = seaLevel;
      }

      for (int bY = 0; bY < 128; bY++)
      {
        curBlock = &netherblocks[bYbX++];
        if (bY >= 126)
        {
          *curBlock = BLOCK_BEDROCK;
          continue;
        }
        if (bY == 0)
        {
          *curBlock = BLOCK_BEDROCK;
          continue;
        }
        if (bY > ciel)
        {
          *curBlock = BLOCK_NETHERSTONE;
          continue;
        }


        if (bY < currentHeight)
        {
          if (bY < stoneHeight)
          {
            *curBlock = BLOCK_NETHERSTONE;
            // Add caves
            //            cave.AddCaves(*curBlock, xBlockpos + bX, bY, zBlockpos + bZ, map);
          }
          else
          {
            *curBlock = BLOCK_NETHERSTONE;
          }
        }
        else if (currentHeight == bY)
        {
          if (bY == seaLevel || bY == seaLevel - 1 || bY == seaLevel - 2)
          {
            *curBlock = BLOCK_SLOW_SAND;  // FF
          }
          else if (bY < seaLevel - 1)
          {
            *curBlock = BLOCK_GRAVEL;  // FF
          }
          else
          {
            *curBlock = BLOCK_NETHERSTONE;  // FF
          }
        }
        else
        {
          if (bY <= seaLevel)
          {
            *curBlock = BLOCK_STATIONARY_LAVA;  // FF
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
  ServerInstance->logger()->log("Mapgen: " + dtos(t_end - t_begin) + "ms");
#else
  gettimeofday(&end, NULL);
  ServerInstance->logger()->log("Mapgen: " + dtos(end.tv_usec - start.tv_usec));
#endif
#endif
}

void NetherGen::ExpandBeaches(int x, int z, int map)
{
  int beachExtentSqr = (beachExtent + 1) * (beachExtent + 1);
  int xBlockpos = x << 4;
  int zBlockpos = z << 4;

  int blockX, blockZ, h;
  uint8_t block;
  uint8_t meta;

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

            ServerInstance->map(map)->getBlock(xBlockpos + xx, hh, zBlockpos + zz, &block, &meta);
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
        ServerInstance->map(map)->sendBlockChange(blockX, h, blockZ, BLOCK_SAND, 0);
        ServerInstance->map(map)->setBlock(blockX, h, blockZ, BLOCK_SAND, 0);

        ServerInstance->map(map)->getBlock(blockX, h - 1, blockZ, &block, &meta);

        if (h > 0 && block == BLOCK_DIRT)
        {
          ServerInstance->map(map)->sendBlockChange(blockX, h - 1, blockZ, BLOCK_SAND, 0);
          ServerInstance->map(map)->setBlock(blockX, h - 1, blockZ, BLOCK_SAND, 0);
        }
      }
    }
  }
}

void NetherGen::AddOre(int x, int z, int map, uint8_t type)
{
  int xBlockpos = x << 4;
  int zBlockpos = z << 4;

  int blockX, blockY, blockZ;
  uint8_t block;
  uint8_t meta;

  int count, startHeight;

  switch (type)
  {
  case BLOCK_GLOWSTONE:
    count = fastrand() % 4 + 15;
    startHeight = 128;
    break;
  case BLOCK_STATIONARY_LAVA:
    count = fastrand() % 20 + 20;
    startHeight = 128;
    break;
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

    blockX += xBlockpos;
    blockZ += zBlockpos;

    // Calculate Y
    blockY = fastrand() % blockY;

    i++;

    ServerInstance->map(map)->getBlock(blockX, blockY, blockZ, &block, &meta);
    // No ore in caves
    if (block != BLOCK_NETHERSTONE)
    {
      continue;
    }

    AddDeposit(blockX, blockY, blockZ, map, type, 2);

  }
}

void NetherGen::AddDeposit(int x, int y, int z, int map, uint8_t block, int depotSize)
{
  for (int bX = x; bX < x + depotSize; bX++)
  {
    for (int bY = y; bY < y + depotSize; bY++)
    {
      for (int bZ = z; bZ < z + depotSize; bZ++)
      {
        if (uniform01() < 0.5)
        {
          ServerInstance->map(map)->sendBlockChange(bX, bY, bZ, block, 0);
          ServerInstance->map(map)->setBlock(bX, bY, bZ, block, 0);
        }
      }
    }
  }
}
