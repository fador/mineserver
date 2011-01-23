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

#include <iostream>

#include "../mineserver.h"
#include "../map.h"
#include "../logger.h"

#include "tracks.h"

bool BlockTracks::affectedBlock(int block)
{
  switch(block)
  {
  case BLOCK_MINECART_TRACKS:
    return true;
  }
  return false;
}


enum {
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

void BlockTracks::onStartedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{

}

void BlockTracks::onDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{

}

void BlockTracks::onStoppedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{

}

bool BlockTracks::onBroken(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  uint8_t block,meta;
  Mineserver::get()->map(map)->getBlock(x, y, z, &block, &meta);

  Mineserver::get()->map(map)->setBlock(x, y, z, BLOCK_AIR, 0);
  Mineserver::get()->map(map)->sendBlockChange(x, y, z, BLOCK_AIR, 0);

  this->spawnBlockItem(x, y, z,map, block, 0);
  return false;
}

void BlockTracks::onNeighbourBroken(User* user, int16_t oldblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  uint8_t block;
  uint8_t meta;
  
  if (!Mineserver::get()->map(map)->getBlock(x, y, z, &block, &meta))
    return;

  if (this->isBlockEmpty(x, y-1, z,map))
  {
    // Break torch and spawn torch item
    Mineserver::get()->map(map)->sendBlockChange(x, y, z, BLOCK_AIR, 0);
    Mineserver::get()->map(map)->setBlock(x, y, z, BLOCK_AIR, 0);
    this->spawnBlockItem(x, y, z, map,block);
  }
}

bool BlockTracks::onPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  uint8_t block;
  uint8_t meta;

  if (!Mineserver::get()->map(map)->getBlock(x, y, z, &block, &meta))
     return true;

  /* Check block below allows blocks placed on top */
  if (!this->isBlockStackable(block))
     return true;

  /* move the x,y,z coords dependent upon placement direction */
  if (!this->translateDirection(&x,&y,&z,map,direction))
     return true;

  if (!this->isBlockEmpty(x,y,z,map))
     return true;
   
  uint8_t metadata = FLAT_NS;
  
  // SOUTH
  if(isTrack(x, y, z-1,map, meta) && isStartPiece(x, y, z-1,map))
  {
    LOG(INFO, "Tracks", "SOUTH");
    metadata = FLAT_NS;
    
    // Rising & falling tracks
    if(isTrack(x, y+1, z+1,map, meta) && isStartPiece(x, y+1, z+1,map))
    {
      metadata = ASCEND_N;
    }
    
    if(isTrack(x+1, y, z,map, meta) && isStartPiece(x+1, y, z,map))
    {
      metadata = CORNER_NE;
    }
    if(isTrack(x-1, y, z,map, meta) && isStartPiece(x-1, y, z,map))
    {
      metadata = CORNER_NW;
    }
    
    // Modify previous trackpiece to form corner
    if(isTrack(x-1, y, z-1,map, meta) && meta != FLAT_NS && meta != CORNER_NW && meta != CORNER_SW)
    {
      Mineserver::get()->map(map)->setBlock(x, y, z-1, (char)newblock, CORNER_SW);
      Mineserver::get()->map(map)->sendBlockChange(x, y, z-1, (char)newblock, CORNER_SW);
    }
    else if(isTrack(x+1, y, z-1,map, meta) && meta != FLAT_NS && meta != CORNER_NE && meta != CORNER_SE)
    {
      Mineserver::get()->map(map)->setBlock(x, y, z-1, (char)newblock, CORNER_SE);
      Mineserver::get()->map(map)->sendBlockChange(x, y, z-1, (char)newblock, CORNER_SE);
    }
  }
  
  // NORTH
  if(isTrack(x, y, z+1,map, meta) && isStartPiece(x, y, z+1,map))
  {
    LOG(INFO, "Tracks", "NORTH");

    metadata = FLAT_NS;
    // Rising & falling tracks
    if(isTrack(x, y+1, z-1,map, meta) && isStartPiece(x, y+1, z-1,map))
    {
      metadata = ASCEND_S;
    }
    
    if(isTrack(x+1, y, z,map, meta) && isStartPiece(x+1, y, z,map))
    {
      metadata = CORNER_SE;
    }
    if(isTrack(x-1, y, z,map, meta) && isStartPiece(x-1, y, z,map))
    {
      metadata = CORNER_SW;
    }
    
    // Modify previous trackpiece to form corner
    if(isTrack(x+1, y, z+1,map, meta) && meta != FLAT_NS && meta != CORNER_NE && meta != CORNER_SE)
    {
      Mineserver::get()->map(map)->setBlock(x, y, z+1, (char)newblock, CORNER_NE);
      Mineserver::get()->map(map)->sendBlockChange(x, y, z+1, (char)newblock, CORNER_NE);
    }
    else if(isTrack(x-1, y, z+1,map, meta) && meta != FLAT_NS && meta != CORNER_NW && meta != CORNER_SW)
    { 
      Mineserver::get()->map(map)->setBlock(x, y, z+1, (char)newblock, CORNER_NW);
      Mineserver::get()->map(map)->sendBlockChange(x, y, z+1, (char)newblock, CORNER_NW);
    }
  }
    
