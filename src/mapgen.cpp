/*
  Copyright (c) 2010 Drew Gottlieb - with code from fragmer and TkTech

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
 */
 
// This code is modified from MySMP C# server by Drew Gottlieb
// Thanks!

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <cmath>

#include "logger.h"
#include "constants.h"

#include "config.h"
#include "nbt.h"
#include "map.h"

// libnoise
#ifdef WIN32
#include <noise/noise.h>
#else
#include <libnoise/noise.h>
#endif
#include "noiseutils.h"

#include "mersenne.h"

#include "mapgen.h"

MapGen::MapGen(int seed)
{
  blocks = new uint8[16*16*128];
  blockdata = new uint8[16*16*128/2];
  skylight = new uint8[16*16*128/2];
  blocklight = new uint8[16*16*128/2];
  heightmap = new uint8[16*16];

  //
  // libnoise
  //
  perlinNoise.SetSeed(seed);
  perlinNoise.SetOctaveCount(3);
  perlinNoise.SetFrequency(1.0); // 1-16
  perlinNoise.SetPersistence(0.25); // 0-1

  perlinScale = 0.7f;

  terrainType.SetFrequency (0.5);
  terrainType.SetPersistence (0.15);

  baseFlatTerrain.SetFrequency(1.50);
  flatTerrain.SetSourceModule(0, baseFlatTerrain);
  flatTerrain.SetScale (0.125);
  flatTerrain.SetBias (0);

    
  finalTerrain.SetSourceModule(0, flatTerrain);
  finalTerrain.SetSourceModule(1, perlinNoise);
  finalTerrain.SetControlModule(terrainType);
  finalTerrain.SetBounds(0.0, 1000.0);
  finalTerrain.SetEdgeFalloff(0.125);

  // Generate heightmap
  heightMapBuilder.SetSourceModule(finalTerrain);
  heightMapBuilder.SetDestNoiseMap(heightMap);
  heightMapBuilder.SetDestSize(16, 16);

  oreDensity = Conf::get().iValue("oreDensity");
  seaLevel = Conf::get().iValue("seaLevel");
  
  m_seed = seed;
}

MapGen::~MapGen()
{
  delete [] blocks;
  delete [] blockdata;
  delete [] skylight;
  delete [] blocklight;
  delete [] heightmap;
}


/*void MapGen::calculateHeightmap() 
{
 // uint8 block; uint8 meta;
  int index;

  for(char x = 0; x < 16; x++) 
  {
    for(char z = 0; z < 16; z++)
    {
      for (char y = 127; y >= 0; y--) 
      {
        index = y + (z * 128) + (x * 128 * 16);
        if(blocks[index] == BLOCK_AIR)
          continue;
        heightmap[getHeightmapIndex(x, z)] = (char)(y + 1);
        break;
      }
    }
  }
}

int MapGen::getHeightmapIndex(char x, char z) 
{
  return z + (x * 16);
}*/

void MapGen::loadFlatgrass() 
{
  for (uint8 bX = 0; bX < 16; bX++) 
  {
    for (uint8 bY = 0; bY < 128; bY++) 
    {
      for (uint8 bZ = 0; bZ < 16; bZ++) 
      {
        if (bY == 0) 
          blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_BEDROCK; 
        else if (bY < 64) 
          blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_DIRT;
        else if (bY == 64) 
          blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_GRASS;
        else 
          blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_AIR;
      }
    }
  }
  //CalculateHeightmap();
}

void MapGen::generateChunk(int x, int z)
{
  NBT_Value *main = new NBT_Value(NBT_Value::TAG_COMPOUND);
  NBT_Value *val = new NBT_Value(NBT_Value::TAG_COMPOUND);
 
  if(Conf::get().bValue("map_flatland"))
    loadFlatgrass();
  else
    generateWithNoise(x, z);
   
  val->Insert("Blocks", new NBT_Value(blocks, 16*16*128));
  val->Insert("Data", new NBT_Value(blockdata, 16*16*128/2));
  val->Insert("SkyLight", new NBT_Value(skylight, 16*16*128/2));
  val->Insert("BlockLight", new NBT_Value(blocklight, 16*16*128/2));
  val->Insert("HeightMap", new NBT_Value(heightmap, 16*16));
  val->Insert("Entities", new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND));
  val->Insert("TileEntities", new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND));
  val->Insert("LastUpdate", new NBT_Value((sint64)time(NULL)));
  val->Insert("xPos", new NBT_Value(x));
  val->Insert("zPos", new NBT_Value(z));
  val->Insert("TerrainPopulated", new NBT_Value((char)1));
  
  main->Insert("Level", val);
  
