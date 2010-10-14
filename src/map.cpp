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


void putSint32(uint8 *buf, sint32 value)
{
  uint8 *pointer = reinterpret_cast<uint8 *>(&value);
  buf[0]=pointer[3];
  buf[1]=pointer[2];
  buf[2]=pointer[1];
  buf[3]=pointer[0];
}

void putShort(uint8 *buf, short value)
{
  uint8 *pointer = reinterpret_cast<uint8 *>(&value);
  buf[0]=pointer[1];
  buf[1]=pointer[0];
}

double getDouble(uint8 *buf)
{
  double temp;
  uint8 *doubleAddress=reinterpret_cast<uint8 *>(&temp);
  for(uint8 i=0;i<8;i++)
  {
    doubleAddress[7-i]=buf[i];
  }

  return temp;
}

uint32 getUint32(uint8 *buf)
{
    return (buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|(buf[3]);
}

uint32 getUint24(uint8 *buf)
{
    return (buf[0]<<16)|(buf[1]<<8)|(buf[2]);
}

uint32 getUint16(uint8 *buf)
{
    return (buf[0]<<8)|(buf[1]);
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