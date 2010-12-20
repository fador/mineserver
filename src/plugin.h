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

#ifndef _PLUGIN_H
#define _PLUGIN_H

#include <string>
#include <map>
#include <vector>

//Fix Winsock2 error that occurs when Windows.h is included before it.
#define _WINSOCKAPI_

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "delegate/delegate.hpp"
#include "constants.h"
#include "tools.h"
#include "user.h"
#include "hook.h"

// A wild INCONSISTENCY appears!
//
//  --------------
// | >FIGHT  PkMn |
// |  ITEM   RUN  |
//  --------------
//
// deoxxa uses MACRO ATTACK!
//
#ifdef WIN32
#define LIBRARY_HANDLE HINSTANCE
#define LIBRARY_LOAD(x) LoadLibrary(x)
#define LIBRARY_SYMBOL(x, y) GetProcAddress(x, y)
#define LIBRARY_ERROR() "error"
//GetLastError()
#define LIBRARY_CLOSE(x) FreeLibrary(x)
#else
#define LIBRARY_HANDLE void*
#define LIBRARY_LOAD(x) dlopen(x, RTLD_LAZY)
#define LIBRARY_SYMBOL(x, y) dlsym(x, y)
#define LIBRARY_ERROR() dlerror()
#define LIBRARY_CLOSE(x) dlclose(x)
#endif
//
// It's SUPER EFFECTIVE!
// Foe INCONSISTENCY fainted!
// You got 374¥ for winning!

typedef srutil::delegate6<void, User*, sint8, sint32, sint8, sint32, sint8> Function;

class Callback
{
public:
  void add(const std::string name, Function func)
  {
    remove(name);
    callbacks.insert(std::pair<std::string, Function>(name, func));
  }
   
  bool remove(const std::string name)
  {
    Events::iterator iter = callbacks.find(name);

    if (iter == callbacks.end())
    {
      return false;
    }
      
    callbacks.erase(iter);
    return true;
  }

  Function* get(const std::string name)
  {
    Events::iterator iter = callbacks.find(name);

    if (iter == callbacks.end())
    {
      return NULL;
    }

    return &iter->second;
  }
   
  bool run(const std::string name, const Function::invoker_type function)
  {
    Events::iterator iter = callbacks.find(name);

    if (iter == callbacks.end())
    {
      return false;
    }

    function(iter->second);
    return true;
  }

  void reset()
  {
    callbacks.clear();
  }

private:
  typedef std::map<std::string, Function > Events;
  Events callbacks;
};

class Plugin
{
public:
  // Un/Load external plugins
  bool loadExternal(const std::string name, const std::string file);
  void unloadExternal(const std::string name);
  // Pointer registry stuff, so plugins can be stateful
  bool hasPointer(const std::string name);
  void setPointer(const std::string name, void* pointer);
  void* getPointer(const std::string name);
  void remPointer(const std::string name);

  Hook3<bool,User*,bool*,std::string*> hookLogin;
  Hook4<bool,User*,std::string,std::string,bool*> hookChat;
  Hook4<void,User*,sint32,sint8,sint32> hookDiggingStarted;
  Hook4<void,User*,sint32,sint8,sint32> hookDigging;
  Hook4<void,User*,sint32,sint8,sint32> hookDiggingStopped;
  Hook4<void,User*,sint32,sint8,sint32> hookBlockBroken;
  Hook5<void,User*,sint32,sint8,sint32,int> hookBlockNeighbourBroken;
  Hook5<void,User*,sint32,sint8,sint32,sint16> hookBlockPlace;
  Hook6<void,User*,sint32,sint8,sint32,sint16,sint16> hookBlockReplace;
  Hook4<void,User*,sint32,sint8,sint32> hookBlockNeighbourPlace;

  // Old code
  // This needs to be phased over to the new plugin architecture

  void init();
  void free();

  void setBlockCallback(const int type, Callback call);
  Callback* getBlockCallback(const int type);
  bool runBlockCallback(const int type, const std::string name, const Function::invoker_type function);
  bool removeBlockCallback(const int type);

private:
  std::map<const std::string, LIBRARY_HANDLE> m_libraryHandles;
  std::map<const std::string, void*> m_registry;

  // Old stuff
  typedef std::map<sint16, Callback> Callbacks;
  Callbacks blockevents;
};

#endif
