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
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string>
#include <errno.h>
#include <iostream>
#include <dirent.h>
#include <sys/types.h>

#ifndef PyWrapper
#define PyWrapper

#include "../../plugin_api.h"

class PyLoc
{
public:
  double x, y, z, rot, pit;
};

class PyBlock
{
public:
  int x, y, z, type, meta;
  int get_type();
  void set_type(int new_type);
  int get_meta();
  void set_meta(int new_meta);
};

class PyUser
{
public:
  const char* name;
  PyLoc* location;
  void teleport(PyLoc new_location);
  void set_health(int health);
};

class PyChat
{
public:
  void send_message_to(const char* user, const char* message);
  void send_message(const char* message);
};

class PymyMap
{
public:
  void save();
  void create_item(int x, int y, int z, int type, int count, int health, std::string user);
  bool set_time(int timeValue);
  PyLoc* get_spawn();
  PyBlock* get_block(int x, int y, int z);
};

class PyScreen
{
public:
  void log(const char* message);
};

class PyMineserver
{
public:
  PymyMap map;
  PyScreen screen;
  PyChat chat;
  void setMineServer(mineserver_pointer_struct* MS);
  PyUser* get_user(const char* player_name);
};

mineserver_pointer_struct* magical();



PyMineserver* get_MS();
#ifndef SWIG
#endif

#endif
