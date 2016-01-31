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

bool InventoryWorkbench::onwindowClick(User* user, int8_t windowID, int16_t slot, int8_t button, int16_t actionNumber, 
                                      int16_t itemID, int8_t itemCount, int16_t itemUses, int8_t mode)
{

  // Safeguard against overflow
  if (slot > MAX_SLOT_CRAFTING_TABLE) return false;
  if (slot != -999 && slot < 0) return false;

  Inventory* inventory = ServerInstance->inventory();
  //Ack
  if(actionNumber)
  {
    // ToDo: actually check the action before ack
    user->writePacket(Protocol::confirmTransaction(windowID, actionNumber, 1));
  }

  // Handle drag mode in a base class helper function
  if (mode == INVENTORY_MODE_DRAG)
  {
    return this->handleDrag(user, windowID, slot, button, actionNumber, itemID, itemCount, itemUses, mode);
  }
  else if (mode == INVENTORY_MODE_NUMBER)
  {
    return this->handleNumber(user, windowID, slot, button, actionNumber, itemID, itemCount, itemUses, mode);
  }
  user->openInv.recordAction = false;

  //Click outside the window
  if (slot == -999)
  {
    //Dropping outside of the window
    if(button == 0 && mode == 0 && user->inventoryHolding.getType() != -1)
    {
      ServerInstance->map(user->pos.map)->createPickupSpawn((int)user->pos.x, (int)user->pos.y, (int)user->pos.z,
          user->inventoryHolding.getType(), user->inventoryHolding.getCount(),
          user->inventoryHolding.getHealth(), user);
      user->inventoryHolding.setType(-1);
      return true;
    }
    return true;
  }
  //on click-and-drag mode, recording the slots used
  else if(user->openInv.recordAction)
  {
    if(mode == 5)
    {
      user->openInv.slotActions.push_back(slot);
    }
    else
    {
      user->openInv.recordAction = false;
    }
    return true;
  }

  if (!user->isOpenInv && windowID != 0)
  {
    return false;
  }

  sChunk* chunk = NULL;
  chunk = ServerInstance->map(user->pos.map)->getChunk(blockToChunk(user->openInv.x), blockToChunk(user->openInv.z));

  if (chunk == NULL)
  {
    return false;
  }

  chunk->changed = true;


  std::vector<User*>* otherUsers = NULL;
  OpenInvPtr currentInventory;
  std::vector<OpenInvPtr>& inv = inventory->openWorkbenches;

  for (size_t i = 0; i < inv.size(); i++)
  {
    if (inv[i]->x == user->openInv.x &&
        inv[i]->y == user->openInv.y &&
        inv[i]->z == user->openInv.z)
    {
      otherUsers = &inv[i]->users;
      currentInventory = inv[i];
      break;
    }
  }

  if (otherUsers == NULL || currentInventory == NULL)
  {
    return false;
  }


  Item* slotItem = NULL;

  if (slot > 9)
  {
    slotItem = &user->inv[slot - 1];
  }
  else
  {
    slotItem = &currentInventory->workbench[slot];
  }


  bool workbenchCrafting = false;
  
  //Empty slot and holding something
  if ((slotItem->getType() == -1 || (slotItem->getType() == user->inventoryHolding.getType() && slotItem->getHealth() == user->inventoryHolding.getHealth() && slotItem->getCount() < 64)) && user->inventoryHolding.getType() != -1)
  {
    //If accessing crafting output slot
    if (slotItem->getType() != -1 && slot == 0)
    {
      if (user->inventoryHolding.getType() == slotItem->getType() && 64 - user->inventoryHolding.getCount() >= slotItem->getCount())
      {
        user->inventoryHolding.decCount(-slotItem->getCount());

        for (uint8_t workbenchSlot = 1; workbenchSlot < 10; workbenchSlot++)
        {
          if (currentInventory->workbench[workbenchSlot].getType() != -1)
          {
            currentInventory->workbench[workbenchSlot].decCount();
            inventory->setSlot(user, windowID, workbenchSlot, &currentInventory->workbench[workbenchSlot]);
          }
        }
        workbenchCrafting = true;

      }
    }
    else
    {
      //ToDo: Make sure we have room for the items!

      //Make sure not putting anything to the crafting space
      if (slot != 0)
      {
        int16_t addCount = (64 - slotItem->getCount() >= user->inventoryHolding.getCount()) ? user->inventoryHolding.getCount() : 64 - slotItem->getCount();

        slotItem->decCount(0 - ((button) ? 1 : addCount));
        slotItem->setHealth(user->inventoryHolding.getHealth());
        slotItem->setType(user->inventoryHolding.getType());

        user->inventoryHolding.decCount((button) ? 1 : addCount);
      }
    }

  }
  //We are not holding anything, get the item we clicked
  else if (user->inventoryHolding.getType() == -1)
  {
    //If accessing crafting output slot, remove from input!
    if (slotItem->getType() != -1 && slot == 0)
    {
      user->inventoryHolding.setType(slotItem->getType());
      user->inventoryHolding.setCount(slotItem->getCount());
      user->inventoryHolding.setHealth(slotItem->getHealth());

      for (uint8_t workbenchSlot = 1; workbenchSlot < 10; workbenchSlot++)
      {
        if (currentInventory->workbench[workbenchSlot].getType() != -1)
        {
          currentInventory->workbench[workbenchSlot].decCount();

          inventory->setSlot(user, windowID, workbenchSlot, &currentInventory->workbench[workbenchSlot]);
        }
      }
      workbenchCrafting = true;

      
    }
    else
    {

      //Shift+click -> items to player inv
      //ToDo: from player inventory to chest
      if(!button && mode &&inventory-> isSpace(user, slotItem->getType(), slotItem->getCount()))
      {
        inventory->addItems(user, slotItem->getType(), slotItem->getCount(), slotItem->getHealth());
        slotItem->setCount(0);
      }
      else
      {
        user->inventoryHolding.setType(slotItem->getType());
        user->inventoryHolding.setHealth(slotItem->getHealth());
        user->inventoryHolding.setCount(slotItem->getCount());
        if (button == 1)
        {
          user->inventoryHolding.decCount(slotItem->getCount() >> 1);
        }
        slotItem->decCount(user->inventoryHolding.getCount());
      }
      
      if (slotItem->getCount() == 0)
      {
        slotItem->setHealth(0);
        slotItem->setType(-1);
      }
    }
  }
  else
  {
    //Swap items if holding something and clicking another, not with craft slot
    if (slot != 0)
    {
      int16_t type = slotItem->getType();
      int8_t count = slotItem->getCount();
      int16_t health = slotItem->getHealth();
      slotItem->setType(user->inventoryHolding.getType());
      slotItem->setCount(user->inventoryHolding.getCount());
      slotItem->setHealth(user->inventoryHolding.getHealth());
      user->inventoryHolding.setType(type);
      user->inventoryHolding.setCount(count);
      user->inventoryHolding.setHealth(health);
    }
  }

  //Update slot
  inventory->setSlot(user, windowID, slot, slotItem);

  //Update item on the cursor
  inventory->setSlot(user, WINDOW_CURSOR, 0, &user->inventoryHolding);


  //Check if crafting
  if (inventory->doCraft(currentInventory->workbench, 3, 3))
  {
    inventory->setSlot(user, windowID, 0, &currentInventory->workbench[0]);
  }
  else
  {
    Item temp(0, -1);
    currentInventory->workbench[0].setType(-1);
    inventory->setSlot(user, windowID, 0, &temp);
  }

  
  //Signal others using the same space
  if(slot < 10)
  {
    for(uint32_t i = 0; i < otherUsers->size(); i++)
    {
      if((*otherUsers)[i] != user)
      {
        inventory->setSlot((*otherUsers)[i], windowID, slot, slotItem);
      }
    }
  }

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