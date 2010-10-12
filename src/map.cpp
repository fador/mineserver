#ifdef WIN32
  #define _CRTDBG_MAP_ALLOC
  #include <stdlib.h>
  #include <crtdbg.h>
  #include <conio.h>
#endif

#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>
#include <cmath>

#include "map.h"

void initMap()
{
    uint32 i;
    uint32 x,y;
}


void freeMap()
{
    uint32 i;
    uint32 x,y;
}




#define BUFFER_SIZE 2048

uint32 getUint32(uint8 *buf)
{
    return (buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|(buf[3]);
}

uint32 getUint24(uint8 *buf)
{
    return (buf[0]<<16)|(buf[1]<<8)|(buf[2]);
}


sint32 getSint32(uint8 *buf)
{
    return (buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|(buf[3]);
}

sint32 getSint16(uint8 *buf)
{
    return (buf[0]<<8)|(buf[1]);
}

/*
void my_itoa(int value, std::string& buf, int base)
{
	std::string hexarray="0123456789abcdef";
	int i = 30;
  buf="";
	
  if(!value) buf="0";
	for(; value && i ; --i, value /= base)
  {
      buf = hexarray[value % base] + buf;
  }
	
}
*/