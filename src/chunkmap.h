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

#ifndef _CHUNKMAP_H_
#define _CHUNKMAP_H_

#include "tools.h"
#include <set>
#include <list>

class User;
class NBT_Value;
struct spawnedItem;

struct sChunk
{
  sChunk() : refCount(0)
  {
  }
  uint8* blocks;
  uint8* data;
  uint8* blocklight;
  uint8* skylight;
  uint8* heightmap;
  sint32 x;
  sint32 z;
  NBT_Value *nbt;
  std::set<User*> users;
  std::vector<spawnedItem *> items;

  bool HasUser(User* user)
  {
	  return users.count(user) != 0;
  }

  void sendPacket(const Packet &packet, User *dontsend = NULL)
  {
	  std::set<User*>::iterator iter = users.begin(), end = users.end();

	  for( ; iter != end ; iter++ )
	  {
		  if((*iter) != dontsend)
		    (*iter)->buffer.addToWrite(packet.getWrite(), packet.getWriteLen());
	  }
  }

  static bool UserBoundry(sChunk *left, std::list<User*> &lusers, sChunk *right, std::list<User*> &rusers)
  {
	 bool diff = false;
	 std::set<User*>::iterator iter = left->users.begin(), end = left->users.end();
	 for( ; iter != end ; iter++)
	 {
		 if(!right->users.count(*iter))
		 {
			 lusers.push_front(*iter);
			 diff = true;
		 }
	 }

	 iter = right->users.begin(), end = right->users.end();
	 for( ; iter != end ; iter++)
	 {
		 if(!left->users.count(*iter))
		 {
			 diff = true;
			 rusers.push_front(*iter);
		 }
	 }
	 return diff;
  }

  int refCount;
};

struct sChunkNode
{
  sChunkNode(sChunk* _chunk, sChunkNode* _next)
    : chunk(_chunk), next(_next)
  {}
  sChunk* chunk;
  sChunkNode* next;
};

class ChunkMap
{
private:
	sChunkNode* m_buckets[441];
public:
	ChunkMap()
	{
		memset(m_buckets, 0, sizeof(m_buckets));
	}

	int Hash(int x, int z)
	{
		x %= 21;
		if(x < 0)
			x += 21;

		z %= 21;
		if(z < 0)
			z += 21;

		return x + z * 21;
	}

	sChunk* GetChunk(int x, int z)
	{
		sChunkNode* node = m_buckets[Hash(x,z)];
		if(node == NULL)
			return NULL;

		do
		{
			if(node->chunk->x == x && node->chunk->z == z)
				return node->chunk;

			node = node->next;
		} while(node != NULL);

		return NULL;
	}

	void UnlinkChunk(int x, int z)
	{
		int _hash = Hash(x, z);

		sChunkNode* node = m_buckets[_hash];
		if(node == NULL)
			return;

		if(node->chunk->x == x && node->chunk->z == z)
		{
			node->chunk->refCount--;
			m_buckets[_hash] = node->next;
			delete node;
		}
		else
		{
			sChunkNode* prev = node;
			node = node->next;
			while(node != NULL)
			{
				if(node->chunk->x == x && node->chunk->z == z)
					break;

				prev = node;
				node = node->next;
			}

			if(node != NULL)
			{
				prev->next = node->next;
				node->chunk->refCount--;
				delete node;
			}
		}
	}

	void LinkChunk(sChunk* chunk, int x, int z)
	{
		int _hash = Hash(x, z);
		chunk->refCount++;
		m_buckets[_hash] = new sChunkNode(chunk, m_buckets[_hash]);
	}

	void Clear()
	{
		for(int i = 0; i < 21 * 21; i++)
		{
			sChunkNode* node = m_buckets[i];
			while(node != NULL)
			{
				sChunkNode* next = node->next;
				delete node;
				node = next;
			}
			m_buckets[i] = NULL;
		}
	}
};

#endif
