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

#include "mineserver.h"
#include "map.h"
#include "plugin.h"
#include "logger.h"
#include "protocol.h"

#include "falling.h"

bool BlockFalling::affectedBlock(int block) const
{
  if (block == BLOCK_SAND || block == BLOCK_SLOW_SAND || block == BLOCK_GRAVEL)
    return true;
  return false;
}

std::string printfify(const char *fmt, ...)
{
  if(fmt)
  {
    va_list args;
    char buf[4096];
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return buf;
  }
  else
    return fmt;
}

void BlockFalling::onNeighbourBroken(User* user, int16_t, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  this->onNeighbourMove(user, 0, x, y, z, direction, map);
}

bool BlockFalling::onPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  uint8_t oldblock;
  uint8_t oldmeta;

  if (!ServerInstance->map(map)->getBlock(x, y, z, &oldblock, &oldmeta))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

  /* Check block below allows blocks placed on top */
  if (!this->isBlockStackable(oldblock))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

  /* move the x,y,z coords dependent upon placement direction */
  if (!this->translateDirection(&x, &y, &z, map, direction))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

  if (this->isUserOnBlock(x, y, z, map))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

  if (!this->isBlockEmpty(x, y, z, map))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

  ServerInstance->map(map)->setBlock(x, y, z, (char)newblock, 0);
  ServerInstance->map(map)->sendBlockChange(x, y, z, (char)newblock, 0);

  applyPhysics(user, x, y, z, map);
  return false;
}

void BlockFalling::onNeighbourMove(User* user, int16_t, int32_t x, int8_t y, int32_t z, int8_t direction, int map)
{
  uint8_t block;
  uint8_t meta;

  if(!ServerInstance->map(map)->getBlock(x, y, z, &block, &meta))
  	return;
  if (affectedBlock(block))
  {
    applyPhysics(user, x, y, z, map);
  }
}

void BlockFalling::applyPhysics(User* user, int32_t x, int8_t y, int32_t z, int map)
{
  uint8_t fallblock, block, neighbour, testbl;
  uint8_t fallmeta, meta, neighbourmeta, testmet;

  if (!ServerInstance->map(map)->getBlock(x, y, z, &fallblock, &fallmeta))
  {
    return;
  }

  // Destroy original block
  ServerInstance->map(map)->sendBlockChange(x, y, z, BLOCK_AIR, 0);
  ServerInstance->map(map)->setBlock(x, y, z, BLOCK_AIR, 0);
  
  while (ServerInstance->map(map)->getBlock(x, y - 1, z, &block, &meta))
  {
    unsigned int falldist = 0;
    switch (block)
    {
    case BLOCK_AIR:
    case BLOCK_WATER:
    case BLOCK_STATIONARY_WATER:
    case BLOCK_LAVA:
    case BLOCK_STATIONARY_LAVA:
      break;
    default:
      if(falldist > 0) {
        const int chunk_x = blockToChunk(x);
        const int chunk_z = blockToChunk(z);
	    
        const ChunkMap::const_iterator it = ServerInstance->map(map)->chunks.find(Coords(blockToChunk(x), blockToChunk(z)));

        if (it == ServerInstance->map(map)->chunks.end())
          return;

        Packet pkt = Protocol::entityRelativeMove(ENTITY_SAND, 0, 1, 0);

        it->second->sendPacket(pkt);
      }
      // stop falling
      ServerInstance->map(map)->setBlock(x, y, z, fallblock, fallmeta);
      ServerInstance->map(map)->sendBlockChange(x, y, z, fallblock, fallmeta);
      return;
      break;
    }

    y--;
    falldist++;
    if(falldist >= 4)
    {
      const int chunk_x = blockToChunk(x);
      const int chunk_z = blockToChunk(z);
	    
      const ChunkMap::const_iterator it = ServerInstance->map(map)->chunks.find(Coords(blockToChunk(x), blockToChunk(z)));

      if (it == ServerInstance->map(map)->chunks.end())
        return;

      Packet pkt = Protocol::entityRelativeMove(ENTITY_SAND, 0, 1, 0);

      it->second->sendPacket(pkt);
    }
    
    //is commented out
    this->notifyNeighbours(x, y + 2, z, map, "onNeighbourMove", user, block, BLOCK_BOTTOM);
    //temporary
    if(!ServerInstance->map(map)->getBlock(x,y+2,z,&neighbour, &neighbourmeta))
    	return;
    if(!ServerInstance->map(map)->getBlock(x,y+1,z,&testbl, &testmet))
    	return;
    
    // Justasic: wtf is this? someone should correct it..
    if(testbl == BLOCK_SAND)
      LOG(INFO, "Gravity", "Sand block fell!");

    if(testbl == BLOCK_GRAVEL)
      LOG(INFO, "Gravity", "Gravel block fell!");
    	
    if(neighbour == BLOCK_SAND)
      LOG(INFO, "Gravity", "Neighboring sand block! Making it fall!");

    if(neighbour == BLOCK_GRAVEL)
      LOG(INFO, "Gravity", "Neighboring gravel block! Making it fall!");
    
    for(unsigned int i = 0; i < ServerInstance->plugin()->getBlockCB().size(); i++)
    {
    	if(ServerInstance->plugin()->getBlockCB()[i]->affectedBlock(neighbour))
	{
	  LOG(INFO, "Gravity", printfify("Falling block %s (x=%i, y=%i, z=%i)", GetBlockName(static_cast<Block>(neighbour)).c_str(), x, y, z));

	  uint8_t neighbor, neighbormeta;
	  while(ServerInstance->map(map)->getBlock(++x,y,z, &neighbor, &neighbormeta) && affectedBlock(neighbor))
	  {
	    switch(block)
	    {
	      case BLOCK_SAND:
		LOG(INFO, "Gravity", printfify("Block x=%i, y=%i, z=%i is sand!", x, y, z));
		break;
	      case BLOCK_SLOW_SAND:
		LOG(INFO, "Gravity", printfify("Block x=%i, y=%i, z=%i is slow sand!", x, y, z));
		break;
	      case BLOCK_GRAVEL:
		LOG(INFO, "Gravity", printfify("Block x=%i, y=%i, z=%i is gravel!", x, y, z));
		break;
	      default:
		LOG(INFO, "Gravity", printfify("Block x=%i, y=%i, z=%i is invalid?", x, y, z));
	    }
	    ServerInstance->plugin()->getBlockCB()[i]->onNeighbourMove(user, 0, x, y, z, 1, map);
	  }

	  while(ServerInstance->map(map)->getBlock(x,y,++z, &neighbor, &neighbormeta) && affectedBlock(neighbor))
	  {
	    switch(block)
	    {
	      case BLOCK_SAND:
		LOG(INFO, "Gravity", printfify("Block x=%i, y=%i, z=%i is sand!", x, y, z));
		break;
	      case BLOCK_SLOW_SAND:
		LOG(INFO, "Gravity", printfify("Block x=%i, y=%i, z=%i is slow sand!", x, y, z));
		break;
	      case BLOCK_GRAVEL:
		LOG(INFO, "Gravity", printfify("Block x=%i, y=%i, z=%i is gravel!", x, y, z));
		break;
	      default:
		LOG(INFO, "Gravity", printfify("Block x=%i, y=%i, z=%i is invalid?", x, y, z));
	    }
	    ServerInstance->plugin()->getBlockCB()[i]->onNeighbourMove(user, 0, x, y, z, 1, map);
	  }
	  ServerInstance->plugin()->getBlockCB()[i]->onNeighbourMove(user, 0, x, y+2, z, 1, map);
    	}
    }
  }
}

