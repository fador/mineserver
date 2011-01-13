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

#ifndef _CHUNKMAP_H
#define _CHUNKMAP_H

#include <set>
#include <list>
#include <vector>

#include "packets.h"
#include "user.h"

class User;
class NBT_Value;
struct spawnedItem;
struct chestData;
struct furnaceData;
struct signData;

struct sChunk
{
  sChunk() : refCount(0),lightRegen(false),changed(false),lastused(0) {}
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

  bool hasUser(User* user)
  {
    return users.count(user) != 0;
  }

  void sendPacket(const Packet &packet, User* nosend = NULL)
  {
    std::set<User*>::iterator iter_a = users.begin(), iter_b = users.end();

    for (;iter_a!=iter_b;++iter_a)
    {
      if ((*iter_a) != nosend)
      {
        (*iter_a)->buffer.addToWrite(packet.getWrite(), packet.getWriteLen());
      }
    }
  }

  static bool userBoundary(sChunk* left, std::list<User*> &lusers, sChunk* right, std::list<User*> &rusers)
  {
    bool diff = false;

    std::set<User*>::iterator iter_a;
    std::set<User*>::iterator iter_b;

    iter_a = left->users.begin(), iter_b = left->users.end();
    for(;iter_a!=iter_b;++iter_a)
    {
      if (!right->users.count(*iter_a))
      {
        lusers.push_front(*iter_a);
        diff = true;
      }
    }

    iter_a = right->users.begin(), iter_b = right->users.end();
    for(;iter_a!=iter_b;++iter_a)
    {
      if (!left->users.count(*iter_a))
      {
        rusers.push_front(*iter_a);
        diff = true;
      }
    }

    return diff;
  }
};

struct sChunkNode
{
  sChunkNode(sChunk* _chunk, sChunkNode* _prev, sChunkNode* _next) : chunk(_chunk),prev(_prev),next(_next) {}
  sChunk* chunk;
  sChunkNode* prev;
  sChunkNode* next;
};

class ChunkMap
{
public:
  ChunkMap()
  {
    memset(m_buckets, 0, sizeof(m_buckets));
  }

  int hash(int x, int z)
  {
    x %= 21;
    if (x < 0)
    {
      x += 21;
    }

    z %= 21;
    if (z < 0)
    {
      z += 21;
    }

    return x + z * 21;
  }

  sChunk* getChunk(int x, int z)
  {
    sChunkNode* node = NULL;

    for (node=m_buckets[hash(x,z)];node!=NULL;node=node->next)
    {
      if ((node->chunk->x == x) && (node->chunk->z == z))
      {
        return node->chunk;
      }
    }

    return NULL;
  }

  void unlinkChunk(int x, int z)
  {
    int _hash = hash(x, z);

    sChunkNode* root = m_buckets[_hash];
    sChunkNode* node = root;

    // Loop until we reach the end of the chain
    while (node != NULL)
    {
      // We've got the right node, time to get to work!
      if ((node->chunk->x == x) && (node->chunk->z == z))
      {
        node->chunk->refCount--;

        // If we have both next and previous nodes, we need to connect them up
        // when we remove this node because we're in the middle of the chain.
        if (node->next != NULL && node->prev != NULL)
        {
          node->next->prev = node->prev;
          node->prev->next = node->next;
        }
        // Otherwise we're at one of the ends of the chain, so we just need to
        // cut it off where it is.
        else if (node->next != NULL)
        {
          node->next->prev = NULL;
        }
        else if (node->prev != NULL)
        {
          node->prev->next = NULL;
        }

        // If the node we're looking at is the root node, we need to update the
        // bucket to point at the new start of the chain.
        if (node == root)
        {
          m_buckets[_hash] = node->next;
        }

        // Free up the memory we were using for this node.
        delete node;

        return;
      }
      // This isn't the right node, look at the next one.
      else
      {
        node = node->next;
      }
    }
  }

  void linkChunk(sChunk* chunk, int x, int z)
  {
    int _hash = hash(x, z);
    chunk->refCount++;
    m_buckets[_hash] = new sChunkNode(chunk, NULL, m_buckets[_hash]);
  }

  void clear()
  {
    for(int i = 0; i < 21 * 21; i++)
    {
      sChunkNode* node = m_buckets[i];
      sChunkNode* next = NULL;
      while (node != NULL)
      {
        next = node->next;
        delete node;
        node = next;
      }
      m_buckets[i] = NULL;
    }
  }

private:
  sChunkNode* m_buckets[441];
};

#endif
