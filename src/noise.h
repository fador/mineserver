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

#ifndef _NOISE_H
#define _NOISE_H

class Noise
{
public:
  enum NoiseInterpolationMode 
  {
    Linear,
    Cosine,
    Bicubic,
    Spline
  };

  void init(int _seed, NoiseInterpolationMode _interpolationMode);
  
  int seed;
  NoiseInterpolationMode interpolationMode;
  
  float InterpolateLinear(float v0, float v1, float x);
  float InterpolateLinear(float v00, float v01, float v10, float v11, float x, float y);
  
  float InterpolateCosine(float v0, float v1, float x);
  float InterpolateCosine(float v00, float v01, float v10, float v11, float x, float y) ;
  
  float InterpolateCubic(float v0, float v1, float v2, float v3, float mu);
  
  float InterpolateSpline(float v0, float v1, float v2, float v3, float mu);
  
  float StaticNoise(int x, int y);
  
  float InterpolatedNoise(float x, float y);
  
  float PerlinNoise(float x, float y, int startOctave, int endOctave, float decay);
  
  void PerlinNoiseMap(float** heightmap, int startOctave, int endOctave, float decay, int offsetX, int offsetY);
  
  void Normalize(float** map);
  void Normalize(float** map, float low, float high);
  
  void Marble(float** map);
  void Blend(float** map1, float** map2, float** blendMap);
  
  void Add(float** map1, float** map2);
  void applyBias(float** heightmap, float c00, float c01, float c10, float c11, float midpoint);
  
  void ScaleAndClip(float** heightmap, float steepness);
  void Invert(float** heightmap);
  float** BoxBlur(float** heightmap);
  float** GaussianBlur5x5(float** heightmap);
  float** CalculateSlope(float** heightmap);
};

#endif

