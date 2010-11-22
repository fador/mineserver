/*
  The MIT License

  Copyright (c) 2009, 2010 Matvei Stefarov <me@matvei.org>

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
#include "noise.h"
#include "nbt.h"
#include "map.h"

#include "mapgen.h"

MapGen::MapGen(int seed)
{
  blocks = new uint8[16*16*128];
  blockdata = new uint8[16*16*128/2];
  skylight = new uint8[16*16*128/2];
  blocklight = new uint8[16*16*128/2];
  heightmap = new uint8[16*16];
  
  
  noise.init(seed, Noise::Bicubic);
}

MapGen::~MapGen()
{
  delete [] blocks;
  delete [] blockdata;
  delete [] skylight;
  delete [] blocklight;
  delete [] heightmap;
}


void MapGen::CalculateHeightmap() 
{
  uint8 block; uint8 meta;
  int index;

  for(char x = 0; x < 16; x++) 
  {
    for(char z = 0; z < 16; z++)
    {
      for (char y = 127; y >= 0; y--) 
      {
        //if (Blocks.SkyLightCarryingBlocks.Contains((Block)blocks[GetBlockIndex(x, y, z)].type))
        index      = y + (z * 128) + (x * 128 * 16);
        if(blocks[index] == BLOCK_AIR)
          continue;
        heightmap[GetHeightmapIndex(x, z)] = (char)(y + 1);
        break;
      }
    }
  }
}

int MapGen::GetHeightmapIndex(char x, char z) 
{
  return z + (x * 16);
}

void MapGen::LoadFlatgrass() 
{
  for (uint8 bX = 0; bX < 16; bX++) 
  {
    for (uint8 bY = 0; bY < 128; bY++) 
    {
      for (uint8 bZ = 0; bZ < 16; bZ++) 
      {
        if (bY == 0) 
        {
          blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_BEDROCK;
        } 
        else if (bY < 64) 
        {
          blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_DIRT;
        } 
        else if (bY == 64) 
        {
          /*if ((x == -1 && z == 0) &&false) 
          {
            if(bX == 0 && bZ == 15)
              blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_LEAVES;
            else
              blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_SAND;
          } 
          else 
          {*/
            blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_GRASS;
          //}
        } 
        else 
        {
          blocks[bY + (bZ * 128 + (bX * 128 * 16))] = BLOCK_AIR;
        }
      }
    }
  }
  CalculateHeightmap();
}