  // EAST
  if(isTrack(x-1, y, z,map, meta) && isStartPiece(x-1, y, z,map))
  {
    LOG(INFO, "Tracks", "EAST");
    metadata = FLAT_EW;
    
    // Rising & falling tracks
    if(isTrack(x+1, y+1, z,map, meta) && isStartPiece(x+1, y+1, z,map))
    {
      metadata = ASCEND_W;
    } 
    else
    {
      // Change previous block meta
      Mineserver::get()->map(map)->setBlock(x-1, y, z, (char)newblock, FLAT_EW);
      Mineserver::get()->map(map)->sendBlockChange(x-1, y, z, (char)newblock, FLAT_EW);
    }
  
    if(isTrack(x, y, z+1,map, meta) && isStartPiece(x, y, z+1,map))
    {
      metadata = CORNER_SW;
    }
    if(isTrack(x, y, z-1,map, meta) && isStartPiece(x, y, z-1,map))
    {
      metadata = CORNER_NW;
    }
    
    // Modify previous trackpiece to form corner
    if(isTrack(x-1, y, z-1,map, meta) && meta != FLAT_EW && meta != CORNER_NE && meta != CORNER_NW)
    {
      Mineserver::get()->map(map)->setBlock(x-1, y, z, (char)newblock, CORNER_NE);
      Mineserver::get()->map(map)->sendBlockChange(x-1, y, z, (char)newblock, CORNER_NE);
    }
    else if(isTrack(x-1, y, z+1,map, meta) && meta != FLAT_EW && meta != CORNER_SW && meta != CORNER_SE)
    {
      Mineserver::get()->map(map)->setBlock(x-1, y, z, (char)newblock, CORNER_SE);
      Mineserver::get()->map(map)->sendBlockChange(x-1, y, z, (char)newblock, CORNER_SE);
    }
  }
  
  // WEST
  if(isTrack(x+1, y, z,map, meta) && isStartPiece(x+1, y, z,map))
  {
    LOG(INFO, "Tracks", "WEST");
    metadata = FLAT_EW;
    // Change previous block meta
    
    // Rising & falling tracks
    if(isTrack(x-1, y+1, z,map, meta) && isStartPiece(x-1, y+1, z,map))
    {
      metadata = ASCEND_E;
    } 
    else
    {
      Mineserver::get()->map(map)->setBlock(x+1, y, z, (char)newblock, FLAT_EW);
      Mineserver::get()->map(map)->sendBlockChange(x+1, y, z, (char)newblock, FLAT_EW);
    }

    if(isTrack(x, y, z+1,map, meta) && isStartPiece(x, y, z+1,map))
    {
      metadata = CORNER_SE;
    }
    if(isTrack(x, y, z-1,map, meta) && isStartPiece(x, y, z-1,map))
    {
      metadata = CORNER_NE;
    }
    
    // Modify previous trackpiece to form corner
    if(isTrack(x+1, y, z-1,map, meta) && meta != FLAT_EW && meta != CORNER_NW && meta != CORNER_NE)
    {
      Mineserver::get()->map(map)->setBlock(x+1, y, z, (char)newblock, CORNER_NW);
      Mineserver::get()->map(map)->sendBlockChange(x+1, y, z, (char)newblock, CORNER_NW);
    }
    else if(isTrack(x+1, y, z+1,map, meta) && meta != FLAT_EW && meta != CORNER_SW && meta != CORNER_SE)
    {
      Mineserver::get()->map(map)->setBlock(x+1, y, z, (char)newblock, CORNER_SW);
      Mineserver::get()->map(map)->sendBlockChange(x+1, y, z, (char)newblock, CORNER_SW);
    }
  }
  
  Mineserver::get()->map(map)->setBlock(x, y, z, (char)newblock, metadata);
  Mineserver::get()->map(map)->sendBlockChange(x, y, z, (char)newblock, metadata);
  return false;
}

void BlockTracks::onNeighbourPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{

}

void BlockTracks::onReplace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  
}

void BlockTracks::onNeighbourMove(User* user, int16_t oldblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  
}

bool BlockTracks::isTrack(int32_t x, int8_t y, int32_t z, int map, uint8_t& meta)
{
  uint8_t block;
  Mineserver::get()->map(map)->getBlock(x, y, z, &block, &meta);
  return (block == BLOCK_MINECART_TRACKS);
}

bool BlockTracks::isStartPiece(int32_t x, int8_t y, int32_t z, int map)
{
  uint8_t block;
  uint8_t meta;
  int32_t x1, x2, z1, z2;
  x1 = x2 = x;
  z1 = z2 = z;
  
  int8_t y1, y2;
  y1 = y2 = y;
  
  Mineserver::get()->map(map)->getBlock(x, y, z, &block, &meta);
  switch(meta)
  {
    case FLAT_EW:
      x1--;
      x2++;
      break;
    case FLAT_NS:
      z1--;
      z2++;
      break;
    case CORNER_SE:
      x1++;
      z2++;
      break;
    case CORNER_SW:
      z1++;
      x2--;
      break;
    case CORNER_NE:
      x1++;
      z2--;
      break;
    case CORNER_NW:
      x1--;
      z2--;
      break;
    case ASCEND_E:
      y1++;
      x1--;
      x2++;
      break;
    case ASCEND_N:
      y1++;
      z1++;
      z2--;
      break;
    case ASCEND_S:
      y1++;
      z1--;
      z2++;
      break;
    case ASCEND_W:
      y1++;
      x1++;
      x2--;
      break;
  }
  
  if((Mineserver::get()->map(map)->getBlock(x1, y1, z1, &block, &meta) && block != BLOCK_MINECART_TRACKS) ||
     (Mineserver::get()->map(map)->getBlock(x2, y2, z2, &block, &meta) && block != BLOCK_MINECART_TRACKS))
  {
    return true;
  }
  else
  {
    return false;
  }
}

