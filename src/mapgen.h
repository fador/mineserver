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

#ifndef _MAPGEN_H
#define _MAPGEN_H

#ifdef WIN32
#include <noise/noise.h>
#else
#include <libnoise/noise.h>
#endif
#include "noiseutils.h"

class MapGen
{
private:
  uint8 *blocks;
  uint8 *blockdata;
  uint8 *skylight;
  uint8 *blocklight;
  uint8 *heightmap;

  /*float** heightMap;
  float** steepnessMap;
  float** caveTop;
  float** caveBottom;
  float** caveTop2;
  float** caveBottom2;*/
  
  int m_seed;
  int oreDensity;
  int seaLevel;

  float perlinScale;
  
  //int getHeightmapIndex(char x, char z);
  //void calculateHeightmap();
  
  void loadFlatgrass();
  void generateWithNoise(int x, int z);
    
  noise::utils::NoiseMap heightMap;
  noise::utils::NoiseMapBuilderPlane heightMapBuilder;
  
  // This is for used for tuning heightmaps (Not for production)
  noise::utils::NoiseMapBuilderPlane debugMapBuilder;
  noise::utils::NoiseMap debugHeightMap;

  // Heightmap composition
  noise::module::Perlin perlinNoise;
  noise::module::ScaleBias perlinBiased;

  noise::module::Perlin baseFlatTerrain;  
  noise::module::ScaleBias flatTerrain;
  
  noise::module::Perlin seaFloor;
  noise::module::ScaleBias seaBias;

  noise::module::Perlin terrainType;

  noise::module::Perlin seaControl;
  
  noise::module::Select seaTerrain;
  noise::module::Select finalTerrain;

public:
  MapGen(int seed);
  ~MapGen();  
  void generateChunk(int x, int z);

};


#endif
