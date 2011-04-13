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

#ifndef _PLUGIN_H
#define _PLUGIN_H

#include <string>
#include <tr1/unordered_map>
#include <vector>
#include <ctime>

//Fix Winsock2 error that occurs when Windows.h is included before it.
#define _WINSOCKAPI_

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

//#include "constants.h"
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
#define LIBRARY_SELF() GetModuleHandle(NULL)
#define LIBRARY_SYMBOL(x, y) GetProcAddress(x, y)
#define LIBRARY_ERROR() "Windows error handling needs work!" // <- NOTE
#define LIBRARY_CLOSE(x) FreeLibrary(x)
#define LIBRARY_EXTENSION ".dll"
#else
#define LIBRARY_HANDLE void*
#define LIBRARY_LOAD(x) dlopen(x, RTLD_LAZY)
#define LIBRARY_SELF() dlopen(NULL, RTLD_LAZY)
#define LIBRARY_SYMBOL(x, y) dlsym(x, y)
#define LIBRARY_ERROR() dlerror()
#define LIBRARY_CLOSE(x) dlclose(x)
#define LIBRARY_EXTENSION ".so"
#endif
#include "blocks/basic.h"
#include "items/itembasic.h"
//
// It's SUPER EFFECTIVE!
// Foe INCONSISTENCY fainted!
// You got 374¥ for winning!

class Plugin
{
public:

  typedef std::tr1::unordered_map<std::string, Hook*> HookMap;
  typedef std::tr1::unordered_map<std::string, LIBRARY_HANDLE> LibHandleMap;
  typedef std::tr1::unordered_map<std::string, void*> PointerMap;
  typedef std::tr1::unordered_map<std::string, float> VersionMap;

  // Hook registry stuff
  inline bool  hasHook(const HookMap::key_type& name) const { return m_hooks.count(name) > 0; }
  inline HookMap::mapped_type getHook(const HookMap::key_type& name) const
  {
    HookMap::const_iterator hook = m_hooks.find(name);
    return hook == m_hooks.end() ? NULL : hook->second;
  }
  inline void  setHook(const HookMap::key_type& name, HookMap::mapped_type hook) { m_hooks[name] = hook; }
  inline void  remHook(const HookMap::key_type& name) { m_hooks.erase(name); /* erases 0 or 1 elements */ }

  // Load/Unload plugins
  bool loadPlugin(const std::string& name, const std::string& path = "", std::string alias = "");
  void unloadPlugin(const std::string name);

  // Plugin version registry
  inline bool  hasPluginVersion(const VersionMap::key_type& name) const { return m_pluginVersions.find(name) != m_pluginVersions.end(); }
  inline float getPluginVersion(const VersionMap::key_type& name) const
  {
    VersionMap::const_iterator pluginVersion = m_pluginVersions.find(name);
    return pluginVersion == m_pluginVersions.end() ? 0 : pluginVersion->second;
  }
  inline void  setPluginVersion(const VersionMap::key_type& name, VersionMap::mapped_type version) { m_pluginVersions[name] = version; }
  inline void  remPluginVersion(const VersionMap::key_type& name) { m_pluginVersions.erase(name); }

  // Pointer registry stuff
  inline bool  hasPointer(const PointerMap::key_type& name) const { return m_pointers.find(name) != m_pointers.end(); }
  inline void* getPointer(const PointerMap::key_type& name) const
  {
    PointerMap::const_iterator pointer = m_pointers.find(name);
    return pointer == m_pointers.end() ? NULL : pointer->second;
  }
  inline void  setPointer(const PointerMap::key_type& name, PointerMap::mapped_type pointer) {m_pointers[name] = pointer; }
  inline void  remPointer(const PointerMap::key_type& name) { m_pointers.erase(name); }

