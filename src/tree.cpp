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

#include <cmath>
#include "tree.h"
#include "mineserver.h"

#include "tools.h"


Tree::Tree(int32_t x, int32_t y, int32_t z, int map, uint8_t limit)
{
  n_branches = 0;
  _x = x, _y = y, _z = z, _map = map;
  this->generate(limit);
}


Tree::~Tree(void)
{
}
void Tree::generate(uint8_t limit)
{
  uint8_t type = 0;

  uint8_t m_trunkHeight = getBetterRandInt(MIN_TRUNK, limit);

  bool smalltree = false;

  if (m_trunkHeight < BRANCHING_HEIGHT)
  {
    smalltree = true;
  }
  if (BetterRand() > 0.5) // 1/2 chance
  {
    type++;
    if (BetterRand() > 0.5) // 1/4
    {
      type++;
    }
  }
  uint8_t th = m_trunkHeight - 1; // Trunk Height
  uint8_t i;
  for (i = 0; i < th; i++)
  {
    if (smalltree)
    {
      Trunk* v = new Trunk(_x, _y + i, _z, _map, type);
      if (i >= MIN_TRUNK - 1)
      {
        m_Branch[n_branches] = v;
        n_branches++;
      }
      else
      {
        delete v;
      }
    }
    else
    {
      Trunk* v = new Trunk(_x, _y + i, _z, _map, type);
      if (i > BRANCHING_HEIGHT - 1)
      {
        generateBranches(v);
        m_Branch[n_branches] = v;
        n_branches++;
      }
      else
      {
        delete v;
      }
    }
  }
  Trunk* v = new Trunk(_x, _y + i, _z, _map, type);
  m_Branch[n_branches] = v;
  n_branches++;
  generateBranches(v);
  generateCanopy();
}
void Tree::generateBranches(Trunk* wrap)
{
  uint8_t blocktype;
  uint8_t meta;

  int32_t x = wrap->_x;
  uint8_t y = wrap->_y;
  int32_t z = wrap->_z;

  uint32_t schanse = BRANCHING_CHANCE;

  if (BetterRand() > 1.0 - (1.0 / BRANCHING_CHANCE))
  {
    float r = BetterRand();
    if (r < 0.2)
    {
      x--;
    }
    else if (r < 0.4)
    {
      x++;
    }
    else if (r < 0.6)
    {
      z++;
    }
    else if (r < 0.8)
    {
      z--;
    }
    if (r > 0.5)
    {
      y++;
    }

    Trunk* v = new Trunk(x, y, z, _map);
    m_Branch[n_branches] = v;
    n_branches++;
    generateBranches(v);
  }
}

void Tree::generateCanopy()
{
  uint8_t block, meta;
  uint8_t canopySize;

  uint8_t canopy_type = 0;

  int32_t posx, posy, posz;
  int32_t t_posx, t_posy, t_posz;

  if (BetterRand() > 0.5) // 1/2
  {
    canopy_type++;
    if (BetterRand() > 0.5) // 1/4
    {
      canopy_type++;
    }
  }
  canopySize = 3;
  //canopySize = (BetterRand()*(MAX_CANOPY - MIN_CANOPY)) + MIN_CANOPY;

  for (uint8_t i = 0; i < n_branches; i++)
  {

    posx = m_Branch[i]->_x;
    posy = m_Branch[i]->_y;
    posz = m_Branch[i]->_z;
    delete m_Branch[i];

    for (int8_t xi = (-canopySize); xi <= canopySize; xi++)
    {
      for (int8_t yi = (-canopySize); yi <= canopySize; yi++)
      {
        for (int8_t zi = (-canopySize); zi <= canopySize; zi++)
        {
          if (abs(xi) + abs(yi) + abs(zi) <= canopySize)
          {
            t_posx = posx + xi;
            t_posy = posy + yi;
            t_posz = posz + zi;

            if (Mineserver::get()->map(_map)->getBlock(t_posx, t_posy, t_posz, &block, &meta, true) && block == BLOCK_AIR)
            {
              Canopy u(t_posx, t_posy, t_posz, _map, canopy_type);
            }
          }
        }
      }
    }
  }
}
