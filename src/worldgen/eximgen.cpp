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

#include "eximgen.h"

#include "mineserver.h"
#include "config.h"
#include "map.h"
#include "tree.h"
#include "tools.h"
#include "random.h"

EximGen::EximGen()
  : blocks(16 * 16 * 128, 0),
    blockdata(16 * 16 * 128 / 2, 0),
    skylight(16 * 16 * 128 / 2, 0),
    blocklight(16 * 16 * 128 / 2, 0),
    heightmap(16 * 16, 0)
{

}

void EximGen::init(int seed)
{
  cave.init(seed + 7);

  mountainTerrain.SetSeed(seed);
  mountainTerrain.SetFrequency(0.005);
  mountainTerrain.SetOctaveCount(5);

  mountainScale.SetSourceModule(0, mountainTerrain);
  mountainScale.SetScale(-1.0);
  mountainScale.SetBias(- (1.0 / 128) * 13);

  baseFlatTerrain.SetSeed(seed);
  baseFlatTerrain.SetFrequency(0.005);
  baseFlatTerrain.SetOctaveCount(5);
  baseFlatTerrain.SetPersistence(0.5);

  flatTerrain.SetSourceModule(0, baseFlatTerrain);
  flatTerrain.SetScale(0.125);
  flatTerrain.SetBias(0.05);

  terrainType.SetSeed(seed);
  terrainType.SetFrequency(0.005);
  terrainType.SetOctaveCount(4);
  terrainType.SetPersistence(0.5);

  terrainSelector.SetSourceModule(0, flatTerrain);
  terrainSelector.SetSourceModule(1, mountainScale);
  terrainSelector.SetControlModule(terrainType);
  terrainSelector.SetBounds(0.5, 1000.0);
  terrainSelector.SetEdgeFalloff(0.125);

  finalTerrain.SetSourceModule(0, terrainSelector);
  finalTerrain.SetScale(62);
  finalTerrain.SetBias(62);

  treenoise.SetSeed(seed + 404);
  treenoise.SetFrequency(0.01);
  treenoise.SetOctaveCount(3);

  seaLevel = ServerInstance->config()->iData("mapgen.sea.level");
  addTrees = ServerInstance->config()->bData("mapgen.trees.enabled");
  expandBeaches = ServerInstance->config()->bData("mapgen.beaches.expand");
  beachExtent = ServerInstance->config()->iData("mapgen.beaches.extent");
  beachHeight = ServerInstance->config()->iData("mapgen.beaches.height");

  addOre = ServerInstance->config()->bData("mapgen.addore");
  addCaves = ServerInstance->config()->bData("mapgen.caves.enabled");

  winterEnabled = ServerInstance->config()->bData("mapgen.winter.enabled");

}

void EximGen::re_init(int seed)
{
  cave.init(seed + 7);

  mountainTerrain.SetSeed(seed);
  baseFlatTerrain.SetSeed(seed);
  terrainType.SetSeed(seed);

  treenoise.SetSeed(seed + 404);
}


