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
#include <fstream>
#include <vector>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <algorithm>

#include <zlib.h>
#include <sys/stat.h>

#include "inventory.h"
#include "constants.h"
#include "map.h"
#include "user.h"
#include "mineserver.h"
#include "logger.h"
#include "tools.h"

Inventory::Inventory()
{
  std::ifstream ifs("recipes/ENABLED_RECIPES.cfg");

  if(ifs.fail())
  {
    ifs.close();
    return;
  }

  std::string temp;
  std::vector<std::string> receiptFiles;
  std::string text;
  while(getline(ifs, temp))
  {
    //If empty line
    if(temp.size() == 0)
      continue;

    // If commentline -> skip to next
    if(temp[0] == COMMENTPREFIX)
      continue;
    
    receiptFiles.push_back(temp + ".recipe");
  }
  ifs.close();
  
  for(unsigned int i = 0; i < receiptFiles.size(); i++)
  {
    readRecipe("recipes/" + receiptFiles[i]);
  }
}

bool Inventory::addRecipe(int width, int height, int16_t* inputrecipe, int outputCount, int16_t outputType, int16_t outputHealth)
{
  Recipe *recipe = new Recipe;

  recipe->width  = width;
  recipe->height = height;
  recipe->slots  = new int16_t[width*height];
  recipe->output.count  = outputCount;
  recipe->output.type   = outputType;
  recipe->output.health = outputHealth;

  memcpy(recipe->slots, inputrecipe, width*height*sizeof(int16_t));

  recipes.push_back(recipe);

  return true;
}

bool Inventory::readRecipe(std::string recipeFile)
{
  std::ifstream ifs(recipeFile.c_str());

  if (ifs.fail())
  {
    LOG(WARNING, "Inventory", "Could not find: " + recipeFile);
    ifs.close();
    return false;
  }

  //LOG(INFO, "Inventory", "Reading: " + recipeFile);

  std::string temp;
  
  int height = 0, width = 0, outCount = 0;
  int16_t outType = 0, outHealth = 0;

  // Reading row at a time
  int del;
  bool readingRecipe = false;
  std::vector<std::string> line;
  std::vector<int16_t> recipetable;
  std::string text;
  while(getline(ifs, temp))
  {
    //If empty line
    if(temp.size() == 0)
      continue;

    // If commentline -> skip to next
    if(temp[0] == COMMENTPREFIX)
      continue;

    // Init vars
    del = 0;
    line.clear();

    // Process line
    while(temp.length() > 0)
    {
      // Remove white spaces
      while(temp[0] == ' ')
        temp = temp.substr(1);

      // Split words
      del = temp.find(' ');
      if(del > -1)
      {
        line.push_back(temp.substr(0, del));
        temp = temp.substr(del+1);
      }
      else
      {
        line.push_back(temp);
        break;
      }
    }

    // Begin recipe
    if(line.size() == 1 && line[0] == "<-")
    {
      readingRecipe = true;
      continue;
    }
    // Begin recipe
    if(line.size() == 1 && line[0] == "->")
    {
      readingRecipe = false;
      continue;
    }

    if(readingRecipe)
    {
      for(unsigned int i = 0; i < line.size(); i++)
      {
        recipetable.push_back(atoi(line[i].c_str()));
      }
      continue;
    }
    else
    {
      // Keywords
      if (line[0] == "width")
      {
        width = atoi(line[1].c_str());
      }
      if (line[0] == "height")
      {
        height = atoi(line[1].c_str());
      }
      if (line[0] == "outputcount")
      {
        outCount = atoi(line[1].c_str());
      }
      if (line[0] == "outputtype")
      {
        outType = atoi(line[1].c_str());
      }
      if (line[0] == "outputhealth")
      {
        outHealth = atoi(line[1].c_str());
      }
    }
  }
  ifs.close();
  
  int16_t* inrecipe = new int16_t[height*width];
  for (unsigned int i = 0; i < recipetable.size(); i++)
  {
    inrecipe[i] = recipetable[i];
  }

  addRecipe(width, height, inrecipe, outCount, outType, outHealth);

  delete [] inrecipe;
  
  return true;
}

