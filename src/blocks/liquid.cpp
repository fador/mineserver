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

#include "liquid.h"
#include "../physics.h"

void BlockLiquid::onStartedDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

void BlockLiquid::onDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

void BlockLiquid::onStoppedDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

void BlockLiquid::onBroken(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{
}

void BlockLiquid::onNeighbourBroken(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{
   physics(x,y,z);
}

void BlockLiquid::onPlace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
   uint8 topblock;
   uint8 topmeta;
   if (Map::get()->getBlock(x, y+1, z, &topblock, &topmeta) && topblock == BLOCK_AIR)
   {
      Map::get()->setBlock(x, y+1, z, (char)newblock, 0);
      Map::get()->sendBlockChange(x, y+1, z, (char)newblock, 0);
      
      Physics::get()->addSimulation(vec(x, y, z));
      physics(x,y+1,z);
   }
}

void BlockLiquid::onNeighbourPlace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
}

void BlockLiquid::onReplace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
   uint8 oldblock;
   uint8 oldmeta;

   if (Map::get()->getBlock(x, y, z, &oldblock, &oldmeta))
   {
      Map::get()->sendBlockChange(x, y, z, BLOCK_AIR, 0);
      Map::get()->setBlock(x, y, z, BLOCK_AIR, 0);
      physics(x,y,z);
   }
}

void BlockLiquid::physics(sint32 x, sint8 y, sint32 z)
{
   Physics::get()->checkSurrounding(vec(x, y, z));
}

