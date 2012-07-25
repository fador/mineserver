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

#ifndef _BLOCKS_LEAVES_H
#define _BLOCKS_LEAVES_H

#include <time.h>
#include <set>

#include "basic.h"

struct Decay
{
  Decay(time_t decayStart, int32_t x, int32_t y, int32_t z, int32_t map)
    :
    decayStart(decayStart),
    x(x),
    y(y),
    z(z),
    map(map)
  {
  }

  inline bool operator<(const Decay& o) const
  {
    return decayStart < o.decayStart               // Oldest first
      || (decayStart == o.decayStart && x < o.x)
      || (decayStart == o.decayStart && x == o.x && y < o.y)
      || (decayStart == o.decayStart && x == o.x && y == o.y && z < o.z)
      || (decayStart == o.decayStart && x == o.x && y == o.y && z == o.z && map < o.map);
  }

  time_t decayStart;
  int32_t x, y, z, map;
};

class BlockLeaves : public BlockBasic
{
  std::set<Decay> decaying;

  struct DecayFinder
  {
    DecayFinder(int32_t x, int32_t y, int32_t z, int32_t map) : x(x), y(y), z(z), map(map) { }
    inline bool operator()(const Decay& d) const { return x == d.x && y == d.y && z == d.z && map == d.map; }
  private:
    int32_t x, y, z, map;
  };


public:

  inline bool affectedBlock(int block) const { return block == BLOCK_LEAVES; }

  bool onBroken(User* user, int8_t status, int32_t x, int16_t y, int32_t z, int map, int8_t direction);

  void onNeighbourBroken(User* user, int16_t oldblock, int32_t x, int16_t y, int32_t z, int map, int8_t direction);

  void timer200();

};

#endif //_BLOCKS_LEAVES
