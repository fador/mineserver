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
#include "chunkmap.h"

class User;
class NBT_Value;

class Creation
{
public:
  int output, meta, count;
  Creation();
};

enum
{
  SLOT_INPUT,
  SLOT_FUEL,
  SLOT_OUTPUT
};

class Furnace
{


public:
  Furnace(furnaceData* data_);

  void sendToAllUsers();
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
  void updateItems();

  int16_t fuelBurningTime()
  {
    return data->burnTime;
  }

  int16_t setFuelBurningTime(int16_t burntime)
  {
    data->burnTime = burntime;
    return data->burnTime;
  }

  int16_t cookingTime()
  {
    return data->cookTime;
  }
  int16_t setCookingTime(int16_t cookTime)
  {
    data->cookTime = cookTime;
    return data->cookTime;
  }

  int32_t x()
  {
    return data->x;
  }
  int32_t y()
  {
    return data->y;
  }
  int32_t z()
  {
    return data->z;
  }

  Item* slots()
  {
    return data->items;
  };

private:
  furnaceData* data;
  bool m_burning;
};

void readConfig();

#endif // _FURNACE.H
