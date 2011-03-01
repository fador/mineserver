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

#ifndef _PHYSICS_H
#define _PHYSICS_H

class Physics
{
public:
  bool enabled;
  int map; // Which map are we affecting?

  bool update();
  bool addSimulation(vec pos);
  bool removeSimulation(vec pos);
  bool checkSurrounding(vec pos);

private:
  enum { TYPE_WATER, TYPE_LAVA } SimType;
  enum { M0, M1, M2, M3, M4, M5, M6, M7, M_FALLING } SimState;

  struct SimBlock
  {
    uint8_t id;
    vec pos;
    uint8_t meta;
    SimBlock() {}
    SimBlock(uint8_t id, vec pos, uint8_t meta)
    {
      this->id   = id;
      this->pos  = pos;
      this->meta = meta;
    }
  };

  struct Sim
  {
    char type;
    std::vector<SimBlock> blocks;
    Sim(char stype, SimBlock initblock) : type(stype), blocks(1, initblock) {}
  };

  std::vector<Sim> simList;
};

#endif