void MapGen::generateChunk(int x, int z)
{
  NBT_Value *val = new NBT_Value(NBT_Value::TAG_COMPOUND);
  LoadFlatgrass();
  
  NBT_Value *main = new NBT_Value(NBT_Value::TAG_COMPOUND);

  
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
  
  uint32 chunkid;
  Map::get().posToId(x, z, &chunkid);
  
  Map::get().maps[chunkid].x = x;
  Map::get().maps[chunkid].z = z;

  std::vector<uint8> *t_blocks = (*val)["Blocks"]->GetByteArray();
  std::vector<uint8> *t_data = (*val)["Data"]->GetByteArray();
  std::vector<uint8> *t_blocklight = (*val)["BlockLight"]->GetByteArray();
  std::vector<uint8> *t_skylight = (*val)["SkyLight"]->GetByteArray();
  
  Map::get().maps[chunkid].blocks = &((*t_blocks)[0]);
  Map::get().maps[chunkid].data = &((*t_data)[0]);
  Map::get().maps[chunkid].blocklight = &((*t_blocklight)[0]);
  Map::get().maps[chunkid].skylight = &((*t_skylight)[0]);

  // Update last used time
  Map::get().mapLastused[chunkid] = (int)time(0);

  // Not changed
  Map::get().mapChanged[chunkid] = 0;

  
  Map::get().maps[chunkid].nbt=main;
}
/*
void GenerateWithNoise() {
  // Ore arrays
  byte[] oreX, oreY, oreZ, oreType;

  // Get main heightmap
  float[,] heightMap = new float[16, 16];
  Noise noise = new Noise((int)world.randomSeed, NoiseInterpolationMode.Bicubic);
  noise.PerlinNoiseMap(heightMap, -2, 2, 0.4f, x, z);

  // Get secondary heightmap
  float[,] steepnessMap = new float[16, 16];
  Noise steepnessNoise = new Noise((int)world.randomSeed, NoiseInterpolationMode.Cosine);
  steepnessNoise.PerlinNoiseMap(steepnessMap, -9, 0, 0.6f, x, z);
  for (int i = 0; i < 16; i++) {
      for (int j = 0; j < 16; j++) {
          steepnessMap[i, j] = steepnessMap[i, j] / 10;
      }
  }

  //Noise.Add(heightMap, steepnessMap);   // Sometimes made mountains too tall or valleys too low

  // Get cave heightmaps for upper caves (top & bottom)
  float[,] caveTop = new float[16, 16];
  Noise caveTopNoise = new Noise((int)world.randomSeed + new Random((int)world.randomSeed).Next(), NoiseInterpolationMode.Spline);
  caveTopNoise.PerlinNoiseMap(caveTop, 0, 4, 0.4f, x, z);
  Noise.Invert(caveTop);
  Noise.Marble(caveTop);
  float[,] caveBottom = new float[16, 16];
  Noise caveBottomNoise = new Noise((int)world.randomSeed + new Random((int)new Random((int)world.randomSeed).Next()).Next(), NoiseInterpolationMode.Spline);
  caveBottomNoise.PerlinNoiseMap(caveBottom, 0, 4, 0.2f, x, z);
  Noise.Invert(caveBottom);
  Noise.Marble(caveBottom);

  // Get cave heightmaps for lower caves (top & bottom)
  float[,] caveTop2 = new float[16, 16];
  Noise caveTopNoise2 = new Noise((int)world.randomSeed + new Random((int)world.randomSeed + 1).Next(), NoiseInterpolationMode.Spline);
  caveTopNoise2.PerlinNoiseMap(caveTop2, 0, 4, 0.4f, x, z);
  Noise.Invert(caveTop2);
  Noise.Marble(caveTop2);
  float[,] caveBottom2 = new float[16, 16];
  Noise caveBottomNoise2 = new Noise((int)world.randomSeed + new Random((int)new Random((int)world.randomSeed + 2).Next() + 1).Next(), NoiseInterpolationMode.Spline);
  caveBottomNoise2.PerlinNoiseMap(caveBottom2, 0, 4, 0.2f, x, z);
  Noise.Invert(caveBottom2);
  Noise.Marble(caveBottom2);

  // Determine whether there should be ore (random, not seed-related) and where it should be and what type
  int oreChance = world.rand.Next(0, 10);
  int numOre = 0;
  if (world.oreDensity >= 18) {
      if (oreChance <= 2)
          numOre = world.oreDensity;
      else if (oreChance <= 4)
          numOre = world.oreDensity - 4;
      else if (oreChance <= 8)
          numOre = world.oreDensity - 8;
      else if (oreChance <= 10)
          numOre = world.oreDensity - 12;
  } else
      numOre = world.oreDensity;
  oreX = new byte[numOre];
  oreY = new byte[numOre];
  oreZ = new byte[numOre];
  oreType = new byte[numOre];
  for (int i = 0; i < numOre; i++) {
      oreX[i] = (byte)world.rand.Next(1, 15);
      oreY[i] = (byte)world.rand.Next(1, 64);
      oreZ[i] = (byte)world.rand.Next(1, 15);
      int oreTypeRand = world.rand.Next(0, 20);
      if (oreY[i] < 14) {
          if (oreTypeRand < 10)
              oreType[i] = (byte)Block.RedstoneOre;
          else
              oreType[i] = (byte)Block.DiamondOre;
      } else {
          if (oreTypeRand <= 5 && oreY[i] < 30)
              oreType[i] = (byte)Block.GoldOre;
          else if (oreTypeRand <= 9)
              oreType[i] = (byte)Block.IronOre;
          else if (oreTypeRand <= 20)
              oreType[i] = (byte)Block.Coal;
      }
  }

  // Populate blocks in chunk
  int currentHeight, caveTopHeight, caveBottomHeight, caveTopHeight2, caveBottomHeight2;
  lock (blockslock) {
      for (byte bX = 0; bX < 16; bX++) {
          for (byte bY = 0; bY < 128; bY++) {
              for (byte bZ = 0; bZ < 16; bZ++) {
                  currentHeight = (int)((heightMap[bX, bZ] * 2.49674) + 62.15371);
                  currentHeight = (int)((heightMap[bX, bZ] * 2.49674) + 82.15371);

                  caveTopHeight = (int)((caveTop[bX, bZ] * 4.29674) + 36.15371);
                  caveBottomHeight = (int)((caveBottom[bX, bZ] * 2.29674) + 40.15371);
                  caveTopHeight += (int)((float)currentHeight / 128f * 90f) - 40;
                  caveBottomHeight += (int)((float)currentHeight / 128f * 90f) - 45;

                  caveTopHeight2 = (int)((caveTop2[bX, bZ] * 4.29674) + 36.15371);
                  caveBottomHeight2 = (int)((caveBottom2[bX, bZ] * 2.29674) + 40.15371);
                  caveTopHeight2 += (int)((float)currentHeight / 128f * 100f) - 70;
                  caveBottomHeight2 += (int)((float)currentHeight / 128f * 100f) - 75;

                  if (bY == 0) {
                      blocks[bY + (bZ * 128 + (bX * 128 * 16))] = new ChunkBlock(Block.Admincrete, 0x00, 0x00);
                  } else {
                      if (currentHeight > bY) {
                          if (bY < (int)(currentHeight * 0.94)) {
                              if(bY >= caveBottomHeight && bY <= caveTopHeight)
                                  blocks[bY + (bZ * 128 + (bX * 128 * 16))] = new ChunkBlock(Block.Air, 0x00, 0x00);
                              else if (bY >= caveBottomHeight2 && bY <= caveTopHeight2)
                                  blocks[bY + (bZ * 128 + (bX * 128 * 16))] = new ChunkBlock(Block.Air, 0x00, 0x00);
                              else {
                                  int veinDir = world.rand.Next(0, 8);
                                  Block blockToPlace = Block.Stone;
                                  #region Ore
                                  for (int i = 0; i < numOre; i++) {
                                      if (oreType[i] == (byte)Block.DiamondOre || oreType[i] == (byte)Block.GoldOre) {    // Smaller deposits
                                          if (veinDir <= 2) {         // Cube (2x2)
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
                                          } else if (veinDir <= 4) {  // Long across X
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
                                          } else if (veinDir <= 6) {  // Long across Z
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
                                          } else if (veinDir <= 8) {  // Tall
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
                                      } else {                                                                            // Larger deposits
                                          if (veinDir <= 2) {         // Cube (2x2)
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
                                          } else if (veinDir <= 4) {  // Long across X
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
                                          } else if (veinDir <= 6) {  // Long across Z
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
                                          } else if (veinDir <= 8) {  // Tall
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
                                      }
                                      blockToPlace = (Block)oreType[i];
                                      break;
                                  }
                                  #endregion
                                  blocks[bY + (bZ * 128 + (bX * 128 * 16))] = new ChunkBlock(blockToPlace, 0x00, 0x00);
                              } 
                          } else
                              blocks[bY + (bZ * 128 + (bX * 128 * 16))] = new ChunkBlock(Block.Dirt, 0x00, 0x00);
                      } else if (currentHeight == bY) {
                          if (bY == world.seaLevel || bY == world.seaLevel - 1)
                              blocks[bY + (bZ * 128 + (bX * 128 * 16))] = new ChunkBlock(Block.Sand, 0x00, 0x00); // FF
                          else if (bY < world.seaLevel)
                              blocks[bY + (bZ * 128 + (bX * 128 * 16))] = new ChunkBlock(Block.Gravel, 0x00, 0x00); // FF
                          else
                              blocks[bY + (bZ * 128 + (bX * 128 * 16))] = new ChunkBlock(Block.Grass, 0x00, 0x00); // FF
                      } else {
                          if (bY <= world.seaLevel)
                              blocks[bY + (bZ * 128 + (bX * 128 * 16))] = new ChunkBlock(Block.Water, 0x00, 0x00); // FF
                          else
                              blocks[bY + (bZ * 128 + (bX * 128 * 16))] = new ChunkBlock(Block.Air, 0x00, 0x00); // FF
                      }
                  }
              }
          }
      }
      CalculateHeightmap();
  }
  hasChanged = true;
}

*/
