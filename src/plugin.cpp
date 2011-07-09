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
#include "blocks/leaves.h"
#include "blocks/cake.h"
#include "blocks/note.h"
#include "blocks/blockfurnace.h"
#include "blocks/workbench.h"
#include "blocks/wood.h"
#include "blocks/redstone.h"
#include "blocks/pumpkin.h"
#include "blocks/step.h"
#include "blocks/tnt.h"
#include "blocks/bed.h"
#include "blocks/wool.h"
#include "blocks/jackolantern.h"
#include "blocks/ice.h"

#include "items/food.h"
#include "items/projectile.h"

// Create default hooks
Plugin::Plugin()
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
Plugin::~Plugin()
{
  for (HookMap::iterator it = m_hooks.begin(); it != m_hooks.end(); ++it)
  {
    delete it->second;
  }

  m_hooks.clear();
}

void Plugin::init()
{
  // Create Block objects
  m_block_CBs.push_back(BlockBasicPtr(new BlockRedstone));
  m_block_CBs.push_back(BlockBasicPtr(new BlockWood));
  m_block_CBs.push_back(BlockBasicPtr(new BlockFalling));
  m_block_CBs.push_back(BlockBasicPtr(new BlockTorch));
  m_block_CBs.push_back(BlockBasicPtr(new BlockPlant));
  m_block_CBs.push_back(BlockBasicPtr(new BlockSnow));
  m_block_CBs.push_back(BlockBasicPtr(new BlockLiquid));
  m_block_CBs.push_back(BlockBasicPtr(new BlockFire));
  m_block_CBs.push_back(BlockBasicPtr(new BlockStair));
  m_block_CBs.push_back(BlockBasicPtr(new BlockChest));
  m_block_CBs.push_back(BlockBasicPtr(new BlockDoor));
  m_block_CBs.push_back(BlockBasicPtr(new BlockSign));
  m_block_CBs.push_back(BlockBasicPtr(new BlockTracks));
  m_block_CBs.push_back(BlockBasicPtr(new BlockLadder));
  m_block_CBs.push_back(BlockBasicPtr(new BlockLeaves));
  m_block_CBs.push_back(BlockBasicPtr(new BlockCake));
  m_block_CBs.push_back(BlockBasicPtr(new BlockNote));
  m_block_CBs.push_back(BlockBasicPtr(new BlockFurnace));
  m_block_CBs.push_back(BlockBasicPtr(new BlockWorkbench));
  m_block_CBs.push_back(BlockBasicPtr(new BlockPumpkin));
  m_block_CBs.push_back(BlockBasicPtr(new BlockStep));
  m_block_CBs.push_back(BlockBasicPtr(new BlockBed));
  m_block_CBs.push_back(BlockBasicPtr(new BlockWool));
  m_block_CBs.push_back(BlockBasicPtr(new Blockjackolantern));
  m_block_CBs.push_back(BlockBasicPtr(new BlockIce));
  m_block_CBs.push_back(BlockBasicPtr(new BlockDefault));
  m_block_CBs.push_back(BlockBasicPtr(new BlockTNT));

  m_item_CBs.push_back(ItemBasicPtr(new ItemFood));
  m_item_CBs.push_back(ItemBasicPtr(new ItemProjectile));
}


typedef void (*pfms)(mineserver_pointer_struct*);
typedef void (*pfv)();

bool Plugin::loadPlugin(const std::string& name, const std::string& path, std::string alias)
{
  LIBRARY_HANDLE lhandle = NULL;
  pfms fhandle = NULL;

  if (name.empty()
      || (name.find('/')  != std::string::npos)
      || (name.find('\\') != std::string::npos))
  {
    LOG(INFO, "Plugin", "Invalid name: " + name);
    return false;
  }

  if (alias.empty())
  {
    alias = name;
  }

  if (!path.empty())
  {
    std::string file;
    file = path + '/' + name + LIBRARY_EXTENSION;

    struct stat st;
    int statr = stat(file.c_str(), &st);
    if ((statr == 0) && !(st.st_mode & S_IFDIR))
    {
      LOG(INFO, "Plugin", "Loading: " + file);
      lhandle = LIBRARY_LOAD(file.c_str());
    }
    else
    {
      LOG(INFO, "Plugin", "Could not find: " + file);
      return false;
    }
  }
  else
  {
    LOG(INFO, "Plugin", "Loading built-in: " + name);
    lhandle = LIBRARY_SELF();
  }

  if (lhandle == NULL)
  {
    LOG(INFO, "Plugin", "Could not load: " + name);
    LOG(INFO, "Plugin", LIBRARY_ERROR());
    return false;
  }

  m_libraryHandles[alias] = lhandle;

  *reinterpret_cast<void**>(&fhandle) = (void*)LIBRARY_SYMBOL(lhandle, (name + "_init").c_str());
  if (fhandle == NULL)
  {
    LOG(INFO, "Plugin", "Could not get init function handle!");
    unloadPlugin(alias);
    return false;
  }
  fhandle(&plugin_api_pointers);

  return true;
}

void Plugin::unloadPlugin(const std::string name)
{
  LIBRARY_HANDLE lhandle = NULL;
  pfv fhandle = NULL;

  if (m_pluginVersions.find(name) != m_pluginVersions.end())
  {
    LOG(INFO, "Plugin", "Unloading: " + name);

    if (m_libraryHandles[name] != NULL)
    {
      lhandle = m_libraryHandles[name];
      m_libraryHandles.erase(name);
    }
    else
    {
      lhandle = LIBRARY_SELF();
    }

    *reinterpret_cast<void**>(&fhandle) = (void*)LIBRARY_SYMBOL(lhandle, (name + "_shutdown").c_str());
    if (fhandle == NULL)
    {
      LOG(INFO, "Plugin", "Could not get shutdown function handle!");
    }
    else
    {
      LOG(INFO, "Plugin", "Calling shutdown function for: " + name);
      fhandle();
    }

    LIBRARY_CLOSE(m_libraryHandles[name]);
  }
  else
  {
    LOG(WARNING, "Plugin", name + " is not loaded!");
  }
}
