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

#include "lighting.h"
#include "mineserver.h"
#include "map.h"
#include "nbt.h"

Lighting* Lighting::mLight;


bool Lighting::generateLight(int x, int z, sChunk* chunk)
{

  uint8_t* blocks     = chunk->blocks;
  uint8_t* skylight   = chunk->skylight;
  uint8_t* blocklight = chunk->blocklight;
  int32_t* heightmap  = chunk->heightmap;

  uint64_t* blocks64 = (uint64_t*)blocks;
  uint32_t* skylight32 = (uint32_t*)skylight;
  uint8_t meta, block, blockl, skyl;

  std::queue<lightInfo> lightQueue;

  int highest_y = 0;

  // Clear lightmaps
  memset(skylight, 0, 16 * 16 * 128 / 2);
  memset(blocklight, 0, 16 * 16 * 128 / 2);

  int light = 0;

  // Sky light optimized search
  for (int block_x = 0; block_x < 16; block_x++)
  {
    for (int block_z = 0; block_z < 16; block_z++)
    {
      int blockx_blockz = ((block_z << 7) + (block_x << 11)) >> 3;
      int absolute_x    = (x << 4) + block_x;
      int absolute_z    = (z << 4) + block_z;

      for (int block_y = (127 / 8) - 1; block_y >= 0; block_y--)
      {
        int index      = block_y + blockx_blockz;
        uint64_t block64 = blocks64[index];

        // if one of these 8 blocks is
        if (block64 != 0)
        {
          //Iterate which of the 8 is the first non-air
          for (int i = 7; i >= 0; i--)
          {
            //Set light value if air
            setLight(absolute_x, (block_y << 3) + i, absolute_z, 15, 0, 1, chunk);
            light = 15;
            int block = blocks[(index << 3) + i];
            if (block != BLOCK_AIR)
            {
              //lightQueue.push(lightInfo(absolute_x,(block_y<<3)+i+1,absolute_z,15));
              heightmap[block_z + (block_x << 4)] = (block_y << 3) + i;
              for (int block_yy = (block_y << 3) + i; block_yy >= 0; block_yy --)
              {
                setLight(absolute_x, block_yy, absolute_z, light, 0, 1, chunk);
                block = blocks[(blockx_blockz << 3) + block_yy];
                light -= stopLight[block];

                if (light < 1)
                {
                  break;
                }

                lightQueue.push(lightInfo(absolute_x, block_yy, absolute_z, light));
              }
              break;
            }
          }

          if (heightmap[block_z + (block_x << 4)] > highest_y)
          {
            highest_y = heightmap[block_z + (block_x << 4)];
          }
          break;
        }
        //These 8 blocks are full lit
        skylight32[index] = 0xffffffff;
      }
    }
  }

  for (int block_x = 0; block_x < 16; block_x++)
  {
    for (int block_z = 0; block_z < 16; block_z++)
    {
      int absolute_x = (x << 4) + block_x;
      int absolute_z = (z << 4) + block_z;

      //ToDo: heigth according to neighboring blocks
      //int height = highest_y;
      //if(block_x == 0 || block_x == 15 || block_z == 0 || block_z == 15)
      //{
      //}
      if (heightmap[block_z + (block_x << 4)] < highest_y)
      {
        for (int i = heightmap[block_z + (block_x << 4)] + 1; i <= highest_y; i++)
        {
          lightQueue.push(lightInfo(absolute_x, i, absolute_z, 15));
        }
      }
    }
  }




  spreadLight(&lightQueue, chunk);


  //Get light from border chunks
  for (int block_x = 0; block_x < 16; block_x += 15)
  {
    for (int block_z = 0; block_z < 16; block_z += 15)
    {
      int absolute_x = (x << 4) + block_x;
      int absolute_z = (z << 4) + block_z;

      int xdir = absolute_x;
      int zdir = absolute_z;
      int skipdir = -1;

      //Which border?
      if (block_z == 0)
      {
        zdir--;
        skipdir = 5;
      }
      else if (block_z == 15)
      {
        zdir++;
        skipdir = 4;
      }
      else if (block_x == 0)
      {
        xdir--;
        skipdir = 3;
      }
      else if (block_x == 15)
      {
        xdir++;
        skipdir = 2;
      }
      //ToDo: Corner case

      for (int block_y = heightmap[block_z + (block_x << 4)]; block_y >= 0; block_y--)
      {
        if (ServerInstance->map(0)->getBlock(xdir, block_y, zdir, &block, &meta, false))
        {
          uint8_t curblocklight, curskylight;
          if (getLight(xdir, block_y, zdir, &skyl, &blockl, chunk)                            &&
              getLight(absolute_x, block_y, absolute_z, &curskylight, &curblocklight, chunk))
          {
            if (skyl - stopLight[block] - 1 > curskylight)
            {
              uint8_t curblock, curmeta;
              if (ServerInstance->map(0)->getBlock(absolute_x, block_y, absolute_z, &curblock, &curmeta, false) &&
                  skyl - stopLight[block] - stopLight[curblock] > 1)
              {
                lightQueue.push(lightInfo(absolute_x, block_y, absolute_z, skyl - stopLight[block] - stopLight[curblock] - 1, skipdir));
              }
              setLight(absolute_x, block_y, absolute_z, skyl - stopLight[block] - 1, 0, 1, chunk);
            }
            if (blockl - stopLight[block] - 1 > curblocklight)
            {
              //ToDo: get blocklight from this chunk
            }
          }
        }
      }
    }
  }

  spreadLight(&lightQueue, chunk);



  return true;
}

