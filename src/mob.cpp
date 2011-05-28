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

#include "mob.h"
#include <algorithm>

Mob::Mob()
  :
  UID(Mineserver::generateEID()),
  x(0),
  y(0),
  z(0),
  map(0),
  yaw(0),
  pitch(0),
  meta(0),
  spawned(false)
{
}

//Can be 0 (no animation), 1 (swing arm), 2 (damage animation)
//, 3 (leave bed), 104 (crouch), or 105 (uncrouch). Getting 102 somewhat often, too. 
void Mob::animateMob(int animID)
{ 
  for (std::set<User*>::iterator it = Mineserver::get()->users().begin(); it != Mineserver::get()->users().end(); ++it)
  {
    (*it)->buffer << (int8_t)PACKET_ARM_ANIMATION << (int32_t)UID << (int8_t)animID;
  }
}

void Mob::sethealth(int health)
{
  if (health < 0)
  {
    health = 0;
  }
  if (health > 30)
  {
    health = 30;
  }
  if (health < this->health)
  {
    for (std::set<User*>::iterator it = Mineserver::get()->users().begin(); it != Mineserver::get()->users().end(); ++it)
    {
      (*it)->buffer << (int8_t)PACKET_ARM_ANIMATION << (int32_t)UID << (int8_t)2;
      // Hurt animation
    }
  }
  this->health = health;
  if (this->health <= 0)
  {
    deSpawnToAll();
  }
}
//Possible values: 2 (entity hurt), 3 (entity dead?), 4, 5
void Mob::animateDamage(int animID)
{ 
  for (std::set<User*>::iterator it = Mineserver::get()->users().begin(); it != Mineserver::get()->users().end(); ++it)
  {
    (*it)->buffer << (int8_t)PACKET_DEATH_ANIMATION << (int32_t)UID << (int8_t)animID;
  }
}

void Mob::moveAnimal()
{
  /*std::string user2(userIn);
  User* user = User::byNick(user2);
  float tempMult = 1.f - abs(user->pos.pitch / 25.f);
    vec pos = vec(user->pos.x * 32, (user->pos.y + 1.5) * 32, user->pos.z * 32);
    vec vel = vec(
      sinf(-(user->pos.yaw / 360.f) * 2.f * M_PI) * tempMult * 32768.f,
      -(user->pos.pitch / 90.f) * 32768.f,
      cosf(-(user->pos.yaw / 360.f) * 2.f * M_PI) * tempMult * 32768.f
      );
    x=vel.x(); y=vel.y();z=vel.z();
  x += vel.x()*0.01;
  y += vel.y()*0.01;  
  z += vel.z()*0.01;  
  for (int i = 0; i < Mineserver::get()->users().size(); i++) {
    User* user2 = Mineserver::get()->users()[i];
  user2->buffer << (int8_t)PACKET_ENTITY_VELOCITY << (int32_t)UID << (int16_t)vel.x() << (int16_t)vel.y() << (int16_t)vel.z();
  }*/
}

void Mob::spawnToAll()
{
  if (type == MOB_PIG)
  {
    health = 10;
  }
  if (type == MOB_SHEEP)
  {
    health = 10;
  }
  if (type == MOB_COW)
  {
    health = 10;
  }
  if (type == MOB_CHICKEN)
  {
    health = 4;
  }
  if (type == MOB_SQUID)
  {
    health = 10;
  }

  for (std::set<User*>::iterator it = Mineserver::get()->users().begin(); it != Mineserver::get()->users().end(); ++it)
  {
    if ((*it)->logged)
    {
      (*it)->buffer << (int8_t)PACKET_MOB_SPAWN << (int32_t) UID << (int8_t) type
                   << (int32_t)(x * 32.0) << (int32_t)(y * 32.0) << (int32_t)(z * 32.0) << (int8_t) yaw
                   << (int8_t) pitch;
      if (type == MOB_SHEEP)
      {
        (*it)->buffer << (int8_t) 0 << (int8_t) meta << (int8_t) 127;
      }
      else
      {
        (*it)->buffer << (int8_t) 127;
      }
    }
    spawned = true;
  }
}

