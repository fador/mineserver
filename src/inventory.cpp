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


  Mineserver::get()->screen()->log(1,"window: " + dtos(windowID) + " slot: " + dtos(slot) + " (" + dtos(actionNumber) + ") itemID: " + dtos(itemID));
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

  if(!user->isOpenInv && windowID != 0)
  {
    return false;
  }

  sChunk* chunk = Mineserver::get()->map()->chunks.GetChunk(blockToChunk(user->openInv.x),blockToChunk(user->openInv.z));

  if(chunk == NULL)
  {
    return false;
  }


  std::vector<User *> *otherUsers = NULL;
  openInventory *currentInventory = NULL;

  if(windowID != WINDOW_PLAYER)
  {
    std::vector<openInventory *> *inv;
    switch(user->openInv.type)
    {
      case WINDOW_CHEST:
        inv = &openChests;
        break;
      case WINDOW_FURNACE:
        inv = &openFurnaces;
        break;
      case WINDOW_WORKBENCH:
        inv = &openWorkbenches;
        break;
    }

    for(uint32 i = 0; i < inv->size(); i++)
    {
      if((*inv)[i]->x == user->openInv.x &&
         (*inv)[i]->y == user->openInv.y &&
         (*inv)[i]->z == user->openInv.z)
      {
        otherUsers = &(*inv)[i]->users;
        currentInventory = (*inv)[i];
        break;
      }
    }
  }

  if(otherUsers == NULL || currentInventory == NULL)
  {
    return false;
  }

  Item *slotItem = NULL;
  
  switch(windowID)
  {
     //Player inventory
    case WINDOW_PLAYER:
      slotItem=&user->inv[slot];
      break;
    case WINDOW_CHEST:
      if(slot>27)
      {
        slotItem=&user->inv[slot-18];
      }
      else
      {
        for(uint32 i = 0; i < chunk->chests.size(); i ++)
        {
          if(chunk->chests[i]->x == user->openInv.x &&
             chunk->chests[i]->y == user->openInv.y &&
             chunk->chests[i]->z == user->openInv.z)
          {
            slotItem = &chunk->chests[i]->items[slot];
            break;
          }
        }
        //Create chest data if it doesn't exist
        if(slotItem == NULL)
        {
          chestData *newChest = new chestData;
          newChest->x = user->openInv.x;
          newChest->y = user->openInv.y;
          newChest->z = user->openInv.z;
          chunk->chests.push_back(newChest);
          slotItem = &newChest->items[slot];
        }
      }
      break;
    case WINDOW_LARGE_CHEST:
      if(slot>56)
      {
        slotItem=&user->inv[slot-47];
      }
      else
      {
        //ToDo: Handle large chest
      }
      break;
    case WINDOW_FURNACE:
      if(slot>3)
      {
        slotItem=&user->inv[slot+6];
      }
      else
      {
        for(uint32 i = 0; i < chunk->furnaces.size(); i ++)
        {
          if(chunk->furnaces[i]->x == user->openInv.x &&
             chunk->furnaces[i]->y == user->openInv.y &&
             chunk->furnaces[i]->z == user->openInv.z)
          {
            slotItem = &chunk->furnaces[i]->items[slot];
          }
        }
        //Create furnace data if it doesn't exist
        if(slotItem == NULL)
        {
          furnaceData *newFurnace = new furnaceData;
          newFurnace->x = user->openInv.x;
          newFurnace->y = user->openInv.y;
          newFurnace->z = user->openInv.z;
          newFurnace->burnTime = 0;
          newFurnace->cookTime = 0;
          chunk->furnaces.push_back(newFurnace);
          slotItem = &newFurnace->items[slot];
        }
      }
      break;
    case WINDOW_WORKBENCH:
      if(slot > 9)
      {
        slotItem=&user->inv[slot-1];
      }
      else
      {
        slotItem=&currentInventory->workbench[slot];
      }
      break;
  }
  

  //Empty slot and holding something
  if((itemID == -1 || (slotItem->type == itemID && slotItem->count < 64) ) && user->inventoryHolding.type != -1)
  {
    //If accessing crafting output slot, deny
    if((windowID == WINDOW_WORKBENCH || windowID == WINDOW_PLAYER) && slot == 0)
    {
      //Do something?
    }
    else
    {
      sint16 addCount = (64-slotItem->count>=user->inventoryHolding.count)?user->inventoryHolding.count:64-slotItem->count;

      slotItem->count  += (rightClick?1:addCount);
      slotItem->health  = user->inventoryHolding.health;
      slotItem->type    = user->inventoryHolding.type;

      user->inventoryHolding.count -= (rightClick?1:addCount);
      if(user->inventoryHolding.count == 0)
      {
        user->inventoryHolding.type  = -1;
        user->inventoryHolding.health= 0;
      }
    }
  }
  else if(user->inventoryHolding.type == -1)
  {
    //If accessing crafting output slot, remove from input!
    if((windowID == WINDOW_WORKBENCH || windowID == WINDOW_PLAYER) && slot == 0)
    {
      //ToDo: use recipe and remove blocks from the crafting slots
    }
    else
    {
      user->inventoryHolding.type   = slotItem->type;
      user->inventoryHolding.health = slotItem->health;
      user->inventoryHolding.count  = slotItem->count;
      if(rightClick == 1)
      {
        user->inventoryHolding.count  -= slotItem->count>>1;
      }

      slotItem->count  -= (rightClick?slotItem->count>>1:user->inventoryHolding.count);
      if(slotItem->count == 0)
      {
        slotItem->health = 0;
        slotItem->type   =-1;
      }
    }
  }
    
  
  //Update slot
  setSlot(user, windowID, slot, slotItem->type, slotItem->count, slotItem->health);

  //Update item on the cursor
  setSlot(user, WINDOW_CURSOR, 0, user->inventoryHolding.type, user->inventoryHolding.count, user->inventoryHolding.health);


  //Check if crafting
  if(windowID == WINDOW_WORKBENCH && slot < 10 && slot > 0 )
  {
    if(doCraft(currentInventory->workbench, 3, 3))
    {
      setSlot(user, windowID, 0, currentInventory->workbench[0].type, currentInventory->workbench[0].count, currentInventory->workbench[0].health);
    }
    else
    {
      currentInventory->workbench[0] = Item();
      setSlot(user, windowID, 0, -1, 0, 0);
    }
  }
  else if(windowID == WINDOW_PLAYER && slot < 4 && slot > 0)
  {
    if(doCraft(user->inv, 2, 2))
    {
      setSlot(user, windowID, 0, user->inv[0].type, user->inv[0].count, user->inv[0].health);
    }
    else
    {
      currentInventory->workbench[0] = Item();
      setSlot(user, windowID, 0, -1, 0, 0);
    }
  }

  /*
  //Signal others using the same space
  switch(windowID)
  {
    case WINDOW_WORKBENCH:
      if(slot < 10)        
      {
        for(uint32 i = 0; i < otherUsers->size(); i++)
        {          
          (*otherUsers)[i]->buffer << (sint8)PACKET_SET_SLOT << (sint8)windowID << (sint16)slot << (sint16)slotItem->type;
          if(slotItem->type != -1)
          {
            (*otherUsers)[i]->buffer << (sint8)slotItem->count << (sint8)slotItem->health;
          }
        }
      }
      break;

    case WINDOW_CHEST:
      chunk->changed = true;
      if(slot < 27)        
      {
        for(uint32 i = 0; i < otherUsers->size(); i++)
        {
          (*otherUsers)[i]->buffer << (sint8)PACKET_SET_SLOT << (sint8)windowID << (sint16)slot << (sint16)slotItem->type;
          if(slotItem->type != -1)
          {
            (*otherUsers)[i]->buffer << (sint8)slotItem->count << (sint8)slotItem->health;
          }
        }
      }
      break;

    case WINDOW_FURNACE:
      chunk->changed = true;
      if(slot < 3)        
      {
        for(uint32 i = 0; i < otherUsers->size(); i++)
        {
          (*otherUsers)[i]->buffer << (sint8)PACKET_SET_SLOT << (sint8)windowID << (sint16)slot << (sint16)slotItem->type;
          if(slotItem->type != -1)
          {
            (*otherUsers)[i]->buffer << (sint8)slotItem->count << (sint8)slotItem->health;
          }
        }
      }
      break;
  }
  */


  return true;
}


