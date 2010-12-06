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

#pragma once

#include <cstdlib>

#include "constants.h"
#include "map.h"
#include "tools.h"

class User;

struct sSlot
{
  sint8 count;
  sint16 damage;
  sint16 id;
};

enum
{
  SLOT_INPUT,
  SLOT_FUEL,
  SLOT_OUTPUT
};

class Furnace
{
private:
  
  
public:
  sint16 fuelBurningTime;
  sint16 initialBurningTime;
  sint16 cookingTime;
  sint16 activeCookDuration;
 // sint16 activeBurnDuration;  
  sint32 x;
  sint32 y;
  sint32 z;
  sSlot slots[3];

  bool burning;
  
  Furnace(NBT_Value *entity, uint8 blockType);
  
  void sendToAllUsers();
  NBT_Value* getSlotEntity(sint8 slotNumber);
  void smelt();
  bool isBurningFuel();
  bool isCooking();
  bool hasValidIngredient();
  void consumeFuel();
  void updateBlock();
  sint16 burnTime();
  sint16 cookTime();
};


