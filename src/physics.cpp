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


#include "physics.h"
#include "logger.h"
#include "constants.h"
#include "mineserver.h"
#include "map.h"

namespace
{

inline bool isWaterBlock(int id)
{
  return ((id == BLOCK_WATER) || (id == BLOCK_STATIONARY_WATER));
}

inline bool isLavaBlock(int id)
{
  return ((id == BLOCK_LAVA) || (id == BLOCK_STATIONARY_LAVA));
}

inline bool isLiquidBlock(int id)
{
  return ((id == BLOCK_LAVA) || (id == BLOCK_STATIONARY_LAVA) || (id == BLOCK_WATER) || (id == BLOCK_STATIONARY_WATER));
}

inline bool mayFallThrough(int id)
{
  return ((id == BLOCK_AIR) || (id == BLOCK_WATER) || (id == BLOCK_STATIONARY_WATER) || (id == BLOCK_SNOW));
}

}

// Physics loop
bool Physics::update()
{
  if (!enabled)
  {
    return true;
  }

  // Check if needs to be updated
  if (simList.empty())
  {
    return true;
  }

  std::vector<vec> toAdd;
  std::vector<vec> toRem;
  std::set<vec> changed;

  clock_t starttime = clock();

  LOG(INFO, "Physics", "Simulating " + dtos(simList.size()) + " items!");

  uint32_t listSize = simList.size();

  for (uint32_t simIt = 0; simIt < listSize; simIt++)
  {
    vec pos = simList[simIt].blocks[0].pos;
    // Blocks
    uint8_t block, meta;
    ServerInstance->map(map)->getBlock(pos, &block, &meta);
    simList[simIt].blocks[0].id   = block;
    simList[simIt].blocks[0].meta = meta;

    bool used = false;
    for (int i = 0; i < 5; i++)
    {
      vec local(pos);
      bool falling = false;
      switch (i)
      {
      case 0:
        local += vec(0, -1, 0); // First tries to go down
        falling = true;
        break;
      case 1:
        local += vec(1, 0, 0); // Might be bad to have the 4 cardinal dir'
        // so predictable
        break;
      case 2:
        local += vec(-1, 0, 0);
        break;
      case 3:
        local += vec(0, 0, 1);
        break;
      case 4:
        local += vec(0, 0, -1);
        break;
      case 5:
        //        local += vec(0,1,0); // Going UP
        break;
      }
      uint8_t newblock, newmeta;
      ServerInstance->map(map)->getBlock(pos, &block, &meta);
      ServerInstance->map(map)->getBlock(local, &newblock, &newmeta);
      if (!isLiquidBlock(block))
      {
        toRem.push_back(pos);
        break;
      }
      if ((isWaterBlock(newblock) && isWaterBlock(block)) || (isLavaBlock(newblock) && isLavaBlock(block)) || (isLiquidBlock(block) && mayFallThrough(newblock)))
      {
        if (falling && !isLiquidBlock(newblock))
        {
          ServerInstance->map(map)->setBlock(local, block, meta);
          changed.insert(local);
          ServerInstance->map(map)->setBlock(pos, BLOCK_AIR, 0);
          changed.insert(pos);
          toRem.push_back(pos);
          toAdd.push_back(local);
          used = true;
          continue;
        }
        if (falling && isLiquidBlock(newblock))
        {
          int top = 8 - meta;
          int bot = 8 - newmeta;
          int volume = top + bot;
          if (volume > 8)
          {
            top = volume - 8;
            bot = 8;
          }
          else
          {
            top = 0;
            bot = volume;
          }
          int a_meta = 8 - top;
          int a_newmeta = 8 - bot;
          toAdd.push_back(local);
          if (a_meta == meta && a_newmeta == newmeta)
          {
            toRem.push_back(pos);
            toRem.push_back(local);
            continue;
          }
          if ((isWaterBlock(block) && a_meta < 8) || (isLavaBlock(block) && a_meta < 4))
          {
            ServerInstance->map(map)->setBlock(pos, block, a_meta);

            changed.insert(pos);
          }
          else
          {
            ServerInstance->map(map)->setBlock(pos, BLOCK_AIR, 0);
            changed.insert(pos);
          }
          ServerInstance->map(map)->setBlock(local, block, a_newmeta);
          used = true;
          toAdd.push_back(local);
          toAdd.push_back(pos);
          changed.insert(pos);
          continue;
        }

        if (!isLiquidBlock(newblock))
        {
          if (!falling)
          {
            if ((isWaterBlock(block) && meta == 7) || (isLavaBlock(block) && meta >= 3))
            {
              toRem.push_back(pos);
              break;
            }
          }
          // We are spreading onto dry area.
          newmeta = 7;
          ServerInstance->map(map)->setBlock(local, block, newmeta);
          changed.insert(local);
          meta++;
          if (meta < 8)
          {
            ServerInstance->map(map)->setBlock(pos, block, meta);
            changed.insert(pos);
          }
          else
          {
            ServerInstance->map(map)->setBlock(pos, BLOCK_AIR, 0);
            changed.insert(pos);
            toRem.push_back(pos);
          }
          toAdd.push_back(local);
          used = true;
          continue;
        }
        if (meta < newmeta - 1 || (meta == newmeta && falling))
        {
          newmeta --;
          ServerInstance->map(map)->setBlock(local, block, newmeta);
          changed.insert(local);
          meta ++;
          if (meta < 8)
          {
            ServerInstance->map(map)->setBlock(pos, block, meta);
            changed.insert(pos);
          }
          else
          {
            ServerInstance->map(map)->setBlock(pos, BLOCK_AIR, 0);
            changed.insert(pos);
            toRem.push_back(pos);
          }
          toAdd.push_back(local);
          used = true;
          continue;
        }
      }
    }
    if (!used)
    {
      toRem.push_back(pos);
    }
  }
  for (int i = int(toRem.size()) - 1; i >= 0; i--)
  {
    removeSimulation(toRem[i]);
  }
  for (size_t i = 0; i < toAdd.size(); i++)
  {
    addSimulation(toAdd[i]);
  }
  ServerInstance->map(map)->sendMultiBlocks(changed);

  clock_t endtime = clock() - starttime;
  //  LOG(INFO, "Physics", "Exit simulation, took " + dtos(endtime * 1000 / CLOCKS_PER_SEC) + " ms, " + dtos(simList.size()) + " items left");
  return true;
}