bool Inventory::windowClick(User *user,int8_t windowID, int16_t slot, int8_t rightClick, int16_t actionNumber, int16_t itemID, int8_t itemCount,int16_t itemUses)
{  
  //Ack
  user->buffer << (int8_t)PACKET_TRANSACTION << (int8_t)windowID << (int16_t)actionNumber << (int8_t)1;

  //Mineserver::get()->logger()->log(1,"window: " + dtos(windowID) + " slot: " + dtos(slot) + " (" + dtos(actionNumber) + ") itemID: " + dtos(itemID));
  //Click outside the window
  if(slot == -999)
  {
    if(user->inventoryHolding.type != -1)
    {
      Mineserver::get()->map(user->pos.map)->createPickupSpawn((int)user->pos.x, (int)user->pos.y, (int)user->pos.z, 
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

  sChunk* chunk = NULL;
  if(windowID != 0)
  {
    chunk = Mineserver::get()->map(user->pos.map)->chunks.getChunk(blockToChunk(user->openInv.x),blockToChunk(user->openInv.z));

    if(chunk == NULL)
    {
      return false;
    }

    chunk->changed = true;
  }

  std::vector<User*>* otherUsers = NULL;
  OpenInventory* currentInventory = NULL;

  if(windowID != WINDOW_PLAYER)
  {
    std::vector<OpenInventory*>* inv = NULL;
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

    for(uint32_t i = 0; i < inv->size(); i++)
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

    if(otherUsers == NULL || currentInventory == NULL)
    {
      return false;
    }
  }

  Item* slotItem = NULL;

  switch(windowID)
  {
     //Player inventory
    case WINDOW_PLAYER:
      slotItem=&user->inv[slot];
      break;
    case WINDOW_CHEST:
      if(slot>26)
      {
        slotItem=&user->inv[slot-18];
      }
      else
      {
        for(uint32_t i = 0; i < chunk->chests.size(); i ++)
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
      if(slot>54)
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
        for(uint32_t i = 0; i < chunk->furnaces.size(); i ++)
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
  
  bool workbenchCrafting = false;
  bool playerCrafting    = false;

  //Empty slot and holding something
  if((itemID == -1 || (slotItem->type == user->inventoryHolding.type && slotItem->count < 64) ) && user->inventoryHolding.type != -1)
  {
    //If accessing crafting output slot
    if(slotItem->type != -1 && (windowID == WINDOW_WORKBENCH || windowID == WINDOW_PLAYER) && slot == 0)
    {
      if(user->inventoryHolding.type == slotItem->type && 64-user->inventoryHolding.count >= slotItem->count)
      {
        user->inventoryHolding.count += slotItem->count;
        if(windowID == WINDOW_WORKBENCH)
        {
          for(uint8_t workbenchSlot = 1; workbenchSlot < 10; workbenchSlot++)
          {
            if(currentInventory->workbench[workbenchSlot].type != -1)
            {
              currentInventory->workbench[workbenchSlot].count --;
              if(currentInventory->workbench[workbenchSlot].count == 0)
              {
                currentInventory->workbench[workbenchSlot] = Item();
              }
              setSlot(user, windowID, workbenchSlot, currentInventory->workbench[workbenchSlot].type, 
                                                     currentInventory->workbench[workbenchSlot].count, 
                                                     currentInventory->workbench[workbenchSlot].health);
            }
          }
          workbenchCrafting = true;
        }
        else
        {
          for(uint8_t playerSlot = 1; playerSlot < 5; playerSlot++)
          {
            if(user->inv[playerSlot].type != -1)
            {
              user->inv[playerSlot].count --;
              if(user->inv[playerSlot].count == 0)
              {
                user->inv[playerSlot] = Item();
              }
              setSlot(user, windowID, playerSlot, user->inv[playerSlot].type, 
                                                  user->inv[playerSlot].count, 
                                                  user->inv[playerSlot].health);
            }
          }
          playerCrafting = true;
        }
      }
    }
    else
    {
      //ToDo: Make sure we have room for the items!

      //Make sure not putting anything to the crafting space
      if((windowID != WINDOW_WORKBENCH && windowID != WINDOW_PLAYER) || slot != 0)
      {
        int16_t addCount = (64-slotItem->count>=user->inventoryHolding.count)?user->inventoryHolding.count:64-slotItem->count;

        slotItem->count  += ((rightClick)?1:addCount);
        slotItem->health  = user->inventoryHolding.health;
        slotItem->type    = user->inventoryHolding.type;

        user->inventoryHolding.count -= ((rightClick)?1:addCount);
        if(user->inventoryHolding.count == 0)
        {
          user->inventoryHolding.type  = -1;
          user->inventoryHolding.health= 0;
        }
      }
    }
    
  }
  else if(user->inventoryHolding.type == -1)
  {
    //If accessing crafting output slot, remove from input!
    if(slotItem->type != -1 && (windowID == WINDOW_WORKBENCH || windowID == WINDOW_PLAYER) && slot == 0)
    {
      user->inventoryHolding.type = slotItem->type;
      user->inventoryHolding.count = slotItem->count;
      user->inventoryHolding.health = slotItem->health;

      if(windowID == WINDOW_WORKBENCH)
      {
        for(uint8_t workbenchSlot = 1; workbenchSlot < 10; workbenchSlot++)
        {
          if(currentInventory->workbench[workbenchSlot].type != -1)
          {
            currentInventory->workbench[workbenchSlot].count --;
            if(currentInventory->workbench[workbenchSlot].count == 0)
            {
              currentInventory->workbench[workbenchSlot] = Item();
            }

            setSlot(user, windowID, workbenchSlot,currentInventory->workbench[workbenchSlot].type, 
                                                  currentInventory->workbench[workbenchSlot].count, 
                                                  currentInventory->workbench[workbenchSlot].health);
          }
        }
        workbenchCrafting = true;
      }
      else
      {
        for(uint8_t playerSlot = 1; playerSlot < 5; playerSlot++)
        {
          if(user->inv[playerSlot].type != -1)
          {
            user->inv[playerSlot].count --;
            if(user->inv[playerSlot].count == 0)
            {
              user->inv[playerSlot] = Item();
            }
            setSlot(user, windowID, playerSlot, user->inv[playerSlot].type, 
                                                user->inv[playerSlot].count, 
                                                user->inv[playerSlot].health);
          }
        }
        playerCrafting = true;
      }
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

      slotItem->count  -= user->inventoryHolding.count;
      if(slotItem->count == 0)
      {
        slotItem->health = 0;
        slotItem->type   =-1;
      }      
    }
  }
  else
  {
    //Swap items if holding something and clicking another, not with craft slot
    if((windowID != WINDOW_WORKBENCH && windowID != WINDOW_PLAYER) || slot != 0)
    {
      Item tempItem            = user->inventoryHolding;
      user->inventoryHolding   = (*slotItem);
      *slotItem                = tempItem;
    }      
  }
    
  
  //Update slot
  setSlot(user, windowID, slot, slotItem->type, slotItem->count, slotItem->health);

  //Update item on the cursor
  setSlot(user, WINDOW_CURSOR, 0, user->inventoryHolding.type, user->inventoryHolding.count, user->inventoryHolding.health);


  //Check if crafting
  if((windowID == WINDOW_WORKBENCH && slot < 10 && slot > 0) || workbenchCrafting)
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
  else if((windowID == WINDOW_PLAYER && slot < 5 && slot > 0) || playerCrafting)
  {
    if(doCraft(user->inv, 2, 2))
    {
      setSlot(user, windowID, 0, user->inv[0].type, user->inv[0].count, user->inv[0].health);
    }
    else
    {
      user->inv[0] = Item();
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
        for(uint32_t i = 0; i < otherUsers->size(); i++)
        {          
          (*otherUsers)[i]->buffer << (int8_t)PACKET_SET_SLOT << (int8_t)windowID << (int16_t)slot << (int16_t)slotItem->type;
          if(slotItem->type != -1)
          {
            (*otherUsers)[i]->buffer << (int8_t)slotItem->count << (int16_t)slotItem->health;
          }
        }
      }
      break;

    case WINDOW_CHEST:
      chunk->changed = true;
      if(slot < 27)        
      {
        for(uint32_t i = 0; i < otherUsers->size(); i++)
        {
          if((*otherUsers)[i] != user)
          {
            (*otherUsers)[i]->buffer << (int8_t)PACKET_SET_SLOT << (int8_t)windowID << (int16_t)slot << (int16_t)slotItem->type;
            if(slotItem->type != -1)
            {
              (*otherUsers)[i]->buffer << (int8_t)slotItem->count << (int16_t)slotItem->health;
            }
          }
        }
      }
      break;

    case WINDOW_FURNACE:
      chunk->changed = true;
      if(slot < 3)        
      {
        for(uint32_t i = 0; i < otherUsers->size(); i++)
        {
          if((*otherUsers)[i] != user)
          {
            (*otherUsers)[i]->buffer << (int8_t)PACKET_SET_SLOT << (int8_t)windowID << (int16_t)slot << (int16_t)slotItem->type;
            if(slotItem->type != -1)
            {
              (*otherUsers)[i]->buffer << (int8_t)slotItem->count << (int16_t)slotItem->health;
            }
          }
        }
      }
      break;
  }
  */


  return true;
}


bool Inventory::windowOpen(User *user, int8_t type, int32_t x, int32_t y, int32_t z)
{
  sChunk* chunk = Mineserver::get()->map(user->pos.map)->chunks.getChunk(blockToChunk(x),blockToChunk(z));

  if(chunk == NULL)
  {
    return false;
  }

  onwindowOpen(user,type,x,y,z);

  switch(type)
  {
    case WINDOW_CHEST:    
      user->buffer << (int8_t)PACKET_OPEN_WINDOW << (int8_t)WINDOW_CHEST  << (int8_t)INVENTORYTYPE_CHEST << std::string("Chest") << (int8_t)27;

      for(uint32_t i = 0;i < chunk->chests.size(); i++)
      {
        if(chunk->chests[i]->x == x && chunk->chests[i]->y == y && chunk->chests[i]->z == z)
        {
          for(int j = 0;j < 27; j++)
          {
            if(chunk->chests[i]->items[j].type != -1)
            {
              user->buffer << (int8_t)PACKET_SET_SLOT << (int8_t)WINDOW_CHEST << (int16_t)j << (int16_t)chunk->chests[i]->items[j].type 
                           << (int8_t)(chunk->chests[i]->items[j].count) << (int16_t)chunk->chests[i]->items[j].health;
            }
          }
          break;
        }
      }
      break;
    case WINDOW_WORKBENCH:
      user->buffer << (int8_t)PACKET_OPEN_WINDOW << (int8_t)WINDOW_WORKBENCH  << (int8_t)INVENTORYTYPE_WORKBENCH << std::string("Workbench") << (int8_t)0;

      for(uint32_t i = 0; i < openWorkbenches.size(); i++)
      {
        if(openWorkbenches[i]->x == user->openInv.x &&
           openWorkbenches[i]->y == user->openInv.y &&
           openWorkbenches[i]->z == user->openInv.z)
        {
          for(int j = 0; j < 10; j++)
          {
            if(openWorkbenches[i]->workbench[j].type != -1)
            {
              user->buffer << (int8_t)PACKET_SET_SLOT << (int8_t)WINDOW_WORKBENCH << (int16_t)j << (int16_t)openWorkbenches[i]->workbench[j].type 
                           << (int8_t)(openWorkbenches[i]->workbench[j].count) << (int16_t)openWorkbenches[i]->workbench[j].health;
            }
          }
          break;
        }
      }
      break;
    case WINDOW_FURNACE:
      
      user->buffer << (int8_t)PACKET_OPEN_WINDOW << (int8_t)WINDOW_FURNACE  << (int8_t)INVENTORYTYPE_FURNACE << std::string("Furnace") << (int8_t)0;

      for(uint32_t i = 0;i < chunk->furnaces.size(); i++)
      {
        if(chunk->furnaces[i]->x == x && chunk->furnaces[i]->y == y && chunk->furnaces[i]->z == z)
        {
          for(int j = 0; j < 3; j++)
          {
            if(chunk->furnaces[i]->items[j].type != -1)
            {
              user->buffer << (int8_t)PACKET_SET_SLOT << (int8_t)WINDOW_FURNACE << (int16_t)j << (int16_t)chunk->furnaces[i]->items[j].type 
                           << (int8_t)(chunk->furnaces[i]->items[j].count) << (int16_t)chunk->furnaces[i]->items[j].health;
            }
          }
          break;
        }
      }
      break;
  }

  return true;
}

bool Inventory::isSpace(User *user,int16_t itemID, char count)
{
  int leftToFit = count;
  for(uint8_t i = 0; i < 36; i++)
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


bool Inventory::addItems(User *user,int16_t itemID, int16_t count, int16_t health)
{
  bool checkingTaskbar = true;

  for(uint8_t i = 36-9; i < 36-9 || checkingTaskbar; i++)
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
      user->buffer << (int8_t)PACKET_SET_SLOT << (int8_t)0 << (int16_t)(i+9) << (int16_t)itemID << (int8_t)count << (int16_t)health;
      slot->type   = itemID;
      slot->count  = count;
      slot->health = health;
      break;
    }

    //If same item type
    if(slot->type == itemID)
    {
      if(slot->health == health){
        //Put to the stack
        if(64-slot->count >= count)
        {
          user->buffer << (int8_t)PACKET_SET_SLOT << (int8_t)0 << (int16_t)(i+9) << (int16_t)itemID << (int8_t)(slot->count+count) << (int16_t)health;
          slot->type   = itemID;
          slot->count += count;
          break;
        }
      //Put some of the items to this stack and continue searching for space
        else if(64-slot->count > 0)
        {
          user->buffer << (int8_t)PACKET_SET_SLOT << (int8_t)0 << (int16_t)(i+9) << (int16_t)itemID << (int8_t)64 << (int16_t)health;
          slot->type = itemID;
          slot->count = 64;
          count -= 64-slot->count;
        }
      }
    }
  }

  return true;
}

bool Inventory::windowClose(User *user,int8_t windowID)
{
  //If still holding something, dump the items to ground
  if(user->inventoryHolding.type != -1)
  {
    Mineserver::get()->map(user->pos.map)->createPickupSpawn((int)user->pos.x, (int)user->pos.y, (int)user->pos.z, 
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

bool Inventory::onwindowOpen(User *user,int8_t type, int32_t x, int32_t y, int32_t z)
{
  std::vector<OpenInventory *> *inv;
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
  for(uint32_t i = 0; i < inv->size(); i++)
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
  OpenInventory *newInv = new OpenInventory();
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

bool Inventory::onwindowClose(User *user, int8_t type, int32_t x, int32_t y, int32_t z)
{
  std::vector<OpenInventory*>* inv = NULL;

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
  default:
    return false;
  }

  for(uint32_t i = 0; i < inv->size(); i++)
  {
    if((*inv)[i]->x == user->openInv.x &&
       (*inv)[i]->y == user->openInv.y &&
       (*inv)[i]->z == user->openInv.z)
    {
      for(uint32_t j = 0; j < (*inv)[i]->users.size(); j++)
      {
        if((*inv)[i]->users[j] == user)
        {
          (*inv)[i]->users.erase((*inv)[i]->users.begin()+j);

          if((*inv)[i]->users.size() == 0)
          {
            //Dump stuff to ground if workbench and no other users
            if(type == WINDOW_WORKBENCH)
            {
              for(uint32_t slotNumber = 1; slotNumber < 10; slotNumber ++)
              {
                if((*inv)[i]->workbench[slotNumber].type != -1)
                {
                  Mineserver::get()->map(user->pos.map)->createPickupSpawn((int)user->pos.x, (int)user->pos.y, (int)user->pos.z, 
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




bool Inventory::doCraft(Item *slots, int8_t width, int8_t height)
{
  for(uint32_t i = 0; i < recipes.size(); i++)
  {
    //Skip if recipe doesn't fit
    if(width < recipes[i]->width || height < recipes[i]->height)
    {
      continue;
    }
    
    int8_t offsetX = 0, offsetY = 0;

    //Check for any possible position the recipe would fit
    do
    {
      offsetX = 0;
      do
      {
        bool mismatch = false;
        //Check for the recipe match on this position
        for(int32_t recipePosX = 0; recipePosX < recipes[i]->width; recipePosX++)
        {
          for(int32_t recipePosY = 0; recipePosY < recipes[i]->height; recipePosY++)
          {
            if(slots[(recipePosY+offsetY)*width+recipePosX+1+offsetX].type != recipes[i]->slots[recipePosY*recipes[i]->width+recipePosX])
            {
              mismatch = true;
              break;
            }
          }
        }

        //Found match!
        if(!mismatch)
        {
          //Check that other areas are empty!
          bool foundItem = false;
          for(int32_t craftingPosX = 0; craftingPosX < width; craftingPosX++)
          {
            for(int32_t craftingPosY = 0; craftingPosY < height; craftingPosY++)
            {
              //If not inside the recipe boundaries
              if(craftingPosX < offsetX || craftingPosX>=offsetX+recipes[i]->width ||
                 craftingPosY < offsetY || craftingPosY>=offsetY+recipes[i]->height)
              {
                if(slots[(craftingPosY)*width+craftingPosX+1].type != -1)
                {
                  foundItem = true;
                  break;
                }
              }
            }
          }
          if(!foundItem)
          {
            slots[0] = recipes[i]->output;
            return true;
          }
        }

        offsetX++;
      } while(offsetX<=width-recipes[i]->width);

      offsetY++;
    } while(offsetY<=height-recipes[i]->height);
  }

  return false;
}

bool Inventory::setSlot(User *user, int8_t windowID, int16_t slot, int16_t itemID, int8_t count, int16_t health)
{
  //Mineserver::get()->logger()->log(1,"Setslot: " + dtos(slot) + " to " + dtos(itemID) + " (" + dtos(count) + ") health: " + dtos(health));
  user->buffer << (int8_t)PACKET_SET_SLOT << (int8_t)windowID << (int16_t)slot   << (int16_t)itemID;
  if(itemID != -1)
  {
    user->buffer << (int8_t)count << (int16_t)health;
  }

  return true;
}


int16_t Inventory::itemHealth(int16_t itemID, int8_t block, bool &rightUse)
{
  int16_t health=0;
  rightUse = false;
  switch(itemID)
  {
    case ITEM_GOLD_AXE:
    case ITEM_GOLD_PICKAXE:
    case ITEM_GOLD_HOE:
    case ITEM_GOLD_SPADE:
    case ITEM_WOODEN_AXE:
    case ITEM_WOODEN_PICKAXE:
    case ITEM_WOODEN_HOE:
    case ITEM_WOODEN_SPADE:
      health = 32;
    break;
    case ITEM_STONE_AXE:
    case ITEM_STONE_PICKAXE:
    case ITEM_STONE_HOE:
    case ITEM_STONE_SPADE:
      health = 64;
    break;
    case ITEM_IRON_AXE:
    case ITEM_IRON_PICKAXE:
    case ITEM_IRON_HOE:
    case ITEM_IRON_SPADE:
      health = 128;
    break;
    case ITEM_DIAMOND_AXE:
    case ITEM_DIAMOND_PICKAXE:
    case ITEM_DIAMOND_HOE:
    case ITEM_DIAMOND_SPADE:
      health = 1024;
    break;

    default:
      health = 0;
  }


  switch(itemID)
  {
    case ITEM_WOODEN_AXE:
    case ITEM_GOLD_AXE:
    case ITEM_STONE_AXE:
    case ITEM_IRON_AXE:
    case ITEM_DIAMOND_AXE:
     if(block == BLOCK_WOOD || block == BLOCK_PLANK)
     {
       rightUse = true;
     }
    break;
    
    case ITEM_WOODEN_PICKAXE:
    case ITEM_STONE_PICKAXE:
    case ITEM_GOLD_PICKAXE:
    case ITEM_IRON_PICKAXE:
    case ITEM_DIAMOND_PICKAXE:
     switch(block)
     {
       case BLOCK_STONE:
       case BLOCK_COBBLESTONE:
       case BLOCK_MOSSY_COBBLESTONE:
       case BLOCK_COAL_ORE:
       case BLOCK_IRON_ORE:
       case BLOCK_GOLD_ORE:
       case BLOCK_DIAMOND_ORE:
       case BLOCK_OBSIDIAN:
       case BLOCK_GLOWSTONE:
       case BLOCK_NETHERSTONE:
       case BLOCK_WOODEN_STAIRS:
       case BLOCK_COBBLESTONE_STAIRS:
       case BLOCK_IRON_DOOR:
       case BLOCK_ICE:
        rightUse = true;
       break;
       default:
         rightUse = false;
     }
    break;
    
    case ITEM_WOODEN_HOE:
    case ITEM_GOLD_HOE:
    case ITEM_STONE_HOE:
    case ITEM_IRON_HOE:
    case ITEM_DIAMOND_HOE:
      //ToDo: add this
      rightUse = false;
    break;
          
    case ITEM_WOODEN_SPADE:
    case ITEM_GOLD_SPADE:          
    case ITEM_STONE_SPADE:
    case ITEM_IRON_SPADE:
    case ITEM_DIAMOND_SPADE:
     switch(block)
     {
       case BLOCK_DIRT:
       case BLOCK_GRASS:
       case BLOCK_SAND:
       case BLOCK_GRAVEL:
       case BLOCK_CLAY:
       case BLOCK_SNOW:
        rightUse = true;
       break;
       default:
         rightUse = false;
     }
    break;

    default:
      health = 0;
  }
  return health;
}