  // Create default hooks
  Plugin()
  {
    setHook("Timer200", new Hook0<bool>);
    setHook("Timer1000", new Hook0<bool>);
    setHook("Timer10000", new Hook0<bool>);
    setHook("PlayerLoginPre", new Hook2<bool, const char*, char***>);
    setHook("PlayerLoginPost", new Hook1<bool, const char*>);
    setHook("PlayerNickPost", new Hook2<bool, const char*, const char*>);
    setHook("PlayerKickPost", new Hook2<bool, const char*, const char*>);
    setHook("PlayerQuitPost", new Hook1<bool, const char*>);
    setHook("PlayerChatPre", new Hook3<bool, const char*, time_t, const char*>);
    setHook("PlayerChatPost", new Hook3<bool, const char*, time_t, const char*>);
    setHook("PlayerArmSwing", new Hook1<bool, const char*>);
    setHook("PlayerDamagePre", new Hook3<bool, const char*, const char*, int>);
    setHook("PlayerDamagePost", new Hook3<bool, const char*, const char*, int>);
    setHook("PlayerDisconnect", new Hook3<bool, const char*, uint32_t, uint16_t>);
    setHook("PlayerDiggingStarted", new Hook5<bool, const char*, int32_t, int8_t, int32_t, int8_t>);
    setHook("PlayerDigging", new Hook5<bool, const char*, int32_t, int8_t, int32_t, int8_t>);
    setHook("PlayerDiggingStopped", new Hook5<bool, const char*, int32_t, int8_t, int32_t, int8_t>);
    setHook("PlayerBlockInteract", new Hook4<bool, const char*, int32_t, int8_t, int32_t>);
    setHook("BlockBreakPre", new Hook4<bool, const char*, int32_t, int8_t, int32_t>);
    setHook("BlockBreakPost", new Hook4<bool, const char*, int32_t, int8_t, int32_t>);
    setHook("BlockNeighbourBreak", new Hook7<bool, const char*, int32_t, int8_t, int32_t, int32_t, int8_t, int32_t>);
    setHook("BlockPlacePre", new Hook6<bool, const char*, int32_t, int8_t, int32_t, int16_t, int8_t>);
    setHook("ItemRightClickPre", new Hook6<bool, const char*, int32_t, int8_t, int32_t, int16_t, int8_t>);
    setHook("BlockPlacePost", new Hook6<bool, const char*, int32_t, int8_t, int32_t, int16_t, int8_t>);
    setHook("BlockNeighbourPlace", new Hook7<bool, const char*, int32_t, int8_t, int32_t, int32_t, int8_t, int32_t>);
    setHook("BlockReplacePre", new Hook6<bool, const char*, int32_t, int8_t, int32_t, int16_t, int16_t>);
    setHook("BlockReplacePost", new Hook6<bool, const char*, int32_t, int8_t, int32_t, int16_t, int16_t>);
    setHook("BlockNeighbourReplace", new Hook9<bool, const char*, int32_t, int8_t, int32_t, int32_t, int8_t, int32_t, int16_t, int16_t>);
    setHook("LogPost", new Hook3<bool, int, const char*, const char*>);
    setHook("PlayerChatCommand", new Hook4<bool, const char*, const char*, int, const char**>);
    setHook("PlayerRespawn", new Hook1<bool, const char*>);
    setHook("gotAttacked", new Hook2<bool, const char*, int32_t>);

    init();
  }
  // Remove existing hooks
  ~Plugin()
  {
    for (HookMap::iterator it = m_hooks.begin(); it != m_hooks.end(); ++it)
    {
      delete it->second;
    }

    m_hooks.clear();

    free();
  }

  void init();

  void free();

  inline const std::vector<BlockBasic*> & getBlockCB() const { return BlockCB; }

  inline const std::vector<ItemBasic*> & getItemCB() const { return ItemCB; }

private:
  HookMap      m_hooks;
  LibHandleMap m_libraryHandles;
  PointerMap   m_pointers;
  VersionMap   m_pluginVersions;

  std::vector<BlockBasic*> BlockCB;
  std::vector<ItemBasic*> ItemCB;
};

#endif
