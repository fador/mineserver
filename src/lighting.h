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

#ifndef _LIGHTING_H_
#define _LIGHTING_H_

#include "constants.h"
#include <queue>


struct lightInfo
{
  lightInfo() { };
  lightInfo(int _x, int _y, int _z, int _light)
  {
    x = _x;
    y = _y;
    z = _z;
    light = _light;
    skipdir = -1;
  }
  lightInfo(int _x, int _y, int _z, int _light, int _skipdir)
  {
    x = _x;
    y = _y;
    z = _z;
    light = _light;
    skipdir = _skipdir;
  }
  int x;
  int y;
  int z;
  int light;
  int skipdir;
};

class Lighting
{
private:

  Lighting()
  {
    for (int i = 0; i < 256; i++)
    {
      emitLight[i] = 0;
    }
    emitLight[0x0A] = 15; // Lava
    emitLight[0x0B] = 15; // Stationary Lava
    emitLight[0x27] = 1;  // Brown mushroom
    emitLight[0x32] = 14; // Torch
    emitLight[0x33] = 15; // Fire
    emitLight[0x3E] = 14; // Lit furnace
    emitLight[0x4A] = 9;  // Redstone ore (Glowing)
    emitLight[0x4C] = 7;  // Redstone Torch (On)
    emitLight[0x59] = 15; // Lightstone
    emitLight[0x5A] = 11; // Portal
    emitLight[0x5B] = 15; // Jack-O-Lantern

    for (int i = 0; i < 256; i++)
    {
      stopLight[i] = 16;
    }
    stopLight[0x00] = 0; // Empty
    stopLight[0x06] = 0; // Sapling
    stopLight[0x08] = 3; // Water
    stopLight[0x09] = 3; // Stationary water
    stopLight[0x12] = 3; // Leaves
    stopLight[0x14] = 0; // Glass
    stopLight[0x25] = 0; // Yellow flower
    stopLight[0x26] = 0; // Red rose
    stopLight[0x27] = 0; // Brown mushroom
    stopLight[0x28] = 0; // Red mushroom
    stopLight[0x32] = 0; // Torch
    stopLight[0x33] = 0; // Fire
    stopLight[0x34] = 0; // Mob spawner
    stopLight[0x35] = 0; // Wooden stairs
    stopLight[0x37] = 0; // Redstone wire
    stopLight[0x40] = 0; // Wooden door
    stopLight[0x41] = 0; // Ladder
    stopLight[0x42] = 0; // Minecart track
    stopLight[0x43] = 0; // Cobblestone stairs
    stopLight[0x47] = 0; // Iron door
    stopLight[0x4b] = 0; // Redstone Torch (Off)
    stopLight[0x4C] = 0; // Redstone Torch (On)
    stopLight[0x4e] = 0; // Snow
    stopLight[0x4f] = 3; // Ice
    stopLight[0x55] = 0; // Fence
    stopLight[0x5A] = 0; // Portal
    stopLight[0x5B] = 0; // Jack-O-Lantern
    stopLight[BLOCK_SIGN_POST] = 0; // Sign post
    stopLight[BLOCK_WALL_SIGN] = 0; // Wall sign
    mLight = NULL;
  }

  ~Lighting()
  {

  }

  static Lighting* mLight;

public:


  // How blocks affect light
  int stopLight[256];

  // Blocks that emit light
  int emitLight[256];

  // Generate light maps for chunk
  bool generateLight(int x, int z, sChunk* chunk);


  // Light get/set
  bool getLight(int x, int y, int z, uint8_t* blocklight, uint8_t* skylight, sChunk* chunk);
  bool setLight(int x, int y, int z, int blocklight, int skylight, int setLight, sChunk* chunk);

  bool spreadLight(std::queue<lightInfo> *lightQueue, sChunk* chunk);

  static Lighting* get()
  {
    if (!mLight)
    {
      mLight = new Lighting();
    }
    return mLight;
  }
};


#endif
