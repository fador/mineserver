/*
   Copyright (c) 2012, The Mineserver Project
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


#include "redstoneSimulation.h"
#include "logger.h"
#include "constants.h"
#include "mineserver.h"
#include "map.h"
#include "protocol.h"

// Main loop
bool RedstoneSimulation::update()
{
  if (!enabled)
  {
    return true;
  }

  uint32_t listSize = simList.size();

  std::vector<uint32_t> toRemove;

  for (uint32_t simIt = 0; simIt < listSize; simIt++)
  {
    uint8_t curBlock = simList[simIt].id;
    uint8_t curPower = simList[simIt].power;
    uint8_t newPower = curPower;
    vec pos = simList[simIt].pos;
    //ToDo: handle removed

    for (int i = 0; i <= 5; i++)
    {
      if(simList[simIt].direction == i) continue;
      vec local(pos);
      uint8_t disableDir = -1;
      switch (i)
      {
      case 0:
        local += vec(0, -1, 0);
        disableDir = 5;
        break;
      case 1:
        local += vec(1, 0, 0);
        disableDir = 2;
        break;
      case 2:
        local += vec(-1, 0, 0);
        disableDir = 1;
        break;
      case 3:
        local += vec(0, 0, 1);
        disableDir = 4;
        break;
      case 4:
        local += vec(0, 0, -1);
        disableDir = 3;
        break;
      case 5:
        local += vec(0,1,0);
        disableDir = 0;
        break;
      default:
        break;
      }

      uint8_t block, meta;
      ServerInstance->map(map)->getBlock(local, &block, &meta);
      //Skip air blocks
      if(block != BLOCK_AIR)
      {
        if(curBlock == BLOCK_REDSTONE_WIRE)
        {
          if(block == BLOCK_REDSTONE_TORCH_ON)
          {
            newPower = 15;
          }
          if(block == BLOCK_REDSTONE_WIRE)
          {
            if(meta-1 > newPower)
            {
              newPower = meta-1;
            }
          }
        }

        if(block == BLOCK_REDSTONE_WIRE)
        {
          if(meta < curPower-1)
          {
            simList.push_back(RedstoneSim(BLOCK_REDSTONE_WIRE,local,curPower-1, disableDir));
            listSize++;
            ServerInstance->map(map)->setBlock(local, block, curPower-1);
            ServerInstance->map(map)->sendBlockChange(local, block, curPower-1);
          }
        }
        else if(curBlock == BLOCK_REDSTONE_WIRE || curBlock == BLOCK_REDSTONE_TORCH_ON)
        {
          simList.push_back(RedstoneSim(block,local,curPower-1, disableDir));
          listSize++;
        }

      }

      //We got power from neighbouring blocks
      if(newPower > curPower && curBlock == BLOCK_REDSTONE_WIRE)
      {
          simList.push_back(RedstoneSim(curBlock,pos,newPower, i));
          listSize++;
          ServerInstance->map(map)->setBlock(pos, curBlock, newPower);
          ServerInstance->map(map)->sendBlockChange(pos, curBlock, newPower);
      }
      
    }
    //Add queue to be removed
    toRemove.push_back(simIt);

  }

  //Remove from the simulation
  listSize = toRemove.size();
  for (int32_t simIt = listSize-1; simIt >= 0; simIt--)
  {
    simList.erase(simList.begin()+toRemove[simIt]);
  }


  return true;
}


// Add world simulation
bool RedstoneSimulation::addSimulation(vec pos)
{  
  if (!enabled)
  {
    return false;
  }

  uint8_t block;
  uint8_t meta;
  ServerInstance->map(map)->getBlock(pos, &block, &meta);

  //Removed torch/wire/etc
  if(block == BLOCK_AIR)
  {

  }

  uint8_t power = 0;


  switch(block)
  {
    case BLOCK_REDSTONE_TORCH_ON:
      power = 16;
      break;
    default:
      break;
  }
  RedstoneSim simulationBlock(block, pos, power);

  // Dont add duplicates
  for (std::vector<RedstoneSim>::iterator simIt = simList.begin(); simIt != simList.end(); simIt++)
  {
    vec itpos = simIt->pos;
    if (itpos.x() == pos.x() && itpos.y() == pos.y() && itpos.z() == pos.z())
    {
      return false;
    }
  }

  simList.push_back(simulationBlock);


  return true;
}

RedstoneSimulation::Power RedstoneSimulation::getPower(int32_t x, int16_t y, int32_t z)
{
	uint8_t block;
	uint8_t meta = 0;
	bool weak = false;

	// Check block type
	if (!ServerInstance->map(map)->getBlock(x + 1, y, z, &block, &meta))
	{
		return POWER_NONE;
	}
	if (!isBlockSolid(block)) {
		return POWER_NONE;
	}

	//
	// Check for switches or wires on sides
	//

	// North
	if (!ServerInstance->map(map)->getBlock(x + 1, y, z, &block, &meta))
	{
		return POWER_NONE;
	}

	if ((block == BLOCK_STONE_BUTTON || block == BLOCK_LEVER) && (meta & 0x4) == 0x4 && (meta & 0x8) == 0x8)
	{
		return POWER_NORMAL;
	}
	else if (block == BLOCK_REDSTONE_WIRE && meta != 0)
	{
		// TODO: Check if wire ends here
		weak = true;
	}

	// South
	if (!ServerInstance->map(map)->getBlock(x - 1, y, z, &block, &meta))
	{
		return POWER_NONE;
	}

	if ((block == BLOCK_STONE_BUTTON || block == BLOCK_LEVER) && (meta & 0x3) == 0x3 && (meta & 0x8) == 0x8)
	{
		return POWER_NORMAL;
	}
	else if (block == BLOCK_REDSTONE_WIRE && meta != 0)
	{
		weak = true;
	}

	// East
	if (!ServerInstance->map(map)->getBlock(x, y, z + 1, &block, &meta))
	{
		return POWER_NONE;
	}

	if ((block == BLOCK_STONE_BUTTON || block == BLOCK_LEVER) && (meta & 0x1) == 0x1 && (meta & 0x8) == 0x8)
	{
		return POWER_NORMAL;
	}
	else if (block == BLOCK_REDSTONE_WIRE && meta != 0)
	{
		weak = true;
	}

	// West
	if (!ServerInstance->map(map)->getBlock(x, y, z - 1, &block, &meta))
	{
		return POWER_NONE;
	}

	if ((block == BLOCK_STONE_BUTTON || block == BLOCK_LEVER) && (meta & 0x2) == 0x2 && (meta & 0x8) == 0x8)
	{
		return POWER_NORMAL;
	}
	else if (block == BLOCK_REDSTONE_WIRE && meta != 0)
	{
		weak = true;
	}

	// Switch or pressure plate on top of the block
	if (!ServerInstance->map(map)->getBlock(x, y + 1, z, &block, &meta))
	{
		return POWER_NONE;
	}

	if (block == BLOCK_WOODEN_PRESSURE_PLATE || block == BLOCK_STONE_PRESSURE_PLATE && (meta & 0x1) == 0x1)
	{
		return POWER_NORMAL;
	}
	else if (block == BLOCK_LEVER && (meta & 0x8) == 0x8)
	{
		return POWER_NORMAL;
	}

	// Wire on top of the block
	else if (block == BLOCK_REDSTONE_WIRE && meta != 0)
	{
		weak = true;
	}

	// Torch beneath the block
	if (!ServerInstance->map(map)->getBlock(x, y - 1, z, &block, &meta))
	{
		return POWER_NONE;
	}
	if (block == BLOCK_REDSTONE_TORCH_ON)
	{
		return POWER_NORMAL;
	}

	if (weak) {
		return POWER_WEAK;
	}

	// No power
	return POWER_NONE;
}

bool RedstoneSimulation::isBlockSolid(const uint8_t block) {
	switch (block)
	{
	case BLOCK_AIR:
	case BLOCK_STONE_BUTTON:
	case BLOCK_WATER:
	case BLOCK_LAVA:
	case BLOCK_STATIONARY_WATER:
	case BLOCK_STATIONARY_LAVA:
	case BLOCK_FIRE:
	case BLOCK_LEVER:
	case BLOCK_PORTAL:
	case BLOCK_REDSTONE_WIRE:
	case BLOCK_REDSTONE_TORCH_OFF:
	case BLOCK_REDSTONE_TORCH_ON:
	case BLOCK_SAPLING:
	case BLOCK_SIGN_POST:
	case BLOCK_GRASS:
	case BLOCK_YELLOW_FLOWER:
	case BLOCK_RED_ROSE:
	case BLOCK_RED_MUSHROOM:
	case BLOCK_BROWN_MUSHROOM:
	case BLOCK_CROPS:
	case BLOCK_TORCH:
	case BLOCK_WOODEN_DOOR:
	case BLOCK_FENCE:
	case BLOCK_WALL_SIGN:
	case BLOCK_DOUBLE_STEP:
	case BLOCK_IRON_DOOR:
	case BLOCK_LADDER:
	case BLOCK_SNOW:
	case BLOCK_STEP:
	case BLOCK_WOODEN_STAIRS:
	case BLOCK_COBBLESTONE_STAIRS:
    return false;
    break;
  default:
    break;
  }

  return true;
}