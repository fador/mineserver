/*
    Copyright (c) 2012, The Mineserver Project
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

#include "biomegen.h"

#include "mineserver.h"
#include "config.h"
#include "tree.h"
#include "random.h"

BiomeGen::BiomeGen()
  : blocks(16 * 16 * 128, 0),
    blockdata(16 * 16 * 128 / 2, 0),
    skylight(16 * 16 * 128 / 2, 0),
    blocklight(16 * 16 * 128 / 2, 0),
    heightmap(16 * 16, 0)
{
}

void BiomeGen::init(int seed)
{
  cave.init(seed + 7);
  //###### TREE GEN #####
  treenoise.SetSeed(seed + 404);
  treenoise.SetFrequency(0.01);
  treenoise.SetOctaveCount(3);
  //###### END TREE GEN #######
  seaLevel = ServerInstance->config()->iData("mapgen.sea.level");
  addTrees = ServerInstance->config()->bData("mapgen.trees.enabled");
  expandBeaches = ServerInstance->config()->bData("mapgen.beaches.expand");
  beachExtent = ServerInstance->config()->iData("mapgen.beaches.extent");
  beachHeight = ServerInstance->config()->iData("mapgen.beaches.height");

  addOre = ServerInstance->config()->bData("mapgen.addore");
  addCaves = ServerInstance->config()->bData("mapgen.caves.enabled");

  BiomeBase.SetFrequency(0.2);
  BiomeBase.SetSeed(seed - 1);
  BiomeSelect.SetSourceModule(0, BiomeBase);
  BiomeSelect.SetScale(2.5);
  BiomeSelect.SetBias(2.5);
  mountainTerrainBase.SetSeed(seed + 1);
  mountainTerrain.SetSourceModule(0, mountainTerrainBase);
  mountainTerrain.SetScale(0.5);
  mountainTerrain.SetBias(0.5);
  jaggieEdges.SetSourceModule(0, terrainType);
  jaggieEdges.SetSourceModule(1, plain);
  plain.SetConstValue(0.5);
  jaggieEdges.SetControlModule(jaggieControl);
  jaggieEdges.SetBounds(0.5, 1.0);
  jaggieEdges.SetEdgeFalloff(0.11);
  jaggieControl.SetSeed(seed + 20);
  baseFlatTerrain.SetSeed(seed);
  baseFlatTerrain.SetFrequency(0.2);
  flatTerrain.SetSourceModule(0, baseFlatTerrain);
  flatTerrain.SetScale(0.125);
  flatTerrain.SetBias(0.07);
  baseWater.SetSeed(seed - 1);
  water.SetSourceModule(0, baseWater);
  water.SetScale(0.3);
  water.SetBias(-0.5);
  terrainType.SetSeed(seed + 2);
  terrainType.SetFrequency(0.5);
  terrainType.SetPersistence(0.25);
  terrainType2.SetSeed(seed + 7);
  terrainType2.SetFrequency(0.5);
  terrainType2.SetPersistence(0.25);
  waterTerrain.SetSourceModule(0, water);
  waterTerrain.SetSourceModule(1, flatTerrain);
  waterTerrain.SetControlModule(terrainType2);
  waterTerrain.SetEdgeFalloff(0.1);
  waterTerrain.SetBounds(-0.5, 1.0);
  secondTerrain.SetSourceModule(1, waterTerrain);
  secondTerrain.SetSourceModule(0, mountainTerrain);
  secondTerrain.SetControlModule(terrainType);
  secondTerrain.SetEdgeFalloff(0.3);
  secondTerrain.SetBounds(-0.5, 1.0);
  finalTerrain.SetSourceModule(0, secondTerrain);
  finalTerrain.SetSourceModule(1, waterTerrain);
  finalTerrain.SetControlModule(jaggieEdges);
  finalTerrain.SetEdgeFalloff(0.2);
  finalTerrain.SetBounds(-0.3, 1.0);
  flowers.SetSeed(seed + 10);
  flowers.SetFrequency(3);
  winterEnabled = false;
}

void BiomeGen::re_init(int seed)
{
  cave.init(seed + 7);
  treenoise.SetSeed(seed + 404);
  BiomeBase.SetSeed(seed - 1);
  mountainTerrainBase.SetSeed(seed + 1);
  baseFlatTerrain.SetSeed(seed);
  baseWater.SetSeed(seed - 1);
  terrainType2.SetSeed(seed + 7);
  terrainType.SetSeed(seed + 2);
}


void BiomeGen::generateFlatgrass(int x, int z, int map)
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
      heightmap_pointer[(bZ << 4) + bX] = 64;
      for (int bY = 0; bY < 128; bY++)
      {
        if (bY == 0)
        {
          chunk->blocks[bX + (bZ << 4) + (bY << 8)] = BLOCK_BEDROCK;
        }
        else if (bY < 64)
        {
          chunk->blocks[bX + (bZ << 4) + (bY << 8)] = BLOCK_DIRT;
        }
        else if (bY == 64)
        {
          chunk->blocks[bX + (bZ << 4) + (bY << 8)] = top;
        }
        else
        {
          chunk->blocks[bX + (bZ << 4) + (bY << 8)] = BLOCK_AIR;
        }
      }
    }
  }
}

void BiomeGen::generateChunk(int x, int z, int map)
{
  NBT_Value* main = new NBT_Value(NBT_Value::TAG_COMPOUND);
  NBT_Value* val = new NBT_Value(NBT_Value::TAG_COMPOUND);

  val->Insert("Sections", new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND));

  val->Insert("HeightMap", new NBT_Value(heightmap));
  val->Insert("Entities", new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND));
  val->Insert("TileEntities", new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND));
  val->Insert("LastUpdate", new NBT_Value((int64_t)time(NULL)));
  val->Insert("xPos", new NBT_Value(x));
  val->Insert("zPos", new NBT_Value(z));
  val->Insert("TerrainPopulated", new NBT_Value((int8_t)1));

  main->Insert("Level", val);

  sChunk* chunk = new sChunk();
  chunk->blocks = new uint8_t[16 * 16 * 256];
  chunk->addblocks = new uint8_t[16 * 16 * 256 / 2];
  chunk->data = new uint8_t[16 * 16 * 256 / 2];
  chunk->blocklight = new uint8_t[16 * 16 * 256 / 2];
  chunk->skylight = new uint8_t[16 * 16 * 256 / 2];
  chunk->heightmap = &((*(*val)["HeightMap"]->GetIntArray())[0]);
  heightmap_pointer = chunk->heightmap;
  chunk->nbt = main;
  chunk->x = x;
  chunk->z = z;

  memset(chunk->blocks, 0, 16*16*256);
  memset(chunk->addblocks, 0, 16*16*256/2);
  memset(chunk->data, 0, 16*16*256/2);
  memset(chunk->blocklight, 0, 16*16*256/2);
  memset(chunk->skylight, 0, 16*16*256/2);
  chunk->chunks_present = 0xffff;

  ServerInstance->map(map)->chunks.insert(ChunkMap::value_type(ChunkMap::key_type(x, z), chunk));

  if (ServerInstance->config()->bData("mapgen.flatgrass"))
  {
    generateFlatgrass(x, z, map);
  }
  else
  {
    generateWithNoise(x, z, map);
  }


  // Update last used time
  //ServerInstance->map()->mapLastused[chunkid] = (int)time(0);

  // Not changed
  chunk->changed = ServerInstance->config()->bData("map.save_unchanged_chunks");

  //ServerInstance->map()->maps[chunkid].nbt = main;

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

}

//#define PRINT_MAPGEN_TIME


void BiomeGen::AddTrees(int x, int z, int map)
{
  int32_t xBlockpos = x << 4;
  int32_t zBlockpos = z << 4;
  int blockX, blockZ;
  uint8_t blockY, block, meta;

  bool empty[16][16]; // is block empty 

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
      blockY = heightmap_pointer[(b<<4)+a];

      // Another dirty haxx!
      if (blockY > 120)
      {
        i++;
        continue;
      }
      ServerInstance->map(map)->getBlock(blockX, blockY, blockZ, &block, &meta);

      int biome = BiomeSelect.GetValue(blockX / 100.0, 0, blockZ / 100.0);
      if (biome == 1 &&
          treenoise.GetValue(blockX, 0, blockZ) > -0.3 &&
          ((rand() % 16) < 7)) // Dirty haxx!
      {
        // Desert, make cactus
        int count = 3;
        if ((count + blockY) > 126)
        {
          continue;
        }
          //LOGLF("testing reed area");
          ServerInstance->map(map)->getBlock(blockX, (blockY + i), blockZ, &block, &meta);
          if(block == BLOCK_SAND){
            ServerInstance->map(map)->setBlock(blockX, (blockY + 1 + i), blockZ, (char)BLOCK_CACTUS, (char)meta);
            ServerInstance->map(map)->setBlock(blockX, (blockY + 2 + i), blockZ, (char)BLOCK_CACTUS, (char)meta);
            ServerInstance->map(map)->setBlock(blockX, (blockY + 3 + i), blockZ, (char)BLOCK_CACTUS, (char)meta);
            //printf("successful cactus! x%d y%d z%d\n", blockX, blockY, blockZ);
          }



        // Check that it's not in water
        /*ServerInstance->map(map)->getBlock(blockX, ++blockY, blockZ, &block, &meta);
        if (!(block == BLOCK_WATER || block == BLOCK_STATIONARY_WATER))
        {
          sChunk* chunk = ServerInstance->map(map)->getChunk(x, z);

          uint8_t* curBlock;
          int count = (fastrand() % 3) + 3;
          if (count + blockY > 127)
          {
            continue;
          }
          curBlock = &(chunk->blocks[(a << 7) + (b << 11) + blockY]);
          for (int i = 0; i < count; i++)
          {
            curBlock[i] = BLOCK_CACTUS;
          }
        }*/
      }
      else if (biome == 4 &&
               block != BLOCK_WATER &&
               block != BLOCK_STATIONARY_WATER &&
               treenoise.GetValue(blockX, 0, blockZ) > -0.2)
      {
        //Reed forest
        int count = 3;
        if ((count + blockY) > 126)
        {
          continue;
        }
          //LOGLF("testing reed area");
	  int xOffset = 0;
	  ServerInstance->map(map)->getBlock(blockX + xOffset, (blockY + i), blockZ, &block, &meta);
          if(block == BLOCK_DIRT || block == BLOCK_REED || block == BLOCK_GRASS){
            if(block == BLOCK_GRASS){
	      ServerInstance->map(map)->setBlock(blockX + xOffset, (blockY + i), blockZ, (char)BLOCK_DIRT, (char)meta);
              block = BLOCK_DIRT;
            }
            ServerInstance->map(map)->setBlock(blockX + xOffset, (blockY + 1 + i), blockZ, (char)BLOCK_REED, (char)meta);
            ServerInstance->map(map)->setBlock(blockX + xOffset, (blockY + 2 + i), blockZ, (char)BLOCK_REED, (char)meta);
            ServerInstance->map(map)->setBlock(blockX + xOffset, (blockY + 3 + i), blockZ, (char)BLOCK_REED, (char)meta);
            //printf("successful reed! x%d y%d z%d\n", blockX + xOffset, blockY, blockZ);
          }

      }
      else if (biome == 2 || biome == 3)
      {
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

void BiomeGen::generateWithNoise(int x, int z, int map)
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

  double xBlockpos = x << 4;
  double zBlockpos = z << 4;
  for (int bX = 0; bX < 16; bX++)
  {
    for (int bZ = 0; bZ < 16; bZ++)
    {
      heightmap_pointer[(bZ << 4) + bX] = ymax = currentHeight = (uint8_t)((finalTerrain.GetValue((xBlockpos + bX) / 100.0, 0, (zBlockpos + bZ) / 100.0) * 60) + 64);
      int biome = BiomeSelect.GetValue((xBlockpos + bX) / 100.0, 0, (zBlockpos + bZ) / 100.0);
      char toplayer;
      if (biome == 0)
      {
        toplayer = BLOCK_DIRT;
      }
      if (biome == 1)
      {
        toplayer = BLOCK_SAND;
      }
      if (biome == 2)
      {
        toplayer = BLOCK_DIRT;
      }
      if (biome == 3)
      {
        toplayer = BLOCK_DIRT;
      }
      if (biome == 4)
      {
        toplayer = BLOCK_DIRT;
      }
      if (biome == 5)
      {
        toplayer = BLOCK_CLAY;
      }

      int32_t stoneHeight = (int32_t)currentHeight - ((64 - (currentHeight % 64)) / 8) + 1;
      //int32_t bYbX = ((bZ << 7) + (bX << 11));

      if (ymax < seaLevel)
      {
        ymax = seaLevel;
      }
      ymax++;

      for (int bY = 0; bY <= ymax; bY++)
      {
        curBlock = &(chunk->blocks[bX + (bZ << 4) + (bY << 8)]);

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
            *curBlock = toplayer;
          }
        }
        else if ((currentHeight + 1) == bY && bY > seaLevel && biome == 3)
        {
          *curBlock = BLOCK_SNOW;
          continue;
        }
        else if ((currentHeight + 1) == bY && bY > seaLevel + 1)
        {
          if (biome == 1 || biome == 0)
          {
            continue;
          }
          double f = flowers.GetValue(xBlockpos + bX / 10.0, 0, zBlockpos + bZ / 10.0);
          if (f < -0.999)
          {
            *curBlock = BLOCK_RED_ROSE;
          }
          else if (f > 0.999)
          {
            *curBlock = BLOCK_YELLOW_FLOWER;
          }
          else if (f < 0.001 && f > -0.001)
          {
            *curBlock = BLOCK_PUMPKIN;
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
          else if (toplayer == BLOCK_DIRT)
          {
            *curBlock = BLOCK_GRASS;
          }
          else
          {
            *curBlock = toplayer; // FF
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
          if (bY == seaLevel && biome == 3)
          {
            *curBlock = BLOCK_ICE;
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


MAKE_UNIFORM_DIST(10, 30)
MAKE_UNIFORM_DIST(20, 30)
MAKE_UNIFORM_DIST( 1,  3)
MAKE_UNIFORM_DIST(10, 18)
MAKE_UNIFORM_DIST( 4,  9)
MAKE_UNIFORM_DIST( 5, 10)
MAKE_UNIFORM_DIST( 8, 12)
MAKE_UNIFORM_DIST( 0,  1)


void BiomeGen::AddOre(int x, int z, int map, uint8_t type)
{
  sChunk* chunk = ServerInstance->map(map)->getChunk(x, z);

  int blockX, blockY, blockZ;
  uint8_t block;

  // Parameters for deposits
  int startHeight, minDepoSize, maxDepoSize;
  unsigned int count;

  switch (type)
  {
  case BLOCK_COAL_ORE:
    count = uniform_20_30(); // 20-30 coal deposits
    startHeight = 90;
    minDepoSize = 8;
    maxDepoSize = 20;
    break;
  case BLOCK_IRON_ORE:
    count = uniform_10_18(); // 10-18 iron deposits
    startHeight = 60;
    minDepoSize = 5;
    maxDepoSize = 10;
    break;
  case BLOCK_GOLD_ORE:
    count = uniform_4_9(); // 4-9 gold deposits
    startHeight = 32;
    minDepoSize = 5;
    maxDepoSize = 8;
    break;
  case BLOCK_DIAMOND_ORE:
    count = uniform_1_3(); // 1-3 diamond deposits
    startHeight = 17;
    minDepoSize = 4;
    maxDepoSize = 7;
    break;
  case BLOCK_REDSTONE_ORE:
    count = uniform_5_10(); // 5-10 redstone deposits
    startHeight = 25;
    minDepoSize = 5;
    maxDepoSize = 20;
    break;
  case BLOCK_LAPIS_ORE:
    count = uniform_1_3(); // 1-3 lapis lazuli deposits
    startHeight = 17;
    minDepoSize = 5;
    maxDepoSize = 20;
    break;
  case BLOCK_GRAVEL:
    count = uniform_10_30(); // 10-30 gravel deposits
    startHeight = 90;
    minDepoSize = 5;
    maxDepoSize = 50;
    break;
  default:
    return;
  }

  for (unsigned int i = 0; i < count; ++i)
  {
    blockX = uniform_8_12();
    blockZ = uniform_8_12();

    blockY = heightmap_pointer[(blockZ << 4) + blockX];
    blockY -= 5;

    // Check that startheight is not higher than height at that column
    if (blockY > startHeight)
    {
      blockY = startHeight;
    }

    //blockX += xBlockpos;
    //blockZ += zBlockpos;

    // Calculate Y
    blockY = uniformUINT(0, blockY);

    
    block = chunk->blocks[blockX + (blockZ << 4) + (blockY << 8)];

    // No ore in caves
    if (block == BLOCK_AIR)
    {
      continue;
    }

    AddDeposit(blockX, blockY, blockZ, map, type, minDepoSize, maxDepoSize, chunk);
  }
}

void BiomeGen::AddDeposit(int x, int y, int z, int map, uint8_t block, int minDepoSize, int maxDepoSize, sChunk* chunk)
{
  int depoSize = uniformUINT(maxDepoSize - minDepoSize, maxDepoSize);

  for (int i = 0; i < depoSize; i++)
  {
    if (chunk->blocks[x + (z << 4) + (y << 8)] == BLOCK_STONE)
    {
      chunk->blocks[x + (z << 4) + (y << 8)] = block;
    }

    z = z + int(uniform_0_1()) - 1;
    x = x + int(uniform_0_1()) - 1;
    y = y + int(uniform_0_1()) - 1;

    // If over chunk borders
    if (z < 0 || z > 15 || x < 0 || x > 15 || y < 1)
    {
      break;
    }
  }
}
