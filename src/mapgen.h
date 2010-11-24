/*
  Copyright (c) 2010 Drew Gottlieb - with code from fragmer and TkTech

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
 
// This code is modified from MySMP C# server by Drew Gottlieb
// Thanks!

#ifndef _MAPGEN_H
#define _MAPGEN_H

#include "noise.h"

class MapGen
{
private:
  uint8 *blocks;
  uint8 *blockdata;
  uint8 *skylight;
  uint8 *blocklight;
  uint8 *heightmap;
  
  int randomSeed;
  
  int GetHeightmapIndex(char x, char z);
  void CalculateHeightmap();
  void LoadFlatgrass();
  //void generateWithNoise(int x, int z);


public:
  MapGen(int seed);
  ~MapGen();  
  void generateChunk(int x, int z);

};


#endif
