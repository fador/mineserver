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

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string>
#include <errno.h>
#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include "PyScript.h"
#include "python_plugin_api.h"

PyMineserver* pymines = new PyMineserver();

PyMineserver* get_MS()
{
  return pymines;
}

int PyBlock::get_type()
{
  return type;
}

void PyBlock::set_type(int new_type)
{
  getMineServer()->map.setBlock(x, y, z, new_type, meta);
}

int PyBlock::get_meta()
{
  return meta;
}

void PyBlock::set_meta(int new_meta)
{
  getMineServer()->map.setBlock(x, y, z, type, new_meta);
}

void PyUser::teleport(PyLoc new_location)
{
  getMineServer()->user.teleport(name, new_location.x, new_location.y, new_location.z);
}

void PyUser::set_health(int health)
{
  getMineServer()->user.sethealth(name, health);
}

void PyChat::send_message_to(const char* user, const char* message)
{
  getMineServer()->chat.sendmsgTo(user, message);
}

void PyChat::send_message(const char* message)
{
  getMineServer()->chat.sendmsg(message);
}

void PymyMap::create_item(int x, int y, int z, int type, int count, int health, std::string user)
{
  getMineServer()->map.createPickupSpawn(x, y, z, type, count, health, user.c_str());
}
mineserver_pointer_struct* magical()
{
  return getMineServer();
}
bool PymyMap::set_time(int timeValue)
{
  return ::ms->map.setTime(timeValue);
}
PyLoc* PymyMap::get_spawn()
{
  PyLoc* L = new PyLoc();
  int* x, *y, *z;
  double dx, dy, dz;
  getMineServer()->map.getSpawn(x, y, z);
  dx = (double) * x;
  dy = (double) * y;
  dz = (double) * z;
  L->x = dx;
  L->y = dy;
  L->z = dz;
  return L;
}
void PymyMap::save()
{
  getMineServer()->map.saveWholeMap();
}
PyBlock* PymyMap::get_block(int x, int y, int z)
{
  unsigned char type, meta;
  getMineServer()->map.getBlock(x, y, z, &type, &meta);
  PyBlock* block = new PyBlock();
  block->x = x;
  block->y = y;
  block->z = z;
  block->type = (int)type;
  block->meta = (int)meta;
  return block;
}

void PyScreen::log(const char* message)
{
  std::string m2(message);
  getMineServer()->logger.log(6, "Plugin", m2.c_str());
}

PyUser* PyMineserver::get_user(const char* player_name)
{
  PyLoc* loc = new PyLoc();
  double stance;
  if (getMineServer()->user.getPosition(
        player_name, &loc->x, &loc->y, &loc->z,
        (float*)&loc->rot, (float*)&loc->pit, &stance))
  {
    PyUser* user = new PyUser();
    user->location = loc;
    user->name = player_name;
    return user;
  }
  return NULL;
}

void PyMineserver::setMineServer(mineserver_pointer_struct* MS)
{
  ::ms = MS;
}
