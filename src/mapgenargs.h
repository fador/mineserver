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

#ifndef _MAPGENARGS_H
#define _MAPGENARGS_H

enum MapGenTheme {
  Arctic,
  Desert,
  Forest,
  Hell,
  Swamp
};

class MapGenArgs
{
public:
  MapGenArgs();

  int FormatVersion;

  MapGenTheme theme;
  
  int  seed,
       dimX,
       dimY,
       dimH,
       maxHeight,
       maxDepth,
       maxHeightVariation,
       maxDepthVariation;

  bool addWater,
       customWaterLevel,
       matchWaterCoverage;
         
  int   waterLevel;
  float waterCoverage;

  bool  useBias,
        delayBias;
  float bias;
  int   raisedCorners,
        loweredCorners,
        midPoint;

  int   detailScale,
        featureScale;
  float roughness;
  bool  layeredHeightmap,
        marbledHeightmap,
        invertHeightmap;
  float aboveFuncExponent,
        belowFuncExponent;

  bool  addTrees;
  int   treeSpacingMin,
        treeSpacingMax,
        treeHeightMin,
        treeHeightMax;

  bool  addCaves,
        addOre,
        addCaveWater,
        addCaveLava;
  float caveDensity,
        caveSize;

  bool  addSnow;
  int   snowAltitude,
        snowTransition;

  bool  addCliffs,
        cliffSmoothing;
  float cliffThreshold;

  bool  addBeaches;
  int   beachExtent,
        beachHeight;

};

#endif