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

#pragma once

#include "basic.h"
class User;


class BlockLadder: public BlockBasic
{
public:
  bool affectedBlock(int block);

  /** Fired when the player's begins swinging to hit a block
     @param user The user who is digging
     @param status
     @param x The x position of the current block being hit
     @param y The y position of the current block being hit
     @param z The z position of the current block being hit
     @param direction The direction that the user is facing
   */
  void onStartedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction);

  /** Fired when the player is digging
     @param user The user who is digging
     @param status
     @param x The x position of the current block being hit
     @param y The y position of the current block being hit
     @param z The z position of the current block being hit
     @param direction The direction that the user is facing
   */
  void onDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction);

  /** Fired when the player stops digging but hasn't broken the block
     @param user The user who has stopped digging
     @param status
     @param x The x position of the current block not being hit anymore
     @param y The y position of the current block not being hit anymore
     @param z The z position of the current block not being hit anymore
     @param direction The direction that the user is facing
   */
  void onStoppedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction);

  /** Fired when the player has broken the block
     @param user The user who has broken the block
     @param status
     @param x The x position of the block that has been broken
     @param y The y position of the block that has been broken
     @param z The z position of the block that has been broken
     @param direction The direction that the user is facing
   */
  bool onBroken(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction);

  /** Fired when a neighbour block is broken.
     @param user The user who broke the block
     @param oldblock The type of block that was just broken
     @param x The x position of the current neighbour block being called
     @param y The y position of the current neighbour block being called
     @param z The z position of the current neighbour block being called
     @param direction The direction of the neighbour block that was broken
   */
  void onNeighbourBroken(User* user, int16_t oldblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction);

  /** Fired when a block is placed
     @param user The user who placed the block
     @param newblock The block that is attempting to be placed
     @param x The x position of where the block was placed
     @param y The y position of where the block was placed
     @param z The z position of where the block was placed
     @param direction The direction that the user is facing
   */
  bool onPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction);

  /** Fired when a neighbour block is placed
     @param user The user who placed the block
     @param newblock The block that is attempting to be placed
     @param x The x position of the current neighbour block being called
     @param y The y position of the current neighbour block being called
     @param z The z position of the current neighbour block being called
     @param direction The direction that the user is facing

     @note You should only check if the block is of type BLOCK_AIR before placing. This
     allows another block's onReplace method to either allow of deny the replacement
     of a block with the block you're replacing it with.
   */
  void onNeighbourPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction);

  /** Fired when a block is to be placed
     @param user The user who placed the block
     @param newblock The block that is attempting to replaced this block
     @param x The x position of where the block is to be replaced
     @param y The y position of where the block is to be replaced
     @param z The z position of where the block is to be replaced
     @param direction The direction of the neighbour block that was broken

     @note To deny the replace simply to not change the block. To replace
     the block change the block to be BLOCK_AIR and check for BLOCK_AIR
     in the onPlace callback.
   */
  void onReplace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction);

  /** Fired when a neighbour block is moving position.
     @param user The user who broke the block
     @param oldblock The type of block that has moved
     @param x The x position of the current neighbour block being called
     @param y The y position of the current neighbour block being called
     @param z The z position of the current neighbour block being called
     @param direction The direction that the block was in
   */
  void onNeighbourMove(User* user, int16_t oldblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction);
};