//  uint32 chunkid;
//  Map::get().posToId(x, z, &chunkid);
  
//  Map::get().maps[chunkid].x = x;
//  Map::get().maps[chunkid].z = z;

  std::vector<uint8> *t_blocks = (*val)["Blocks"]->GetByteArray();
  std::vector<uint8> *t_data = (*val)["Data"]->GetByteArray();
  std::vector<uint8> *t_blocklight = (*val)["BlockLight"]->GetByteArray();
  std::vector<uint8> *t_skylight = (*val)["SkyLight"]->GetByteArray();
  std::vector<uint8> *heightmap = (*val)["HeightMap"]->GetByteArray();

  sChunk *chunk = new sChunk();
  
  chunk->x = x;
  chunk->z = z;
  chunk->blocks = &((*t_blocks)[0]);
  chunk->data = &((*t_data)[0]);
  chunk->blocklight = &((*t_blocklight)[0]);
  chunk->skylight = &((*t_skylight)[0]);
  chunk->heightmap = &((*heightmap)[0]);
  chunk->nbt = main;

  Map::get().chunks.LinkChunk(chunk, x, z);

  // Update last used time
  // TODO: Map::get().mapLastused[chunkid] = (int)time(0);

  // Not changed
  // TODO: Map::get().mapChanged[chunkid] = 0;
  
  
}

