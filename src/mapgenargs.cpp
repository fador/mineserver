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
#include <ctime>

#include "constants.h"
#include "mapgenargs.h"

MapGenArgs::MapGenArgs():FormatVersion(2),
                         theme(Forest),
                         dimX(DIMX),
                         dimY(DIMY),
                         dimH(DIMH),
                         maxHeight(20),
                         maxDepth(12),
                         maxHeightVariation(4),
                         maxDepthVariation(0),
                         addWater(true),
                         customWaterLevel(false),
                         matchWaterCoverage(false),
                         waterLevel(48),
                         waterCoverage(0.5),
                         useBias(false),
                         delayBias(false),
                         bias(0),
                         raisedCorners(0),
                         loweredCorners(0),
                         midPoint(0),
                         detailScale(7),
                         featureScale(1),
                         roughness(0.5),
                         layeredHeightmap(false),
                         marbledHeightmap(false),
                         invertHeightmap(false),
                         aboveFuncExponent(1),
                         belowFuncExponent(1),
                         addTrees(true),
                         treeSpacingMin(7),
                         treeSpacingMax(11),
                         treeHeightMin(5),
                         treeHeightMax(7),
                         addCaves(false),
                         addOre(false),
                         addCaveWater(false),
                         addCaveLava(false),
                         caveDensity(2),
                         caveSize(1),
                         addSnow(false),
                         snowAltitude(70),
                         snowTransition(7),
                         addCliffs(true),
                         cliffSmoothing(true),
                         cliffThreshold(1),
                         addBeaches(false),
                         beachExtent(6),
                         beachHeight(2)
{
  srand(time(NULL));
  seed = rand();
}