// Add world simulation
bool Physics::addSimulation(vec pos)
{
  if (!enabled)
  {
    return true;
  }

  uint8_t block;
  uint8_t meta;
  ServerInstance->map(map)->getBlock(pos, &block, &meta);
  SimBlock simulationBlock(block, pos, meta);

  // Dont add duplicates
  for (std::vector<Sim>::iterator simIt = simList.begin(); simIt != simList.end(); simIt++)
  {
    vec itpos = simIt->blocks[0].pos;
    if (itpos.x() == pos.x() && itpos.y() == pos.y() && itpos.z() == pos.z())
    {
      return true;
    }
  }


  // Simulating water
  if (isWaterBlock(block))
  {
    simList.push_back(Sim(TYPE_WATER, simulationBlock));
    return true;
  }
  // Simulating lava
  else if (isLavaBlock(block))
  {
    simList.push_back(Sim(TYPE_LAVA, simulationBlock));
    return true;
  }

  return false;
}

bool Physics::removeSimulation(vec pos)
{
  if (!enabled)
  {
    return true;
  }

  // Iterate each simulation
  for (std::vector<Sim>::iterator simIt = simList.begin(); simIt != simList.end(); simIt++)
  {
    vec itpos = simIt->blocks[0].pos;
    if (itpos.x() == pos.x() && itpos.y() == pos.y() && itpos.z() == pos.z())
    {
      simList.erase(simIt);
      return true;
    }
  }
  return false;
}


bool Physics::checkSurrounding(vec pos)
{
  if (!enabled)
  {
    return true;
  }

  uint8_t block;
  uint8_t meta;

  for (int i = 0; i < 6; i++)
  {
    vec local(pos);
    switch (i)
    {
    case 0:
      local += vec(0,  1,  0);
      break;

    case 1:
      local += vec(1,  0,  0);
      break;

    case 2:
      local += vec(-1,  0,  0);
      break;

    case 3:
      local += vec(0,  0,  1);
      break;

    case 4:
      local += vec(0,  0, -1);
      break;

    case 5:
      local += vec(0, -1,  0);
      break;
    }

    //Add liquid blocks to simulation if they are affected by breaking a block
    if (ServerInstance->map(map)->getBlock(local, &block, &meta) &&
        isLiquidBlock(block))
    {
      addSimulation(local);
    }
  }

  return true;
}
