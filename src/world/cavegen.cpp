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

#include <cstdlib>
#include <cstdio>
#include <iostream>

#include "../logger.h"
#include "../constants.h"

#include "../tools.h"
#include "../map.h"
#include "../config.h"
#include "../mersenne.h"

// libnoise
#ifdef DEBIAN
#include <libnoise/noise.h>
#else
#include <noise/noise.h>
#endif

#include "cavegen.h"

CaveGen &CaveGen::get()
{
  static CaveGen instance;
  return instance;
}

void CaveGen::init(int seed)
{
  // Set up us the Perlin-noise module.
  caveNoise.SetSeed (seed);
  caveNoise.SetFrequency (0.7);
  //caveNoise.SetLacunarity (0.5);
  caveNoise.SetOctaveCount (3);
  caveNoise.SetPersistence (0.2);
  caveNoise.SetNoiseQuality (noise::QUALITY_STD);
  
  addCaves = Conf::get().bValue("addCaves");
  caveDensity = Conf::get().iValue("caveDensity");
  caveSize = Conf::get().iValue("caveSize");
  addCaveLava = Conf::get().bValue("addCaveLava");
  addCaveWater = Conf::get().bValue("addCaveWater");
  addOre = Conf::get().bValue("addOre");
}

// Cave generation method from Omen 0.70, used with osici's permission
void CaveGen::AddSingleCave(uint8 bedrockType, uint8 fillingType, int length, double maxDiameter)
{
  int startX = rand.uniform(15);
  int startZ = rand.uniform(15);
  int startY = rand.uniform(127);
  
  std::cout << startX << " " << startZ << " " << startY << std::endl;

  int k1;
  for(k1 = 0; blocks[(int)(startY + (startZ * 128 + (startX * 128 * 16)))] != bedrockType && k1 < 10000; k1++)
  {
    startX = rand.uniform(15);
    startZ = rand.uniform(15);
    startY = rand.uniform(127);
  }

  if( k1 >= 10000 )
    return;

  int x = startX;
  int y = startZ;
  int h = startY;

  for(int k2 = 0; k2 < length; k2++)
  {
    int diameter = (int)(maxDiameter * rand.uniform() * 16);
    if( diameter < 1 ) diameter = 2;
    int radius = diameter / 2;
    if( radius == 0 ) radius = 1;

    x += (int)(0.7 * (rand.uniform() - 0.5) * diameter);
    y += (int)(0.7 * (rand.uniform() - 0.5) * diameter);
    h += (int)(0.7 * (rand.uniform() - 0.5) * diameter);

    for(int j3 = 0; j3 < diameter; j3++)
    {
      for(int k3 = 0; k3 < diameter; k3++)
      {
        for(int l3 = 0; l3 < diameter; l3++)
        {
          if((j3 - radius) * (j3 - radius) + (k3 - radius) * (k3 - radius) + (l3 - radius) * (l3 - radius) >= radius * radius ||
              x + j3 >= 16 || h + k3 >= 128 || y + l3 >= 16 ||
              x + j3 < 0 || h + k3 < 0 || y + l3 < 0 )
          {
            continue;
          }

          int index = (startY+k3) + ((startZ+l3) * 128 + ((startX+j3) * 128 * 16));

          if(blocks[index] == bedrockType)
            blocks[index] = fillingType;

          if((fillingType == 10 || fillingType == 11 || fillingType == 8 || fillingType == 9) &&
              h + k3 < startY )
            blocks[index] = 0;

        }
      }
    }
  }
  std::cout << "Cave added" << std::endl;
}

void CaveGen::AddSingleVein(uint8 bedrockType, uint8 fillingType, int k, double maxDiameter, int l)
{
  AddSingleVein(bedrockType, fillingType, k, maxDiameter, l, 10 );
}


