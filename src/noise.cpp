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
#define _USE_MATH_DEFINES
#include <math.h>
#include <climits>
#include <cfloat>

#include "noise.h"
#include "constants.h"

using std::min;
using std::max;

void Noise::init(int _seed, NoiseInterpolationMode _interpolationMode) 
{
  seed = _seed;
  interpolationMode = _interpolationMode;
}

float Noise::InterpolateLinear(float v0, float v1, float x) 
{
  return v0 * (1 - x) + v1 * x;
}

float Noise::InterpolateLinear(float v00, float v01, float v10, float v11, float x, float y) 
{
  return InterpolateLinear(InterpolateLinear(v00, v10, x),
                           InterpolateLinear(v01, v11, x),
                           y );
}

float Noise::InterpolateCosine(float v0, float v1, float x) 
{
  double f = (1 - cos(x * M_PI)) * .5;
  return (float)(v0 * (1 - f) + v1 * f);
}

float Noise::InterpolateCosine(float v00, float v01, float v10, float v11, float x, float y) 
{
  return InterpolateCosine(InterpolateCosine(v00, v10, x),
                           InterpolateCosine(v01, v11, x),
                           y);
}

// Cubic and Catmull-Rom Spline interpolation methods by Paul Bourke
// http://local.wasp.uwa.edu.au/~pbourke/miscellaneous/interpolation/
float Noise::InterpolateCubic(float v0, float v1, float v2, float v3, float mu) 
{
  float a0, a1, a2, a3, mu2;
  mu2 = mu * mu;
  a0 = v3 - v2 - v0 + v1;
  a1 = v0 - v1 - a0;
  a2 = v2 - v0;
  a3 = v1;
  return (float)(a0 * mu * mu2 + a1 * mu2 + a2 * mu + a3);
}

float Noise::InterpolateSpline(float v0, float v1, float v2, float v3, float mu) 
{
  float a0, a1, a2, a3, mu2;
  mu2 = mu * mu;
  a0 = -0.5f * v0 + 1.5f * v1 - 1.5f * v2 + 0.5f * v3;
  a1 = v0 - 2.5f * v1 + 2 * v2 - 0.5f * v3;
  a2 = -0.5f * v0 + 0.5f * v2;
  a3 = v1;
  return (float)(a0 * mu * mu2 + a1 * mu2 + a2 * mu + a3);
}

float Noise::StaticNoise(int x, int y) 
{
  int n = seed + x + y * SHRT_MAX;
  n = (n << 13) ^ n;
  return (float)(1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7FFFFFFF) / 1073741824);
}

float Noise::InterpolatedNoise(float x, float y)
{
  int xInt = (int)x;
  float xFloat = x - xInt;

  int yInt = (int)y;
  float yFloat = y - yInt;

  float p00, p01, p10, p11;
  float** points;

  switch(interpolationMode) {
    case Linear:
      p00 = StaticNoise(xInt, yInt);
      p01 = StaticNoise(xInt, yInt + 1);
      p10 = StaticNoise(xInt + 1, yInt);
      p11 = StaticNoise(xInt + 1, yInt + 1);
      return InterpolateLinear(InterpolateLinear(p00, p10, xFloat), InterpolateLinear(p01, p11, xFloat), yFloat);

    case Cosine:
      p00 = StaticNoise(xInt, yInt);
      p01 = StaticNoise(xInt, yInt + 1);
      p10 = StaticNoise(xInt + 1, yInt);
      p11 = StaticNoise(xInt + 1, yInt + 1);
      return InterpolateCosine(InterpolateCosine(p00, p10, xFloat), InterpolateCosine(p01, p11, xFloat), yFloat);

    case Bicubic:
      points = new float*[4];
      for(int i=0;i<4;i++)
      {
        points[i] = new float[4];
      }
      for(int xOffset = -1; xOffset < 3; xOffset++) 
      {
        for(int yOffset = -1; yOffset < 3; yOffset++) 
        {
          points[xOffset + 1][yOffset + 1] = StaticNoise(xInt + xOffset, yInt + yOffset);
        }
      }
      p00 = InterpolateCubic(points[0][0], points[1][0], points[2][0], points[3][0], xFloat);
      p01 = InterpolateCubic(points[0][1], points[1][1], points[2][1], points[3][1], xFloat);
      p10 = InterpolateCubic(points[0][2], points[1][2], points[2][2], points[3][2], xFloat);
      p11 = InterpolateCubic(points[0][3], points[1][3], points[2][3], points[3][3], xFloat);
      return InterpolateCubic(p00, p01, p10, p11, yFloat);

    case Spline:
      points = new float*[4];
      for(int i=0;i<4;i++)
      {
        points[i] = new float[4];
      }
      for(int xOffset = -1; xOffset < 3; xOffset++)
      {
        for(int yOffset = -1; yOffset < 3; yOffset++)
        {
          points[xOffset + 1][yOffset + 1] = StaticNoise(xInt + xOffset, yInt + yOffset);
        }
      }
     
      p00 = InterpolateSpline(points[0][0], points[1][0], points[2][0], points[3][0], xFloat);
      p01 = InterpolateSpline(points[0][1], points[1][1], points[2][1], points[3][1], xFloat);
      p10 = InterpolateSpline(points[0][2], points[1][2], points[2][2], points[3][2], xFloat);
      p11 = InterpolateSpline(points[0][3], points[1][3], points[2][3], points[3][3], xFloat);
      return InterpolateSpline(p00, p01, p10, p11, yFloat);
      
    default:
      std::cout << "Error!" << std::endl;
  }
}


