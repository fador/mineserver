/*
   Copyright (c) 2013, The Mineserver Project
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

#include <stdint.h>
#include <vector>
#include "vec.h"
#include "tools.h"

struct entity_position
{
  double x;
  double y;
  double z;
  double vel_x;
  double vel_y;
  double vel_z;

  entity_position() { x = y = z = vel_x = vel_y = vel_z = 0.0; };

  entity_position(double _x, double _y, double _z)
  {
    x = _x; y = _y; z = _z;
    vel_x = vel_y = vel_z = 0.0;
  }

  entity_position(double _x, double _y, double _z,double _vel_x, double _vel_y, double _vel_z)
  {
    x = _x; y = _y; z = _z;
    vel_x = _vel_x;
    vel_y = _vel_y;
    vel_z = _vel_z;
  }
};

class Physics
{
public:
  Physics(Map* map)
    :map(map) {}

  bool enabled;
  Map* map; // Which map are we affecting?

  bool update();
  bool updateFall();
  bool updateEntity();
  bool addSimulation(vec pos);
  bool addFallSimulation(uint8_t block, vec pos, uint32_t EID);
  bool addEntitySimulation(int16_t entity, entity_position pos, uint32_t EID, uint32_t UID);
  bool removeSimulation(vec pos);
  bool checkSurrounding(vec pos);
  bool updateMinecart();

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

  struct Falling
  {
    uint8_t block;
    uint32_t EID;
    vec pos;
    int16_t lastY;
    uint64_t startTime;
    int ticks;
    Falling() {};
    Falling(uint8_t _block, vec _pos, uint32_t _EID)
    {
      block = _block;
      pos = _pos;
      lastY = pos.y();
      EID = _EID;
      ticks = 0;
      startTime = microTime();
    }
  };

  struct simulationEntity
  {
    int16_t entity;
    uint32_t EID;
    uint32_t UID;
    entity_position startPos;
    entity_position pos;
    uint64_t startTime;
    uint64_t lastTime;
    int32_t ticks;
    simulationEntity() {};
    simulationEntity(int16_t _entity, entity_position _pos, uint32_t _EID, uint32_t _UID)
    {
      entity = _entity;
      pos = _pos;
      startPos = _pos;
      EID = _EID;
      UID = _UID;
      ticks = 0;
      startTime = microTime();
      lastTime = startTime;
    }
  };

  std::vector<Sim> simList;

  std::vector<Falling> fallSimList;
  std::vector<simulationEntity> entitySimList;
};

#endif
