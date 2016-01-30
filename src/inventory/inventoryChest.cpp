/*
  Copyright (c) 2016, The Mineserver Project
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

#include <stdint.h>
#include <string>
#include "InventoryChest.h"
#include "user.h"
#include "mineserver.h"
#include "plugin.h"
#include "protocol.h"
#include "chunkmap.h"
#include "map.h"

bool InventoryChest::onwindowOpen(User* user, int8_t type, int32_t x, int32_t y, int32_t z) const
{
  sChunk* chunk = ServerInstance->map(user->pos.map)->getChunk(blockToChunk(x), blockToChunk(z));
  if (chunk == NULL)
  {
    return false;
  }

  std::vector<OpenInvPtr>& inv = ServerInstance->inventory()->openChests;

  for (size_t i = 0; i < inv.size(); ++i)
  {
    if (inv[i]->x == user->openInv.x &&
        inv[i]->y == user->openInv.y &&
        inv[i]->z == user->openInv.z)
    {
      inv[i]->users.push_back(user);
      user->isOpenInv = true;
      break;
    }
  }

  if (!user->isOpenInv)
  {
    //If the inventory not yet opened, create it
    OpenInvPtr newInv(new OpenInventory());
    newInv->type = type;
    newInv->x    = x;
    newInv->y    = y;
    newInv->z    = z;
    user->openInv = *newInv;

    newInv->users.push_back(user);

    inv.push_back(newInv);
    user->isOpenInv = true;

    //Chest opening animation and sound
    user->sendAll(Protocol::blockAction(x,y,z,1,1,BLOCK_CHEST));
    user->sendAll(Protocol::namedSoundEffect("random.chestopen", x<<3, y<<3, z<<3, 1.0, 63));
  }

  // Get the actual data in the chest and dump it to the player
  chestDataPtr _chestData;
  for (uint32_t i = 0; i < chunk->chests.size(); i++)
  {
    if ((chunk->chests[i]->x() == x)
      && (chunk->chests[i]->y() == y)
      && (chunk->chests[i]->z() == z) )
    {
      _chestData = chunk->chests[i];
      break;
    }
  }
  if(_chestData != NULL)
  {
    std::string windowName = _chestData->large() ? "Large chest" : "Chest";

    user->writePacket(Protocol::openWindow(type,INVENTORYTYPE_CHEST,"{\"text\": \""+json_esc(windowName)+"\"}", _chestData->size()));

    for (size_t j = 0; j < _chestData->size(); j++)
    {
      // Send all non-empty slots
      if ((*_chestData->items())[j]->getType() != -1)
      {
        user->writePacket(Protocol::setSlot(type, j, *(*_chestData->items())[j]));
      }
    }
  }

  return true;
}

bool InventoryChest::onwindowClose(User* user, int8_t type, int32_t x, int32_t y, int32_t z) const
{
  std::vector<OpenInvPtr>& inv = ServerInstance->inventory()->openChests;

  for (size_t i = 0; i < inv.size(); ++i)
  {
    // Find this chest
    if (inv[i]->x == user->openInv.x &&
        inv[i]->y == user->openInv.y &&
        inv[i]->z == user->openInv.z)
    {
      // Find this user
      for (size_t j = 0; j < inv[i]->users.size(); ++j)
      {
        if (inv[i]->users[j] == user)
        {
          // We should make users into a container that supports fast erase.
          inv[i]->users.erase(inv[i]->users.begin() + j);

          if (inv[i]->users.empty())
          {
            user->sendAll(Protocol::blockAction(x,y,z,1,0,BLOCK_CHEST));
            user->sendAll(Protocol::namedSoundEffect("random.chestclosed", x<<3, y<<3, z<<3, 1.0, 63));
            inv.erase(inv.begin() + i);
          }

          user->isOpenInv = false;
          return true;
        }
      }
      break;
    }
  }

  user->isOpenInv = false;
  return true;
}