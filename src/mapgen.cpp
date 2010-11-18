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

#include "mapgenargs.h"
#include "mapgen.h"

MapGen::MapGen(MapGenArgs mArgs): WaterCoveragePasses(10),
                                  CliffsideBlockThreshold(0.01),
                                  groundThickness(5), 
                                  seaFloorThickness(3)
{
  args = mArgs;

  if( !args.customWaterLevel ) 
    args.waterLevel = (args.dimH - 1) / 2;

  // Seed random number generator
  //srand(args.seed);
  noise.init(args.seed, Noise::Bicubic);
  applyTheme(args.theme);
}


void MapGen::GenerateHeightmap() 
{
  //reportProgress(10, "Heightmap: Priming");
  std::cout << "Heightmap: Priming" << std::endl;
  
  float** heightmap = new float*[args.dimX];
  for(int i = 0; i < args.dimX; i++)
  {
    heightmap[i] = new float[args.dimY];
  }

  noise.PerlinNoiseMap(heightmap, args.featureScale, args.detailScale, args.roughness, 0, 0);
  
  /*if(args.useBias && !args.delayBias) 
  {
    //reportProgress(2, "Heightmap: Biasing");
    noise.Normalize(heightmap);
    applyBias();
  }*/

    for(int y=0; y<16;y++) {
    for(int x=0; x<16;x++) {
      std::cout << heightmap[x][y] << " ";
    }
    std::cout << std::endl;
  }
  noise.Normalize(heightmap);
  
     for(int y=0; y<16;y++) {
    for(int x=0; x<16;x++) {
      std::cout << heightmap[x][y] << " ";
    }
    std::cout << std::endl;
  }

  /*if(args.layeredHeightmap) 
  {
    //reportProgress(10, "Heightmap: Layering");

    // needs a new Noise object to randomize second map
    float** heightmap2 = new float*[args.dimX];
    for(int i = 0; i < args.dimX; i++)
    {
      heightmap2[i] = new float[args.dimY];
    }
    
    Noise tempNoise;
    tempNoise.init(rand(), Noise::Bicubic);
    tempNoise.PerlinNoiseMap( heightmap2, 0, args.detailScale, args.roughness, 0, 0 );
    noise.Normalize( heightmap2 );

    // make a blendmap
    blendmap = new float*[args.dimX];
    for(int i = 0; i < args.dimX; i++)
    {
      blendmap[i] = new float[args.dimY];
    }
    
    int blendmapDetailSize = (int)log2( (double)std::max( args.dimX, args.dimY ) ) - 2;
    
    tempNoise.init(rand(), Noise::Cosine);
    tempNoise.PerlinNoiseMap(blendmap, 3, blendmapDetailSize, 0.5f, 0, 0);
    
    noise.Normalize(blendmap);
    float cliffSteepness = DIMXY / 6.0f;
    noise.ScaleAndClip(blendmap, cliffSteepness);

    noise.Blend(heightmap, heightmap2, blendmap);
  }

  if( args.marbledHeightmap ) {
    //reportProgress(1, "Heightmap: Marbling");
    noise.Marble(heightmap);
  }

  if(args.invertHeightmap) {
    //reportProgress(1, "Heightmap: Inverting");
    noise.Invert(heightmap);
  }

  if(args.useBias && args.delayBias) {
    //reportProgress(2, "Heightmap: Biasing");
    noise.Normalize(heightmap);
    applyBias();
  }
  noise.Normalize(heightmap);
  */
}


void MapGen::applyBias() 
{
  // set corners and midpoint
  std::vector<float> corners;
  int c = 0;
  for(int i = 0; i < args.raisedCorners; i++) 
  {
    corners[c++] = args.bias;
  }
  for(int i = 0; i < args.loweredCorners; i++) 
  {
    corners[c++] = -args.bias;
  }
  float midpoint = (args.midPoint * args.bias);

  // shuffle corners
  std::random_shuffle(corners.begin(), corners.end());

  // overlay the bias
  noise.applyBias(heightmap, corners[0], corners[1], corners[2], corners[3], midpoint);
}

// assumes normalized heightmap
float MapGen::MatchWaterCoverage(float** heightmap, float desiredWaterCoverage) 
{
  if( desiredWaterCoverage == 0 ) 
    return 0.0;
  if( desiredWaterCoverage == 1 ) 
    return 1.0;
    
  float waterLevel = 0.5;
  for(int i = 0; i < WaterCoveragePasses; i++) 
  {
    if(CalculateWaterCoverage(heightmap, waterLevel) > desiredWaterCoverage) 
    {
      waterLevel = waterLevel - 1 / (float)(4 << i);
    } 
    else 
    {
      waterLevel = waterLevel + 1 / (float)(4 << i);
    }
  }
  return waterLevel;
}


