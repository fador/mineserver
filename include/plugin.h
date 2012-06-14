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
#include <vector>
#include <ctime>

#include "tr1.h"
#include TR1INCLUDE(memory)
#include TR1INCLUDE(unordered_map)

#include "hook.h"

//Fix Winsock2 error that occurs when Windows.h is included before it.
#define _WINSOCKAPI_

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <dlfcn.h>
#endif


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
static char *LIBRARY_ERROR(void)
{
  char errbuf[513];
  DWORD err = GetLastError();
  if(!err)
    return NULL;
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, 0, errbuf, 512, NULL);
  SetLastError(0);
  return errbuf;
}
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

typedef std::tr1::shared_ptr<BlockBasic> BlockBasicPtr;
typedef std::tr1::shared_ptr<ItemBasic>  ItemBasicPtr;

class Plugin
{
public:

  typedef std::tr1::unordered_map<std::string, Hook*> HookMap;
  typedef std::tr1::unordered_map<std::string, LIBRARY_HANDLE> LibHandleMap;
  typedef std::tr1::unordered_map<std::string, void*> PointerMap;
  typedef std::tr1::unordered_map<std::string, float> VersionMap;

  typedef std::vector<BlockBasicPtr> BlockCBs;
  typedef std::vector<ItemBasicPtr>  ItemCBs;

   Plugin();
  ~Plugin();

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

  void init();

  inline const BlockCBs & getBlockCB() const { return m_block_CBs; }

  inline const ItemCBs  & getItemCB()  const { return m_item_CBs; }

private:
  HookMap      m_hooks;
  LibHandleMap m_libraryHandles;
  PointerMap   m_pointers;
  VersionMap   m_pluginVersions;

  BlockCBs     m_block_CBs;
  ItemCBs      m_item_CBs;
};

#endif
