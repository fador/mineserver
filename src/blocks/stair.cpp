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

#include "stair.h"

void BlockStair::onStartedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int8_t direction)
{

}

void BlockStair::onDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int8_t direction)
{

}

void BlockStair::onStoppedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int8_t direction)
{

}

void BlockStair::onBroken(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int8_t direction)
{
}

void BlockStair::onNeighbourBroken(User* user, int8_t oldblock, int32_t x, int8_t y, int32_t z, int8_t direction)
{
   /* TODO: add code to align stairs? */
}

void BlockStair::onPlace(User* user, int8_t newblock, int32_t x, int8_t y, int32_t z, int8_t direction)
{
}

void BlockStair::onNeighbourPlace(User* user, int8_t newblock, int32_t x, int8_t y, int32_t z, int8_t direction)
{
   /* Align neighbour to this stair */
}

void BlockStair::onReplace(User* user, int8_t newblock, int32_t x, int8_t y, int32_t z, int8_t direction)
{
}
