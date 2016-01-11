/*
   Copyright (c) 2013, The Mineserver Project
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

#include <deque>
#include <string>
#include <cstring>
#include <stdint.h>

#include <stdio.h>

/// endian functions... because windows htons are actually library functions

inline uint16_t _htons(uint16_t x){
#ifdef __GNUC__
  /// return __builtin_bswap16(x); /// TODO: use when works, doesn't work in mingw64 4.7.1
  return x<<8 | x>>8;
#else
  return x<<8 | x>>8;
#endif
}

inline uint32_t _htonl(uint32_t x){
#ifdef __GNUC__
  return __builtin_bswap32(x);
#else
  uint16_t low  = x>>16;
  uint16_t high = x;

  return uint32_t(_htons(high))<<16 | _htons(low);
#endif
}

inline uint64_t _htonll(uint64_t x){
#ifdef __GNUC__
  return __builtin_bswap64(x);
#else
  uint32_t low = uint32_t(x >> 32);
  uint32_t high = uint32_t(x);

  return uint64_t(_htonl(high))<<32 | _htonl(low);
#endif
}

#ifndef __BIG_ENDIAN__
#define htons(x)  _htons ((uint16_t)x)
#define htonl(x)  _htonl ((uint32_t)x)
#define htonll(x) _htonll((uint64_t)x)
#else
#define htons
#define htonl
#define htonll
#endif
/// bswap c++ endian flipper!

#define bswap_def(TYPE, FLIPPER) \
  inline TYPE bswap(const TYPE& x){ return (TYPE)FLIPPER(x); }

bswap_def(int8_t, )
bswap_def(uint8_t, )

bswap_def(int16_t, htons)
bswap_def(uint16_t, htons)

bswap_def(int32_t, htonl)
bswap_def(uint32_t, htonl)

bswap_def(int64_t, htonll)
bswap_def(uint64_t, htonll)

/// because float/double casting is fucking special in c++
/// FIXME: find a way to cast float types the right way to data types of uint
inline float bswap(const float& x){
  uint32_t i = uint32_t(htonl(*(uint32_t*)&x));
  return *(float*)&i;
}

inline double bswap(const double& x){
  uint64_t i = htonll(*(uint64_t*)&x);
  return *(double*)&i;
}

#undef bswap_def

#undef htons
#undef htonl
#undef htonll

#define PACKET_NEED_MORE_DATA -3
#define PACKET_DOES_NOT_EXIST -2
#define PACKET_VARIABLE_LEN   -1
#define PACKET_OK             0

#ifndef M_PI
#define M_PI 3.141592653589793238462643
#endif

class PacketHandler;
class User;

// gameState names
enum {
  STATE_HANDSHAKE = 0,
  STATE_STATUS    = 1,
  STATE_LOGIN     = 2,
  STATE_PLAY      = 3
};

//Packet names
enum
{
  //Client to server

  // State: Handshake
  PACKET_IN_HANDSHAKE                 = 0x00,

  // State: Status
  PACKET_IN_SERVER_LIST_PING          = 0x00,
  PACKET_IN_PING                      = 0x01,

  // State: Login
  PACKET_IN_LOGIN_REQUEST             = 0x00,
  PACKET_IN_ENCRYPTION_RESPONSE       = 0x01,
  
  // State: Play
  PACKET_IN_KEEP_ALIVE                = 0x00,
  PACKET_IN_CHAT_MESSAGE              = 0x01,
  PACKET_IN_USE_ENTITY                = 0x02,
  PACKET_IN_PLAYER                    = 0x03,
  PACKET_IN_PLAYER_POSITION           = 0x04,
  PACKET_IN_PLAYER_LOOK               = 0x05,
  PACKET_IN_PLAYER_POSITION_AND_LOOK  = 0x06,
  PACKET_IN_PLAYER_DIGGING            = 0x07,
  PACKET_IN_PLAYER_BLOCK_PLACEMENT    = 0x08,
  PACKET_IN_HELD_ITEM_CHANGE          = 0x09,    
  PACKET_IN_ANIMATION                 = 0x0a,
  PACKET_IN_ENTITY_ACTION             = 0x0b,
  PACKET_IN_CLOSE_WINDOW              = 0x0d,
  PACKET_IN_CLICK_WINDOW              = 0x0e,
  PACKET_IN_CONFIRM_TRANSACTION       = 0x0f,
  PACKET_IN_CREATIVE_INVENTORY_ACTION = 0x10,
  PACKET_IN_ENCHANT_ITEM              = 0x11,
  PACKET_IN_UPDATE_SIGN               = 0x12,
  PACKET_IN_PLAYER_ABILITIES          = 0x13,
  PACKET_IN_TAB_COMPLETE              = 0x14,
  PACKET_IN_CLIENT_SETTINGS           = 0x15,
  PACKET_IN_CLIENT_STATUS             = 0x16,
  PACKET_IN_PLUGIN_MESSAGE            = 0x17,


  PACKET_IN_INVENTORY                 = 0x99,
  PACKET_IN_BLOCK_CHANGE              = 0x99,


  //Server to client

  // State: Login  
  PACKET_OUT_ENCRYPTION_REQUEST        = 0x01,
  PACKET_OUT_LOGIN_SUCCESS             = 0x02,
  PACKET_OUT_SET_COMPRESSION           = 0x03,

  // State: play
  PACKET_OUT_KEEP_ALIVE                = 0x00,
  PACKET_OUT_JOIN_GAME                 = 0x01,
  PACKET_OUT_CHAT_MESSAGE              = 0x02,
  PACKET_OUT_TIME_UPDATE               = 0x03,
  PACKET_OUT_ENTITY_EQUIPMENT          = 0x04,
  PACKET_OUT_SPAWN_POSITION            = 0x05,
  PACKET_OUT_UPDATE_HEALTH             = 0x06,
  PACKET_OUT_client_status                   = 0x07,
  PACKET_OUT_PLAYER_POSITION_AND_LOOK  = 0x08,
  PACKET_OUT_HELD_ITEM_CHANGE          = 0x09,
  PACKET_OUT_USE_BED                   = 0x0a,
  PACKET_OUT_ANIMATION                 = 0x0b,
  PACKET_OUT_SPAWN_PLAYER              = 0x0c,
  PACKET_OUT_COLLECT_ITEM              = 0x0d,
  PACKET_OUT_SPAWN_OBJECT              = 0x0e,
  PACKET_OUT_SPAWN_MOB                 = 0x0f,
  PACKET_OUT_SPAWN_PAINTING            = 0x10,
  PACKET_OUT_SPAWN_EXPERIENCE_ORB      = 0x11,
  PACKET_OUT_ENTITY_VELOCITY           = 0x12,
  PACKET_OUT_DESTROY_ENTITIES          = 0x13,
  PACKET_OUT_ENTITY                    = 0x14,
  PACKET_OUT_ENTITY_RELATIVE_MOVE      = 0x15,
  PACKET_OUT_ENTITY_LOOK               = 0x16,
  PACKET_OUT_ENTITY_LOOK_RELATIVE_MOVE = 0x17,
  PACKET_OUT_ENTITY_TELEPORT           = 0x18,
  PACKET_OUT_ENTITY_HEAD_LOOK          = 0x19,
  PACKET_OUT_ENTITY_STATUS             = 0x1a,
  PACKET_OUT_ATTACH_ENTITY             = 0x1b,
  PACKET_OUT_ENTITY_METADATA           = 0x1c,
  PACKET_OUT_ENTITY_EFFECT             = 0x1d,
  PACKET_OUT_REMOVE_ENTITY_EFFECT      = 0x1e,
  PACKET_OUT_SET_EXPERIENCE            = 0x1f,
  PACKET_OUT_ENTITY_PROPERTIES         = 0x20,
  PACKET_OUT_MAP_CHUNK                 = 0x21,
  PACKET_OUT_MULTI_BLOCK_CHANGE        = 0x22,
  PACKET_OUT_BLOCK_CHANGE              = 0x23,
  PACKET_OUT_BLOCK_ACTION              = 0x24,
  PACKET_OUT_BLOCK_BREAK_ANIMATION     = 0x25,
  PACKET_OUT_MAP_CHUNK_BULK            = 0x26,
  PACKET_OUT_EXPLOSION                 = 0x27,
  PACKET_OUT_EFFECT                    = 0x28,
  PACKET_OUT_SOUND_EFFECT              = 0x29,
  PACKET_OUT_PARTICLE                  = 0x2a,
  PACKET_OUT_CHANGE_GAME_STATE         = 0x2b,
  PACKET_OUT_SPAWN_GLOBAL_ENTITY       = 0x2c,
  PACKET_OUT_OPEN_WINDOW               = 0x2d,
  PACKET_OUT_CLOSE_WINDOW              = 0x2e,
  PACKET_OUT_SET_SLOT                  = 0x2f,
  PACKET_OUT_WINDOW_ITEMS              = 0x30,
  PACKET_OUT_WINDOW_PROPERTY           = 0x31,
  PACKET_OUT_CONFIRM_TRANSACTION       = 0x32,
  PACKET_OUT_UPDATE_SIGN               = 0x33,
  PACKET_OUT_MAP                       = 0x34,
  PACKET_OUT_UPDATE_BLOCK_ENTITY       = 0x35,
  PACKET_OUT_OPEN_SIGN_EDITOR          = 0x36,
  PACKET_OUT_STATISTICS                = 0x37,
  PACKET_OUT_PLAYER_LIST_ITEM          = 0x38,
  PACKET_OUT_PLAYER_ABILITIES          = 0x39,
  PACKET_OUT_TAB_COMPLETE              = 0x3a,

  PACKET_OUT_SCOREBOARD_OBJECTIVE      = 0x3b,
  PACKET_OUT_UPDATE_SCORE              = 0x3c,
  PACKET_OUT_DISPLAY_SCOREBOARD        = 0x3d,
  PACKET_OUT_TEAMS                     = 0x3e,

  PACKET_OUT_PLUGIN_MESSAGE            = 0x3f,
  PACKET_OUT_DISCONNECT                = 0x40,
  PACKET_OUT_SERVER_DIFFICULTY         = 0x41,
  PACKET_OUT_COMBAT_EVENT              = 0x42,
  PACKET_OUT_CAMERA                    = 0x43,
  PACKET_OUT_WORLD_BORDER              = 0x44,
  PACKET_OUT_TITLE                     = 0x45,
  PACKET_OUT_PLAYERLIST_HEADER_FOOTER  = 0x47,
  PACKET_OUT_RESOURCE_PACK_SEND        = 0x48,
  PACKET_OUT_UPDATE_ENTITY_NBT         = 0x49  
  

};

typedef struct VarInt_internal {
  VarInt_internal() {};
  VarInt_internal(int64_t init_value) : val(init_value) {};
  VarInt_internal(uint32_t init_value) : val((int64_t)init_value) {};
  operator int64_t() const { return val; }
  int64_t val;
} MS_VarInt;

class Packet
{
  // A deque has random-access iterators, so we can track the read position in an integer.
  typedef std::deque<uint8_t> BufferVector;

public:
  BufferVector m_readBuffer;
  BufferVector m_writeBuffer;
  size_t m_readPos;
  bool m_isValid;

public:
  Packet()
    :
      m_readBuffer(),
      m_writeBuffer(),
      m_readPos(0),
      m_isValid(true)
  {
  }

  inline uint8_t firstwrite() const { return m_writeBuffer.front(); }

  inline bool haveData(int requiredBytes)
  {
    return m_isValid = m_isValid && (m_readPos + requiredBytes <= m_readBuffer.size());
  }

  inline operator bool() const
  {
    return m_isValid;
  }

  inline void reset()
  {
    m_readPos = 0;
    m_isValid = true;
  }

  inline void addToReadBegin(const uint8_t* const data, const size_t len)
  {
    m_readBuffer.insert(m_readBuffer.begin(), data, data + len);
    m_isValid = true;
  }

  inline void addToRead(const uint8_t* const data, const size_t len)
  {
    m_readBuffer.insert(m_readBuffer.end(), data, data + len);
    m_isValid = true;
  }

  inline void writePacket(const Packet& p, uint16_t compression);

  inline void addToWrite(const Packet& p)
  {
    m_writeBuffer.insert(m_writeBuffer.end(), p.m_writeBuffer.begin(), p.m_writeBuffer.end());
  }

  inline void addToWrite(const uint8_t* const buffer, const size_t len)
  {
    m_writeBuffer.insert(m_writeBuffer.end(), buffer, buffer + len);
  }

  size_t read( void* buffer, size_t size){
    if(haveData(size)){
      for(size_t i=0;i<size;i++)
        *((uint8_t*&)buffer)++ = m_readBuffer[m_readPos++];
      return size;
    }
    else return 0;
  }

  void write( const void* data, size_t size){
    addToWrite((uint8_t*)data,size);
  }

  inline void removePacket()
  {
    m_readBuffer.erase(m_readBuffer.begin(), m_readBuffer.begin() + m_readPos);
    m_readPos = 0;
  }
  inline void removePacketLen(uint32_t len)
  {
    m_readBuffer.erase(m_readBuffer.begin(), m_readBuffer.begin() + len);
    m_readPos = 0;
  }

  /// This was too much to write by hand... so I macro'ed - Exim
  /// Oh wow! These macros are truly Majestic! I can hardly keep my breath. Astounding... - Everyone

#define _rwop(T) \
  Packet& operator<<(const T& x){ \
  T y = bswap(x); \
  write(&y, sizeof(T)); \
  return *this; \
} \
  Packet& operator>>(T& x){ \
  if(read(&x,sizeof(T))) \
  x = bswap(x); \
  return *this; \
}

  _rwop( int8_t )
  _rwop(uint8_t )

  _rwop( int16_t )
  _rwop(uint16_t )

  _rwop( int32_t )
  _rwop(uint32_t )

  _rwop( int64_t )
  _rwop(uint64_t )

  _rwop(float  )
  _rwop(double )
#undef _rwop


  // convert to wstring and call that operator
  Packet& operator<<(const std::string& str);
  Packet& operator>>(std::string& str);

  // Varint reading and writing
  Packet& operator<<(const MS_VarInt& varint);
  Packet& operator>>(MS_VarInt& varint);

  void writeString(const std::string& str);
  std::string readString();

  void writeVarInt(int64_t varint);
  int64_t readVarInt();

  Packet& operator<<(const Packet& other);

  inline void getData(uint8_t* buf, size_t count)
  {
    if (haveData(count))
    {
      std::copy(m_readBuffer.begin() + m_readPos, m_readBuffer.begin() + m_readPos + count, buf);
      m_readPos += count;
    }
  }

  inline bool getWriteEmpty() const
  {
    return m_writeBuffer.empty();
  }

  inline void getWriteData(std::vector<char>& buf) const
  {
    buf.clear();
    buf.resize(m_writeBuffer.size(), 0);
    std::copy(m_writeBuffer.begin(), m_writeBuffer.end(), buf.begin());
  }

  inline void clearWrite(size_t count)
  {
    BufferVector::iterator it = m_writeBuffer.begin();
    std::advance(it, count);
    m_writeBuffer.erase(m_writeBuffer.begin(), it);
  }
};


struct Packets
{
  typedef int (*handler_function)(User*);

  int len;
  handler_function function;

  Packets(int newlen = PACKET_DOES_NOT_EXIST)
    : len(newlen)
  {
  }

  Packets(handler_function newfunction)
    : len(PACKET_OK), function(newfunction)
  {
  }

  Packets(const Packets & other)
    : len(other.len), function(other.function)
  {
  }
};

struct packet_login_request
{
  int version;
  std::string Username;
  std::string Password;
  int64_t map_seed;
  uint8_t dimension;
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
public:
  void init();

  // Information of all the packets
  // around 4*2kB of memory
  // 4 gamestates with max 256 packets each
  Packets packets[4][256];

  // The packet functions
  static int keep_alive(User* user);
  static int login_request(User* user);
  static int handshake(User* user);
  static int chat_message(User* user);
  static int player(User* user);
  static int player_position(User* user);
  static int player_look(User* user);
  static int player_position_and_look(User* user);
  static int player_digging(User* user);
  static int player_block_placement(User* user);
  static int held_item_change(User* user);
  static int animation(User* user);
  static int pickup_spawn(User* user);
  static int use_entity(User* user);
  static int client_status(User* user);
  static int update_sign(User* user);
  static int confirm_transaction(User* user);
  static int tab_complete(User* user);
  static int encryption_response(User* user);
  static int plugin_message(User* user);

  static int client_settings(User* user);

  static int creative_inventory(User* user);
  static int player_abilities(User* user);

  static int click_window(User* user);
  static int close_window(User* user);
  static int destroy_entity(User* user);

  static int entity_action(User* user);

  static int unhandledPacket(User* user);

  static int ping(User* user);
  static int server_list_ping(User* user);


  static int block_change(User* user);

};

#endif
