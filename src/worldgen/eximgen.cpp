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

#include <cassert>

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
    seaLevel = ServerInstance->config()->iData("mapgen.sea.level");
    addTrees = ServerInstance->config()->bData("mapgen.trees.enabled");
    expandBeaches = ServerInstance->config()->bData("mapgen.beaches.expand");
    beachExtent = ServerInstance->config()->iData("mapgen.beaches.extent");
    beachHeight = ServerInstance->config()->iData("mapgen.beaches.height");

    addOre = ServerInstance->config()->bData("mapgen.addore");
    addCaves = ServerInstance->config()->bData("mapgen.caves.enabled");

    winterEnabled = ServerInstance->config()->bData("mapgen.winter.enabled");


    //cave.init(seed + 7);

    int upperLevel = 255-seaLevel;

    mountainTerrain.SetSeed(seed);
    mountainTerrain.SetFrequency(1.0/180.0);
    mountainTerrain.SetOctaveCount(6);

    mountainScale.SetSourceModule(0, mountainTerrain);
    mountainScale.SetScale(upperLevel * 0.4);
    mountainScale.SetBias(upperLevel * 0.4 + seaLevel);

    baseFlatTerrain.SetSeed(seed);
    baseFlatTerrain.SetFrequency(1.0/512.0);
    baseFlatTerrain.SetOctaveCount(6);
    baseFlatTerrain.SetPersistence(0.3);

    flatTerrain.SetSourceModule(0, baseFlatTerrain);
    flatTerrain.SetScale(seaLevel / 2.0);
    flatTerrain.SetBias(seaLevel / 2.0 + 40);

    terrainType.SetSeed(seed);
    terrainType.SetFrequency(1.0/512.0);
    terrainType.SetOctaveCount(4);
    terrainType.SetPersistence(0.3);

    terrainSelector.SetSourceModule(0, flatTerrain);
    terrainSelector.SetSourceModule(1, mountainScale);
    terrainSelector.SetControlModule(terrainType);
    terrainSelector.SetBounds(0.3, 1.1);
    terrainSelector.SetEdgeFalloff(0.25);

    finalTerrain.SetSourceModule(0, terrainSelector);
    finalTerrain.SetScale(1.0);
    finalTerrain.SetBias(0);

    treenoise.SetSeed(seed + 404);
    treenoise.SetFrequency(1.0/64);
    treenoise.SetOctaveCount(3);

    earthNoise.SetSeed(seed + 900);
    earthNoise.SetFrequency(1.0/32);
    earthNoise.SetOctaveCount(5);

    caveNoise.SetSeed(seed + 1000);
    caveNoise.SetFrequency(1.0/20);
    caveNoise.SetOctaveCount(2);

}

void EximGen::re_init(int seed)
{
    //cave.init(seed + 7);

    mountainTerrain.SetSeed(seed);
    baseFlatTerrain.SetSeed(seed);
    terrainType.SetSeed(seed);

    treenoise.SetSeed(seed + 404);
    earthNoise.SetSeed(seed + 900);
    caveNoise.SetSeed(seed + 1000);
}


