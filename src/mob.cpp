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
#include "protocol.h"
#include <algorithm>

Mob::Mob()
  :
  UID(Mineserver::generateEID()),
  type(0),
  x(0),
  y(0),
  z(0),
  map(0),
  yaw(0),
  pitch(0),
  head_yaw(0), // TODO: actually use head_yaw
  spawned(false),
  respawnable(false),
  health(0)
{
}

//Can be 0 (no animation), 1 (swing arm), 2 (damage animation)
//, 3 (leave bed), 104 (crouch), or 105 (uncrouch). Getting 102 somewhat often, too. 
void Mob::animateMob(int animID)
{
  User::sendAll(Protocol::armAnimation(UID, animID));
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
	if (health == 0)
	{
		animateDamage(ANIMATE_DEAD);
//		deSpawnToAll();
	}
	else if (health < this->health)
  {
		animateDamage(ANIMATE_HURT);
		animateMob(ANIMATE_DAMAGE);
  }
  this->health = health;
}
//Possible values: 2 (entity hurt), 3 (entity dead?), 4, 5
void Mob::animateDamage(int animID)
{
  User::sendAll(Protocol::deathAnimation(UID, animID));
}

void Mob::updateMetadata()
{
  User::sendAll(Protocol::entityMetadata(UID, metadata));
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
  for (int i = 0; i < ServerInstance->users().size(); i++) {
    User* user2 = ServerInstance->users()[i];
  user2->buffer << (int8_t)PACKET_ENTITY_VELOCITY << (int32_t)UID << (int16_t)vel.x() << (int16_t)vel.y() << (int16_t)vel.z();
  }*/
}

void Mob::spawnToAll()
{
  User::sendAll(Protocol::mobSpawn(*this));
  spawned = true;
}

void Mob::deSpawnToAll()
{
  User::sendAll(Protocol::destroyEntity(UID));
  spawned = false;
}

void Mob::relativeMoveToAll()
{

}

void Mob::teleportToAll()
{
  if (spawned)
  {
    User::sendAll(Protocol::entityTeleport(UID, x, y, z, yaw, pitch));
  }
}

void Mob::moveTo(double to_x, double to_y, double to_z, int to_map)
{
  double dx = to_x - x,
         dy = to_y - y,
         dz = to_z - z;

  x = to_x;
  y = to_y;
  z = to_z;
  if (to_map != -1)
  {
    map = to_map;
  }
  if(dx <= 4 && dy <= 4 && dz <= 4)
  {
    User::sendAll(Protocol::entityRelativeMove(UID, dx, dy, dz));
  }
  else
  {
    teleportToAll();
  }
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
  if(y_byte != this->yaw || p_byte != this->pitch)
  {
    User::sendAll(Protocol::entityLook(UID, yaw, pitch));
  }
  this->pitch = p_byte;
  this->yaw = y_byte;
}

void Mob::headLook(int16_t head_yaw)
{
  while(head_yaw < 0) {
    head_yaw += 360;
  }
  head_yaw = head_yaw % 360;
  int8_t h_byte = (int8_t)((head_yaw * 1.0) / 360.0 * 256.0);
  if(h_byte != this->head_yaw)
  {
    User::sendAll(Protocol::entityHeadLook(UID, head_yaw));
  }
  this->head_yaw = h_byte;
}
