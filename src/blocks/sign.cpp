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

#include "sign.h"
#include <math.h>

void BlockSign::onStartedDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{
}

void BlockSign::onDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{
}

void BlockSign::onStoppedDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{
}

void BlockSign::onBroken(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{
}

void BlockSign::onNeighbourBroken(User* user, sint8 oldblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
}

void BlockSign::onPlace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
   uint8 oldblock;
   uint8 oldmeta;

   if (!Map::get()->getBlock(x, y, z, &oldblock, &oldmeta))
      return;

   /* Check block below allows blocks placed on top */
   if (!this->isBlockStackable(oldblock))
      return;

   // 0x0 -> West  West  West  West
   // 0x1 -> West  West  West  North
   // 0x2 -> West  West  North North
   // 0x3 -> West  North North North
   // 0x4 -> North North North North
   // 0x5 -> North North North East
   // 0x6 -> North North East  East
   // 0x7 -> North East  East  East
   // 0x8 -> East  East  East  East
   // 0x9 -> East  East  East  South
   // 0xA -> East  East  South South
   // 0xB -> East  South South South
   // 0xC -> South South South South
   // 0xD -> South South South West
   // 0xE -> South South West  West
   // 0xF -> South West  West  West

   //           The values were the signs are facing
   //
   //                          North
   //
   //      -X |                  4
   //         |              3       5
   //         |           2             6
   //         |         1       [=]       7
   //  West   |        0         |         8           East
   //         |         F                 9
   //         |           E             A
   //         |              D       B
   //      +X |                  C
   //           ----------------------------------
   //           +Z                              -Z
   //
   //                          South


   // Were it should be placed depending on the user's position
   //
   //                          North
   //
   //      -X |                  C
   //         |              B       D
   //         |           A      o      E
   //         |         9       [ ]       F
   //  West   |        8        | |         0           East
   //         |         7                 1
   //         |           6             2
   //         |              5       3
   //      +X |                  4
   //           ----------------------------------
   //           +Z                              -Z
   //
   //                          South
   
   // We place according to the player's position
   double diffX = (x + 0.5) - user->pos.x; // + 0.5 to get the middle of the square
   double diffZ = (z + 0.5) - user->pos.z; // + 0.5 to get the middle of the square

   double angleDegree = ((atan2(diffZ, diffX) * 180 / M_PI + 90) / 22.5);

   uint8 metadata;
   if (angleDegree < 0)
   {
      angleDegree += 16;
   }
   newblock = BLOCK_WALL_SIGN;
   metadata = 6-direction;
   switch(direction)
   {
      case BLOCK_SOUTH:
         x--;
      break;
      case BLOCK_NORTH:
         x++;
      break;
      case BLOCK_EAST:
         z++;
      break;
      case BLOCK_WEST:
         z--;
      break;
      case BLOCK_TOP:
         y++;
         newblock = BLOCK_SIGN_POST;
         metadata = (uint8)(angleDegree + 0.5);
      break;
      case BLOCK_BOTTOM:
      default:
         return;
      break;
   }

   if (!this->isBlockEmpty(x,y,z))
      return;

   Map::get()->setBlock(x, y, z, (char)newblock, metadata);
   Map::get()->sendBlockChange(x, y, z, (char)newblock, metadata);
}

void BlockSign::onNeighbourPlace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
}

void BlockSign::onReplace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
}
