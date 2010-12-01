/*
   Copyright (c) 2010, The Mineserver Project
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
 * Neither the name of the The Mineserver Project nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _PACKETS_H
#define _PACKETS_H

#include <string.h>

#define PACKET_NEED_MORE_DATA -3
#define PACKET_DOES_NOT_EXIST -2
#define PACKET_VARIABLE_LEN   -1
#define PACKET_OK             0

#ifndef M_PI
#define M_PI 3.141592653589793238462643
#endif

class PacketHandler;
class User;

//Packet names
enum
{
  //Client to server
  PACKET_KEEP_ALIVE                = 0x00,
  PACKET_LOGIN_REQUEST             = 0x01,
  PACKET_HANDSHAKE                 = 0x02,
  PACKET_CHAT_MESSAGE              = 0x03,
  PACKET_PLAYER_INVENTORY          = 0x05,
  PACKET_RESPAWN                   = 0x09,
  PACKET_PLAYER                    = 0x0a,
  PACKET_PLAYER_POSITION           = 0x0b,
  PACKET_PLAYER_LOOK               = 0x0c,
  PACKET_PLAYER_POSITION_AND_LOOK  = 0x0d,
  PACKET_PLAYER_DIGGING            = 0x0e,
  PACKET_PLAYER_BLOCK_PLACEMENT    = 0x0f,
  PACKET_HOLDING_CHANGE            = 0x10,
  PACKET_ARM_ANIMATION             = 0x12,
  PACKET_DISCONNECT                = 0xff,
  //Server to client
  PACKET_LOGIN_RESPONSE            = 0x01,
  PACKET_TIME_UPDATE               = 0x04,
  PACKET_SPAWN_POSITION            = 0x06,
  PACKET_UPDATE_HEALTH             = 0x08,
  PACKET_ADD_TO_INVENTORY          = 0x11,
  PACKET_NAMED_ENTITY_SPAWN        = 0x14,
  PACKET_PICKUP_SPAWN              = 0x15,
  PACKET_COLLECT_ITEM              = 0x16,
  PACKET_ADD_OBJECT                = 0x17,
  PACKET_MOB_SPAWN                 = 0x18,
  PACKET_DESTROY_ENTITY            = 0x1d,
  PACKET_ENTITY                    = 0x1e,
  PACKET_ENTITY_RELATIVE_MOVE      = 0x1f,
  PACKET_ENTITY_LOOK               = 0x20,
  PACKET_ENTITY_LOOK_RELATIVE_MOVE = 0x21,
  PACKET_ENTITY_TELEPORT           = 0x22,
  PACKET_DEATH_ANIMATION           = 0x26,
  PACKET_PRE_CHUNK                 = 0x32,
  PACKET_MAP_CHUNK                 = 0x33,
  PACKET_MULTI_BLOCK_CHANGE        = 0x34,
  PACKET_BLOCK_CHANGE              = 0x35,
  PACKET_COMPLEX_ENTITIES          = 0x3b,
  PACKET_KICK                      = 0xff,


  //v4 Packets
  PACKET_USE_ENTITY      = 0x07,
  PACKET_ENTITY_VELOCITY = 0x1c,
  PACKET_ATTACH_ENTITY   = 0x27
};

class Packet
{
private:
  typedef std::vector<uint8> bufVector;
  bufVector m_readBuffer;
  bufVector::size_type m_readPos;
  bool m_isValid;

  bufVector m_writeBuffer;
public:
  Packet() : m_readPos(0), m_isValid(true) {}

  bool haveData(int requiredBytes)
  {
    return m_isValid = m_isValid && ((m_readPos + requiredBytes) <= m_readBuffer.size());
  }

  operator bool() const
  {
    return m_isValid;
  }

  void reset()
  {
    m_readPos = 0;
    m_isValid = true;
  }

  void addToRead(std::vector<uint8> &buffer)
  {
    m_readBuffer.insert(m_readBuffer.end(), buffer.begin(), buffer.end());
  }

  void addToRead(const void * data, bufVector::size_type dataSize)
  {
    bufVector::size_type start = m_readBuffer.size();
    m_readBuffer.resize(start + dataSize);
    memcpy(&m_readBuffer[start], data, dataSize);
  }

  void addToWrite(const void * data, bufVector::size_type dataSize)
  {
    if(dataSize == 0)
      return;
    bufVector::size_type start = m_writeBuffer.size();
    m_writeBuffer.resize(start + dataSize);
    memcpy(&m_writeBuffer[start], data, dataSize);
  }

  void removePacket()
  {
    m_readBuffer.erase(m_readBuffer.begin(), m_readBuffer.begin() + m_readPos);
    m_readPos = 0;
  }

  Packet & operator<<(sint8 val)
  {
    m_writeBuffer.push_back(val);
    return *this;
  }

  Packet & operator>>(sint8 &val)
  {
    if(haveData(1))
    {
      val = *reinterpret_cast<const sint8*>(&m_readBuffer[m_readPos]);
      m_readPos += 1;
    }
    return *this;
  }

  Packet & operator<<(sint16 val)
  {
    uint16 nval = htons(val);
    addToWrite(&nval, 2);
    return *this;
  }

  Packet & operator>>(sint16 &val)
  {
    if(haveData(2))
    {
      val = ntohs(*reinterpret_cast<const sint16*>(&m_readBuffer[m_readPos]));
      m_readPos += 2;
    }
    return *this;
  }

  Packet & operator<<(sint32 val)
  {
    uint32 nval = htonl(val);
    addToWrite(&nval, 4);
    return *this;
  }

  Packet & operator>>(sint32 &val)
  {
    if(haveData(4))
    {
      val = ntohl(*reinterpret_cast<const sint32*>(&m_readBuffer[m_readPos]));
      m_readPos += 4;
    }
    return *this;
  }

  Packet & operator<<(sint64 val)
  {
    uint64 nval = ntohll(val);
    addToWrite(&nval, 8);
    return *this;
  }

  Packet & operator>>(sint64 &val)
  {
    if(haveData(8))
    {
      val = *reinterpret_cast<const sint64*>(&m_readBuffer[m_readPos]);
      val = ntohll(val);
      m_readPos += 8;
    }
    return *this;
  }

  Packet & operator<<(float val)
  {
    uint32 nval;
    memcpy(&nval, &val , 4);
    nval = htonl(nval);
    addToWrite(&nval, 4);
    return *this;
  }

  Packet & operator>>(float &val)
  {
    if(haveData(4))
    {
      sint32 ival = ntohl(*reinterpret_cast<const sint32*>(&m_readBuffer[m_readPos]));
      memcpy(&val, &ival, 4);
      m_readPos += 4;
    }
    return *this;
  }

  Packet & operator<<(double val)
  {
    uint64 nval;
    memcpy(&nval, &val, 8);
    nval = ntohll(nval);
    addToWrite(&nval, 8);
    return *this;
  }


  Packet & operator>>(double &val)
  {
    if(haveData(8))
    {
      uint64 ival = *reinterpret_cast<const uint64*>(&m_readBuffer[m_readPos]);
      ival = ntohll(ival);
      memcpy((void*)&val, (void*)&ival, 8);
      m_readPos += 8;
    }
    return *this;
  }

  Packet & operator<<(const std::string &str)
  {
    uint16 lenval = htons(str.size());
    addToWrite(&lenval, 2);

    addToWrite(&str[0], str.size());
    return *this;
  }

  Packet & operator>>(std::string &str)
  {
    sint16 lenval;
    if(haveData(2))
    {
      lenval = ntohs(*reinterpret_cast<const sint16*>(&m_readBuffer[m_readPos]));
      m_readPos += 2;

      if(haveData(lenval))
      {
        str.assign((char*)&m_readBuffer[m_readPos], lenval);
        m_readPos += lenval;
      }
    }    
    return *this;
  }

  void operator<<(Packet &other)
  {
    int dataSize = other.getWriteLen();
    if(dataSize == 0)
      return;
    bufVector::size_type start = m_writeBuffer.size();
    m_writeBuffer.resize(start + dataSize);
    memcpy(&m_writeBuffer[start], other.getWrite(), dataSize);
  }

  void getData(void *buf, int count)
  {
    if(haveData(count))
    {
      memcpy(buf, &m_readBuffer[m_readPos], count);
      m_readPos += count;
    }
  }

  void *getWrite()
  {
    return &m_writeBuffer[0];
  }

  size_t getWriteLen() const
  {
    return m_writeBuffer.size();
  }

  void clearWrite(int count)
  {
    m_writeBuffer.erase(m_writeBuffer.begin(), m_writeBuffer.begin() + count);
  }
};



struct Packets
{
  int len;
  int (PacketHandler::*function)(User *);

  //int (PacketHandler::*varLenFunction)(User *);

  Packets()
  {
    len = PACKET_DOES_NOT_EXIST;
  }
  Packets(int newlen)
  {
    len = newlen;
  }
/*  Packets(int newlen, void (PacketHandler::*newfunction)(uint *, User *))
  {
    len      = newlen;
    function = newfunction;
  }

  Packets(int newlen, void (PacketHandler::*newfunction)(User *))
  {
    len            = newlen;
    function = newfunction;
  }
*/
  Packets(int newlen, int (PacketHandler::*newfunction)(User *))
  {
    len            = newlen;
    function = newfunction;
  }
};

