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

#ifndef _PLUGIN_API_H
#define _PLUGIN_API_H

#include <string>
#include <map>
#include <vector>
#include <ctime>

#ifdef WIN32
#define PLUGIN_API_EXPORT extern "C" __declspec(dllexport) 
#define CALLCONVERSION __cdecl
#else
#define PLUGIN_API_EXPORT extern "C"
#define CALLCONVERSION
#endif

struct plugin_pointer_struct
{
  float (*getPluginVersion)(const char* name,size_t nameLen);
  void (*setPluginVersion)(const char* name,size_t nameLen, float version);
  void *temp[10];
};

struct user_pointer_struct
{
  bool (*teleport)(const char* user,size_t userLen,double x, double y, double z);
  bool (*getPosition)(const char* user,size_t userLen, double* x, double* y, double* z, float* yaw, float* pitch, double *stance);
  void *temp[100];
};

struct chat_pointer_struct
{
  bool (*sendmsgTo)(const char* user,size_t userLen,const char* msg,size_t msgLen);
  bool   (*sendmsg)(const char* msg,size_t msgLen);
  void *temp[100];
};

struct screen_pointer_struct
{
  void (*log)(const char* msg,size_t msgLen);
  void *temp[100];
};


struct map_pointer_struct
{
  void (*createPickupSpawn)(int x, int y, int z, int type, int count, int health, const char* user,size_t userLen);
  bool (*setTime)(int timeValue);
  void (*getSpawn)(int* x, int* y, int* z);
  bool (*getBlock)(int x, int y, int z, unsigned char* type,unsigned char* meta);
  bool (*setBlock)(int x, int y, int z, unsigned char type,unsigned char meta);
  void *temp[100];
};

struct callback_pointer_struct
{
  bool (*add_hook)(const char* name,size_t nameLen, void *function);
  void *temp[100];
};

struct mineserver_pointer_struct
{
  map_pointer_struct map;
  screen_pointer_struct screen;
  chat_pointer_struct chat;
  plugin_pointer_struct plugin;
  user_pointer_struct user;
  callback_pointer_struct callback;

  void *temp[100];
};

//Ignore these, only used when compiling with Mineserver
#ifdef MINESERVER
void init_plugin_api(void);
extern mineserver_pointer_struct plugin_api_pointers;
#endif


#endif
