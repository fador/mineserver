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

#ifdef WIN32
  #include <conio.h>
  #include <direct.h>
  #include <winsock2.h>
#else
  #include <netinet/in.h>
#endif
#include <string.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <algorithm>

#include <zlib.h>
#include <sys/stat.h>

#include "map.h"
#include "user.h"
#include "mineserver.h"


#include "inventory.h"


bool Inventory::windowClick(User *user,sint8 windowID, sint16 slot, sint8 rightClick, sint16 actionNumber, sint16 itemID, sint8 itemCount,sint8 itemUses)
{  
  if(windowID == 0) //Player inventory
  {
    //Click outside the window
    if(slot == -999)
    {
      if(user->inventoryHolding.type != -1)
      {
        Mineserver::get()->map()->createPickupSpawn((int)user->pos.x, (int)user->pos.y, (int)user->pos.z, 
                                                    user->inventoryHolding.type, user->inventoryHolding.count,
                                                    user->inventoryHolding.health,user);
        user->inventoryHolding.count = 0;
        user->inventoryHolding.type  =-1;
        user->inventoryHolding.health= 0;
      }
      return true;
    }

    Item slotItem;
    slotItem=user->inv[slot];

    //Empty slot and holding something
    if((itemID == -1 || (slotItem.type == itemID && slotItem.count < 64) ) && user->inventoryHolding.type != -1)
    {
      sint16 addCount = (64-slotItem.count>=user->inventoryHolding.count)?user->inventoryHolding.count:64-slotItem.count;

      user->inv[slot].count  += rightClick?1:addCount;
      user->inv[slot].health = user->inventoryHolding.health;
      user->inv[slot].type   = user->inventoryHolding.type;

      user->inventoryHolding.count -= rightClick?1:addCount;
      if(user->inventoryHolding.count == 0)
      {
        user->inventoryHolding.type  = -1;
        user->inventoryHolding.health= 0;
      }
    }
    else if(user->inventoryHolding.type == -1)
    {
      user->inventoryHolding.type   = itemID;
      user->inventoryHolding.health = itemUses;
      user->inventoryHolding.count  = itemCount;
      if(rightClick == 1)
      {
        user->inventoryHolding.count  -= itemCount>>1;
      }

      user->inv[slot].count  -= rightClick?itemCount>>1:user->inventoryHolding.count;
      if(user->inv[slot].count == 0)
      {
        user->inv[slot].health = 0;
        user->inv[slot].type   =-1;
      }
    }

    //Accept transaction
    user->buffer << (sint8)PACKET_TRANSACTION << (sint8)0 << (sint16)actionNumber << (sint8)1;

    //Update slot
    user->buffer << (sint8)PACKET_SET_SLOT << (sint8)0 << (sint16)slot << (sint16)itemID << (sint8)user->inv[slot].count << (sint8)user->inv[slot].health;
    Mineserver::get()->screen()->log(1,"Setslot: " + dtos(slot) + " to " + dtos(itemID) + " (" + dtos(user->inv[slot].count) + ") health: " + dtos(user->inv[slot].health));
    //Update item on the cursor
    //user->buffer << (sint8)PACKET_SET_SLOT << (sint8)-1 << (sint16)0   << (sint16)user->inventoryHolding.type << (sint8)0 << (sint8)0;
  }
  return true;
}



bool Inventory::isSpace(User *user,sint16 itemID, char count)
{
  int leftToFit = count;
  for(uint8 i = 0; i < 36; i++)
  {
    Item *slot=&user->inv[i+9];
    if(slot->type == -1)
    {
      return true;
    }

    if(slot->type == itemID)
    {
      if(64-slot->count >= leftToFit)
      {
        return true;
      }
      else if(64-slot->count > 0)
      {
        leftToFit -= 64-slot->count;
      }
    }
  }
  return false;
}


bool Inventory::addItems(User *user,sint16 itemID, char count, sint16 health)
{
  bool checkingTaskbar = true;

  for(uint8 i = 36-9; i < 36-9 || checkingTaskbar; i++)
  {
    //First, the "task bar"
    if(i == 36)
    {
      checkingTaskbar = false;
      i=0;
    }

    //The main slots are in range 9-44
    Item *slot = &user->inv[i+9];

    //If slot empty, put item there
    if(slot->type == -1)
    {
      user->buffer << (sint8)PACKET_SET_SLOT << (sint8)0 << (sint16)(i+9) << (sint16)itemID << (sint8)count << (sint8)health;
      slot->type   = itemID;
      slot->count  = count;
      slot->health = health;
      break;
    }

    //If same item type
    if(slot->type == itemID)
    {
      //Put to the stack
      if(64-slot->count >= count)
      {
        user->buffer << (sint8)PACKET_SET_SLOT << (sint8)0 << (sint16)(i+9) << (sint16)itemID << (sint8)(slot->count+count) << (sint8)health;
        slot->type   = itemID;
        slot->count += count;
        break;
      }
      //Put some of the items to this stack and continue searching for space
      else if(64-slot->count > 0)
      {
        user->buffer << (sint8)PACKET_SET_SLOT << (sint8)0 << (sint16)(i+9) << (sint16)itemID << (sint8)64 << (sint8)health;
        slot->type = itemID;
        slot->count = 64;
        count -= 64-slot->count;
      }
    }
  }

  return true;
}