void MapGen::generateWithNoise(int x, int z) 
{
  // Ore arrays
  //uint8* oreX;
  //uint8* oreY;
  //uint8* oreZ;
  //uint8* oreType;

  heightMapBuilder.SetBounds(1000 + x*perlinScale, 1000 + (x+1)*perlinScale, 1000 + z*perlinScale, 1000 + (z+1)*perlinScale);
  heightMapBuilder.Build();

  // Image render
  /*noise::utils::RendererImage renderer;
  noise::utils::Image image;
  renderer.SetSourceNoiseMap (heightMap);
  renderer.SetDestImage (image);
  renderer.Render ();

  noise::utils::WriterBMP writer;
  writer.SetSourceImage (image);
  writer.SetDestFilename ("tutorial.bmp");
  writer.WriteDestFile ();*/



  // Get cave heightmaps for upper caves (top & bottom)
  /*Noise caveTopNoise;
  Random rnd(randomSeed);
  caveTopNoise.init(randomSeed + rnd.uniform(RAND_MAX), caveTopNoise.Spline);
  caveTopNoise.PerlinNoiseMap(caveTop, 0, 4, 0.4f, x, z);
  caveTopNoise.Invert(caveTop);
  caveTopNoise.Marble(caveTop);
  
  Noise caveBottomNoise;
  Random rndBot(rnd.uniform(RAND_MAX));
  caveBottomNoise.init(randomSeed + rndBot.uniform(RAND_MAX), caveBottomNoise.Spline);
  caveBottomNoise.PerlinNoiseMap(caveBottom, 0, 4, 0.2f, x, z);
  caveBottomNoise.Invert(caveBottom);
  caveBottomNoise.Marble(caveBottom);

  // Get cave heightmaps for lower caves (top & bottom)
  Noise caveTopNoise2;
  Random rnd2(randomSeed + 1);
  caveTopNoise2.init(randomSeed + rnd2.uniform(RAND_MAX), caveTopNoise2.Spline);
  caveTopNoise2.PerlinNoiseMap(caveTop2, 0, 4, 0.4f, x, z);
  caveTopNoise2.Invert(caveTop2);
  caveTopNoise2.Marble(caveTop2);
  
  Noise caveBottomNoise2;
  Random rnd3(randomSeed + 2);
  Random rnd4(rnd3.uniform(RAND_MAX) + 1);
  caveBottomNoise2.init(randomSeed + rnd4.uniform(RAND_MAX), caveBottomNoise2.Spline);
  caveBottomNoise2.PerlinNoiseMap(caveBottom2, 0, 4, 0.2f, x, z);
  caveBottomNoise2.Invert(caveBottom2);
  caveBottomNoise2.Marble(caveBottom2);

  // Determine whether there should be ore (random, not seed-related) and where it should be and what type
  int oreChance = rand()%9 + 1; // 1-10
  int numOre = 0;
  if (oreDensity >= 18) 
  {
    if (oreChance <= 2)
      numOre = oreDensity;
    else if (oreChance <= 4)
      numOre = oreDensity - 4;
    else if (oreChance <= 8)
      numOre = oreDensity - 8;
    else if (oreChance <= 10)
      numOre = oreDensity - 12;
  } 
  else
    numOre = oreDensity;

  oreX = new uint8[numOre];
  oreY = new uint8[numOre];
  oreZ = new uint8[numOre];
  oreType = new uint8[numOre];

  for(int i = 0; i < numOre; i++) 
  {
    oreX[i] = (char)rand()%14 + 1; // 1-15
    oreY[i] = (char)rand()%63 + 1; // 1-64
    oreZ[i] = (char)rand()%14 + 1; // 1-15
    int oreTypeRand = rand()%19 + 1; // 1-20
    if(oreY[i] < 14) 
    {
      if (oreTypeRand < 10)
        oreType[i] = BLOCK_REDSTONE_ORE;
      else
        oreType[i] = BLOCK_DIAMOND_ORE;
    } 
    else 
    {
      if (oreTypeRand <= 5 && oreY[i] < 30)
        oreType[i] = BLOCK_GOLD_ORE;
      else if (oreTypeRand <= 9)
        oreType[i] = BLOCK_IRON_ORE;
      else if (oreTypeRand <= 20)
        oreType[i] = BLOCK_COAL_ORE;
    }
  }
  */
  // Populate blocks in chunk
  int currentHeight;//, caveTopHeight, caveBottomHeight, caveTopHeight2, caveBottomHeight2;
  for (uint8 bX = 0; bX < 16; bX++) 
  {
    for (uint8 bY = 0; bY < 128; bY++) 
    {
      for (uint8 bZ = 0; bZ < 16; bZ++) 
      {
        currentHeight = (int)((heightMap.GetValue(bX,bZ) * 7.49674) + 64.15371);

        //currentHeight = (int)((heightMap[bX][bZ] * 2.49674) + 82.15371);

        /*caveTopHeight = (int)((caveTop[bX][bZ] * 4.29674) + 36.15371);
        caveBottomHeight = (int)((caveBottom[bX][bZ] * 2.29674) + 40.15371);
        caveTopHeight += (int)((float)currentHeight / 128.0 * 90.0) - 40;
        caveBottomHeight += (int)((float)currentHeight / 128.0 * 90.0) - 45;

        caveTopHeight2 = (int)((caveTop2[bX][bZ] * 4.29674) + 36.15371);
        caveBottomHeight2 = (int)((caveBottom2[bX][bZ] * 2.29674) + 40.15371);
        caveTopHeight2 += (int)((float)currentHeight / 128.0 * 100.0) - 70;
        caveBottomHeight2 += (int)((float)currentHeight / 128.0 * 100.0) - 75;*/

        if(bY == 0) 
        {
          blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_BEDROCK;
        } 
        else 
        {
          if(bY < currentHeight) 
          {
            if (bY < (int)(currentHeight * 0.94)) 
            {
              /*if(bY >= caveBottomHeight && bY <= caveTopHeight)
                blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_AIR;
              else if (bY >= caveBottomHeight2 && bY <= caveTopHeight2)
                blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_AIR;
              else 
              {
                int veinDir = rand()%7 + 1; // 1-8
                */Block blockToPlace = BLOCK_STONE;
                         
                /*for(int i = 0; i < numOre; i++) 
                {
                  if(oreType[i] == BLOCK_DIAMOND_ORE || oreType[i] == BLOCK_GOLD_ORE) {    // Smaller deposits
                    if (veinDir <= 2) 
                    {         // Cube (2x2)
                      if (oreX[i] < bX - 0)
                        continue;
                      if (oreX[i] > bX + 1)
                        continue;
                      if (oreY[i] < bY - 0)
                        continue;
                      if (oreY[i] > bY + 1)
                        continue;
                      if (oreZ[i] < bZ - 0)
                        continue;
                      if (oreZ[i] > bZ + 1)
                        continue;
                    } 
                    else if (veinDir <= 4) 
                    {  // Long across X
                      if (oreX[i] < bX - 0)
                        continue;
                      if (oreX[i] > bX + 2)
                        continue;
                      if (oreY[i] < bY - 0)
                        continue;
                      if (oreY[i] > bY + 1)
                        continue;
                      if (oreZ[i] < bZ - 0)
                        continue;
                      if (oreZ[i] > bZ + 1)
                        continue;
                    } 
                    else if (veinDir <= 6) 
                    {  // Long across Z
                      if (oreX[i] < bX - 0)
                        continue;
                      if (oreX[i] > bX + 1)
                        continue;
                      if (oreY[i] < bY - 0)
                        continue;
                      if (oreY[i] > bY + 1)
                        continue;
                      if (oreZ[i] < bZ - 0)
                        continue;
                      if (oreZ[i] > bZ + 2)
                        continue;
                    } 
                    else if (veinDir <= 8) 
                    {  // Tall
                      if (oreX[i] < bX - 0)
                        continue;
                      if (oreX[i] > bX + 1)
                        continue;
                      if (oreY[i] < bY - 0)
                        continue;
                      if (oreY[i] > bY + 2)
                        continue;
                      if (oreZ[i] < bZ - 0)
                        continue;
                      if (oreZ[i] > bZ + 1)
                        continue;
                    }
                  } 
                  else 
                  {
                    // Larger deposits
                    if (veinDir <= 2) 
                    {         // Cube (2x2)
                      if (oreX[i] < bX - 0)
                        continue;
                      if (oreX[i] > bX + 1)
                        continue;
                      if (oreY[i] < bY - 0)
                        continue;
                      if (oreY[i] > bY + 1)
                        continue;
                      if (oreZ[i] < bZ - 0)
                        continue;
                      if (oreZ[i] > bZ + 1)
                        continue;
                    } 
                    else if (veinDir <= 4) 
                    {  // Long across X
                      if (oreX[i] < bX - 1)
                        continue;
                      if (oreX[i] > bX + 2)
                        continue;
                      if (oreY[i] < bY - 0)
                        continue;
                      if (oreY[i] > bY + 1)
                        continue;
                      if (oreZ[i] < bZ - 0)
                        continue;
                      if (oreZ[i] > bZ + 1)
                        continue;
                    } 
                    else if (veinDir <= 6) 
                    {  // Long across Z
                      if (oreX[i] < bX - 0)
                        continue;
                      if (oreX[i] > bX + 1)
                        continue;
                      if (oreY[i] < bY - 0)
                        continue;
                      if (oreY[i] > bY + 1)
                        continue;
                      if (oreZ[i] < bZ - 1)
                        continue;
                      if (oreZ[i] > bZ + 2)
                        continue;
                    } 
                    else if (veinDir <= 8) 
                    {  // Tall
                      if (oreX[i] < bX - 0)
                         continue;
                      if (oreX[i] > bX + 1)
                         continue;
                      if (oreY[i] < bY - 1)
                         continue;
                      if (oreY[i] > bY + 2)
                         continue;
                      if (oreZ[i] < bZ - 0)
                         continue;
                      if (oreZ[i] > bZ + 1)
                         continue;
                    }
                  }*/
                  //blockToPlace = (Block)oreType[i];
                  //break;
                //}
                blocks[bY + (bZ * 128 + (bX * 128 * 16))] = blockToPlace;
              //} 
            } 
            else
              blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_DIRT;
          } 
          else if (currentHeight == bY) 
          {
            if (bY == seaLevel || bY == seaLevel - 1 || bY == seaLevel - 2)
              blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_SAND; // FF
            else if (bY < seaLevel - 1)
              blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_GRAVEL; // FF
            else
              blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_GRASS; // FF
          } 
          else 
          {
            if (bY <= seaLevel)
              blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_STATIONARY_WATER; // FF
            else
              blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_AIR; // FF
          }
        }
      }
    }
  }
  //CalculateHeightmap();
  /*delete [] oreX;
  delete [] oreY;
  delete [] oreZ;
  delete [] oreType;*/
}
