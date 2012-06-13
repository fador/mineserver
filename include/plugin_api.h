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

#ifndef _PLUGIN_API_H
#define _PLUGIN_API_H

#ifdef __cplusplus
#ifndef MINESERVER_C_API
#define USE_HOOKS
#endif
#else
#include <stdbool.h>
#endif

#include <stdint.h>

#ifdef USE_HOOKS
#include "hook.h"
#endif

#include "logtype.h"
#include "configure.h"

#ifdef WIN32
#define PLUGIN_API_EXPORT extern "C" __declspec(dllexport)
#define CALLCONVENSION __cdecl
#else
#define PLUGIN_API_EXPORT extern "C"
#define CALLCONVENSION
#endif
#define CALLCONVERSION CALLCONVENSION
#include "constants_num.h"

typedef void (*voidF)(); // voidF is a "void"-like function pointer

struct plugin_pointer_struct
{
  bool (*hasPluginVersion)(const char* name);
  float(*getPluginVersion)(const char* name);
  void (*setPluginVersion)(const char* name, float version);
  void (*remPluginVersion)(const char* name);

  bool (*hasPointer)(const char* name);
  void*(*getPointer)(const char* name);
  void (*setPointer)(const char* name, void* pointer);
  void (*remPointer)(const char* name);

  bool (*hasHook)(const char* hookID);
#ifdef USE_HOOKS
  Hook*(*getHook)(const char* hookID);
  void (*setHook)(const char* hookID, Hook* hook);
#else
  void*(*getHook)(const char* hookID);
  void (*setHook)(const char* hookID, void* hook);
#endif
  void (*remHook)(const char* hookID);

  bool (*hasCallback)(const char* hookID, voidF function);
  void (*addCallback)(const char* hookID, voidF function);
  void (*addIdentifiedCallback)(const char* hookID, void* identifier, voidF function);
  void (*remCallback)(const char* hookID, voidF function);
  bool (*doUntilTrue)(const char* hookID, ...);
  bool (*doUntilFalse)(const char* hookID, ...);
  void (*doAll)(const char* hookID, ...);

  void* temp[10];
};

struct user_pointer_struct
{
  bool (*teleport)(const char* user, double x, double y, double z);
  bool (*toggleDND)(const char* user);
  bool (*getPosition)(const char* user, double* x, double* y, double* z, float* yaw, float* pitch, double* stance);
  bool (*sethealth)(const char* user, int userHealth);
  bool (*teleportMap)(const char* user, double x, double y, double z, size_t map);
  int (*getCount)();
  const char* (*getUserNumbered)(int c);
  bool (*getPositionW)(const char* user, double* x, double* y, double* z, int* w, float* yaw, float* pitch, double* stance);
  bool (*addItem)(const char* user, int item, int count, int health);
  bool (*hasItem)(const char* user, int item, int count, int health);
  bool (*delItem)(const char* user, int item, int count, int health);
  int (*gethealth)(const char* user);
  bool (*getItemInHand)(const char* user, int* type, int* meta, int* quant);
  bool (*setItemInHand)(const char* user, int type, int meta, int quant);
  bool (*kick)(const char* user);
  bool (*getItemAt)(const char* user, int slot, int* type, int* meta, int* quant);
  bool (*setItemAt)(const char* user, int slot, int type, int meta, int quant);

  void* temp[96];
};

struct chat_pointer_struct
{
  bool (*sendmsgTo)(const char* user, const char* msg);
  bool (*sendmsg)(const char* msg);
  bool (*sendUserlist)(const char* user);
  bool (*handleMessage)(const char* user, const char* msg);
  void* temp[100];
};

struct logger_pointer_struct
{
  void (*log)(int type, const char* source, const char* message);
  void* temp[100];
};

struct map_pointer_struct
{
  void (*createPickupSpawn)(int x, int y, int z, int type, int count, int health, const char* user);
  bool (*setTime)(int timeValue);
  int (*getTime)();
  void (*getSpawn)(int* x, int* y, int* z);
  void (*setSpawn)(int x, int y, int z);
  bool (*getBlock)(int x, int y, int z, unsigned char* type, unsigned char* meta);
  bool (*setBlock)(int x, int y, int z, unsigned char type, unsigned char meta);
  void (*saveWholeMap)(void);
  unsigned char*(*getMapData_block)(int x, int z);
  unsigned char*(*getMapData_meta)(int x, int z);
  unsigned char*(*getMapData_skylight)(int x, int z);
  unsigned char*(*getMapData_blocklight)(int x, int z);
  bool (*getBlockW)(int x, int y, int z, int w, unsigned char* type, unsigned char* meta);
  bool (*setBlockW)(int x, int y, int z, int w, unsigned char type, unsigned char meta);
  void* temp[100];
};

struct config_pointer_struct
{
  bool (*has)(const char* name);
  int (*iData)(const char* name);
  int64_t (*lData)(const char* name);
  float(*fData)(const char* name);
  double(*dData)(const char* name);
  const char*(*sData)(const char* name);
  bool (*bData)(const char* name);
  void* temp[100];
};

struct mob_pointer_struct
{
  int (*createMob)(int type);
  int (*createSpawnMob)(int type);
  void (*spawnMob)(int uid);
  void (*despawnMob)(int uid);
  void (*moveMob)(int uid, double x, double y, double z);
  void (*moveMobW)(int uid, double x, double y, double z, int map);
  bool (*getMobPositionW)(int uid, double* x, double* y, double* z, int* w);
  int (*getMobID)(int uid);
  int (*getHealth)(int uid);
  void (*setHealth)(int uid, int mobHealth);
  int (*getType)(size_t uid);
  bool (*getLook)(int uid, double* yaw, double* pitch, double *head_yaw);
  bool (*setLook)(int uid, double yaw, double pitch, double head_yaw);
  void (*moveAnimal)(const char* userIn, size_t mobID);
  void (*animateMob)(const char* userIn, size_t mobID, int animID);
  void (*animateDamage)(const char* userIn, size_t mobID, int animID);
  bool (*setByteMetadata)(int uid, int8_t idx, int8_t byte);
  bool (*updateMetadata)(int uid);
  int8_t (*getByteMetadata)(int uid, int idx);
  void* temp[97];

};

struct permission_pointer_struct
{
  bool (*setAdmin)(const char* name);
  bool (*setOp)(const char* name);
  bool (*setMember)(const char* name);
  bool (*setGuest)(const char* name);
  bool (*isAdmin)(const char* name);
  bool (*isOp)(const char* name);
  bool (*isMember)(const char* name);
  bool (*isGuest)(const char* name);
  void* temp[100];

};

struct tools_pointer_struct
{
  int (*uniformInt)(int a, int b);
  double (*uniform01)();
};

struct mineserver_pointer_struct
{
  struct map_pointer_struct map;
  struct logger_pointer_struct logger;
  struct chat_pointer_struct chat;
  struct plugin_pointer_struct plugin;
  struct user_pointer_struct user;
  struct config_pointer_struct config;
  struct mob_pointer_struct mob;
  struct permission_pointer_struct permissions;
  struct tools_pointer_struct tools;
  void* temp[99];
};

// Ignore these, only used when compiling with Mineserver
#ifdef MINESERVER
void init_plugin_api(void);
extern mineserver_pointer_struct plugin_api_pointers;
#endif

#endif
