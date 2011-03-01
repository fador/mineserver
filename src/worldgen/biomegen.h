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

#ifndef _BIOMEGEN_H
#define _BIOMEGEN_H

#ifdef LIBNOISE
#include <libnoise/noise.h>
#else
#include <noise/noise.h>
#endif

#include "cavegen.h"
#include "mapgen.h"
#include "../map.h"

class BiomeGen: public MapGen
{
public:
  BiomeGen();
  void init(int seed);
  void re_init(int seed); // Used when generating multiple maps
  void generateChunk(int x, int z, int map);

private:
  std::vector<uint8_t> blocks;
  std::vector<uint8_t> blockdata;
  std::vector<uint8_t> skylight;
  std::vector<uint8_t> blocklight;
  std::vector<uint8_t> heightmap;

  int seaLevel;

  bool addTrees;

  bool expandBeaches;
  int beachExtent;
  int beachHeight;

  bool addOre;
  bool addCaves;
  bool winterEnabled;

  void generateFlatgrass(int x, int z, int map);
  void generateWithNoise(int x, int z, int map);

  void AddTrees(int x, int z, int map);

  void AddOre(int x, int z, int map, uint8_t type);
  void AddDeposit(int x, int y, int z, int map, uint8_t block, int minDepoSize, int maxDepoSize, sChunk* chunk);

  CaveGen cave;

  // Heightmap composition
  noise::module::Perlin BiomeBase;
  noise::module::ScaleBias BiomeSelect;
  noise::module::RidgedMulti mountainTerrainBase;
  noise::module::ScaleBias mountainTerrain;
  noise::module::Billow baseFlatTerrain;
  noise::module::ScaleBias flatTerrain;
  noise::module::Billow baseWater;
  noise::module::ScaleBias water;
  noise::module::Perlin terrainType;
  noise::module::Perlin terrainType2;
  noise::module::Select waterTerrain;
  noise::module::Select finalTerrain;
  noise::module::Voronoi flowers;
  noise::module::Select jaggieEdges;
  noise::module::Select secondTerrain;
  noise::module::Const plain;
  noise::module::Billow jaggieControl;
  // ##### TREE GEN #####
  noise::module::Billow treenoise;
  // ##### END TREE GEN ####
};

#endif
