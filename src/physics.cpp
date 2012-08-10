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
#include "protocol.h"
#include "vec.h"

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

enum
{
  FLAT_NS = 0,
  FLAT_EW,
  ASCEND_W,
  ASCEND_E,
  ASCEND_S,
  ASCEND_N,
  CORNER_SE,
  CORNER_SW,
  CORNER_NW,
  CORNER_NE
};

//Minecart physics loop
bool Physics::updateMinecart()
{
  std::vector<MinecartData> &minecarts = ServerInstance->map(map)->minecarts;
  uint32_t listSize = minecarts.size();

  for (int32_t simIt = 0; simIt < listSize; simIt++)
  {
    //Check if the cart is suppose to move
    if(minecarts[simIt].speed.x() != 0 ||
       minecarts[simIt].speed.y() != 0 ||
       minecarts[simIt].speed.z() != 0)
    {      
      uint64_t timeNow = microTime();
      double timeDiff = (timeNow-minecarts[simIt].timestamp*1.0)/1000000.0; //s
      minecarts[simIt].timestamp = timeNow;

      vec diff = vec(int8_t(float(minecarts[simIt].speed.x())*(timeDiff)),
                     int8_t(float(minecarts[simIt].speed.y())*(timeDiff)),
                     int8_t(float(minecarts[simIt].speed.z())*(timeDiff)) );
      minecarts[simIt].pos = vec(minecarts[simIt].pos.x()+diff.x(),
                                 minecarts[simIt].pos.y()+diff.y(),
                                 minecarts[simIt].pos.z()+diff.z());



      vec blockPos = vec((int)(minecarts[simIt].pos.x()/32),
                          (int)(minecarts[simIt].pos.y()/32),
                          (int)(minecarts[simIt].pos.z()/32)); 
      uint8_t block, meta;
      ServerInstance->map(map)->getBlock(blockPos.x(),
                                         blockPos.y(),
                                         blockPos.z(),
                                         &block, &meta);
      bool changed = false;
      if(block == BLOCK_AIR)
      {
        minecarts[simIt].pos.y() -= 32;
        changed = true;
      }
      else if((minecarts[simIt].lastBlock.x() != blockPos.x() ||
               minecarts[simIt].lastBlock.y() != blockPos.y() ||
               minecarts[simIt].lastBlock.z() != blockPos.z()) &&
              block == BLOCK_MINECART_TRACKS)
      {
        minecarts[simIt].lastBlock = blockPos;
        if((meta == FLAT_NS && minecarts[simIt].speed.x() != 0)
           || (meta == FLAT_EW && minecarts[simIt].speed.z() != 0))
        {

        }
        else
        {
          //z = north
          //-x = east

          //Going west
          if(minecarts[simIt].speed.x() > 0 && meta == CORNER_NW)
          {
            minecarts[simIt].speed.z() = -minecarts[simIt].speed.x();
            minecarts[simIt].speed.x() = 0;
            changed = true;
          }
          //Going west
          else if(minecarts[simIt].speed.x() > 0 && meta == CORNER_SW)
          {
            minecarts[simIt].speed.z() = minecarts[simIt].speed.x();
            minecarts[simIt].speed.x() = 0;
            changed = true;
          }
          //Going east
          else if(minecarts[simIt].speed.x() < 0 && meta == CORNER_NE)
          {
            minecarts[simIt].speed.z() = minecarts[simIt].speed.x();
            minecarts[simIt].speed.x() = 0;
            changed = true;
          }
          //Going east
          else if(minecarts[simIt].speed.x() < 0 && meta == CORNER_SE)
          {
            minecarts[simIt].speed.z() = -minecarts[simIt].speed.x();
            minecarts[simIt].speed.x() = 0;
            changed = true;
          }
          //Going north
          else if(minecarts[simIt].speed.z() > 0 && meta == CORNER_NW)
          {
            minecarts[simIt].speed.x() = -minecarts[simIt].speed.z();
            minecarts[simIt].speed.z() = 0;
            changed = true;
          }
          //Going north
          else if(minecarts[simIt].speed.z() > 0 && meta == CORNER_NE)
          {
            minecarts[simIt].speed.x() = minecarts[simIt].speed.z();
            minecarts[simIt].speed.z() = 0;
            changed = true;            
          }
          //Going south
          else if(minecarts[simIt].speed.z() < 0 && meta == CORNER_SE)
          {
            minecarts[simIt].speed.x() = -minecarts[simIt].speed.z();
            minecarts[simIt].speed.z() = 0;
            changed = true;            
          }
          //Going south
          else if(minecarts[simIt].speed.z() < 0 && meta == CORNER_SW)
          {
            minecarts[simIt].speed.x() = minecarts[simIt].speed.z();
            minecarts[simIt].speed.z() = 0;
            changed = true;            
          }
        }
      }
      else
      {
        //minecarts[simIt].pos.y() += 32;
        //changed = true;
      }

      //Signal clients about the new pos
      Packet pkt;
      if(changed)
      {
        minecarts[simIt].pos.x() = blockPos.x()*32;
        minecarts[simIt].pos.y() = blockPos.y()*32;
        minecarts[simIt].pos.z() = blockPos.z()*32;
        pkt = Protocol::entityTeleport(minecarts[simIt].EID,
                                       (minecarts[simIt].pos.x()+16.0)/32.0,
                                       (minecarts[simIt].pos.y()+16.0)/32.0,
                                       (minecarts[simIt].pos.z()+16.0)/32.0, 0, 0);
      }
      else
      {
        pkt = Protocol::entityRelativeMove(minecarts[simIt].EID,(int8_t)diff.x(),(int8_t)diff.y(),(int8_t)diff.z());

      }
      User::sendAll(pkt);
    }
  }
  return true;
}