bool Inventory::windowOpen(User *user, sint8 type, sint32 x, sint32 y, sint32 z)
{
  sChunk* chunk = Mineserver::get()->map()->chunks.GetChunk(blockToChunk(x),blockToChunk(z));

  if(chunk == NULL)
  {
    return false;
  }

  onwindowOpen(user,type,x,y,z);

  switch(type)
  {
    case WINDOW_CHEST:    
      user->buffer << (sint8)PACKET_OPEN_WINDOW << (sint8)WINDOW_CHEST  << (sint8)INVENTORYTYPE_CHEST << std::string("Chest") << (sint8)27;

      for(uint32 i = 0;i < chunk->chests.size(); i++)
      {
        if(chunk->chests[i]->x == x && chunk->chests[i]->y == y && chunk->chests[i]->z == z)
        {
          for(int j = 0;j < 27; j++)
          {
            if(chunk->chests[i]->items[j].type != -1)
            {
              user->buffer << (sint8)PACKET_SET_SLOT << (sint8)WINDOW_CHEST << (sint16)j << (sint16)chunk->chests[i]->items[j].type 
                           << (sint8)(chunk->chests[i]->items[j].count) << (sint8)chunk->chests[i]->items[j].health;
            }
          }
          break;
        }
      }
      break;
    case WINDOW_WORKBENCH:
      user->buffer << (sint8)PACKET_OPEN_WINDOW << (sint8)WINDOW_WORKBENCH  << (sint8)INVENTORYTYPE_WORKBENCH << std::string("Workbench") << (sint8)0;

      for(uint32 i = 0; i < openWorkbenches.size(); i++)
      {
        if(openWorkbenches[i]->x == user->openInv.x &&
           openWorkbenches[i]->y == user->openInv.y &&
           openWorkbenches[i]->z == user->openInv.z)
        {
          for(int j = 0; j < 10; j++)
          {
            if(openWorkbenches[i]->workbench[j].type != -1)
            {
              user->buffer << (sint8)PACKET_SET_SLOT << (sint8)WINDOW_WORKBENCH << (sint16)j << (sint16)openWorkbenches[i]->workbench[j].type 
                           << (sint8)(openWorkbenches[i]->workbench[j].count) << (sint8)openWorkbenches[i]->workbench[j].health;
            }
          }
          break;
        }
      }
      break;
    case WINDOW_FURNACE:
      
      user->buffer << (sint8)PACKET_OPEN_WINDOW << (sint8)WINDOW_FURNACE  << (sint8)INVENTORYTYPE_FURNACE << std::string("Furnace") << (sint8)0;

      for(uint32 i = 0;i < chunk->furnaces.size(); i++)
      {
        if(chunk->furnaces[i]->x == x && chunk->furnaces[i]->y == y && chunk->furnaces[i]->z == z)
        {
          for(int j = 0; j < 3; j++)
          {
            if(chunk->furnaces[i]->items[j].type != -1)
            {
              user->buffer << (sint8)PACKET_SET_SLOT << (sint8)WINDOW_FURNACE << (sint16)j << (sint16)chunk->furnaces[i]->items[j].type 
                           << (sint8)(chunk->furnaces[i]->items[j].count) << (sint8)chunk->furnaces[i]->items[j].health;
            }
          }
          break;
        }
      }
      break;
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

bool Inventory::windowClose(User *user,sint8 windowID)
{
  //If still holding something, dump the items to ground
  if(user->inventoryHolding.type != -1)
  {
    Mineserver::get()->map()->createPickupSpawn((int)user->pos.x, (int)user->pos.y, (int)user->pos.z, 
                                                user->inventoryHolding.type, user->inventoryHolding.count,
                                                user->inventoryHolding.health,user);
    user->inventoryHolding.count = 0;
    user->inventoryHolding.type  =-1;
    user->inventoryHolding.health= 0;
  }

  if(user->isOpenInv)
  {
    onwindowClose(user,user->openInv.type, user->openInv.x, user->openInv.y, user->openInv.z);
  }

  return true;
}

bool Inventory::onwindowOpen(User *user,sint8 type, sint32 x, sint32 y, sint32 z)
{
  std::vector<openInventory *> *inv;
  switch(type)
  {
    case WINDOW_CHEST:
      inv = &openChests;
      break;
    case WINDOW_FURNACE:
      inv = &openFurnaces;
      break;
    case WINDOW_WORKBENCH:
      inv = &openWorkbenches;
      break;
  }
  for(uint32 i = 0; i < inv->size(); i++)
  {
    if((*inv)[i]->x == user->openInv.x &&
       (*inv)[i]->y == user->openInv.y &&
       (*inv)[i]->z == user->openInv.z)
    {
      (*inv)[i]->users.push_back(user);
      user->isOpenInv = true;
      return true;
    }
  }

  //If the inventory not yet opened, create it
  openInventory *newInv = new openInventory();
  newInv->type = type;
  newInv->x    = x;
  newInv->y    = y;
  newInv->z    = z;
  user->openInv = *newInv;

  newInv->users.push_back(user);

  inv->push_back(newInv);
  user->isOpenInv = true;

  return true;
}
bool Inventory::onwindowClose(User *user,sint8 type,sint32 x, sint32 y, sint32 z)
{
  std::vector<openInventory *> *inv;
  switch(type)
  {
    case WINDOW_CHEST:
      inv = &openChests;
      break;
    case WINDOW_FURNACE:
      inv = &openFurnaces;
      break;
    case WINDOW_WORKBENCH:
      inv = &openWorkbenches;
      break;
  }

  for(uint32 i = 0; i < inv->size(); i++)
  {
    if((*inv)[i]->x == user->openInv.x &&
       (*inv)[i]->y == user->openInv.y &&
       (*inv)[i]->z == user->openInv.z)
    {
      for(uint32 j = 0; j < (*inv)[i]->users.size(); j++)
      {
        if((*inv)[i]->users[j] == user)
        {
          (*inv)[i]->users.erase((*inv)[i]->users.begin()+j);

          if((*inv)[i]->users.size() == 0)
          {
            //Dump stuff to ground if workbench and no other users
            if(type == WINDOW_WORKBENCH)
            {
              for(uint32 slotNumber = 1; slotNumber < 10; slotNumber ++)
              {
                if((*inv)[i]->workbench[slotNumber].type != -1)
                {
                  Mineserver::get()->map()->createPickupSpawn((int)user->pos.x, (int)user->pos.y, (int)user->pos.z, 
                                                  (*inv)[i]->workbench[slotNumber].type, (*inv)[i]->workbench[slotNumber].count,
                                                  (*inv)[i]->workbench[slotNumber].health,user);
                }
              }
            }
            delete (*inv)[i];
            (*inv).erase((*inv).begin()+i);
          }

          user->isOpenInv = false;
          return true;
        }
      }
    }
  }

  user->isOpenInv = false;
  return true;
}




bool Inventory::doCraft(Item *slots, sint8 width, sint8 height)
{
  for(uint32 i = 0; i < recipes.size(); i++)
  {
    //Skip if recipe doesn't fit
    if(width < recipes[i]->width || height < recipes[i]->height)
    {
      continue;
    }
    
    sint8 offsetX = 0, offsetY = 0;

    //Check for any possible position the recipe would fit
    do
    {
      offsetX = 0;
      do
      {
        bool mismatch = false;
        //Check for the recipe match on this position
        for(uint32 recipePosX = 0; recipePosX < recipes[i]->width; recipePosX++)
        {
          for(uint32 recipePosY = 0; recipePosY < recipes[i]->height; recipePosY++)
          {
            if(slots[(recipePosY+offsetY)*width+recipePosX+1+offsetX].type != recipes[i]->slots[recipePosY*recipes[i]->height+recipePosX])
            {
              mismatch = true;
              break;
            }
          }
        }

        //Found match!
        if(!mismatch)
        {
          slots[0] = recipes[i]->output;
          return true;
        }

        offsetX++;
      } while(offsetX<=width-recipes[i]->width);

      offsetY++;
    } while(offsetY<=height-recipes[i]->height);
  }

  return false;
}

bool Inventory::setSlot(User *user, sint8 windowID, sint16 slot, sint16 itemID, sint8 count, sint16 health)
{
  Mineserver::get()->screen()->log(1,"Setslot: " + dtos(slot) + " to " + dtos(itemID) + " (" + dtos(count) + ") health: " + dtos(health));
  user->buffer << (sint8)PACKET_SET_SLOT << (sint8)windowID << (sint16)slot   << (sint16)itemID;
  if(itemID != -1)
  {
    user->buffer << (sint8)count << (sint8)health;
  }

  return true;
}