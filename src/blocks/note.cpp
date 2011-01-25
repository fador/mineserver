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

#include "../mineserver.h"
#include "../map.h"

#include "note.h"

bool BlockNote::affectedBlock(int block)
{
  switch(block)
  {
  case BLOCK_NOTE_BLOCK:
    return true;
  }
  return false;
}


void BlockNote::onStartedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  uint8_t block,metadata;
  Mineserver::get()->map(map)->getBlock(x, y, z, &block, &metadata);
  Mineserver::get()->map(map)->sendNote(x, y, z, BlockNote::getInstrument(x,y - 1,z,map), metadata);
}

bool BlockNote::onPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction){
  uint8_t oldblock;
  uint8_t oldmeta;

  if (!Mineserver::get()->map(map)->getBlock(x, y, z, &oldblock, &oldmeta))
  {
    return true;
  }
  /* Check block below allows blocks placed on top */
  if (!this->isBlockStackable(oldblock))
  {
    return true;
  }

  /* move the x,y,z coords dependent upon placement direction */
  if (!this->translateDirection(&x,&y,&z,map,direction))
  {
    return true;
  }

  if (this->isUserOnBlock(x,y,z,map))
  {
    return true;
  }

  if (!this->isBlockEmpty(x,y,z,map))
  {
    return true; 
  }

  //direction = user->relativeToBlock(x, y, z);

  Mineserver::get()->map(map)->setBlock(x, y, z, BLOCK_NOTE_BLOCK, 0);
  Mineserver::get()->map(map)->sendBlockChange(x, y, z, BLOCK_NOTE_BLOCK, 0);
  return false;
}
int BlockNote::getInstrument(int32_t x, int8_t y, int32_t z, int map)
{
 /* There has to be a cleaner way of doing this. */
 uint8_t block,meta;
 Mineserver::get()->map(map)->getBlock(x, y, z, &block, &meta);
 if(block == BLOCK_WOOD || block == BLOCK_PLANK){
 return 1;
 }
 else if(block == BLOCK_SAND || block == BLOCK_GRAVEL || block == BLOCK_SLOW_SAND){
 return 2;
 }
 else if(block == BLOCK_GLASS || block ==  BLOCK_GLOWSTONE){
 return 3;
 }
 else if(block == BLOCK_STONE || block ==  BLOCK_COBBLESTONE || block == BLOCK_BRICK || block == BLOCK_OBSIDIAN 
	 || block == BLOCK_NETHERSTONE || block == BLOCK_IRON_ORE || block == BLOCK_DIAMOND_ORE ){
 return 4;
 }
 return 5;
}