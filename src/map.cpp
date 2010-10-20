#ifdef WIN32
  #define _CRTDBG_MAP_ALLOC

  #include <crtdbg.h>
  #include <conio.h>
#endif

#include <cstdlib>
#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>
#include <cmath>

#include "map.h"
#include "tools.h"



Map &Map::getInstance()
{
  static Map instance;
  return instance;
}

void Map::initMap()
{
    uint32 i;
    uint32 x,y;
    this->mapDirectory="testmap";
}


void Map::freeMap()
{
    uint32 i;
    uint32 x,y;
}