void EximGen::generateChunk(int x, int z, int map)
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

    NBT_Value* val1 = (*val)["HeightMap"];
    chunk->heightmap = val1->GetIntArray()->data();
    chunk->nbt = main;
    chunk->x = x<<4;
    chunk->z = z<<4;
    ServerInstance->map(map)->chunks.insert(ChunkMap::value_type(ChunkMap::key_type(x, z), chunk));

    memset(chunk->blocks, 0, 16*16*256);
    memset(chunk->addblocks, 0, 16*16*256/2);
    memset(chunk->data, 0, 16*16*256/2);
    memset(chunk->blocklight, 0, 16*16*256/2);
    memset(chunk->skylight, 0, 16*16*256/2);
    chunk->chunks_present = 0xffff;

    ChunkInfo info(chunk);


    if (ServerInstance->config()->bData("mapgen.flatgrass"))
    {
        generateFlatgrass(x, z, map);
    }
    else
    {
        generateWithNoise(info);
    }


    // Not changed
    chunk->changed = ServerInstance->config()->bData("map.save_unchanged_chunks");

    if (false && addOre)
    {
        AddOre(x, z, map, BLOCK_COAL_ORE);
        AddOre(x, z, map, BLOCK_IRON_ORE);
        AddOre(x, z, map, BLOCK_GOLD_ORE);
        AddOre(x, z, map, BLOCK_DIAMOND_ORE);
        AddOre(x, z, map, BLOCK_REDSTONE_ORE);
        AddOre(x, z, map, BLOCK_LAPIS_ORE);
    }

    //AddOre(x, z, map, BLOCK_GRAVEL);
    //AddOre(x, z, map, BLOCK_DIRT); // guess what, dirt also exists underground

    // Add trees
    if (addTrees)
    {
        //AddTrees(x, z, map);
    }

    if (expandBeaches)
    {
        //ExpandBeaches(x, z, map);
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

    uint8_t trees = uint8_t(uniform01() * 7 + 13);
    uint8_t i = 0;
    while (i < trees)
    {
        uint8_t a = uint8_t(uniform01() * 16);
        uint8_t b = uint8_t(uniform01() * 16);

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

void EximGen::generateWithNoise(ChunkInfo& info)
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

    AnvilAccessor& blocks = info.blocks;

    for(int x=0;x<16;x++){
        for(int z=0;z<16;z++){
            blocks(x,0,z).id(BLOCK_BEDROCK);
        }
    }

    int chunkX = info.chunk->x;
    int chunkZ = info.chunk->z;

    for(int x=0;x<16;x++){
        for(int z=0;z<16;z++){

            int32_t& currentHeight = heightmap[(z << 4) + x];

            currentHeight = (int)finalTerrain.GetValue(chunkX + x,0,chunkZ + z);

            if(currentHeight > 256)
                currentHeight = 256;

            int stoneHeight = currentHeight - (int)uniform01()*4;

            int y=1;
            bool cave=false;
            int cave_start;
            for(;y<=stoneHeight;y++){
                if(caveNoise.GetValue(chunkX +x,y,chunkZ +z) > 0.8){
                    if(!cave){
                        cave =true;
                        cave_start =y;
                    }
                }
                else{
                    cave = false;
                    double density = earthNoise.GetValue(chunkX + x,y, chunkZ + z);

                    BlockRef b = blocks(x,y,z);
                    if(density < -0.75){
                        b.id(BLOCK_SAND);
                    }
                    else if(density < -0.2){
                        b.id(BLOCK_DIRT);
                    }
                    else if(density < 0.0){
                        b.id(BLOCK_GRAVEL);
                    }
                    else b.id(BLOCK_STONE);
                }
            }

            if(cave){
                heightmap[(z << 4) + x] = cave_start;
            }
            if(currentHeight <= seaLevel){
                for(;y<=currentHeight;y++)
                    blocks(x,y,z).id(BLOCK_SAND);
                for(;y<=seaLevel;y++)
                    blocks(x,y,z).id(BLOCK_STATIONARY_WATER);
            }else{
                if(!cave){
                    for(;y<currentHeight;y++)
                        blocks(x,y,z).id(BLOCK_DIRT);
                    blocks(x,y,z).id(BLOCK_GRASS);
                }
                else
                    blocks(x,cave_start,z).id(BLOCK_GRASS);
            }
            if( currentHeight >= (230 + uniform01()*5) ){
                blocks(x, currentHeight +1 ,z).id(BLOCK_SNOW_BLOCK);
            }
            else if(winterEnabled || currentHeight >= (225 + uniform01()*5) ){
                blocks(x, currentHeight +1 ,z).id(BLOCK_SNOW);
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
        count = uint8_t(uniform01() * 10 + 20); // 20-30 coal deposits
        //startHeight = 90;
        minDepoSize = 3;
        maxDepoSize = 7;
        break;
    case BLOCK_IRON_ORE:
        count = uint8_t(uniform01() * 8 + 10); // 10-18 iron deposits
        startHeight = 90;
        minDepoSize = 2;
        maxDepoSize = 5;
        break;
    case BLOCK_GOLD_ORE:
        count = uint8_t(uniform01() * 4 + 5); // 4-9 gold deposits
        startHeight = 42;
        minDepoSize = 2;
        maxDepoSize = 4;
        break;
    case BLOCK_DIAMOND_ORE:
        count = uint8_t(uniform01() * 1 + 2); // 1-3 diamond deposits
        startHeight = 17;
        minDepoSize = 1;
        maxDepoSize = 2;
        break;
    case BLOCK_REDSTONE_ORE:
        count = uint8_t(uniform01() * 5 + 5); // 5-10 redstone deposits
        startHeight = 25;
        minDepoSize = 2;
        maxDepoSize = 4;
        break;
    case BLOCK_LAPIS_ORE:
        count = uint8_t(uniform01() * 1 + 2); // 1-3 lapis lazuli deposits
        startHeight = 17;
        minDepoSize = 1;
        maxDepoSize = 2;
        break;
    case BLOCK_GRAVEL:
        count = uint8_t(uniform01() * 10 + 20); // 20-30 gravel deposits
        //startHeight = 90;
        minDepoSize = 6;
        maxDepoSize = 10;
        break;
    case BLOCK_DIRT:
        count = uint8_t(uniform01() * 10 + 20); // 20-30 gravel deposits
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
        blockX = int32_t(uniform01() * 16);
        blockZ = int32_t(uniform01() * 16);

        blockY = heightmap[(blockZ<<4)+blockX];
        blockY -= uint8_t(uniform01() * 5);

        // Check that startheight is not higher than height at that column
        if (blockY > startHeight)
        {
            blockY = startHeight;
        }

        // Calculate Y
        blockY = uint8_t(uniform01() * (blockY));

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
    uint8_t depoSize = uint8_t((uniform01() * (maxDepoSize - minDepoSize) + minDepoSize) / 2);
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