bool Lighting::spreadLight(std::queue<lightInfo> *lightQueue, sChunk* chunk)
{
  uint8_t meta, block, blockl, skyl;
  //Next up, skylight spreading
  while (!lightQueue->empty())
  {
    lightInfo info = lightQueue->front();
    lightQueue->pop();

    for (int direction = 0; direction < 6; direction++)
    {
      int xdir = info.x;
      int ydir = info.y;
      int zdir = info.z;
      int skipdir = -1;

      //If we came from this direction, skip
      if (direction == info.skipdir)
      {
        continue;
      }

      switch (direction)
      {
      case 0:
        ydir--;
        skipdir = 1;
        break;
      case 1:
        ydir++;
        skipdir = 0;
        break;
      case 2:
        xdir--;
        skipdir = 3;
        break;
      case 3:
        xdir++;
        skipdir = 2;
        break;
      case 4:
        zdir--;
        skipdir = 5;
        break;
      case 5:
        zdir++;
        skipdir = 4;
        break;
      }
      //Going too high
      if (ydir == 128)
      {
        continue;
      }

      int light = info.light - 1;

      //Stop of this block light value already higher
      if (getLight(xdir, ydir, zdir, &skyl, &blockl, chunk) && skyl < light)
      {
        //If still light left, generate for this block also!
        if (light > 1)
        {
          if (ServerInstance->map(0)->getBlock(xdir, ydir, zdir, &block, &meta, false) && light - stopLight[block] > 1)
          {
            lightQueue->push(lightInfo(xdir, ydir, zdir, light - stopLight[block], skipdir));
          }
        }
        setLight(xdir, ydir, zdir, light, 0, 1, chunk);
      }
    }
  }
  return true;
}

// Light get/set
bool Lighting::getLight(int x, int y, int z, uint8_t* skylight, uint8_t* blocklight, sChunk* chunk)
{
  return ServerInstance->map(0)->getLight(x, y, z, skylight, blocklight, chunk);
}

bool Lighting::setLight(int x, int y, int z, int skylight, int blocklight, int setLight, sChunk* chunk)
{
  return ServerInstance->map(0)->setLight(x, y, z, skylight, blocklight, setLight, chunk);
}
