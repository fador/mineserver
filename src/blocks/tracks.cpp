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

#include "tracks.h"
#include <iostream>

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

void BlockTracks::onStartedDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

void BlockTracks::onDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

void BlockTracks::onStoppedDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

void BlockTracks::onBroken(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{
  
}

void BlockTracks::onNeighbourBroken(User* user, sint8 oldblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
  uint8 block;
  uint8 meta;
  
  if (!Map::get()->getBlock(x, y, z, &block, &meta))
    return;

  if (this->isBlockEmpty(x, y-1, z))
  {
    // Break torch and spawn torch item
    Map::get()->sendBlockChange(x, y, z, BLOCK_AIR, 0);
    Map::get()->setBlock(x, y, z, BLOCK_AIR, 0);
    this->spawnBlockItem(x, y, z, block);
  }
}

void BlockTracks::onPlace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
  uint8 block;
  uint8 meta;

  if (!Map::get()->getBlock(x, y, z, &block, &meta))
     return;

  /* Check block below allows blocks placed on top */
  if (!this->isBlockStackable(block))
     return;

  /* move the x,y,z coords dependent upon placement direction */
  if (!this->translateDirection(&x,&y,&z,direction))
     return;

  if (!this->isBlockEmpty(x,y,z))
     return;
   
  uint8 metadata = FLAT_NS;
  
  // SOUTH
  if(isTrack(x, y, z-1, meta) && isStartPiece(x, y, z-1))
  {
    std::cout << "SOUTH" << std::endl;
    metadata = FLAT_NS;
    
    // Rising & falling tracks
    if(isTrack(x, y+1, z+1, meta) && isStartPiece(x, y+1, z+1))
    {
      metadata = ASCEND_N;
    }
    
    if(isTrack(x+1, y, z, meta) && isStartPiece(x+1, y, z))
    {
      metadata = CORNER_NE;
    }
    if(isTrack(x-1, y, z, meta) && isStartPiece(x-1, y, z))
    {
      metadata = CORNER_NW;
    }
    
    // Modify previous trackpiece to form corner
    if(isTrack(x-1, y, z-1, meta) && meta != FLAT_NS && meta != CORNER_NW && meta != CORNER_SW)
    {
      Map::get()->setBlock(x, y, z-1, (char)newblock, CORNER_SW);
      Map::get()->sendBlockChange(x, y, z-1, (char)newblock, CORNER_SW);
    }
    else if(isTrack(x+1, y, z-1, meta) && meta != FLAT_NS && meta != CORNER_NE && meta != CORNER_SE)
    {
      Map::get()->setBlock(x, y, z-1, (char)newblock, CORNER_SE);
      Map::get()->sendBlockChange(x, y, z-1, (char)newblock, CORNER_SE);
    }
  }
  
  // NORTH
  if(isTrack(x, y, z+1, meta) && isStartPiece(x, y, z+1))
  {
    std::cout << "NORTH" << std::endl;

    metadata = FLAT_NS;
    // Rising & falling tracks
    if(isTrack(x, y+1, z-1, meta) && isStartPiece(x, y+1, z-1))
    {
      metadata = ASCEND_S;
    }
    
    if(isTrack(x+1, y, z, meta) && isStartPiece(x+1, y, z))
    {
      metadata = CORNER_SE;
    }
    if(isTrack(x-1, y, z, meta) && isStartPiece(x-1, y, z))
    {
      metadata = CORNER_SW;
    }
    
    // Modify previous trackpiece to form corner
    if(isTrack(x+1, y, z+1, meta) && meta != FLAT_NS && meta != CORNER_NE && meta != CORNER_SE)
    {
      Map::get()->setBlock(x, y, z+1, (char)newblock, CORNER_NE);
      Map::get()->sendBlockChange(x, y, z+1, (char)newblock, CORNER_NE);
    }
    else if(isTrack(x-1, y, z+1, meta) && meta != FLAT_NS && meta != CORNER_NW && meta != CORNER_SW)
    { 
      Map::get()->setBlock(x, y, z+1, (char)newblock, CORNER_NW);
      Map::get()->sendBlockChange(x, y, z+1, (char)newblock, CORNER_NW);
    }
  }
    
  // EAST
  if(isTrack(x-1, y, z, meta) && isStartPiece(x-1, y, z))
  {
    std::cout << "EAST" << std::endl;
    metadata = FLAT_EW;
    
    // Rising & falling tracks
    if(isTrack(x+1, y+1, z, meta) && isStartPiece(x+1, y+1, z))
    {
      metadata = ASCEND_W;
    } 
    else
    {
      // Change previous block meta
      Map::get()->setBlock(x-1, y, z, (char)newblock, FLAT_EW);
      Map::get()->sendBlockChange(x-1, y, z, (char)newblock, FLAT_EW);
    }
  
    if(isTrack(x, y, z+1, meta) && isStartPiece(x, y, z+1))
    {
      metadata = CORNER_SW;
    }
    if(isTrack(x, y, z-1, meta) && isStartPiece(x, y, z-1))
    {
      metadata = CORNER_NW;
    }
    
    // Modify previous trackpiece to form corner
    if(isTrack(x-1, y, z-1, meta) && meta != FLAT_EW && meta != CORNER_NE && meta != CORNER_NW)
    {
      Map::get()->setBlock(x-1, y, z, (char)newblock, CORNER_NE);
      Map::get()->sendBlockChange(x-1, y, z, (char)newblock, CORNER_NE);
    }
    else if(isTrack(x-1, y, z+1, meta) && meta != FLAT_EW && meta != CORNER_SW && meta != CORNER_SE)
    {
      Map::get()->setBlock(x-1, y, z, (char)newblock, CORNER_SE);
      Map::get()->sendBlockChange(x-1, y, z, (char)newblock, CORNER_SE);
    }
  }
  
  // WEST
  if(isTrack(x+1, y, z, meta) && isStartPiece(x+1, y, z))
  {
    std::cout << "WEST" << std::endl;
    metadata = FLAT_EW;
    // Change previous block meta
    
    // Rising & falling tracks
    if(isTrack(x-1, y+1, z, meta) && isStartPiece(x-1, y+1, z))
    {
      metadata = ASCEND_E;
    } 
    else
    {
      Map::get()->setBlock(x+1, y, z, (char)newblock, FLAT_EW);
      Map::get()->sendBlockChange(x+1, y, z, (char)newblock, FLAT_EW);
    }

    if(isTrack(x, y, z+1, meta) && isStartPiece(x, y, z+1))
    {
      metadata = CORNER_SE;
    }
    if(isTrack(x, y, z-1, meta) && isStartPiece(x, y, z-1))
    {
      metadata = CORNER_NE;
    }
    
    // Modify previous trackpiece to form corner
    if(isTrack(x+1, y, z-1, meta) && meta != FLAT_EW && meta != CORNER_NW && meta != CORNER_NE)
    {
      Map::get()->setBlock(x+1, y, z, (char)newblock, CORNER_NW);
      Map::get()->sendBlockChange(x+1, y, z, (char)newblock, CORNER_NW);
    }
    else if(isTrack(x+1, y, z+1, meta) && meta != FLAT_EW && meta != CORNER_SW && meta != CORNER_SE)
    {
      Map::get()->setBlock(x+1, y, z, (char)newblock, CORNER_SW);
      Map::get()->sendBlockChange(x+1, y, z, (char)newblock, CORNER_SW);
    }
  }
  
  
  std::cout << std::endl;
  Map::get()->setBlock(x, y, z, (char)newblock, metadata);
  Map::get()->sendBlockChange(x, y, z, (char)newblock, metadata);
}

void BlockTracks::onNeighbourPlace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

void BlockTracks::onReplace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
  
}

void BlockTracks::onNeighbourMove(User* user, sint8 oldblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
  
}

bool BlockTracks::isTrack(sint32 x, sint8 y, sint32 z, uint8& meta)
{
  uint8 block;
  Map::get()->getBlock(x, y, z, &block, &meta);
  return (block == BLOCK_MINECART_TRACKS);
}

bool BlockTracks::isStartPiece(sint32 x, sint8 y, sint32 z)
{
  uint8 block;
  uint8 meta;
  sint32 x1, x2, z1, z2;
  x1 = x2 = x;
  z1 = z2 = z;
  
  sint8 y1, y2;
  y1 = y2 = y;
  
  Map::get()->getBlock(x, y, z, &block, &meta);
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
  
  if((Map::get()->getBlock(x1, y1, z1, &block, &meta) && block != BLOCK_MINECART_TRACKS) ||
     (Map::get()->getBlock(x2, y2, z2, &block, &meta) && block != BLOCK_MINECART_TRACKS))
  {
    return true;
  }
  else
  {
    return false;
  }
}

