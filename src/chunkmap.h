/*
   Copyright (c) 2011, The Mineserver Project
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

#ifndef _CHUNKMAP_H
#define _CHUNKMAP_H

#include <set>
#include <list>
#include <vector>
#include <ctime>
#include <unordered_map>

#include "packets.h"
#include "user.h"
#include "nbt.h"

class NBT_Value;

struct spawnedItem
{
  int EID;
  int16_t item;
  char count;
  int16_t health;
  vec pos;
  time_t spawnedAt;
  uint32_t spawnedBy;

  spawnedItem() : spawnedAt(std::time(NULL)), spawnedBy(0)
  {
  }
};

struct chestData
{
  int32_t x;
  int32_t y;
  int32_t z;
  Item items[27];
};

struct signData
{
  int32_t x;
  int32_t y;
  int32_t z;
  std::string text1;
  std::string text2;
  std::string text3;
  std::string text4;
};

struct furnaceData
{
  int32_t x;
  int32_t y;
  int32_t z;
  Item items[3];
  int16_t burnTime;
  int16_t cookTime;
  int32_t map;
};
void removeFurnace(furnaceData* data_);

struct sChunk
{
  uint8_t* blocks;
  uint8_t* data;
  uint8_t* blocklight;
  uint8_t* skylight;
  uint8_t* heightmap;
  int32_t x;
  int32_t z;

  int refCount;
  bool lightRegen;
  bool changed;
  time_t lastused;

  NBT_Value* nbt;
  std::set<User*>           users;
  std::vector<spawnedItem*> items;

  // ToDo: clear these
  std::vector<chestData*>   chests;
  std::vector<signData*>    signs;
  std::vector<furnaceData*> furnaces;

  sChunk() : refCount(0), lightRegen(false), changed(false), lastused(0), nbt(NULL)
  {
  }

  ~sChunk()
  {
    if (!chests.empty())
    {
      for (std::vector<chestData*>::iterator chest_it = chests.begin(); chest_it != chests.end(); ++chest_it)
      {
        delete *chest_it;
      }
      chests.clear();
    }

    if (!signs.empty())
    {
      for (std::vector<signData*>::iterator sign_it = signs.begin(); sign_it != signs.end(); ++sign_it)
      {
        delete *sign_it;
      }
      signs.clear();
    }

    if (!furnaces.empty())
    {
      for (std::vector<furnaceData*>::iterator furnace_it = furnaces.begin(); furnace_it != furnaces.end(); ++furnace_it)
      {
        removeFurnace((*furnace_it));
        delete *furnace_it;
      }
      furnaces.clear();
    }

    if (nbt != NULL) // unnecessary check, it's safe to delete the null pointer
    {
      delete nbt;
    }
  }

  bool hasUser(User* user) const
  {
    return users.count(user) != 0;
  }

  void sendPacket(const Packet& packet, User* nosend = NULL)
  {
    for (std::set<User*>::iterator it = users.begin(); it != users.end(); ++it)
    {
      if ((*it) != nosend && (*it)->logged)
      {
        (*it)->buffer.addToWrite(packet.getWrite(), packet.getWriteLen());
      }
    }
  }

  static bool userBoundary(sChunk* left, std::list<User*>& lusers, sChunk* right, std::list<User*>& rusers)
  {
    bool diff = false;

    for (std::set<User*>::const_iterator it = left->users.begin(); it != left->users.end(); ++it)
    {
      if (right->users.count(*it) == 0)
      {
        lusers.push_front(*it);
        diff = true;
      }
    }

    for (std::set<User*>::const_iterator it = right->users.begin(); it != right->users.end(); ++it)
    {
      if (left->users.count(*it) == 0)
      {
        rusers.push_front(*it);
        diff = true;
      }
    }

    return diff;
  }
};

/*  STL does not yet come with a hash_combine(), so I'm lifting this
 *  implementation from boost. It creates a hash function for every
 *  pair of hashable types. For further generalizations, see boost/functional/hash.
 */

/** Update: We are temporarily switching to Boost.TR1, so we can use Boost's hash_combine(). **/

template <class T>
inline void my_hash_combine(std::size_t & seed, T const & v)
{
  std::tr1::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

template<typename S, typename T>
struct PairHash : public std::unary_function<std::pair<S, T>, size_t>
{
  inline size_t operator()(const std::pair<S, T> & v) const
  {
    std::size_t seed = 0;
    my_hash_combine(seed, v.first);
    my_hash_combine(seed, v.second);
    return seed;
  }
};

typedef std::pair<int, int> Coords;
typedef std::tr1::unordered_map<Coords, sChunk*, PairHash<int, int> > ChunkMap;
//typedef std::tr1::unordered_map<Coords, sChunk*> ChunkMap;


#endif