float MapGen::CalculateWaterCoverage(float** heightmap, float waterLevel)
{
  int underwaterBlocks = 0;
  for(int x = args.dimX - 1; x >= 0; x--) 
  {
    for(int y = args.dimY - 1; y >= 0; y--) 
    {
      if(heightmap[x][y] < waterLevel) 
        underwaterBlocks++;
    }
  }
  return underwaterBlocks / (float)(args.dimX*args.dimY);
}


void MapGen::CalculateSlope(float** heightmap, float** slopemap)
{
}
       
//
// Erosion
//
/*void MapGen::Erode() 
{
  float[BLOCKCOUNT] volume;
  for( int i = 0; i < BLOCKCOUNT; i++ ) 
  {
    if( map.blocks[i] != BLOCK_AIR )
      volume[i] = 0.5;
  }
  ErosionPass(volume);
}

void MapGen::ErosionPass(float* volume) 
{
  for( int x = 0; x < map.widthX; x += 2) 
  {
    for( int y = 0; y < map.widthY; y+=2 )
      ErosionDrop( x, y, volume, map );
  }
  for( int x = 1; x < map.widthX; x += 2 ) 
  {
  for( int y = 1; y < map.widthY; y += 2 )
        ErosionDrop( x, y, volume, map );
  }
}

void ErosionDrop( int x, int y, float[] volume, Map map ) 
{
  WaterParticle particle = new WaterParticle() 
  {
    x = x,
    y = y,
    h = map.height,
    mass = .5f,
    deposit = 0,
    velocity = 0
  };
}*/

//
// Themes
//

void MapGen::applyTheme( MapGenTheme theme ) 
{
  args.theme = theme;
  switch( theme ) {
    case Arctic:
      bWaterSurface = BLOCK_GLASS;
      bDeepWaterSurface = BLOCK_WATER;
      bGroundSurface = BLOCK_SNOW;
      bWater = BLOCK_WATER;
      bGround = BLOCK_SNOW;
      bSeaFloor = BLOCK_SNOW;
      bBedrock = BLOCK_STONE;
      bCliff = BLOCK_STONE;
      groundThickness = 1;
      break;
    case Desert:
      bWaterSurface = BLOCK_WATER;
      bDeepWaterSurface = BLOCK_WATER;
      bGroundSurface = BLOCK_SAND;
      bWater = BLOCK_WATER;
      bGround = BLOCK_SAND;
      bSeaFloor = BLOCK_SAND;
      bBedrock = BLOCK_STONE;
      bCliff = BLOCK_GRAVEL;
      break;
    case Hell:
      bWaterSurface = BLOCK_LAVA;
      bDeepWaterSurface = BLOCK_LAVA;
      bGroundSurface = BLOCK_OBSIDIAN;
      bWater = BLOCK_LAVA;
      bGround = BLOCK_STONE;
      bSeaFloor = BLOCK_OBSIDIAN;
      bBedrock = BLOCK_STONE;
      bCliff = BLOCK_STONE;
      break;
    case Forest:
      bWaterSurface = BLOCK_WATER;
      bDeepWaterSurface = BLOCK_WATER;
      bGroundSurface = BLOCK_GRASS;
      bWater = BLOCK_WATER;
      bGround = BLOCK_DIRT;
      bSeaFloor = BLOCK_SAND;
      bBedrock = BLOCK_STONE;
      bCliff = BLOCK_STONE;
      break;
    case Swamp:
      bWaterSurface = BLOCK_WATER;
      bDeepWaterSurface = BLOCK_WATER;
      bGroundSurface = BLOCK_DIRT;
      bWater = BLOCK_WATER;
      bGround = BLOCK_DIRT;
      bSeaFloor = BLOCK_LEAVES;
      bBedrock = BLOCK_STONE;
      bCliff = BLOCK_STONE;
      break;
  }
}


