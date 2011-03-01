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

#ifndef _TREE_H_
#define _TREE_H_

#include "constants.h"
#include "mineserver.h"
#include "map.h"
#include "vec.h"
#include <stack>

enum { MAX_TRUNK = 13, MIN_TRUNK = 4, MAX_CANOPY = 3, MIN_CANOPY = 2 ,
       BRANCHING_HEIGHT = 6, BRANCHING_CHANCE = 40,
       MIN_TREE_SPACE = 5
     };// Lucky 7 for good branches ^^

class ITree
{
public:
  ITree() { }
  virtual ~ITree() { }

  virtual void update()
  {
    Mineserver::get()->map(_map)->setBlock(_x, _y, _z, _type, _meta);
    Mineserver::get()->map(_map)->sendBlockChange(_x, _y, _z, _type, _meta);
  }
  virtual void setY(int32_t y)
  {
    _y = y;
  }
  virtual const int32_t getY(void)
  {
    return _y;
  }
  virtual const vec location(void)
  {
    return vec(_x, _y, _z);
  }
  virtual const int type(void)
  {
    return _type;
  }
protected:
  int32_t _x;
  int32_t _y;
  int32_t _z;
  int _map;
  uint8_t _type;
  char _meta;
};

class Trunk : public ITree
{
public:
  Trunk(int32_t x, int32_t y, int32_t z, int map, char meta = 0)
  {
    _x = x, _y = y, _z = z, _map = map, _type = BLOCK_WOOD, _meta = meta;
    update();
  }
  ~Trunk() { }
protected:
};

class Canopy : public ITree
{
public:
  Canopy(int32_t x, int32_t y, int32_t z, int map, char meta = 0)
  {
    _x = x, _y = y, _z = z, _map = map, _type = BLOCK_LEAVES, _meta = meta;
    update();
  }
  ~Canopy() { }
protected:
};

class Tree : public ITree
{
public:
  Tree(int32_t x, int32_t y, int32_t z, int map, uint8_t limit = MAX_TRUNK);
  void generate(uint8_t);
  ~Tree(void);
protected:
  void set(int32_t xloc, int32_t yloc, int32_t zloc, int blocktType, char metaData);
private:
  Trunk* m_Branch[256]; // 1KB on x86 and 2KB on x86_64 Faster than stack or vector tho :)
  // With full array of allocated classes it rounds up to...
  // 3.6KB on x86 :F 4.6KB on x86_64
  // it is a good enough buffer for absolutely MASSIVE MASSIVE TREES
  // Like in Avatar *_*
  uint8_t n_branches;

  void generateCanopy();
  void generateBranches(Trunk*);
};

#endif
