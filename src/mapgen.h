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

#ifndef _MAPGEN_H
#define _MAPGEN_H

#ifdef DEBIAN
#include <libnoise/noise.h>
#else
#include <noise/noise.h>
#endif

#include "noiseutils.h"
#include "world/cavegen.h"

class MapGen
{
private:
  MapGen() {}

  uint8 blocks[16*16*128];
  uint8 blockdata[16*16*128/2];
  uint8 skylight[16*16*128/2];
  uint8 blocklight[16*16*128/2];
  uint8 heightmap[16*16];
  
  int m_seed;
  int seaLevel;

  float perlinScale;
  
  void generateFlatgrass();
  void generateWithNoise(int x, int z);
  
  void AddBeaches();
  
  CaveGen cave;
    
  noise::utils::NoiseMap heightMap;
  noise::utils::NoiseMapBuilderPlane heightMapBuilder;
  
  // This is for used for tuning heightmaps (Not for production)
  //noise::utils::NoiseMapBuilderPlane debugMapBuilder;
  //noise::utils::NoiseMap debugHeightMap;

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

  static MapGen *mMapGen;
public:
  static MapGen* get()
  {
     if(!mMapGen) {
        mMapGen = new MapGen();
     }
     return mMapGen;
  }

  void init(int seed);
  void free();
  void generateChunk(int x, int z);
};


#endif
