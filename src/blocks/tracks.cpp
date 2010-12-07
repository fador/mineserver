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

enum {
  FLAT_EW, //x
  FLAT_NS,//z
  ASCEND_S,
  ASCEND_N,
  ASCEND_E,
  ASCEND_W,
  CORNER_NE,
  CORNER_SE,
  CORNER_SW,
  CORNER_NW
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

}

void BlockTracks::onPlace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
  uint8 oldblock;
  uint8 oldmeta;

  if (!Map::get()->getBlock(x, y, z, &oldblock, &oldmeta))
     return;

  /* Check block below allows blocks placed on top */
  if (!this->isBlockStackable(oldblock))
     return;

  /* move the x,y,z coords dependent upon placement direction */
  if (!this->translateDirection(&x,&y,&z,direction))
     return;

  if (!this->isBlockEmpty(x,y,z))
     return;

  //direction = user->relativeToBlock(x, y, z);
   
  uint8 metadata = FLAT_EW;
  
  // If north south track.
  if(isTrack(x-1, y, z) || isTrack(x+1, y, z))
  {
    metadata = FLAT_NS;
  }
  
  // Corners
  if(isTrack(x, y, z+1)  && isTrack(x+1, y, z))
  {
    metadata = CORNER_NE;
  }
  if(isTrack(x, y, z-1)  && isTrack(x-1, y, z) )
  {
    metadata = CORNER_SW;
  }
  if(isTrack(x, y, z+1) && isTrack(x-1, y, z))
  {
    metadata = CORNER_SE;
  }
  if(isTrack(x, y, z-1) && isTrack(x+1, y, z))
  {
    metadata = CORNER_NW;
  }
  
  // NW & SW
  if(isTrack(x, y, z-1) && isTrack(x-1, y, z-1) && !isTrack(x+1, y, z-1))
  {
    Map::get()->setBlock(x, y, z-1, (char)newblock, CORNER_NW);
    Map::get()->sendBlockChange(x, y, z-1, (char)newblock, CORNER_NW);
  }
  if(isTrack(x, y, z-1) && !isTrack(x-1, y, z-1) && isTrack(x+1, y, z-1))
  {
    Map::get()->setBlock(x, y, z-1, (char)newblock, CORNER_SW);
    Map::get()->sendBlockChange(x, y, z-1, (char)newblock, CORNER_SW);
  }

  // SE & SW
  if(isTrack(x-1, y, z) && isTrack(x-1, y, z-1) && !isTrack(x-1, y, z+1))
  {
    Map::get()->setBlock(x-1, y, z, (char)newblock, CORNER_SE);
    Map::get()->sendBlockChange(x-1, y, z, (char)newblock, CORNER_SE);
  }
  if(isTrack(x-1, y, z) && !isTrack(x-1, y, z-1) && isTrack(x-1, y, z+1))
  {
    Map::get()->setBlock(x-1, y, z, (char)newblock, CORNER_SW);
    Map::get()->sendBlockChange(x-1, y, z, (char)newblock, CORNER_SW);
  }
  
  // NW & NE
  if(isTrack(x, y, z+1) && isTrack(x+1, y, z+1) && !isTrack(x-1, y, z+1))
  {
    Map::get()->setBlock(x, y, z+1, (char)newblock, CORNER_NW);
    Map::get()->sendBlockChange(x, y, z+1, (char)newblock, CORNER_NW);
  }
  if(isTrack(x, y, z+1) && !isTrack(x+1, y, z+1) && isTrack(x-1, y, z+1))
  {
    Map::get()->setBlock(x, y, z+1, (char)newblock, CORNER_NE);
    Map::get()->sendBlockChange(x, y, z+1, (char)newblock, CORNER_NE);
  }
  
  // SW & SE
  if(isTrack(x-1, y, z) && isTrack(x-1, y, z+1) && !isTrack(x-1, y, z-1))
  {
    Map::get()->setBlock(x-1, y, z, (char)newblock, CORNER_SW);
    Map::get()->sendBlockChange(x-1, y, z, (char)newblock, CORNER_SW);
  }
  if(isTrack(x-1, y, z) && !isTrack(x-1, y, z+1) && isTrack(x-1, y, z-1))
  {
    Map::get()->setBlock(x, y, z+1, (char)newblock, CORNER_SE);
    Map::get()->sendBlockChange(x, y, z+1, (char)newblock, CORNER_SE);
  }
  
  Map::get()->setBlock(x, y, z, (char)newblock, metadata);
  Map::get()->sendBlockChange(x, y, z, (char)newblock, metadata);
}

void BlockTracks::onNeighbourPlace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
   uint8 oldblock;
   uint8 oldmeta;

   if (!Map::get()->getBlock(x, y, z, &oldblock, &oldmeta))
      return;
  
   /* move the x,y,z coords dependent upon placement direction */
   //if (!this->translateDirection(&x,&y,&z,direction))
   //   return;
  
   uint8 metadata = FLAT_EW;
   /*if((Map::get()->getBlock(x, y, z-1, &oldblock, &oldmeta) && oldblock == BLOCK_MINECART_TRACKS) ||
      (Map::get()->getBlock(x, y, z+1, &oldblock, &oldmeta) && oldblock == BLOCK_MINECART_TRACKS))
     metadata = FLAT_NS;
     
   if((Map::get()->getBlock(x-1, y, z, &oldblock, &oldmeta) && oldblock == BLOCK_MINECART_TRACKS) ||
      (Map::get()->getBlock(x+1, y, z, &oldblock, &oldmeta) && oldblock == BLOCK_MINECART_TRACKS))
     metadata = FLAT_EW;*/
     
  if (direction == BLOCK_NORTH && Map::get()->getBlock(x+1, y, z, &oldblock, &oldmeta) && oldblock == BLOCK_MINECART_TRACKS)
  {
     //x++;
     metadata = FLAT_NS;
  }
  else if (direction == BLOCK_SOUTH && Map::get()->getBlock(x-1, y, z, &oldblock, &oldmeta) && oldblock == BLOCK_MINECART_TRACKS)
  {
     //x--;
     metadata = FLAT_NS;
  }
  /*else if (direction == BLOCK_EAST && meta == BLOCK_WEST && this->isBlockEmpty(x, y, z+1))
  {
      destroy = true;
   }
   else if (direction == BLOCK_WEST && meta == BLOCK_EAST && this->isBlockEmpty(x, y, z-1))
   {
      destroy = true;
   }*/
  
   /*if(newblock == BLOCK_MINECART_TRACKS)
   {
     switch(direction)
     {
       case BLOCK_SOUTH:
       case BLOCK_NORTH:
         metadata = FLAT_NS;
         break;
     }
   }*/
     
   Map::get()->setBlock(x, y, z, (char)oldblock, metadata);
   Map::get()->sendBlockChange(x, y, z, (char)oldblock, metadata);
}

void BlockTracks::onReplace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
  
}

void BlockTracks::onNeighbourMove(User* user, sint8 oldblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
  
}

bool BlockTracks::isTrack(sint32 x, sint8 y, sint32 z)
{
  uint8 block;
  uint8 meta;
  Map::get()->getBlock(x, y, z, &block, &meta);
  return (block == BLOCK_MINECART_TRACKS);
}

