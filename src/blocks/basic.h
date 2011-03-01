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

#ifndef _BLOCKS_BASIC_H
#define _BLOCKS_BASIC_H

#include <stdint.h>
#include <string>

#include "../constants.h"

class User;

/**
 * BlockBasic can be used to extend your own block classes. It contains useful
 * methods for reuse in your own block extensions.
 */
class BlockBasic
{
public:
  virtual bool affectedBlock(int block);
  virtual void notifyNeighbours(const int32_t x, const int8_t y, const int32_t z, const int map, const std::string callback, User* user, const uint8_t oldblock, const int8_t ignore_direction);

  virtual void timer200() { }
  virtual void onStartedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction);
  virtual void onDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map,  int8_t direction);
  virtual void onStoppedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map,  int8_t direction);
  virtual bool onBroken(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map,  int8_t direction);
  virtual void onNeighbourBroken(User* user, int16_t oldblock, int32_t x, int8_t y, int32_t z, int map,  int8_t direction);
  virtual bool onPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map,  int8_t direction);
  virtual void onNeighbourPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map,  int8_t direction);
  virtual void onReplace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map,  int8_t direction);
  virtual void onNeighbourMove(User* user, int16_t oldblock, int32_t x, int8_t y, int32_t z, int8_t direction);
  virtual bool onInteract(User* user, int32_t x, int8_t y, int32_t z, int map);
  void revertBlock(User* user, int32_t x, int8_t y, int32_t z, int map);


protected:
  bool isBlockStackable(const uint8_t block);
  bool isUserOnBlock(const int32_t x, const int8_t y, const int32_t z, const int map);
  virtual bool translateDirection(int32_t* x, int8_t* y, int32_t* z, const int map, const int8_t direction);
  bool isBlockEmpty(const int32_t x, const int8_t y, const int32_t z, const int map);
  bool spawnBlockItem(const int32_t x, const int8_t y, const int32_t z, const int map, const uint8_t block, uint8_t meta = 0);
};

#endif