void Mob::deSpawnToAll()
{
  for (std::set<User*>::iterator it = Mineserver::get()->users().begin(); it != Mineserver::get()->users().end(); ++it)
  {
    if ((*it)->logged)
    {
      (*it)->buffer << PACKET_DESTROY_ENTITY << (int32_t) UID;
    }
  }
  spawned = false;
}

void Mob::relativeMoveToAll()
{

}

void Mob::teleportToAll()
{
  if (!spawned)
  {
    return;
  }

  for (std::set<User*>::iterator it = Mineserver::get()->users().begin(); it != Mineserver::get()->users().end(); ++it)
  {
    if ((*it)->logged)
    {
      (*it)->buffer << PACKET_ENTITY_TELEPORT << (int32_t) UID
                   << (int32_t)(x * 32.0) << (int32_t)(y * 32.0) << (int32_t)(z * 32.0)
                   << (int8_t) yaw << (int8_t) pitch;
    }
  }
}

void Mob::moveTo(double to_x, double to_y, double to_z, int to_map)
{
  //  int distx = abs(x-to_x);
  //  int disty = abs(y-to_y);
  //  int distz = abs(z-to_z);
  x = to_x;
  y = to_y;
  z = to_z;
  if (to_map != -1)
  {
    map = to_map;
  }
  //  if(distx < 4 && disty < 4 && distz < 4)
  //  {
  //    // Work out how to use the relative move?
  //    teleportToAll();
  //  }
  //  else
  //  {
  teleportToAll();
  //  }
}

void Mob::look(int16_t yaw, int16_t pitch)
{
  // Yaw and Pitch need to be between 0 and 360
  while (yaw < 0)
  {
    yaw += 360;
  }
  while (pitch < 0)
  {
    pitch += 360;
  }
  yaw = yaw % 360;
  pitch = pitch % 360;
  int8_t y_byte = (int8_t)((yaw * 1.0) / 360.0 * 256.0);
  int8_t p_byte = (int8_t)((pitch * 1.0) / 360.0 * 256.0);
  this->pitch = p_byte;
  this->yaw = y_byte;
  Packet pkt;
  pkt << PACKET_ENTITY_LOOK << (int32_t) UID << (int8_t) y_byte << (int8_t) p_byte;
  if (!User::all().empty())
  {
    (*User::all().begin())->sendAll((uint8_t*)pkt.getWrite(), pkt.getWriteLen());
  }
}

int Mobs::mobNametoType(std::string name)
{
  std::transform(name.begin(), name.end(), name.begin(), ::toupper);
  if (name == "CREEPER")
  {
    return MOB_CREEPER;
  }
  if (name == "SKELETON")
  {
    return MOB_SKELETON;
  }
  if (name == "SPIDER")
  {
    return MOB_SPIDER;
  }
  if (name == "GIANTZOMBIE")
  {
    return MOB_GIANT_ZOMBIE;
  }
  if (name == "GIANT")
  {
    return MOB_GIANT_ZOMBIE;
  }
  if (name == "ZOMBIE")
  {
    return MOB_ZOMBIE;
  }
  if (name == "SLIME")
  {
    return MOB_SLIME;
  }
  if (name == "GHAST")
  {
    return MOB_GHAST;
  }
  if (name == "ZOMBIEPIGMAN")
  {
    return MOB_ZOMBIE_PIGMAN;
  }
  if (name == "PIGMAN")
  {
    return MOB_ZOMBIE_PIGMAN;
  }
  if (name == "PIG")
  {
    return MOB_PIG;
  }
  if (name == "SHEEP")
  {
    return MOB_SHEEP;
  }
  if (name == "COW")
  {
    return MOB_COW;
  }
  if (name == "CHICKEN")
  {
    return MOB_CHICKEN;
  }
  return -1;
}
