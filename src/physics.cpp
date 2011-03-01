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
#ifdef WIN32
#include <stdlib.h>
#include <conio.h>
#include <winsock2.h>
typedef  int socklen_t;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>

#include "logger.h"
#include "constants.h"
#include "map.h"
#include "vec.h"
#include "mineserver.h"
#include "tools.h"

#include "physics.h"

namespace
{

bool isWaterBlock(int id)
{
  return ((id == BLOCK_WATER) || (id == BLOCK_STATIONARY_WATER));
}

bool isLavaBlock(int id)
{
  return ((id == BLOCK_LAVA) || (id == BLOCK_STATIONARY_LAVA));
}

bool isLiquidBlock(int id)
{
  return ((id == BLOCK_LAVA) || (id == BLOCK_STATIONARY_LAVA) || (id == BLOCK_WATER) || (id == BLOCK_STATIONARY_WATER));
}

bool mayFallThrough(int id)
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

  clock_t starttime = clock();

  std::vector<int32_t> toRemove;
  std::vector<vec> toAdd;

  LOG(INFO, "Physics", "Simulating " + dtos(simList.size()) + " items!");

  uint32_t listSize = simList.size();
  // Iterate each simulation
  for (uint32_t simIt = 0; simIt < listSize; simIt++)
  {
    vec pos = simList[simIt].blocks[0].pos;
    // Blocks
    uint8_t block, meta;
    Mineserver::get()->map(map)->getBlock(pos, &block, &meta);

    simList[simIt].blocks[0].id   = block;
    simList[simIt].blocks[0].meta = meta;

    // Water simulation
    if (simList[simIt].type == TYPE_WATER)
    {
      if (isWaterBlock(block))
      {
        int32_t it = 0;

        //for(int32_t it=simList[simIt].blocks.size()-1; it>=0; it--)
        {
          bool havesource = false;

          // Search for water source if this is not the source
          if (simList[simIt].blocks[it].id != BLOCK_STATIONARY_WATER)
          {
            for (int i = 0; i < 6; i++)
            {
              vec local(pos);

              switch (i)
              {
              case 0:
                local += vec(0,  1,  0);
                break; //y++
              case 1:
                local += vec(1,  0,  0);
                break; //x++
              case 2:
                local += vec(-1,  0,  0);
                break; //x--
              case 3:
                local += vec(0,  0,  1);
                break; //z++
              case 4:
                local += vec(0,  0, -1);
                break; //z--
              case 5:
                local += vec(0, -1,  0);
                break; //y--
              }

              // Search neighboring water blocks for source current
              if (Mineserver::get()->map(map)->getBlock(local, &block, &meta) && isWaterBlock(block))
              {
                // is this the source block
                if (i != 5 && (block == BLOCK_STATIONARY_WATER || (meta & 0x07) < (simList[simIt].blocks[it].meta & 0x07) || i == 0))
                {
                  havesource = true;
                }
                //Else we have to search for source to this block also
                else if (i == 5 || (meta & 0x07) > (simList[simIt].blocks[it].meta & 0x07))
                {
                  toAdd.push_back(local);
                }
              }
            }
          }
          //Stationary water block is the source
          else
          {
            havesource = true;
          }

          //If no source, dry block away
          if (!havesource)
          {
            //This block will change so add surrounding blocks to simulation
            for (uint32_t i = 0; i < toAdd.size(); i++)
            {
              addSimulation(toAdd[i]);
            }
            //If not dried out yet
            if (!(simList[simIt].blocks[it].meta & 0x8) && (simList[simIt].blocks[it].meta & 0x07) < M7)
            {
              // Set new water level
              block = BLOCK_WATER;
              meta  = simList[simIt].blocks[it].meta + 1;
              Mineserver::get()->map(map)->setBlock(pos, block, meta);
              Mineserver::get()->map(map)->sendBlockChange(pos, block, meta);

              toRemove.push_back(simIt);
              addSimulation(pos);
              // Update simulation meta information
              // simList[simIt].blocks[it].meta = meta;
            }
            //Else this block has dried out
            else
            {
              //Clear and remove simulation
              Mineserver::get()->map(map)->setBlock(pos, BLOCK_AIR, 0);
              Mineserver::get()->map(map)->sendBlockChange(pos, BLOCK_AIR, 0);
              toRemove.push_back(simIt);

              //If below this block has another waterblock, simulate it also
              if (Mineserver::get()->map(map)->getBlock(pos - vec(0, 1, 0), &block, &meta) &&
                  isWaterBlock(block))
              {
                addSimulation(pos - vec(0, 1, 0));
              }

            }
          }
          //Have source!
          else
          {
            toAdd.clear();
            vec belowPos(pos - vec(0, 1, 0));
            // If below is free to fall
            if (Mineserver::get()->map(map)->getBlock(belowPos, &block, &meta) &&
                mayFallThrough(block))
            {
              // Set new fallblock there
              block = BLOCK_WATER;
              meta  = M_FALLING;
              Mineserver::get()->map(map)->setBlock(belowPos, block, meta);
              Mineserver::get()->map(map)->sendBlockChange(belowPos, block, meta);
              // Change simulation-block to current block
              toRemove.push_back(simIt);
              addSimulation(belowPos);
            }
            //Else if spreading to sides
            //If water level is at minimum, dont simulate anymore
            else if ((simList[simIt].blocks[it].meta & M7) != M7)
            {
              for (int i = 0; i < 4; i++)
              {
                vec local(pos);
                switch (i)
                {
                case 0:
                  local += vec(1,  0,  0);
                  break;

                case 1:
                  local += vec(-1,  0,  0);
                  break;

                case 2:
                  local += vec(0,  0,  1);
                  break;

                case 3:
                  local += vec(0,  0, -1);
                  break;
                }

                if (Mineserver::get()->map(map)->getBlock(local, &block, &meta) &&
                    mayFallThrough(block))
                {
                  //Decrease water level each turn
                  if (!isWaterBlock(block) || meta > (simList[simIt].blocks[it].meta & 0x07) + 1)
                  {
                    meta = (simList[simIt].blocks[it].meta & 0x07) + 1;
                    Mineserver::get()->map(map)->setBlock(local, BLOCK_WATER, meta);
                    Mineserver::get()->map(map)->sendBlockChange(local, BLOCK_WATER, meta);
                    addSimulation(local);
                  }
                }
              } // End for i=0:3

              //Remove this block from simulation
              toRemove.push_back(simIt);
            }
            //Water level at minimum
            else
            {
              //Remove this block from simulation
              toRemove.push_back(simIt);
            }
          }
        }
      }
      //Block has changes
      else
      {
        //Remove this block from simulation
        toRemove.push_back(simIt);
      }
    }
    else
    {
      //Remove this block from simulation
      toRemove.push_back(simIt);
    }
  }

  std::vector<int>::reverse_iterator rit;
  for (rit = toRemove.rbegin() ; rit < toRemove.rend(); ++rit)
  {
    simList.erase(simList.begin() + *rit);
  }

  clock_t endtime = clock() - starttime;
  LOG(INFO, "Physics", "Exit simulation, took " + dtos(endtime * 1000 / CLOCKS_PER_SEC) + " ms, " + dtos(simList.size()) + " items left");
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
  Mineserver::get()->map(map)->getBlock(pos, &block, &meta);
  SimBlock simulationBlock(block, pos, meta);

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
    if (Mineserver::get()->map(map)->getBlock(local, &block, &meta) &&
        isLiquidBlock(block))
    {
      addSimulation(local);
    }
  }

  return true;
}
