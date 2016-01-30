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
#include "inventoryWorkbench.h"
#include "user.h"
#include "mineserver.h"
#include "plugin.h"
#include "protocol.h"
#include "map.h"

bool InventoryWorkbench::onwindowClick(User* user, int8_t windowID, int16_t slot, int8_t button, int16_t actionNumber, int16_t itemID, int8_t itemCount, int16_t itemUses, int8_t mode) const
{

  return true;
}


bool InventoryWorkbench::onwindowOpen(User* user, int8_t type, int32_t x, int32_t y, int32_t z) const
{
  std::vector<OpenInvPtr>& inv = ServerInstance->inventory()->openWorkbenches;

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
  }

  user->writePacket(Protocol::openWindow(WINDOW_CRAFTING_TABLE,INVENTORYTYPE_CRAFTING_TABLE,"{\"text\": \""+json_esc("Workbench")+"\"}", 0));

  // ToDo: option for shared workbenches
  for (uint32_t i = 0; i < inv.size(); i++)
  {
    if (inv[i]->x == user->openInv.x &&
        inv[i]->y == user->openInv.y &&
        inv[i]->z == user->openInv.z)
    {
      for (int j = 0; j < 10; j++)
      {
        if (inv[i]->workbench[j].getType() != -1)
        {
          user->writePacket(Protocol::setSlot(WINDOW_CRAFTING_TABLE, j, inv[i]->workbench[j]));
        }
      }
      break;
    }
  }

  return true;
}

bool InventoryWorkbench::onwindowClose(User* user, int8_t type, int32_t x, int32_t y, int32_t z) const
{
  std::vector<OpenInvPtr>& inv = ServerInstance->inventory()->openWorkbenches;

  for (size_t i = 0; i < inv.size(); ++i)
  {
    if (inv[i]->x == user->openInv.x &&
        inv[i]->y == user->openInv.y &&
        inv[i]->z == user->openInv.z)
    {
      for (size_t j = 0; j < inv[i]->users.size(); ++j)
      {
        if (inv[i]->users[j] == user)
        {
          // We should make users into a container that supports fast erase.
          inv[i]->users.erase(inv[i]->users.begin() + j);

          if (inv[i]->users.empty())
          {
            //Dump stuff to ground if no other users

            for (uint32_t slotNumber = 1; slotNumber < 10; ++slotNumber)
            {
              if (inv[i]->workbench[slotNumber].getType() != -1)
              {
                ServerInstance->map(user->pos.map)->createPickupSpawn((int)user->pos.x, (int)user->pos.y, (int)user->pos.z,
                    inv[i]->workbench[slotNumber].getType(), inv[i]->workbench[slotNumber].getCount(),
                    inv[i]->workbench[slotNumber].getHealth(), user);
              }
            }

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