float Noise::PerlinNoise(float x, float y, int startOctave, int endOctave, float decay)
{
  float total = 0;

  float frequency = (float)(1<<startOctave);
  float amplitude = (float)pow(decay, startOctave);

  for(int n = startOctave; n <= endOctave; n++) 
  {
    total += InterpolatedNoise(x * frequency + frequency, y * frequency + frequency) * amplitude;
    frequency *= 2;
    amplitude *= decay;
  }
  return total;
}

void Noise::PerlinNoiseMap(float** heightmap, int startOctave, int endOctave, float decay, int offsetX, int offsetY)
{
  float maxDim = 1.0 / 16;
  
  for(int x = 16 - 1; x >= 0; x--) 
  {
    for(int y = 16 - 1; y >= 0; y--) 
    {
      heightmap[x][y] += PerlinNoise(x * maxDim + offsetX, y * maxDim + offsetY, startOctave, endOctave, decay);
    }
  }
}


void Noise::Normalize(float** map)
{
  Normalize(map, 0, 1);
}

void Noise::Normalize(float** map, float low, float high)
{
  float minV = FLT_MAX, maxV = FLT_MIN;
  for( int x = 16 - 1; x >= 0; x-- ) 
  {
    for( int y = 16 - 1; y >= 0; y-- ) 
    {
      minV = min( minV, map[x][y] );
      maxV = max( maxV, map[x][y] );
    }
  }

  float multiplier = (high - low) / (maxV - minV);
  float constant = -minV * (high - low) / (maxV - minV) + low;

  for(int x = 16 - 1; x >= 0; x--) 
  {
    for(int y = 16 - 1; y >= 0; y--) 
    {
      map[x][y] = map[x][y] * multiplier + constant;
    }
  }
}


// assumes normalized input
void Noise::Marble(float** map) 
{
  for(int x = 16 - 1; x >= 0; x--) 
  {
    for(int y = 16 - 1; y >= 0; y--) 
    {
      map[x][y] = abs(map[x][y] * 2 - 1);
    }
  }
}


// assumes normalized input
void Noise::Blend(float** map1, float** map2, float** blendMap)
{
  for(int x = 16 - 1; x >= 0; x--) 
  {
    for(int y = 16 - 1; y >= 0; y--) 
    {
      map1[x][y] = map1[x][y] * blendMap[x][y] + map2[x][y] * (1 - blendMap[x][y]);
    }
  }
}


void Noise::Add(float** map1, float** map2)
{
  for(int x = 16 - 1; x >= 0; x--) 
  {
    for(int y = 16 - 1; y >= 0; y--) 
    {
      map1[x][y] += map2[x][y];
    }
  }
}


void Noise::applyBias(float** heightmap, float c00, float c01, float c10, float c11, float midpoint) 
{
  float maxX = 2.0 / 16;
  float maxY = 2.0 / 16;
  int offsetX = 16 / 2;
  int offsetY = 16 / 2;

  for(int x = offsetX - 1; x >= 0; x--) 
  {
    for(int y = offsetY - 1; y >= 0; y--) 
    {
      heightmap[x][y] += InterpolateCosine(c00, (c00 + c01) / 2, (c00 + c10) / 2, midpoint, x * maxX, y * maxY);
      heightmap[x + offsetX][y] += InterpolateCosine((c00 + c10) / 2, midpoint, c10, (c11 + c10) / 2, x * maxX, y * maxY);
      heightmap[x][y + offsetY] += InterpolateCosine((c00 + c01) / 2, c01, midpoint, (c01 + c11) / 2, x * maxX, y * maxY);
      heightmap[x + offsetX][y + offsetY] += InterpolateCosine(midpoint, (c01 + c11) / 2, (c11 + c10) / 2, c11, x * maxX, y * maxY);
    }
  }
}