struct packet_login_request
{
  int version;
  std::string Username;
  std::string Password;
  sint64 map_seed;
  uint8 dimension;
};

struct packet_player_position
{
  double x;
  double y;
  double stance;
  double z;
  char onground;
};

struct packet_player_look
{
  float yaw;
  float pitch;
  char onground;
};

struct packet_player_position_and_look
{
  double x;
  double y;
  double stance;
  double z;
  float yaw;
  float pitch;
  char onground;
};

class PacketHandler
{

private:
  PacketHandler()
  {
  }
  ~PacketHandler()
  {
  }

   static PacketHandler *mPacketHandler;
public:

  void init();
  void free();

  static PacketHandler* get()
  {
    if(!mPacketHandler) {
      mPacketHandler = new PacketHandler();
    }
    return mPacketHandler;
  }

  //Information of all the packets
  //around 2kB of memory
  Packets packets[256];

  //The packet functions
  int keep_alive(User *user);
  int login_request(User *user);
  int handshake(User *user);
  int chat_message(User *user);
  int player_inventory(User *user);
  int player(User *user);
  int player_position(User *user);
  int player_look(User *user);
  int player_position_and_look(User *user);
  int player_digging(User *user);
  int player_block_placement(User *user);
  int holding_change(User *user);
  int arm_animation(User *user);
  int pickup_spawn(User *user);
  int disconnect(User *user);
  int complex_entities(User *user);
  int use_entity(User *user);
  int respawn(User *user);

};


#endif
