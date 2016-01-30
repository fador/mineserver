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
#include "inventoryPlayer.h"
#include "user.h"
#include "mineserver.h"
#include "plugin.h"
#include "protocol.h"
#include "map.h"


bool InventoryPlayer::onwindowClick(User* user, int8_t windowID, int16_t slot, int8_t button,int16_t actionNumber,
                                   int16_t itemID, int8_t itemCount, int16_t itemUses, int8_t mode) const
{
  Inventory* inventory = ServerInstance->inventory();
  //Ack
  if(actionNumber)
  {
    // ToDo: actually check the action before ack
    user->writePacket(Protocol::confirmTransaction(windowID, actionNumber, 1));
  }

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
    if(mode == 5 && user->inventoryHolding.getType() != -1)
    {
      //click-and-drag (begin)
      if(button == 0 || button == 4)
      {
        user->openInv.slotActions.clear();
        user->openInv.recordAction = true;
        return true;
      }
      //click-and-drag (end)
      else if(button == 2 || button == 6)
      {
        user->openInv.recordAction = false;

        //Spread the stack nice and evenly
        if(!user->openInv.slotActions.size() || user->openInv.slotActions.size() > (uint32_t)user->inventoryHolding.getCount())
        {
          //FAILURE (should not happend)
          return true;
        }
        //HAX
        int16_t count = (user->inventoryHolding.getCount()/user->openInv.slotActions.size());
        for(uint32_t i = 0; i < user->openInv.slotActions.size(); i++)
        {
          for(int c = 0; c < count; c++)
          {
            onwindowClick(user, windowID, user->openInv.slotActions[i], 1, 0, -1, itemCount, itemUses, 0);
          }
        }
      }      
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

  Item* slotItem = &user->inv[slot];


  bool workbenchCrafting = false;
  bool playerCrafting    = false;

  if (slot >= 5 && slot <= 8)
  {
    // Armour slots are a strange case. Only a quantity of one should be allowed, so this must be checked for.
    if (slotItem->getType() == -1 && user->inventoryHolding.getType() > 0)
    {
      if (inventory->canBeArmour(slot, user->inventoryHolding.getType()))
      {
        slotItem->setType(user->inventoryHolding.getType());
        slotItem->setHealth(user->inventoryHolding.getHealth());
        slotItem->setCount(1);
        user->inventoryHolding.decCount();
      }
      else
      {
        slotItem->decCount(0);
        user->inventoryHolding.decCount(0); // Refresh both
      }
    }
    else if (slotItem->getType() > 0 && user->inventoryHolding.getType() == -1)
    {
      user->inventoryHolding.setType(slotItem->getType());
      user->inventoryHolding.setCount(slotItem->getCount());
      user->inventoryHolding.setHealth(slotItem->getHealth());
      slotItem->setType(-1);

    }
    else if (slotItem->getType() > 0 && user->inventoryHolding.getType() > 0 && user->inventoryHolding.getCount() == 1)
    {
      if (inventory->canBeArmour(slot, user->inventoryHolding.getType()))
      {
        uint16_t t_type = slotItem->getType();
        uint8_t t_count = slotItem->getCount();
        uint16_t t_health = slotItem->getHealth();
        slotItem->setCount(1);
        slotItem->setHealth(user->inventoryHolding.getHealth());
        slotItem->setType(user->inventoryHolding.getType());
        user->inventoryHolding.setCount(t_count);
        user->inventoryHolding.setHealth(t_health);
        user->inventoryHolding.setType(t_type);
      }
      else
      {
        slotItem->decCount(0);
        user->inventoryHolding.decCount(0); // Refresh both
      }
    }
    inventory->setSlot(user, WINDOW_CURSOR, 0, &user->inventoryHolding);
    return false;
  }


  //Empty slot and holding something
  if ((slotItem->getType() == -1 || (slotItem->getType() == user->inventoryHolding.getType() && slotItem->getHealth() == user->inventoryHolding.getHealth() && slotItem->getCount() < 64)) && user->inventoryHolding.getType() != -1)
  {
    //If accessing crafting output slot
    if (slotItem->getType() != -1 && slot == 0)
    {
      if (user->inventoryHolding.getType() == slotItem->getType() && 64 - user->inventoryHolding.getCount() >= slotItem->getCount())
      {
        user->inventoryHolding.decCount(-slotItem->getCount());

        for (uint8_t playerSlot = 1; playerSlot < 5; playerSlot++)
        {
          if (user->inv[playerSlot].getType() != -1)
          {
            user->inv[playerSlot].decCount();
            inventory->setSlot(user, windowID, playerSlot, &user->inv[playerSlot]);
          }
        }
        playerCrafting = true;

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

      for (uint8_t playerSlot = 1; playerSlot < 5; playerSlot++)
      {
        if (user->inv[playerSlot].getType() != -1)
        {
          user->inv[playerSlot].decCount();
        }
      }
      playerCrafting = true;

    }
    else
    {
      //Shift+click -> items to player inv
      //ToDo: from player inventory to chest
      if(!button && mode && inventory->isSpace(user, slotItem->getType(), slotItem->getCount()))
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
  if ((slot < 5 && slot > 0) || playerCrafting)
  {
    if (inventory->doCraft(user->inv, 2, 2))
    {
      inventory->setSlot(user, windowID, 0, &user->inv[0]);
    }
    else
    {
      Item temp(0, -1);
      user->inv[0].setType(-1);
      inventory->setSlot(user, windowID, 0, &temp);
    }
  }

  return true;
}

bool InventoryPlayer::onwindowOpen(User* user, int8_t type, int32_t x, int32_t y, int32_t z) const
{
  // We don't get info of player opening own inventory

  return true;
}

bool InventoryPlayer::onwindowClose(User* user, int8_t type, int32_t x, int32_t y, int32_t z) const
{
  // We don't get info of player closing own inventory

   return true;
}