void EximGen::generateFlatgrass(int x, int z, int map)
{
  sChunk* chunk = ServerInstance->map(map)->getChunk(x, z);
  Block top = BLOCK_GRASS;
  if (winterEnabled)
  {
    top = BLOCK_SNOW;
  }

  for (int bX = 0; bX < 16; bX++)
  {
    for (int bZ = 0; bZ < 16; bZ++)
    {
      heightmap[(bZ<<4)+bX] = 64;
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

void EximGen::generateChunk(int x, int z, int map)
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

  if (ServerInstance->config()->bData("mapgen.flatgrass"))
  {
    generateFlatgrass(x, z, map);
  }
  else
  {
    generateWithNoise(x, z, map);
  }


  // Not changed
  chunk->changed = ServerInstance->config()->bData("map.save_unchanged_chunks");

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
  AddOre(x, z, map, BLOCK_DIRT); // guess what, dirt also exists underground

  // Add trees
  if (addTrees)
  {
    AddTrees(x, z, map);
  }

  if (expandBeaches)
  {
    ExpandBeaches(x, z, map);
  }

  // AddRiver(x, z, map);

}
#include <iostream>
using namespace std;
void EximGen::AddTrees(int x, int z, int map)
{
  int32_t xBlockpos = x << 4;
  int32_t zBlockpos = z << 4;
  int blockX, blockZ;
  uint8_t blockY, block, meta;

  bool empty[16][16]; // is block emptey~

  memset(empty, 1, 256);

  uint8_t trees = uniform01() * 7 + 13;
  uint8_t i = 0;
  while (i < trees)
  {
    uint8_t a = uniform01() * 16;
    uint8_t b = uniform01() * 16;

    if (empty[a][b])
    {
      blockX = a + xBlockpos;
      blockZ = b + zBlockpos;
      blockY = heightmap[(b<<4)+a] ;

      ServerInstance->map(map)->getBlock(blockX, blockY, blockZ, &block, &meta);
      if (block == BLOCK_DIRT || block == BLOCK_GRASS)
      {
        // Trees only grow on dirt and grass? =b
        ServerInstance->map(map)->getBlock(blockX, ++blockY, blockZ, &block, &meta);
        if (block == BLOCK_AIR || block == BLOCK_SNOW)
        {
          if (treenoise.GetValue(blockX, 0, blockZ) > -0.4)
          {
            Tree tree(blockX, blockY, blockZ, map);
          }
        }
      }
      for (int8_t u = -2; u < 2; u++)
      {
        for (int8_t v = -2; v < 2; v++)
        {
          //Check for array boundaries
          if((a+u) >= 0 && (b+v) >= 0 &&
             (a+u) < 16 && (b+v) < 16)
          {
            empty[a+u][b+v] = false;
          }
        }
      }
      i++;
    }
  }
}

void EximGen::generateWithNoise(int x, int z, int map)
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
  sChunk* chunk = ServerInstance->map(map)->getChunk(x, z);

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

  int32_t xBlockpos = x << 4;
  int32_t zBlockpos = z << 4;
  for (uint8_t bX = 0; bX < 16; bX++) //,xBlockpos++)   // ### optimization that somehow fucks up noise values =b
  {
    for (uint8_t bZ = 0; bZ < 16; bZ++) //,zBlockpos++)
    {
      heightmap[(bZ<<4)+bX]  = ymax = currentHeight = (int32_t)(finalTerrain.GetValue(xBlockpos + bX, 0, zBlockpos + bZ));

      uint8_t stoneHeight = currentHeight - (uniform01() * 3);
      int32_t bYbX = ((bZ << 7) + (bX << 11));

      if (currentHeight < seaLevel)
      {
        ymax = seaLevel;
      }

      for (int bY = 0; bY <= ymax; bY++, bYbX++)
      {
        curBlock = &(chunk->blocks[bYbX]);

        // Place bedrock
        if (bY == 0)
        {
          *curBlock = BLOCK_BEDROCK;
          continue;
        }
        else if (bY <= stoneHeight)
        {
          *curBlock = BLOCK_STONE;
          // Add caves
          if (addCaves)
          {
            cave.AddCaves(*curBlock, xBlockpos, bY, zBlockpos);
          }
        }
        else if (bY <= currentHeight) // -3,-2,-1,0 offset from top block
        {
          if (bY < seaLevel - 2)
          {
            *curBlock = BLOCK_GRAVEL;  // FF
          }
          else if (bY <= seaLevel)
          {
            *curBlock = BLOCK_SAND;  // FF
          }
          else
          {
            if (bY > 70)
            {
              if (uniform01() > 0.999)
              {
                *curBlock = BLOCK_STATIONARY_WATER; // mountain water spring
                if (bYbX & 1)
                {
                  chunk->data[bYbX>>1] &= 0x0f;
                  chunk->data[bYbX>>1] |= 0x4 << 4;
                }
                else
                {
                  chunk->data[bYbX>>1] &= 0xf0;
                  chunk->data[bYbX>>1] |= 0x4;
                }
              }
              else
              {
                *curBlock = topBlock;  // FF
              }
            }
            else
            {
              *curBlock = topBlock;  // FF
            }
          }
        }
        else
        {
          if (bY <= seaLevel)
          {
            *curBlock = BLOCK_STATIONARY_WATER; // FF
            if (bYbX & 1)
            {
              chunk->data[bYbX>>1] &= 0x0f;
              chunk->data[bYbX>>1] |= 0x8 << 4;
            }
            else
            {
              chunk->data[bYbX>>1] &= 0xf0;
              chunk->data[bYbX>>1] |= 0x8;
            }
          }
          else
          {
            *curBlock = BLOCK_AIR;  // FF
          }
        }
        //bYbX++;
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

void EximGen::ExpandBeaches(int x, int z, int map)
{
  sChunk* chunk = ServerInstance->map(map)->getChunk(blockToChunk(x), blockToChunk(z));
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

      h = heightmap[(bZ<<4)+bX];

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

void EximGen::AddOre(int x, int z, int map, uint8_t type)
{
  sChunk* chunk = ServerInstance->map(map)->getChunk(x, z);

  int32_t blockX, blockZ;
  uint8_t block, blockY;

  // Parameters for deposits
  uint8_t count, startHeight = 128, minDepoSize, maxDepoSize;

  switch (type)
  {
  case BLOCK_COAL_ORE:
    count = uniform01() * 10 + 20; // 20-30 coal deposits
    //startHeight = 90;
    minDepoSize = 3;
    maxDepoSize = 7;
    break;
  case BLOCK_IRON_ORE:
    count = uniform01() * 8 + 10; // 10-18 iron deposits
    startHeight = 90;
    minDepoSize = 2;
    maxDepoSize = 5;
    break;
  case BLOCK_GOLD_ORE:
    count = uniform01() * 4 + 5; // 4-9 gold deposits
    startHeight = 42;
    minDepoSize = 2;
    maxDepoSize = 4;
    break;
  case BLOCK_DIAMOND_ORE:
    count = uniform01() * 1 + 2; // 1-3 diamond deposits
    startHeight = 17;
    minDepoSize = 1;
    maxDepoSize = 2;
    break;
  case BLOCK_REDSTONE_ORE:
    count = uniform01() * 5 + 5; // 5-10 redstone deposits
    startHeight = 25;
    minDepoSize = 2;
    maxDepoSize = 4;
    break;
  case BLOCK_LAPIS_ORE:
    count = uniform01() * 1 + 2; // 1-3 lapis lazuli deposits
    startHeight = 17;
    minDepoSize = 1;
    maxDepoSize = 2;
    break;
  case BLOCK_GRAVEL:
    count = uniform01() * 10 + 20; // 20-30 gravel deposits
    //startHeight = 90;
    minDepoSize = 6;
    maxDepoSize = 10;
    break;
  case BLOCK_DIRT:
    count = uniform01() * 10 + 20; // 20-30 gravel deposits
    //startHeight = 90;
    minDepoSize = 6;
    maxDepoSize = 10;
    break;
  default:
    return;
  }

  int i = 0;
  while (i < count)
  {
    blockX = uniform01() * 16;
    blockZ = uniform01() * 16;

    blockY = heightmap[(blockZ<<4)+blockX];
    blockY -= uniform01() * 5;

    // Check that startheight is not higher than height at that column
    if (blockY > startHeight)
    {
      blockY = startHeight;
    }

    // Calculate Y
    blockY = uniform01() * (blockY);

    i++;

    block = chunk->blocks[(blockX << 11) + (blockZ << 7) + blockY];
    // No ore in caves
    if (block == BLOCK_AIR)
    {
      continue;
    }

    AddDeposit(blockX, blockY, blockZ, map, type, minDepoSize, maxDepoSize, chunk);

  }
}

void EximGen::AddDeposit(int x, int y, int z, int map, uint8_t block, uint8_t minDepoSize, uint8_t maxDepoSize, sChunk* chunk)
{
  uint8_t depoSize = (uniform01() * (maxDepoSize - minDepoSize) + minDepoSize) / 2;
  int32_t t_posx, t_posy, t_posz;
  for (int8_t xi = (-depoSize); xi <= depoSize; xi++)
  {
    for (int8_t yi = (-depoSize); yi <= depoSize; yi++)
    {
      for (int8_t zi = (-depoSize); zi <= depoSize; zi++)
      {
        if (abs(xi) + abs(yi) + abs(zi) <= depoSize)
        {
          t_posx = x + xi;
          t_posy = y + yi;
          t_posz = z + zi;

          if (t_posz < 0 || t_posz > 15 || t_posx < 0 || t_posx > 15 || t_posy < 1)
          {
            break;
          }

          if (chunk->blocks[t_posy + (t_posz << 7) + (t_posx << 11)] == BLOCK_STONE)
          {
            chunk->blocks[t_posy + (t_posz << 7) + (t_posx << 11)] = block;
          }
        }
      }
    }
  }
}
