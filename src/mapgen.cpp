#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>

#include "logger.h"
#include "constants.h"

#include "config.h"

#include "mapgen.h"

MapGen &MapGen::get()
{
  static MapGen instance;
  return instance;
}

bool MapGen::setSeed(unsigned int seed)
{
  this->noiseSeed = seed;
  return true;
}

unsigned int MapGen::getSeed()
{
  return this->noiseSeed;
}

double MapGen::psRand(double x,double y)
{
  int n = (int)x + (int)y*57;
  n = (n<<13)^n;
  int nn = (n*(n*n*60493+19990303)+this->noiseSeed)&0x7fffffff; //1376312589
  return 1.0-((double)nn/1073741824.0);
}

double MapGen::interpolate(double a,double b,double x)
{
  double ft = x * 3.1415927;
  double f = (1.0-cos(ft))* 0.5;
  return a*(1.0-f)+b*f;
}

double MapGen::noise(double x,double y)
{
  double floorx = (double)((int)x);
  double floory = (double)((int)y);
  double s,t,u,v;
  s = psRand(floorx,floory);
  t = psRand(floorx+1,floory);
  u = psRand(floorx,floory+1);
  v = psRand(floorx+1,floory+1);
  double int1 = interpolate(s,t,x-floorx);
  double int2 = interpolate(u,v,x-floorx);
  return interpolate(int1,int2,y-floory);
}

