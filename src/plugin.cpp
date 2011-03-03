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

#include "sys/stat.h"

#include "mineserver.h"
#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "constants.h"
#include "logger.h"

#include "plugin.h"
#include "blocks/default.h"
#include "blocks/falling.h"
#include "blocks/torch.h"
#include "blocks/plant.h"
#include "blocks/snow.h"
#include "blocks/liquid.h"
#include "blocks/fire.h"
#include "blocks/stair.h"
#include "blocks/door.h"
#include "blocks/sign.h"
#include "blocks/tracks.h"
#include "blocks/chest.h"
#include "blocks/ladder.h"
#include "blocks/cake.h"
#include "blocks/note.h"
#include "blocks/blockfurnace.h"
#include "blocks/workbench.h"
#include "blocks/dyed.h"
#include "blocks/redstone.h"

#include "items/food.h"
#include "items/projectile.h"

void Plugin::init()
{
  // Create Block* objects and put them away so we can delete them later
  BlockRedstone* redstoneblock = new BlockRedstone();
  BlockCB.push_back(redstoneblock);
  BlockDyed* dyedblock = new BlockDyed();
  BlockCB.push_back(dyedblock);
  BlockFalling* fallingblock = new BlockFalling();
  BlockCB.push_back(fallingblock);
  BlockTorch* torchblock = new BlockTorch();
  BlockCB.push_back(torchblock);
  BlockPlant* plantblock = new BlockPlant();
  BlockCB.push_back(plantblock);
  BlockSnow* snowblock = new BlockSnow();
  BlockCB.push_back(snowblock);
  BlockLiquid* liquidblock = new BlockLiquid();
  BlockCB.push_back(liquidblock);
  BlockFire* fireblock = new BlockFire();
  BlockCB.push_back(fireblock);
  BlockStair* stairblock = new BlockStair();
  BlockCB.push_back(stairblock);
  BlockChest* chestblock = new BlockChest();
  BlockCB.push_back(chestblock);
  BlockDoor* doorblock = new BlockDoor();
  BlockCB.push_back(doorblock);
  BlockSign* signblock = new BlockSign();
  BlockCB.push_back(signblock);
  BlockTracks* tracksblock = new BlockTracks();
  BlockCB.push_back(tracksblock);
  BlockLadder* ladderblock = new BlockLadder();
  BlockCB.push_back(ladderblock);
  BlockCake* cakeblock = new BlockCake();
  BlockCB.push_back(cakeblock);
  BlockNote* noteblock = new BlockNote();
  BlockCB.push_back(noteblock);
  BlockFurnace* furnaceblock = new BlockFurnace();
  BlockCB.push_back(furnaceblock);
  BlockWorkbench* workbenchblock = new BlockWorkbench();
  BlockCB.push_back(workbenchblock);
  BlockDefault* defaultblock = new BlockDefault();
  BlockCB.push_back(defaultblock);
  

  ItemFood* fooditem = new ItemFood();
  ItemCB.push_back(fooditem);
  ItemProjectile* projectileitem = new ItemProjectile();
  ItemCB.push_back(projectileitem);

}

void Plugin::free()
{
  std::vector<BlockBasic*>::iterator it = BlockCB.begin();
  for (; it != BlockCB.end(); ++it)
  {
    delete *it;
  }
}