// assumes normalized input
void Noise::ScaleAndClip(float** heightmap, float steepness)
{
  for(int x = 16 - 1; x >= 0; x--) 
  {
    for(int y = 16 - 1; y >= 0; y--) 
    {
      heightmap[x][y] = min( 1.0f, max( 0.0f, heightmap[x][y] * steepness * 2 - steepness ) );
    }
  }
}

void Noise::Invert(float** heightmap)
{
  for(int x = 16 - 1; x >= 0; x--) 
  {
    for(int y = 16 - 1; y >= 0; y--) 
    {
      heightmap[x][y] = 1 - heightmap[x][y];
    }
  }
}

float** Noise::BoxBlur(float** heightmap) 
{
  float divisor = 1 / 23.0f;
  float** output = new float*[16];
  for(int i=0;i<16;i++)
  {
    output[i] = new float[16];
  }
  for(int x = 16 - 1; x >= 0; x--) 
  {
    for(int y = 16 - 1; y >= 0; y--) 
    {
      if((x == 0) || (y == 0) || (x == 16 - 1) || (y == 16 - 1)) 
      {
        output[x][y] = heightmap[x][y];
      } 
      else 
      {
        output[x][y] = (heightmap[x - 1][y - 1] * 2 + heightmap[x - 1][y] * 3 + heightmap[x - 1][y + 1] * 2 +
                        heightmap[x][y - 1] * 3 + heightmap[x][y] * 3 + heightmap[x][y + 1] * 3 +
                        heightmap[x + 1][y - 1] * 2 + heightmap[x + 1][y] * 3 + heightmap[x + 1][y + 1] * 2) * divisor;
      }
    }
  }
  //delete [] output;
  return output;
}


float** Noise::GaussianBlur5x5(float** heightmap) 
{
  float divisor = 1 / 273.0f;
  float** output = new float*[16];
  for(int i = 0; i < 16; i++)
  {
    output[i] = new float[16];
  }
  for(int x = 16 - 1; x >= 0; x--) 
  {
    for(int y = 16 - 1; y >= 0; y--) 
    {
      if((x < 2) || (y < 2) || (x > 16 - 3) || (y > 16 - 3)) 
      {
        output[x][y] = heightmap[x][y];
      } 
      else 
      {
        output[x][y] = (heightmap[x - 2][y - 2] + heightmap[x - 1][y - 2] * 4 + heightmap[x][y - 2] * 7 + heightmap[x + 1][y - 2] * 4 +heightmap[x + 2][y - 2] +
                        heightmap[x - 1][y - 1] * 4 + heightmap[x - 1][y - 1] * 16 + heightmap[x][y - 1] * 26 + heightmap[x + 1][y - 1] * 16 + heightmap[x + 2][y - 1]*4 +
                        heightmap[x - 2][y] * 7 + heightmap[x - 1][y] * 26 + heightmap[x][y] * 41 + heightmap[x + 1][y] * 26 +heightmap[x + 2][y]*7 +
                        heightmap[x - 2][y + 1] * 4 + heightmap[x - 1][y + 1] * 16 + heightmap[x][y + 1] * 26 + heightmap[x + 1][y + 1] * 16 + heightmap[x + 2][y + 1]*4 +
                        heightmap[x - 2][y + 2] + heightmap[x - 1][y + 2] * 4 + heightmap[x][y + 2] * 7 + heightmap[x + 1][y + 2] * 4 +heightmap[x + 2][y + 2]) * divisor;
      }
    }
  }
  return output;
}


float** Noise::CalculateSlope(float** heightmap)
{
  float** output = new float*[16];
  for(int i = 0; i < 16; i++)
  {
    output[i] = new float[16];
  }

  for(int x = 16 - 1; x >= 0; x--) 
  {
    for(int y = 16 - 1; y >= 0; y--) 
    {
      if((x == 0) || (y == 0) || (x == 16 - 1) || (y == 16 - 1)) 
      {
        output[x][y] = 0;
      } 
      else 
      {
        output[x][y] = (abs( heightmap[x][y - 1] - heightmap[x][y] ) * 3 +
                        abs( heightmap[x][y + 1] - heightmap[x][y] ) * 3 +
                        abs( heightmap[x - 1][y] - heightmap[x][y] ) * 3 +
                        abs( heightmap[x + 1][y] - heightmap[x][y] ) * 3 +
                        abs( heightmap[x - 1][y - 1] - heightmap[x][y] ) * 2 +
                        abs( heightmap[x + 1][y - 1] - heightmap[x][y] ) * 2 +
                        abs( heightmap[x - 1][y + 1] - heightmap[x][y] ) * 2 +
                        abs( heightmap[x + 1][y + 1] - heightmap[x][y] ) * 2) / 20.0f;
      }
    }
  }

  return output;
}

