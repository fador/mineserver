#ifndef _TOOLS_H
#define _TOOLS_H

typedef unsigned char uint8;
typedef char sint8;
typedef unsigned short int uint16;
typedef short int sint16;
typedef unsigned int uint32;
typedef int sint32;
typedef unsigned long long int uint64;
typedef long long int sint64;

void putSint64(uint8 *buf, long long value);
void putUint32(uint8 *buf, uint32 value);
void putSint32(uint8 *buf, sint32 value);
void putSint16(uint8 *buf, short value);
void putDouble(uint8 *buf, double value);
void putFloat(uint8 *buf, float value);

long long getSint64(uint8 *buf);
double getDouble(uint8 *buf);
float  getFloat(uint8 *buf);
uint32 getUint32(uint8 *buf);
sint32 getSint32(uint8 *buf);
uint32 getUint16(uint8 *buf);
sint32 getSint16(uint8 *buf);

void my_itoa(int value, std::string& buf, int base);
std::string base36_encode(int value);
std::string strToLower(std::string temp);

#endif