MapGenArgs MakeTemplate( MapGenTemplate temp ) {
  MapGenArgs tempArgs;
  switch( temp ) {
    case Archipelago:
      tempArgs.maxHeight = 8;
      tempArgs.maxDepth = 20;
      tempArgs.featureScale = 3;
      tempArgs.roughness = .46;
      tempArgs.matchWaterCoverage = true;
      tempArgs.waterCoverage = .85;
      return tempArgs;
    
    case Atoll:     
      tempArgs.theme = Desert;
      tempArgs.maxHeight = 2;
      tempArgs.maxDepth = 39;
      tempArgs.useBias = true;
      tempArgs.bias = .9f;
      tempArgs.midPoint = 1;
      tempArgs.loweredCorners = 4;
      tempArgs.featureScale = 2;
      tempArgs.detailScale = 5;
      tempArgs.marbledHeightmap = true;
      tempArgs.invertHeightmap = true;
      tempArgs.matchWaterCoverage = true;
      tempArgs.waterCoverage = .95f;
      return tempArgs;
    
    case Bay:  
      tempArgs.maxHeight = 22;
      tempArgs.maxDepth = 12;
      tempArgs.useBias = true;
      tempArgs.bias = 1;
      tempArgs.midPoint = -1;
      tempArgs.raisedCorners = 3;
      tempArgs.loweredCorners = 1;
      tempArgs.treeSpacingMax = 12;
      tempArgs.treeSpacingMin = 6;
      tempArgs.marbledHeightmap = true;
      tempArgs.delayBias = true;
      return tempArgs;

    case Default:  
      return tempArgs;

    case Dunes:
      tempArgs.addTrees = false;
      tempArgs.addWater = false;
      tempArgs.theme = Desert;
      tempArgs.maxHeight = 12;
      tempArgs.maxDepth = 7;
      tempArgs.featureScale = 2;
      tempArgs.detailScale = 3;
      tempArgs.roughness = .44f;
      tempArgs.marbledHeightmap = true;
      tempArgs.invertHeightmap = true;
      return tempArgs;


    case Hills:
      tempArgs.addWater = false;
      tempArgs.maxHeight = 8;
      tempArgs.maxDepth = 8;
      tempArgs.featureScale = 2;
      tempArgs.treeSpacingMin = 7;
      tempArgs.treeSpacingMax = 13;
      return tempArgs;

    case Ice:
      tempArgs.addTrees = false;
      tempArgs.theme = Arctic;
      tempArgs.maxHeight = 2;
      tempArgs.maxDepth = 2032;
      tempArgs.featureScale = 2;
      tempArgs.detailScale = 7;
      tempArgs.roughness = .64f;
      tempArgs.marbledHeightmap = true;
      tempArgs.matchWaterCoverage = true;
      tempArgs.waterCoverage = .3f;
      tempArgs.maxHeightVariation = 0;
      return tempArgs;

    case Island:
      tempArgs.maxHeight = 16;
      tempArgs.maxDepth = 39;
      tempArgs.useBias = true;
      tempArgs.bias = .7f;
      tempArgs.midPoint = 1;
      tempArgs.loweredCorners = 4;
      tempArgs.featureScale = 3;
      tempArgs.detailScale = 7;
      tempArgs.marbledHeightmap = true;
      tempArgs.delayBias = true;
      return tempArgs;

    case Lake:
      tempArgs.maxHeight = 14;
      tempArgs.maxDepth = 20;
      tempArgs.useBias = true;
      tempArgs.bias = .65f;
      tempArgs.midPoint = -1;
      tempArgs.raisedCorners = 4;
      tempArgs.featureScale = 2;
      tempArgs.roughness = .56f;
      tempArgs.matchWaterCoverage = true;
      tempArgs.waterCoverage = .3f;
      return tempArgs;

    case Mountains:
      tempArgs.addWater = false;
      tempArgs.maxHeight = 40;
      tempArgs.maxDepth = 10;
      tempArgs.featureScale = 1;
      tempArgs.detailScale = 7;
      tempArgs.marbledHeightmap = true;
      return tempArgs;

    case River:
      tempArgs.maxHeight = 22;
      tempArgs.maxDepth = 8;
      tempArgs.featureScale = 0;
      tempArgs.detailScale = 6;
      tempArgs.marbledHeightmap = true;
      tempArgs.matchWaterCoverage = true;
      tempArgs.waterCoverage = .31f;
      return tempArgs;

    case Streams:
      tempArgs.maxHeight = 5;
      tempArgs.maxDepth = 4;
      tempArgs.featureScale = 2;
      tempArgs.detailScale = 7;
      tempArgs.roughness = .55f;
      tempArgs.marbledHeightmap = true;
      tempArgs.matchWaterCoverage = true;
      tempArgs.waterCoverage = .25f;
      tempArgs.treeSpacingMin = 8;
      tempArgs.treeSpacingMax = 14;
      return tempArgs;

    case Peninsula:
      tempArgs.maxHeight = 22;
      tempArgs.maxDepth = 12;
      tempArgs.useBias = true;
      tempArgs.bias = .5f;
      tempArgs.midPoint = -1;
      tempArgs.raisedCorners = 3;
      tempArgs.loweredCorners = 1;
      tempArgs.treeSpacingMax = 12;
      tempArgs.treeSpacingMin = 6;
      tempArgs.invertHeightmap = true;
      tempArgs.waterCoverage = .5f;
      return tempArgs;

    case Flat:
      tempArgs.maxHeight = 0;
      tempArgs.maxDepth = 0;
      tempArgs.maxHeightVariation = 0;
      tempArgs.addWater = false;
      tempArgs.detailScale = 0;
      tempArgs.featureScale = 0;
      tempArgs.addCliffs = false;
      return tempArgs;
  }
  return tempArgs; // can never happen
}


