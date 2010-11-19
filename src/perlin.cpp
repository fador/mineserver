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
#include <cmath>

#include "perlin.h"

Perlin &Perlin::get()
{
  static Perlin instance;
  return instance;
}

double Perlin::psRand(double x, double y)
{
  int n  = (int)x + (int)y*57;
  n = (n<<13)^n;
  int nn = (n*(n*n*60493+19990303)+mSeed)&0x7fffffff; //1376312589
  return 1.0-((double)nn/1073741824.0);
}

double Perlin::interpolate(double a, double b, double x)
{
  double ft = x * 3.1415927;
  double f  = (1.0-cos(ft))* 0.5;
  return a*(1.0-f)+b*f;
}

double Perlin::noise(double x, double y)
{
  double floorx = (double)((int)x);
  double floory = (double)((int)y);
  double s, t, u, v;
  s = psRand(floorx, floory);
  t = psRand(floorx+1, floory);
  u = psRand(floorx, floory+1);
  v = psRand(floorx+1, floory+1);
  double int1 = interpolate(s, t, x-floorx);
  double int2 = interpolate(u, v, x-floorx);
  return interpolate(int1, int2, y-floory);
}

void Perlin::init(int octaves, double persistence, int seed)
{
  mOctaves = octaves;
  mPersistence = persistence;
  mSeed = seed;
}

double Perlin::getPerlin(double x, double y)
{
  double getnoise = 0;
  for(int a = 0; a < mOctaves-1; a++)
  {
    double frequency = 1<<a;
    double amplitude = pow(mPersistence,(double)a);
    getnoise += noise(((double)x)*frequency,((double)y)*frequency)*amplitude;
  }  
  return getnoise;
};