void CaveGen::AddSingleVein(uint8 bedrockType, uint8 fillingType, int k, double maxDiameter, int l, int i1)
{

  int j1 = rand.uniform(16);
  int k1 = rand.uniform(128);
  int l1 = rand.uniform(16);

  double thirteenOverK = 1 / (double)k;

  for(int i2 = 0; i2 < i1; i2++)
  {
    int j2 = j1 + (int)(.5 * (rand.uniform() - .5) * 16.0);
    int k2 = k1 + (int)(.5 * (rand.uniform() - .5) * 128.0);
    int l2 = l1 + (int)(.5 * (rand.uniform() - .5) * 16.0);

    for(int l3 = 0; l3 < k; l3++)
    {
      int diameter = (int)(maxDiameter * rand.uniform() * 16);
      if( diameter < 1 ) diameter = 2;
      int radius = diameter / 2;
      if( radius == 0 ) radius = 1;

      int i3 = (int)((1 - thirteenOverK) * (double)j1 + thirteenOverK * (double)j2 + (double)(l * radius) * (rand.uniform() - .5));
      int j3 = (int)((1 - thirteenOverK) * (double)k1 + thirteenOverK * (double)k2 + (double)(l * radius) * (rand.uniform() - .5));
      int k3 = (int)((1 - thirteenOverK) * (double)l1 + thirteenOverK * (double)l2 + (double)(l * radius) * (rand.uniform() - .5));
      for(int k4 = 0; k4 < diameter; k4++)
      {
        for(int l4 = 0; l4 < diameter; l4++)
        {
          for(int i5 = 0; i5 < diameter; i5++)
          {
            if((k4 - radius) * (k4 - radius) + (l4 - radius) * (l4 - radius) + (i5 - radius) * (i5 - radius) < radius * radius &&
                i3 + k4 < 16 && j3 + l4 < 128 && k3 + i5 < 16 &&
                i3 + k4 >= 0 && j3 + l4 >= 0 && k3 + i5 >= 0 )
            {
              int index = (j3+l4) + ((k3+i5) * 128 + ((i3+k4) * 128 * 16));
              

              if( blocks[index] == bedrockType )
                blocks[index] = fillingType;
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

void CaveGen::SealLiquids(uint8 sealantType)
{
  for( int x = 1; x < 16 - 1; x++ )
  {
    for( int h = 1; h < 128; h++ )
    {
      for( int y = 1; y < 16 - 1; y++ )
      {
        int index = (h * 16 + y) * 16 + x;
        if( (blocks[index] == 10 || blocks[index] == 11 || blocks[index] == 8 || blocks[index] == 9) &&
            (blocks[(h * 16 + y) * 16 + (x-1)] == 0 || blocks[(h * 16 + y) * 16 + (x+1)] == 0 ||
          blocks[(h * 16 + (y-1)) * 16 + x] == 0 || blocks[(h * 16 + (y+1)) * 16 + x] == 0 ||
          blocks[((h-1) * 16 + y) * 16 + x] == 0) )
        {
          blocks[index] = sealantType;
        }
      }
    }
  }
}

void CaveGen::AddCaves(uint8 &block, double x, double y, double z)
{  
  if(addCaves)
  {
    
    if(y*16.0 < 63.0 && block != BLOCK_WATER && block != BLOCK_STATIONARY_WATER && caveNoise.GetValue(x, y, z) < -0.5)
    {
      block = BLOCK_AIR;
      return;
    }
    /*for(int i1 = 0; i1 < 36 * caveDensity; i1++)
      AddSingleCave(BLOCK_BEDROCK, BLOCK_AIR, 30, 0.05 * caveSize);

    for(int j1 = 0; j1 < 9 * caveDensity; j1++)
      AddSingleVein(BLOCK_BEDROCK, BLOCK_AIR, 500, 0.015 * caveSize, 1);

    for(int k1 = 0; k1 < 30 * caveDensity; k1++)
      AddSingleVein(BLOCK_BEDROCK, BLOCK_AIR, 300, 0.03 * caveSize, 1, 20);
    */

    if(addCaveLava)
    {
      for(int i = 0; i < 8 * caveDensity; i++)
        AddSingleCave(BLOCK_BEDROCK, BLOCK_LAVA, 30, 0.05 * caveSize);

      for(int j = 0; j < 3 * caveDensity; j++)
        AddSingleVein(BLOCK_BEDROCK, BLOCK_LAVA, 1000, 0.015 * caveSize, 1);

    }


    if(addCaveWater)
    {
      for(int k = 0; k < 8 * caveDensity; k++)
        AddSingleCave(BLOCK_BEDROCK, BLOCK_WATER, 30, 0.05 * caveSize);

      for( int l = 0; l < 3 * caveDensity; l++ )
        AddSingleVein(BLOCK_BEDROCK, BLOCK_WATER, 1000, 0.015 * caveSize, 1);
    }

    //SealLiquids(BLOCK_BEDROCK);
  }


  if(addOre)
  {
    for(int l1 = 0; l1 < 12 * caveDensity; l1++)
      AddSingleCave(BLOCK_BEDROCK, BLOCK_COAL_ORE, 500, 0.03);

    for(int i2 = 0; i2 < 32 * caveDensity; i2++)
    {
      AddSingleVein(BLOCK_BEDROCK, BLOCK_COAL_ORE, 200, 0.015, 1);
      AddSingleCave(BLOCK_BEDROCK, BLOCK_IRON_ORE, 500, 0.02);
    }

    for(int k2 = 0; k2 < 8 * caveDensity; k2++ )
    {
      AddSingleVein(BLOCK_BEDROCK, BLOCK_IRON_ORE, 200, 0.015, 1);
      AddSingleVein(BLOCK_BEDROCK, BLOCK_GOLD_ORE, 200, 0.0145, 1);
    }

    for(int l2 = 0; l2 < 20 * caveDensity; l2++)
      AddSingleCave(BLOCK_BEDROCK, BLOCK_GOLD_ORE, 400, 0.0175);
  }
}