//Falling physics loop
bool Physics::updateFall()
{

  uint32_t listSize = fallSimList.size();

  for (int32_t simIt = listSize-1; simIt >= 0; simIt--)
  {
    Falling& f = fallSimList[simIt];

    double timeInSec = (microTime()-f.startTime)/1000000.0;
    f.ticks++;

    const double gravity = 9.81;
    double offset = 0.5*gravity*timeInSec*timeInSec;
    int blockOffset = f.pos.y() - f.lastY;
    if(blockOffset != (int)offset) ///  not necessary, doesn't optimize much
    {
      int yStart = f.pos.y();
      int x = f.pos.x(); int z = f.pos.z();

      int ypos = f.lastY;

      f.lastY = yStart-(int)offset;
      for(; ypos >= f.lastY;ypos--)
      {
        uint8_t block, meta;
        ServerInstance->map(map)->getBlock(x,ypos,z, &block, &meta);
        switch (block)
        {
        case BLOCK_AIR:
        case BLOCK_WATER:
        case BLOCK_STATIONARY_WATER:
        case BLOCK_LAVA:
        case BLOCK_STATIONARY_LAVA:
        case BLOCK_SNOW:
          break;
          //If we hit ground
        default:
          {
            ServerInstance->map(map)->setBlock(x, ++ypos, z, f.block, 0);
            ServerInstance->map(map)->sendBlockChange(x, ypos, z, f.block, 0);
            
            //Despawn entity
            Packet pkt = Protocol::destroyEntity(f.EID);
            const int chunk_x = blockToChunk(x);
            const int chunk_z = blockToChunk(z);
            const ChunkMap::const_iterator it = ServerInstance->map(map)->chunks.find(Coords(chunk_x, chunk_z));
            if (it != ServerInstance->map(map)->chunks.end())
            {               
              it->second->sendPacket(pkt);
            }
            //Erase from the simulation list
            fallSimList.erase(fallSimList.begin()+simIt);
            goto breakout;
          }
        }
      }
    }
breakout:
    continue;
  }
  return true;
}

// Physics loop
bool Physics::update()
{
  updateFall();
  updateMinecart();
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

  //clock_t endtime = clock() - starttime;
  //  LOG(INFO, "Physics", "Exit simulation, took " + dtos(endtime * 1000 / CLOCKS_PER_SEC) + " ms, " + dtos(simList.size()) + " items left");
  return true;
}

bool Physics::addFallSimulation(uint8_t block, vec pos, uint32_t EID)
{  
  fallSimList.push_back(Falling(block,pos,EID));
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
