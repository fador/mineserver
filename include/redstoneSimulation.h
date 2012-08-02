/*
   Copyright (c) 2012, The Mineserver Project
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

#ifndef _REDSTONESIMULATION_H
#define _REDSTONESIMULATION_H

#include <stdint.h>
#include <vector>
#include "vec.h"
#include "tools.h"

class RedstoneSimulation
{
public:
  enum Power { POWER_NONE, POWER_WEAK, POWER_NORMAL };

  bool enabled;
  int map;
  bool addSimulation(vec pos);  
  bool update();
  Power getPower(int32_t x, int16_t y, int32_t z);

private:
  bool isBlockSolid(const uint8_t block);

  enum { TYPE_WIRE, TYPE_TORCH, TYPE_BLOCK } SimType;
  

  struct RedstoneSim
  {
    uint8_t id;
    vec pos;    
    uint8_t power;
    uint8_t direction;
    RedstoneSim() {}
    RedstoneSim(uint8_t id, vec pos, uint8_t power, uint8_t direction = -1)
    {
      this->id    = id;
      this->pos   = pos;
      this->power = power;
      this->direction = direction;
    }
  };

  std::vector<RedstoneSim> simList;

};

#endif