/*void GenerateFlatgrass( Map map ) {
    for( int i = 0; i < map.widthX; i++ ) {
        for( int j = 0; j < map.widthY; j++ ) {
            for( int k = 0; k < map.height / 2 - 1; k++ ) {
                if( k < map.height / 2 - 5 ) {
                    map.SetBlock( i, j, k, BLOCK_STONE );
                } else {
                    map.SetBlock( i, j, k, BLOCK_DIRT );
                }
            }
            map.SetBlock( i, j, map.height / 2 - 1, BLOCK_GRASS );
        }
    }
}*/
/*
Map GenerateMap() 
{
    Map map = new Map( null, args.dimX, args.dimY, args.dimH );


    // Match water coverage
    float desiredWaterLevel = .5f;
    if( args.matchWaterCoverage ) 
    {
        ReportProgress( 2, "Heightmap Processing: Matching water coverage" );
        desiredWaterLevel = MatchWaterCoverage( heightmap, args.waterCoverage );
    }


    // Calculate above/below water multipliers
    float underWaterMultiplier = 0, aboveWaterMultiplier = 0;
    if( desiredWaterLevel != 0 ) 
    {
        underWaterMultiplier = (float)(args.maxDepth / desiredWaterLevel);
    }
    if( desiredWaterLevel != 1 ) 
    {
        aboveWaterMultiplier = (float)(args.maxHeight / (1 - desiredWaterLevel));
    }


    // Apply power functions to above/below water parts of the heightmap
    if( args.belowFuncExponent != 1 || args.aboveFuncExponent != 1 ) {
        ReportProgress( 5, "Heightmap Processing: Adjusting slope" );
        for( int x = heightmap.GetLength( 0 ) - 1; x >= 0; x-- ) {
            for( int y = heightmap.GetLength( 1 ) - 1; y >= 0; y-- ) {
                if( heightmap[x, y] < desiredWaterLevel ) {
                    float normalizedDepth = 1 - heightmap[x, y] / desiredWaterLevel;
                    heightmap[x, y] = desiredWaterLevel - (float)Math.Pow( normalizedDepth, args.belowFuncExponent ) * desiredWaterLevel;
                } else {
                    float normalizedHeight = (heightmap[x, y] - desiredWaterLevel) / (1 - desiredWaterLevel);
                    heightmap[x, y] = desiredWaterLevel + (float)Math.Pow( normalizedHeight, args.aboveFuncExponent ) * (1 - desiredWaterLevel);
                }
            }
        }
    }

    // Calculate the slope
    if( args.cliffSmoothing ) {
        ReportProgress( 2, "Heightmap Processing: Smoothing" );
        slopemap = Noise.CalculateSlope( Noise.GaussianBlur5x5( heightmap ) );
    } else {
        slopemap = Noise.CalculateSlope( heightmap );
    }

    int level;
    float slope;

    float[,] altmap = null;
    if( args.maxHeightVariation != 0 || args.maxDepthVariation != 0 ) {
        ReportProgress( 5, "Heightmap Processing: Randomizing" );
        altmap = new float[map.widthX, map.widthY];
        int blendmapDetailSize = (int)Math.Log( (double)Math.Max( args.dimX, args.dimY ), 2 ) - 2;
        new Noise( rand.Next(), NoiseInterpolationMode.Cosine ).PerlinNoiseMap( altmap, 3, blendmapDetailSize, 0.5f, 0, 0 );
        Noise.Normalize( altmap, -1, 1 );
    }

    int snowStartThreshold = args.snowAltitude - args.snowTransition;
    int snowThreshold = args.snowAltitude;

    ReportProgress( 10, "Filling" );
    for( int x = heightmap.GetLength( 0 ) - 1; x >= 0; x-- ) {
        for( int y = heightmap.GetLength( 1 ) - 1; y >= 0; y-- ) {

            if( heightmap[x, y] < desiredWaterLevel ) {
                float depth = (args.maxDepthVariation != 0 ? (args.maxDepth + altmap[x, y] * args.maxDepthVariation) : args.maxDepth);
                slope = slopemap[x, y] * depth;
                level = args.waterLevel - (int)Math.Round( Math.Pow( 1 - heightmap[x, y] / desiredWaterLevel, args.belowFuncExponent ) * depth );

                if( args.addWater ) {
                    if( args.waterLevel - level > 3 ) {
                        map.SetBlock( x, y, args.waterLevel, bDeepWaterSurface );
                    } else {
                        map.SetBlock( x, y, args.waterLevel, bWaterSurface );
                    }
                    for( int i = args.waterLevel; i > level; i-- ) {
                        map.SetBlock( x, y, i, bWater );
                    }
                    for( int i = level; i >= 0; i-- ) {
                        if( level - i < seaFloorThickness ) {
                            map.SetBlock( x, y, i, bSeaFloor );
                        } else {
                            map.SetBlock( x, y, i, bBedrock );
                        }
                    }
                } else {
                    if( blendmap != null && blendmap[x, y] > .25 && blendmap[x, y] < .75 ) {
                        map.SetBlock( x, y, level, bCliff );
                    } else {
                        if( slope < args.cliffThreshold ) {
                            map.SetBlock( x, y, level, bGroundSurface );
                        } else {
                            map.SetBlock( x, y, level, bCliff );
                        }
                    }

                    for( int i = level - 1; i >= 0; i-- ) {
                        if( level - i < groundThickness ) {
                            if( blendmap != null && blendmap[x, y] > CliffsideBlockThreshold && blendmap[x, y] < (1 - CliffsideBlockThreshold) ) {
                                map.SetBlock( x, y, i, bCliff );
                            } else {
                                if( slope < args.cliffThreshold ) {
                                    map.SetBlock( x, y, i, bGround );
                                } else {
                                    map.SetBlock( x, y, i, bCliff );
                                }
                            }
                        } else {
                            map.SetBlock( x, y, i, bBedrock );
                        }
                    }
                }

            } else {
                float height = (args.maxHeightVariation != 0 ? (args.maxHeight + altmap[x, y] * args.maxHeightVariation) : args.maxHeight);
                slope = slopemap[x, y] * height;
                if( height != 0 ) {
                    level = args.waterLevel + (int)Math.Round( Math.Pow( heightmap[x, y] - desiredWaterLevel, args.aboveFuncExponent ) * aboveWaterMultiplier / args.maxHeight * height );
                } else {
                    level = args.waterLevel;
                }

                bool snow = args.addSnow &&
                            (level > snowThreshold ||
                            (level > snowStartThreshold && rand.NextDouble() < (level - snowStartThreshold) / (double)(snowThreshold - snowStartThreshold)));

                if( blendmap != null && blendmap[x, y] > .25 && blendmap[x, y] < .75 ) {
                    map.SetBlock( x, y, level, bCliff );
                } else {
                    if( slope < args.cliffThreshold ) {
                        if( snow ) {
                            map.SetBlock( x, y, level, Block.White );
                        } else {
                            map.SetBlock( x, y, level, bGroundSurface );
                        }
                    } else {
                        map.SetBlock( x, y, level, bCliff );
                    }
                }

                for( int i = level - 1; i >= 0; i-- ) {
                    if( level - i < groundThickness ) {
                        if( blendmap != null && blendmap[x, y] > CliffsideBlockThreshold && blendmap[x, y] < (1 - CliffsideBlockThreshold) ) {
                            map.SetBlock( x, y, i, bCliff );
                        } else {
                            if( slope < args.cliffThreshold ) {
                                if( snow ) {
                                    map.SetBlock( x, y, i, Block.White );
                                } else {
                                    map.SetBlock( x, y, i, bGround );
                                }
                            } else {
                                map.SetBlock( x, y, i, bCliff );
                            }
                        }
                    } else {
                        map.SetBlock( x, y, i, bBedrock );
                    }
                }
            }
        }
    }

    if( args.addCaves || args.addOre ) {
        AddCaves( map );
    }

    if( args.addBeaches ) {
        ReportProgress( 10, "Processing: Adding beaches" );
        AddBeaches( map );
    }

    if( args.addTrees ) {
        ReportProgress( 5, "Processing: Planting trees" );
        Map outMap = new Map();
        outMap.blocks = (byte[])map.blocks.Clone();
        outMap.widthX = map.widthX;
        outMap.widthY = map.widthY;
        outMap.height = map.height;

        Forester treeGen = new Forester( new Forester.ForesterArgs {
            inMap = map,
            outMap = outMap,
            rand = rand,
            TREECOUNT = (int)(map.widthX * map.widthY * 4 / (1024.0f * (args.treeSpacingMax + args.treeSpacingMin) / 2)),
            OPERATION = Forester.Operation.Add,
            bGroundSurface = bGroundSurface
        } );
        treeGen.Generate();
        map = outMap;

        GenerateTrees( map );
    }

    ReportProgress( 0, "Generation complete" );
    map.ResetSpawn();
    return map;
}


// Cave generation method from Omen 0.70, used with osici's permission
void AddSingleCave( Random rand, Map map, byte bedrockType, byte fillingType, int length, double maxDiameter ) {

    int startX = rand.Next( 0, map.widthX );
    int startY = rand.Next( 0, map.widthY );
    int startH = rand.Next( 0, map.height );

    int k1;
    for( k1 = 0; map.blocks[startX + map.widthX * map.widthY * (map.height - 1 - startH) + map.widthX * startY] != bedrockType && k1 < 10000; k1++ ) 
    {
        startX = rand.Next( 0, map.widthX );
        startY = rand.Next( 0, map.widthY );
        startH = rand.Next( 0, map.height );
    }

    if( k1 >= 10000 )
        return;

    int x = startX;
    int y = startY;
    int h = startH;

    for( int k2 = 0; k2 < length; k2++ ) {
        int diameter = (int)(maxDiameter * rand.NextDouble() * map.widthX);
        if( diameter < 1 ) diameter = 2;
        int radius = diameter / 2;
        if( radius == 0 ) radius = 1;
        x += (int)(0.7 * (rand.NextDouble() - 0.5) * diameter);
        y += (int)(0.7 * (rand.NextDouble() - 0.5) * diameter);
        h += (int)(0.7 * (rand.NextDouble() - 0.5) * diameter);

        for( int j3 = 0; j3 < diameter; j3++ ) {
            for( int k3 = 0; k3 < diameter; k3++ ) {
                for( int l3 = 0; l3 < diameter; l3++ ) {
                    if( (j3 - radius) * (j3 - radius) + (k3 - radius) * (k3 - radius) + (l3 - radius) * (l3 - radius) >= radius * radius ||
                        x + j3 >= map.widthX || h + k3 >= map.height || y + l3 >= map.widthY ||
                        x + j3 < 0 || h + k3 < 0 || y + l3 < 0 ) {
                        continue;
                    }

                    int index = x + j3 + map.widthX * map.widthY * (map.height - 1 - (h + k3)) + map.widthX * (y + l3);

                    if( map.blocks[index] == bedrockType ) {
                        map.blocks[index] = (byte)fillingType;
                    }
                    if( (fillingType == 10 || fillingType == 11 || fillingType == 8 || fillingType == 9) &&
                        h + k3 < startH ) {
                        map.blocks[index] = 0;
                    }
                }
            }
        }
    }
}

void AddSingleVein( Random rand, Map map, byte bedrockType, byte fillingType, int k, double maxDiameter, int l ) {
    AddSingleVein( rand, map, bedrockType, fillingType, k, maxDiameter, l, 10 );
}

void AddSingleVein( Random rand, Map map, byte bedrockType, byte fillingType, int k, double maxDiameter, int l, int i1 ) {

    int j1 = rand.Next( 0, map.widthX );
    int k1 = rand.Next( 0, map.height );
    int l1 = rand.Next( 0, map.widthY );

    double thirteenOverK = 1 / (double)k;

    for( int i2 = 0; i2 < i1; i2++ ) {
        int j2 = j1 + (int)(.5 * (rand.NextDouble() - .5) * (double)map.widthX);
        int k2 = k1 + (int)(.5 * (rand.NextDouble() - .5) * (double)map.height);
        int l2 = l1 + (int)(.5 * (rand.NextDouble() - .5) * (double)map.widthY);
        for( int l3 = 0; l3 < k; l3++ ) {
            int diameter = (int)(maxDiameter * rand.NextDouble() * map.widthX);
            if( diameter < 1 ) diameter = 2;
            int radius = diameter / 2;
            if( radius == 0 ) radius = 1;
            int i3 = (int)((1 - thirteenOverK) * (double)j1 + thirteenOverK * (double)j2 + (double)(l * radius) * (rand.NextDouble() - .5));
            int j3 = (int)((1 - thirteenOverK) * (double)k1 + thirteenOverK * (double)k2 + (double)(l * radius) * (rand.NextDouble() - .5));
            int k3 = (int)((1 - thirteenOverK) * (double)l1 + thirteenOverK * (double)l2 + (double)(l * radius) * (rand.NextDouble() - .5));
            for( int k4 = 0; k4 < diameter; k4++ ) {
                for( int l4 = 0; l4 < diameter; l4++ ) {
                    for( int i5 = 0; i5 < diameter; i5++ ) {
                        if( (k4 - radius) * (k4 - radius) + (l4 - radius) * (l4 - radius) + (i5 - radius) * (i5 - radius) < radius * radius &&
                            i3 + k4 < map.widthX && j3 + l4 < map.height && k3 + i5 < map.widthY &&
                            i3 + k4 >= 0 && j3 + l4 >= 0 && k3 + i5 >= 0 ) {

                            int index = i3 + k4 + map.widthX * map.widthY * (map.height - 1 - (j3 + l4)) + map.widthX * (k3 + i5);

                            if( map.blocks[index] == bedrockType ) {
                                map.blocks[index] = fillingType;
                            }
                        }
                    }
                }
            }
        }
        j1 = j2;
        k1 = k2;
        l1 = l2;
    }
}

void SealLiquids( Map map, byte sealantType ) {
    for( int x = 1; x < map.widthX - 1; x++ ) {
        for( int h = 1; h < map.height; h++ ) {
            for( int y = 1; y < map.widthY - 1; y++ ) {
                int index = map.Index( x, y, h );
                if( (map.blocks[index] == 10 || map.blocks[index] == 11 || map.blocks[index] == 8 || map.blocks[index] == 9) &&
                    (map.GetBlock( x - 1, y, h ) == 0 || map.GetBlock( x + 1, y, h ) == 0 ||
                    map.GetBlock( x, y - 1, h ) == 0 || map.GetBlock( x, y + 1, h ) == 0 ||
                    map.GetBlock( x, y, h - 1 ) == 0) ) {
                    map.blocks[index] = sealantType;
                }
            }
        }
    }
}

void AddCaves( Map map ) {
    Random rand = new Random();

    if( args.addCaves ) {
        ReportProgress( 5, "Processing: Adding caves" );
        for( int i1 = 0; i1 < 36 * args.caveDensity; i1++ )
            AddSingleCave( rand, map, (byte)bBedrock, (byte)Block.Air, 30, 0.05 * args.caveSize );

        for( int j1 = 0; j1 < 9 * args.caveDensity; j1++ )
            AddSingleVein( rand, map, (byte)bBedrock, (byte)Block.Air, 500, 0.015 * args.caveSize, 1 );

        for( int k1 = 0; k1 < 30 * args.caveDensity; k1++ )
            AddSingleVein( rand, map, (byte)bBedrock, (byte)Block.Air, 300, 0.03 * args.caveSize, 1, 20 );


        if( args.addCaveLava ) {
            for( int i = 0; i < 8 * args.caveDensity; i++ ) {
                AddSingleCave( rand, map, (byte)bBedrock, (byte)BLOCK_LAVA, 30, 0.05 * args.caveSize );
            }
            for( int j = 0; j < 3 * args.caveDensity; j++ ) {
                AddSingleVein( rand, map, (byte)bBedrock, (byte)BLOCK_LAVA, 1000, 0.015 * args.caveSize, 1 );
            }
        }


        if( args.addCaveWater ) {
            for( int k = 0; k < 8 * args.caveDensity; k++ ) {
                AddSingleCave( rand, map, (byte)bBedrock, (byte)BLOCK_WATER, 30, 0.05 * args.caveSize );
            }
            for( int l = 0; l < 3 * args.caveDensity; l++ ) {
                AddSingleVein( rand, map, (byte)bBedrock, (byte)BLOCK_WATER, 1000, 0.015 * args.caveSize, 1 );
            }
        }

        SealLiquids( map, (byte)bBedrock );
    }


    if( args.addOre ) {
        ReportProgress( 3, "Processing: Adding ore" );
        for( int l1 = 0; l1 < 12 * args.caveDensity; l1++ ) {
            AddSingleCave( rand, map, (byte)bBedrock, (byte)Block.Coal, 500, 0.03 );
        }

        for( int i2 = 0; i2 < 32 * args.caveDensity; i2++ ) {
            AddSingleVein( rand, map, (byte)bBedrock, (byte)Block.Coal, 200, 0.015, 1 );
            AddSingleCave( rand, map, (byte)bBedrock, (byte)Block.IronOre, 500, 0.02 );
        }

        for( int k2 = 0; k2 < 8 * args.caveDensity; k2++ ) {
            AddSingleVein( rand, map, (byte)bBedrock, (byte)Block.IronOre, 200, 0.015, 1 );
            AddSingleVein( rand, map, (byte)bBedrock, (byte)Block.GoldOre, 200, 0.0145, 1 );
        }

        for( int l2 = 0; l2 < 20 * args.caveDensity; l2++ ) {
            AddSingleCave( rand, map, (byte)bBedrock, (byte)Block.GoldOre, 400, 0.0175 );
        }
    }
}

void AddBeaches( Map map ) {
    int beachExtentSqr = (args.beachExtent + 1) * (args.beachExtent + 1);
    for( int x = 0; x < map.widthX; x++ ) {
        for( int y = 0; y < map.widthY; y++ ) {
            int h = map.SearchColumn( x, y, bGroundSurface );
            if( h < 0 ) continue;
            bool found = false;
            for( int dx = -args.beachExtent; !found && dx <= args.beachExtent; dx++ ) {
                for( int dy = -args.beachExtent; !found && dy <= args.beachExtent; dy++ ) {
                    for( int dh = -args.beachHeight; !found && dh <= 0; dh++ ) {
                        if( dx * dx + dy * dy + dh * dh > beachExtentSqr ) continue;
                        int xx = x + dx;
                        int yy = y + dy;
                        int hh = h + dh;
                        if( xx < 0 || xx >= map.widthX || yy < 0 || yy >= map.widthY || hh < 0 || hh >= map.height ) continue;
                        byte block = map.GetBlock( xx, yy, hh );
                        if( block == (byte)bWater || block == (byte)bWaterSurface ) {
                            found = true;
                            break;
                        }
                    }
                }
            }
            if( found ) {
                map.SetBlock( x, y, h, bSeaFloor );
                if( h > 0 && map.GetBlock( x, y, h - 1 ) == (byte)bGround ) map.SetBlock( x, y, h - 1, bSeaFloor );
            }
        }
    }
}


void GenerateTrees( Map map ) {
    int MinHeight = args.treeHeightMin;
    int MaxHeight = args.treeHeightMax;
    int MinTrunkPadding = args.treeSpacingMin;
    int MaxTrunkPadding = args.treeSpacingMax;
    int TopLayers = 2;
    double Odds = 0.618;
    bool OnlyAir = true;

    Random rn = new Random();
    int nx, ny, nz, nh;
    int radius;

    map.CalculateShadows();

    for( int x = 0; x < map.widthX; x += rn.Next( MinTrunkPadding, MaxTrunkPadding + 1 ) ) {
        for( int y = 0; y < map.widthY; y += rn.Next( MinTrunkPadding, MaxTrunkPadding + 1 ) ) {
            nx = x + rn.Next( -(MinTrunkPadding / 2), (MaxTrunkPadding / 2) + 1 );
            ny = y + rn.Next( -(MinTrunkPadding / 2), (MaxTrunkPadding / 2) + 1 );
            if( nx < 0 || nx >= map.widthX || ny < 0 || ny >= map.widthY ) continue;
            nz = map.shadows[nx, ny];

            if( (map.GetBlock( nx, ny, nz ) == (byte)bGroundSurface) && slopemap[nx, ny] < .5 ) {
                // Pick a random height for the tree between Min and Max,
                // discarding this tree if it would breach the top of the map
                if( (nh = rn.Next( MinHeight, MaxHeight + 1 )) + nz + nh / 2 > map.height )
                    continue;

                // Generate the trunk of the tree
                for( int z = 1; z <= nh; z++ )
                    map.SetBlock( nx, ny, nz + z, Block.Log );

                for( int i = -1; i < nh / 2; i++ ) {
                    // Should we draw thin (2x2) or thicker (4x4) foliage
                    radius = (i >= (nh / 2) - TopLayers) ? 1 : 2;
                    // Draw the foliage
                    for( int xoff = -radius; xoff < radius + 1; xoff++ ) {
                        for( int yoff = -radius; yoff < radius + 1; yoff++ ) {
                            // Drop random leaves from the edges
                            if( rn.NextDouble() > Odds && Math.Abs( xoff ) == Math.Abs( yoff ) && Math.Abs( xoff ) == radius )
                                continue;
                            // By default only replace an existing block if its air
                            if( OnlyAir != true || map.GetBlock( nx + xoff, ny + yoff, nz + nh + i ) == (byte)Block.Air )
                                map.SetBlock( nx + xoff, ny + yoff, nz + nh + i, BLOCK_LEAVES );
                        }
                    }
                }
            }
        }
    }
}
*/