bool Plugin::loadPlugin(const std::string name, const std::string file)
{
  LIBRARY_HANDLE lhandle = NULL;
  void (*fhandle)(mineserver_pointer_struct*) = NULL;

  if (!file.empty())
  {
    LOG(INFO, "Plugin", "Loading plugin `" + name + "' (`" + file + "')...");

    struct stat st;
    int statr = stat(file.c_str(), &st);
    if ((statr == 0) && !(st.st_mode & S_IFDIR))
    {
      lhandle = LIBRARY_LOAD(file.c_str());
    }
    else
    {
      LOG(INFO, "Plugin", "Could not find `" + file + "', trying `" + file + LIBRARY_EXTENSION + "'.");

      statr = stat((file + LIBRARY_EXTENSION).c_str(), &st);
      if ((statr == 0) && !(st.st_mode & S_IFDIR))
      {
        lhandle = LIBRARY_LOAD((file + LIBRARY_EXTENSION).c_str());
      }
      else
      {
        LOG(INFO, "Plugin", "Could not find `" + file + LIBRARY_EXTENSION + "'!");
        return false;
      }
    }

  }
  else
  {
    LOG(INFO, "Plugin", "Loading plugin `" + name + "' (built in)...");
    lhandle = LIBRARY_SELF();
  }

  if (lhandle == NULL)
  {
    LOG(INFO, "Plugin", "Could not load plugin `" + name + "'!");
    LOG(INFO, "Plugin", LIBRARY_ERROR());
    return false;
  }

  m_libraryHandles[name] = lhandle;

  fhandle = (void (*)(mineserver_pointer_struct*)) LIBRARY_SYMBOL(lhandle, (name + "_init").c_str());
  if (fhandle == NULL)
  {
    LOG(INFO, "Plugin", "Could not get init function handle!");
    unloadPlugin(name);
    return false;
  }
  fhandle(&plugin_api_pointers);

  return true;
}

void Plugin::unloadPlugin(const std::string name)
{
  LIBRARY_HANDLE lhandle = NULL;
  void (*fhandle)(void) = NULL;

  if (m_pluginVersions.find(name) != m_pluginVersions.end())
  {
    LOG(INFO, "Plugin", "Unloading plugin `" + name + "'...");

    if (m_libraryHandles[name] != NULL)
    {
      lhandle = m_libraryHandles[name];
      m_libraryHandles.erase(name);
    }
    else
    {
      lhandle = LIBRARY_SELF();
    }

    fhandle = (void (*)(void)) LIBRARY_SYMBOL(lhandle, (name + "_shutdown").c_str());
    if (fhandle == NULL)
    {
      LOG(INFO, "Plugin", "Could not get shutdown function handle!");
    }
    else
    {
      LOG(INFO, "Plugin", "Calling shutdown function for `" + name + "'.");
      fhandle();
    }

    LIBRARY_CLOSE(m_libraryHandles[name]);
  }
  else
  {
    LOG(INFO, "Plugin", "Plugin `" + name + "' not loaded!");
  }
}

bool Plugin::hasHook(const std::string& name) const
{
  return m_hooks.find(name) != m_hooks.end();
}

Hook* Plugin::getHook(const std::string& name) const
{
  std::map<const std::string, Hook*>::const_iterator hook = m_hooks.find(name);

  if (hook == m_hooks.end())
  {
    return NULL;
  }

  return hook->second;
}

void Plugin::setHook(const std::string& name, Hook* hook)
{
  m_hooks[name] = hook;
}

void Plugin::remHook(const std::string& name)
{
  if (hasHook(name))
  {
    m_hooks.erase(name);
  }
}

bool Plugin::hasPluginVersion(const std::string& name) const
{
  return m_pluginVersions.find(name) != m_pluginVersions.end();
}

float Plugin::getPluginVersion(const std::string& name) const
{
  std::map<const std::string, float>::const_iterator pluginVersion = m_pluginVersions.find(name);

  if (pluginVersion == m_pluginVersions.end())
  {
    return 0.0f;
  }

  return pluginVersion->second;
}

void Plugin::setPluginVersion(const std::string& name, float version)
{
  m_pluginVersions[name] = version;
}

void Plugin::remPluginVersion(const std::string& name)
{
  if (hasPluginVersion(name))
  {
    m_pluginVersions.erase(name);
  }
}

bool Plugin::hasPointer(const std::string& name) const
{
  return m_pointers.find(name) != m_pointers.end();
}

void* Plugin::getPointer(const std::string& name) const
{
  std::map<const std::string, void*>::const_iterator pointer = m_pointers.find(name);

  if (pointer == m_pointers.end())
  {
    return NULL;
  }

  return pointer->second;
}

void Plugin::setPointer(const std::string& name, void* pointer)
{
  m_pointers[name] = pointer;
}

void Plugin::remPointer(const std::string& name)
{
  if (hasPointer(name))
  {
    m_pointers.erase(name);
  }
}
