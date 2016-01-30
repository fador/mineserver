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


bool InventoryBasic::handleDrag(User* user, int8_t windowID, int16_t slot, int8_t button, int16_t actionNumber, int16_t itemID, int8_t itemCount, int16_t itemUses, int8_t mode)
{
  Inventory* inventory = ServerInstance->inventory();
  //Start of stop drag
  if (slot == -999)
  {
    if(user->inventoryHolding.getType() != -1)
    {
      //click-and-drag (begin)
      if(button == INVENTORY_BUTTON_START_DRAG_RIGHT || button == INVENTORY_BUTTON_LEFT)
      {
        user->openInv.slotActions.clear();
        user->openInv.recordAction = true;
        return true;
      }
      //click-and-drag (end)
      else if(button == INVENTORY_BUTTON_END_DRAG_LEFT || button == INVENTORY_BUTTON_END_DRAG_RIGHT)
      {
        user->openInv.recordAction = false;

        //Spread the stack nice and evenly
        if(!user->openInv.slotActions.size() || user->openInv.slotActions.size() > (uint32_t)user->inventoryHolding.getCount())
        {
          //FAILURE (should not happen)
          return true;
        }

        // Handle spreading the stack, leftover stays in the Holding
        
        // Store original for a while
        Item tempHolding = user->inventoryHolding;
        
        Item oneStack(user->inventoryHolding.getType(), user->inventoryHolding.getCount(),user->inventoryHolding.getHealth());

        // With left click put the same amount of items in a slot
        if (button == INVENTORY_BUTTON_END_DRAG_LEFT)
        {
          int16_t count = (user->inventoryHolding.getCount()/user->openInv.slotActions.size());
          oneStack.setCount(count);
          tempHolding.setCount(user->inventoryHolding.getCount() - count*user->openInv.slotActions.size());
        }
        else // Right click
        {
          oneStack.setCount(1);
          tempHolding.setCount(user->inventoryHolding.getCount() - user->openInv.slotActions.size());
        }

        // Set "oneStack" to each of the slots
        for(uint32_t i = 0; i < user->openInv.slotActions.size(); i++)
        {
          user->inventoryHolding = oneStack;
          onwindowClick(user, windowID, user->openInv.slotActions[i], INVENTORY_BUTTON_LEFT, 0, -1, 0, 0, INVENTORY_MODE_NORMAL);
        }
        // Restore holding
        user->inventoryHolding = tempHolding;
        inventory->setSlot(user, WINDOW_CURSOR, 0, &user->inventoryHolding);
      }
    }
    return true;
  }
  //on click-and-drag mode, recording the slots used
  else if(user->openInv.recordAction)
  {
    if(mode == INVENTORY_BUTTON_ADD_SLOTS_RIGHT || mode == INVENTORY_BUTTON_ADD_SLOTS_LEFT)
    {
      user->openInv.slotActions.push_back(slot);
    }
    else
    {
      user->openInv.recordAction = false;
    }
    return true;
  }

  return true;
}
