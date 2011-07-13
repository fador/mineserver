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

#include "tr1.h"
#include TR1INCLUDE(unordered_map)
#include TR1INCLUDE(memory)

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

typedef std::tr1::shared_ptr<std::vector<ItemPtr> > ItemVectorPtr;

/** holds items and coordinates for small and large chests
 * note: large chests are in testing
 */
class chestData
{
public:
  int32_t id;
  int32_t x() { return m_x; }
  void x(int32_t newx) { m_x = newx; }
  int32_t y() { return m_y; }
  void y(int32_t newy) { m_y = newy; }
  int32_t z() { return m_z; }
  void z(int32_t newz) { m_z = newz; }
  //Item& getItem(size_t i);
  bool large() { return this->size() > 27; }
  void large(bool _large)
  {
    if(_large != this->large())
    {
      size_t new_length = 0;
      if(_large)
      {
        new_length = 54+3;
      } else {
        new_length = 27+3;
      }
      while(new_length > items()->size())
        items()->push_back(ItemPtr(new Item));
      while(new_length < items()->size())
        items()->pop_back();
    }
    return;
   }

  chestData()
  {
    this->items(ItemVectorPtr(new std::vector<ItemPtr>()));
    while(items()->size() < 27)
      items()->push_back(ItemPtr(new Item));
  }

  ~chestData()
  { }

  size_t size()
  {
    return items()->size();
  }

  ItemVectorPtr items() { return m_items; }
  void items(ItemVectorPtr itemVectorPtr)
  {
    m_items = itemVectorPtr;
  }


private:
  int32_t m_x;
  int32_t m_y;
  int32_t m_z;
  ItemVectorPtr m_items;
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

// The following predicate allows for finding *Data members based on their coordinates.
template <typename T>
struct DataFinder
{
  DataFinder(int32_t x, int32_t y, int32_t z) : x(x), y(y), z(z) { }
  inline bool operator()(const std::tr1::shared_ptr<T> & t) const { return t->x == x && t->y == y && t->z == z; }
private:
  int32_t x;
  int32_t y;
  int32_t z;
};

typedef std::tr1::shared_ptr<chestData>   chestDataPtr;
typedef std::tr1::shared_ptr<signData>    signDataPtr;
typedef std::tr1::shared_ptr<furnaceData> furnaceDataPtr;

void removeFurnace(furnaceDataPtr data);

/** holds chunk data (16x16x16 blocks ?)
 */
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

  std::vector<chestDataPtr>   chests;
  std::vector<signDataPtr>    signs;
  std::vector<furnaceDataPtr> furnaces;

  sChunk() : refCount(0), lightRegen(false), changed(false), lastused(0), nbt(NULL)
  {
  }

  ~sChunk()
  {
    if (!furnaces.empty())
    {
      for (std::vector<furnaceDataPtr>::iterator furnace_it = furnaces.begin(); furnace_it != furnaces.end(); ++furnace_it)
      {
        removeFurnace(*furnace_it);
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
        (*it)->buffer.addToWrite(packet);
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

template <class T>
inline void hash_combine(std::size_t & seed, T const & v)
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
    hash_combine(seed, v.first);
    hash_combine(seed, v.second);
    return seed;
  }
};

typedef std::pair<int, int> Coords;
typedef std::tr1::unordered_map<Coords, sChunk*, PairHash<int, int> > ChunkMap;


#endif
