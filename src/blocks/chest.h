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
#include "chunkmap.h"

class User;

/**
 * BlockChest deals specifically with chests
 * @see BlockBasic
 */
class BlockChest : public BlockBasic
{
public:
  inline bool affectedBlock(int block) const { return block == BLOCK_CHEST; }

  void onStartedDigging(User* user, int8_t status, int32_t x, int16_t y, int map, int32_t z, int8_t direction);
  void onDigging(User* user, int8_t status, int32_t x, int16_t y, int32_t z, int map, int8_t direction);
  void onStoppedDigging(User* user, int8_t status, int32_t x, int16_t y, int map, int32_t z, int8_t direction);
  bool onBroken(User* user, int8_t status, int32_t x, int16_t y, int32_t z, int map, int8_t direction);
  void onNeighbourBroken(User* user, int16_t oldblock, int32_t x, int16_t y, int32_t z, int map, int8_t direction);
  bool onPlace(User* user, int16_t newblock, int32_t x, int16_t y, int32_t z, int map, int8_t direction);
  void onNeighbourPlace(User* user, int16_t newblock, int32_t x, int16_t y, int32_t z, int map, int8_t direction);
  void onReplace(User* user, int16_t newblock, int32_t x, int16_t y, int32_t z, int map,  int8_t direction);
  void onNeighbourMove(User* user, int16_t oldblock, int32_t x, int16_t y, int32_t z, int map, int8_t direction);
  bool onInteract(User* user, int32_t x, int16_t y, int32_t z, int map);

  /** finds the chest, that is connected to a block. gets coordinates. for large chests
   * @param x x coordinate of the chest you know
   * @param y y coordinate of the chest you know
   * @param z z coordinate of the chest you know
   * @param map map
   * @param chest_x contains the x coordinate of the other chest part
   * @param chest_z contains the z coordinate of the other chest part
   * @returns true when a chest could be found, otherwise false.
   */
  bool findConnectedChest(int32_t x, int16_t y, int32_t z, int map, int32_t* chest_x, int32_t* chest_z);

  /** find the chest, that is connected to a block. gets chestData.
   * @param x x coordinate of the chest you know
   * @param y y coordinate of the chest you know
   * @param z z coordinate of the chest you know
   * @param map map
   * @param chest reference to a chestDataPtr. will be filled, if connected chest is found.
   * @returns true when a chest could be found, otherwise false.
   */
  bool findConnectedChest(int32_t x, int16_t y, int32_t z, int map, chestDataPtr& chest);

  /** gets the chestDataPtr for a cest at given coordinates
   * @param x x coordinate of the chest
   * @param y y coordinate of the chest
   * @param z z coordinate of the chest
   * @param chest is filled with the chestData, if the chest can be found
   * @returns true, when a chest at given coordinates was found, otherwise false
   */
  bool getChestByCoordinates(int32_t x, int16_t y, int32_t z, int map, chestDataPtr& chest);

};
