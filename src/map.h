
#ifndef _MAP_H
#define _MAP_H


typedef unsigned char uint8;
typedef char sint8;
typedef unsigned int uint32;
typedef int sint32;


void putUint32(uint8 *buf, uint32 value);
void putSint32(uint8 *buf, sint32 value);
void putSint16(uint8 *buf, short value);

double getDouble(uint8 *buf);
float  getFloat(uint8 *buf);
uint32 getUint32(uint8 *buf);
sint32 getSint32(uint8 *buf);
uint32 getUint16(uint8 *buf);
sint32 getSint16(uint8 *buf);

void initMap();
void freeMap();

void my_itoa(int value, std::string& buf, int base);

#endif
