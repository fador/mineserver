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
#include <string>
#include <cctype>

#include "tools.h"

void putSint64(uint8 *buf, long long value)
{
  uint8 *pointer = reinterpret_cast<uint8 *>(&value);
  for(uint8 i=0;i<8;i++)
  {
    buf[i]=pointer[7-i];  
  }
}

void putUint32(uint8 *buf, uint32 value)
{
  uint8 *pointer = reinterpret_cast<uint8 *>(&value);
  buf[0]=pointer[3];
  buf[1]=pointer[2];
  buf[2]=pointer[1];
  buf[3]=pointer[0];
}

void putSint32(uint8 *buf, sint32 value)
{
  uint8 *pointer = reinterpret_cast<uint8 *>(&value);
  buf[0]=pointer[3];
  buf[1]=pointer[2];
  buf[2]=pointer[1];
  buf[3]=pointer[0];
}

void putSint16(uint8 *buf, short value)
{
  uint8 *pointer = reinterpret_cast<uint8 *>(&value);
  buf[0]=pointer[1];
  buf[1]=pointer[0];
}

void putFloat(uint8 *buf, float value)
{
  uint8 *floatAddress=reinterpret_cast<uint8 *>(&value);
  for(uint8 i=0;i<4;i++)
  {
    buf[i]=floatAddress[3-i];
  }
}

void putDouble(uint8 *buf, double value)
{
  uint8 *doubleAddress=reinterpret_cast<uint8 *>(&value);
  for(uint8 i=0;i<8;i++)
  {
    buf[i]=doubleAddress[7-i];
  }
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

float getFloat(uint8 *buf)
{
  float temp;
  uint8 *floatAddress=reinterpret_cast<uint8 *>(&temp);
  for(uint8 i=0;i<4;i++)
  {
    floatAddress[3-i]=buf[i];
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

long long getSint64(uint8 *buf)
{
  long long temp;
  uint8 *longAddress=reinterpret_cast<uint8 *>(&temp);
  for(uint8 i=0;i<8;i++)
  {
    longAddress[7-i]=buf[i];
  }
  return temp;
}

sint32 getSint32(uint8 *buf)
{
  int temp;
  uint8 *intAddress=reinterpret_cast<uint8 *>(&temp);
  for(uint8 i=0;i<4;i++)
  {
    intAddress[3-i]=buf[i];
  }

  return temp;
}

sint32 getSint16(uint8 *buf)
{
  short temp;
  uint8 *shortAddress=reinterpret_cast<uint8 *>(&temp);
  for(uint8 i=0;i<2;i++)
  {
    shortAddress[1-i]=buf[i];
  }

  return temp;
}

std::string base36_encode(int value)
{
  std::string output;
  my_itoa((int)abs(value),output, 36);
  if(value<0) output.insert (output.begin(),'-');

  return output;
}

void my_itoa(int value, std::string& buf, int base)
{
  std::string hexarray("0123456789abcdefghijklmnopqrstuvwxyz");
  int i = 30;
  buf = "";
	
  if(!value)
    buf="0";

  for(; value && i ; --i, value /= base)
  {
    buf.insert(buf.begin(),(char)hexarray[value % base]);
  }
}

std::string strToLower(std::string temp)
{
  const int len = temp.length();

  for(int i=0; i!=len; ++i)
  {
    temp[i] = std::tolower(temp[i]);
  }

  return temp;
}
