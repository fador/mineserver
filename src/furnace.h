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

#ifndef _FURNACE_H
#define _FURNACE_H

#include <cstdlib>
#include <stdint.h>

class User;
class NBT_Value;

enum
{
  SLOT_INPUT,
  SLOT_FUEL,
  SLOT_OUTPUT
};

class Furnace
{

  struct Slot
  {
    int8_t count;
    int16_t damage;
    int16_t id;
  };

public:
  Furnace(NBT_Value* entity, uint8_t blockType);

  void sendToAllUsers();
  NBT_Value* getSlotEntity(int8_t slotNumber);
  void smelt();
  bool isBurningFuel();
  bool isCooking();
  bool hasValidIngredient();
  void consumeFuel();
  void updateBlock();

  /**
   * Getter & setter methods
   */

  int16_t burnTime();
  int16_t cookTime();

  int16_t fuelBurningTime() { return m_fuelBurningTime; }
  void setFuelBurningTime(int16_t fuelBurningTime) { m_fuelBurningTime = fuelBurningTime; }

  int16_t activeCookDuration() { return m_activeCookDuration; }
  void setActiveCookDuration(int16_t activeCookDuration) { m_activeCookDuration = activeCookDuration; }

  int16_t cookingTime() { return m_cookingTime; }

  int32_t x() { return m_x; }
  int32_t y() { return m_y; }
  int32_t z() { return m_z; }

  Slot* slots() { return m_slots; };

private:
  int16_t m_fuelBurningTime;
  int16_t m_initialBurningTime;
  int16_t m_cookingTime;
  int16_t m_activeCookDuration;
 // int16_t activeBurnDuration;
  int32_t m_x;
  int32_t m_y;
  int32_t m_z;
  int map;
  Slot m_slots[3];

  bool m_burning;
};

#endif // _FURNACE.H
