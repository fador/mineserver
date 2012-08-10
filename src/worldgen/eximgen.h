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

#ifndef _EXIMGEN_H
#define _EXIMGEN_H


#include "mapgen.h"

#include <iostream>

struct BlockRef{
    BlockRef(sChunk* chunk,int x, int y, int z){
        int index = x + (z<<4) + (y<<8);

        m_block = chunk->blocks+index;
        m_data  = chunk->data + (index>>1);

        meta_top = x & 1;
    }

    uint8_t id(){return *m_block;}
    uint8_t meta(){
        if(meta_top) return (*m_data)>>4;

        else return (*m_data)&0xf;

    }

    void id(uint8_t id){
        *m_block = id;
    }
    void meta(uint8_t meta){
        assert(meta <= 15);
        if(meta_top){
            *m_data &= 0xf;
            *m_data += meta<<4;
        }
        else{
            *m_data &= 0xf0;
            *m_data += meta;
        }
    }
private:
    uint8_t* m_block;
    bool     meta_top;
    uint8_t* m_data;
};

struct AnvilAccessor{
    AnvilAccessor(sChunk* chunk)
        :m_chunk(chunk){}

    BlockRef operator()(int x, int y, int z){
        return BlockRef(m_chunk, x, y, z);
    }

private:
    sChunk* m_chunk;
};
struct ChunkInfo{
    ChunkInfo(sChunk* chunk):chunk(chunk),blocks(chunk){}

    sChunk* chunk;
    AnvilAccessor blocks;
};

class EximGen: public MapGen
{
public:
    EximGen();
    void init(int seed);
    void re_init(int seed); // Used when generating multiple maps
    void generateChunk(int x, int z, int map);

private:
    std::vector<uint8_t> blocks;
    std::vector<uint8_t> blockdata;
    std::vector<uint8_t> skylight;
    std::vector<uint8_t> blocklight;
    std::vector<int32_t> heightmap;

    int seaLevel;

    bool addTrees;

    bool expandBeaches;
    int beachExtent;
    int beachHeight;

    bool addOre;
    bool addCaves;
    bool winterEnabled;

    void generateWithNoise(ChunkInfo &info);

    void AddOre  (sChunk* chunk);
    void AddTrees(sChunk* chunk);

    void ExpandBeaches(int x, int z, int map);
    void AddTrees(int x, int z, int map);

    void AddOre(int x, int z, int map, uint8_t type);
    void AddDeposit(int x, int y, int z, int map, uint8_t block, uint8_t minDepoSize, uint8_t maxDepoSize, sChunk* chunk);

    // Heightmap composition

    noise::module::RidgedMulti mountainTerrain;
    noise::module::ScaleBias mountainScale;
    noise::module::Billow baseFlatTerrain;
    noise::module::ScaleBias flatTerrain;
    noise::module::Billow terrainType;
    noise::module::Select terrainSelector;
    noise::module::ScaleBias finalTerrain;

    // stuff

    noise::module::RidgedMulti earthNoise;
    noise::module::Billow caveNoise;
    // ##### TREE GEN #####

    noise::module::Billow treenoise;
    // ##### END TREE GEN ####

};

#endif
