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

#include "basic.h"

#include "../plugin.h"
#include <cmath>
#include <cstdlib>

bool BlockBasic::isBlockStackable(const uint8 block)
{
   /* Check block below allows blocks placed on top */
   switch(block)
   {
      case BLOCK_WORKBENCH:
      case BLOCK_FURNACE:
      case BLOCK_BURNING_FURNACE:
      case BLOCK_CHEST:
      case BLOCK_JUKEBOX:
      case BLOCK_TORCH:
      case BLOCK_REDSTONE_TORCH_OFF:
      case BLOCK_REDSTONE_TORCH_ON:
      case BLOCK_WATER:
      case BLOCK_STATIONARY_WATER:
      case BLOCK_LAVA:
      case BLOCK_STATIONARY_LAVA:
      case BLOCK_AIR:
      case BLOCK_MINECART_TRACKS:
       return false;
      break;
      default:
      break;
   }
   return true;
}

bool BlockBasic::isUserOnBlock(const sint32 x, const sint8 y, const sint32 z)
{
   /* TODO: Get Users by chunk rather then whole list */
   for(unsigned int i = 0; i < User::all().size(); i++)
   {
      /* don't allow block placement on top of player */
      if (User::all()[i]->checkOnBlock(x,y,z))
         return true;
   }
   return false;
}

bool BlockBasic::translateDirection(sint32 *x, sint8 *y, sint32 *z, const sint8 direction)
{
   switch(direction)
   {
      case BLOCK_SOUTH:
         *x-=1;
      break;
      case BLOCK_NORTH:
         *x+=1;
      break;
      case BLOCK_EAST:
         *z+=1;
      break;
      case BLOCK_WEST:
         *z-=1;
      break;
      case BLOCK_TOP:
         *y+=1;
      break;
      case BLOCK_BOTTOM:
         *y-=1;
      break;
      default:
         return false;
      break;
   }
   return true;
}

bool BlockBasic::isBlockEmpty(const sint32 x, const sint8 y, const sint32 z)
{
   uint8 block;
   uint8 meta;
   return Map::get()->getBlock(x, y, z, &block, &meta) && block == BLOCK_AIR;
}

bool BlockBasic::spawnBlockItem(const sint32 x, const sint8 y, const sint32 z, const uint8 block)
{
   if (BLOCKDROPS.count(block) && BLOCKDROPS[block].probability >= rand() % 10000)
   {
       int count = 0;
       uint16 item_id = BLOCKDROPS[block].item_id;
       count = BLOCKDROPS[block].count;
       if (count)
       {
          Map::get()->createPickupSpawn(x, y, z, item_id, count);
          return true;
       }
   }
   return false;
}

void BlockBasic::notifyNeighbours(const sint32 x, const sint8 y, const sint32 z, const std::string callback, User* user,const uint8 oldblock, const sint8 ignore_direction)
{
   uint8 block;
   uint8 meta;
   Function::invoker_type inv(user, oldblock, x, y, z, 0);

   if (ignore_direction != BLOCK_SOUTH
        && Map::get()->getBlock(x+1, y, z, &block, &meta) && block != BLOCK_AIR)
   {
      inv = Function::invoker_type(user, oldblock, x+1, y, z, BLOCK_SOUTH);
      Plugin::get()->runBlockCallback(block, callback, inv);
   }

   if (ignore_direction != BLOCK_NORTH
        && Map::get()->getBlock(x-1, y, z, &block, &meta) && block != BLOCK_AIR)
   {
      inv = Function::invoker_type(user, oldblock, x-1, y, z, BLOCK_NORTH);
      Plugin::get()->runBlockCallback(block, callback, inv);
   }

   if (ignore_direction != BLOCK_TOP
        && Map::get()->getBlock(x, y+1, z, &block, &meta) && block != BLOCK_AIR)
   {
      inv = Function::invoker_type(user, oldblock, x, y+1, z, BLOCK_TOP);
      Plugin::get()->runBlockCallback(block, callback, inv);
   }

   if (ignore_direction != BLOCK_BOTTOM
        && Map::get()->getBlock(x, y-1, z, &block, &meta) && block != BLOCK_AIR)
   {
      inv = Function::invoker_type(user, oldblock, x, y-1, z, BLOCK_BOTTOM);
      Plugin::get()->runBlockCallback(block, callback, inv);
   }

   if (ignore_direction != BLOCK_WEST
        && Map::get()->getBlock(x, y, z+1, &block, &meta) && block != BLOCK_AIR)
   {
      inv = Function::invoker_type(user, oldblock, x, y, z+1, BLOCK_WEST);
      Plugin::get()->runBlockCallback(block, callback, inv);
   }

   if (ignore_direction != BLOCK_EAST
        && Map::get()->getBlock(x, y, z-1, &block, &meta) && block != BLOCK_AIR)
   {
      inv = Function::invoker_type(user, oldblock, x, y, z-1, BLOCK_EAST);
      Plugin::get()->runBlockCallback(block, callback, inv);
   }
}
