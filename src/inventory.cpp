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

Inventory::Inventory()
{
  Recipe *recipe;


  //sign
  recipe = new Recipe;
  recipe->width  = 3;
  recipe->height = 3;
  recipe->slots  = new sint16[recipe->width*recipe->height];
  sint16 inputsign[9] = { BLOCK_WOOD,BLOCK_WOOD,BLOCK_WOOD,
                          BLOCK_WOOD,BLOCK_WOOD,BLOCK_WOOD,
                            -1 ,     ITEM_STICK,     -1};
  recipe->output.count = 1;
  recipe->output.type  = ITEM_SIGN;
  recipe->output.health= 0;
  memcpy(recipe->slots,inputsign,recipe->width*recipe->height*sizeof(sint16));    


  recipes.push_back(recipe);



  //fence
  recipe = new Recipe;
  recipe->width  = 3;
  recipe->height = 2;
  recipe->slots  = new sint16[recipe->width*recipe->height];
  sint16 inputfence[6] = { ITEM_STICK,ITEM_STICK,ITEM_STICK,
                              ITEM_STICK,ITEM_STICK,ITEM_STICK};
  recipe->output.count = 2;
  recipe->output.type  = BLOCK_FENCE;
  recipe->output.health= 0;
  memcpy(recipe->slots,inputfence,recipe->width*recipe->height*sizeof(sint16));

  recipes.push_back(recipe);


  //ladders
  recipe = new Recipe;
  recipe->width  = 3;
  recipe->height = 3;
  recipe->slots  = new sint16[recipe->width*recipe->height];
  sint16 inputladders[9] = { ITEM_STICK,     -1   ,ITEM_STICK,
                            -1         ,ITEM_STICK,        -1,
                            ITEM_STICK ,-1,       ITEM_STICK};
  recipe->output.count = 1;
  recipe->output.type  = BLOCK_LADDER;
  recipe->output.health= 0;
  memcpy(recipe->slots,inputladders,recipe->width*recipe->height*sizeof(sint16));    


  recipes.push_back(recipe);

  //stone shovel
  recipe = new Recipe;
  recipe->width  = 1;
  recipe->height = 3;
  recipe->slots  = new sint16[recipe->width*recipe->height];
  sint16 inputshovel1[3] = { BLOCK_COBBLESTONE,
                              ITEM_STICK,
                              ITEM_STICK};
  recipe->output.count = 1;
  recipe->output.type  = ITEM_STONE_SPADE;
  recipe->output.health= 0;
  memcpy(recipe->slots,inputshovel1,recipe->width*recipe->height*sizeof(sint16));    


  recipes.push_back(recipe);

  //wooden shovel
  recipe = new Recipe;
  recipe->width  = 1;
  recipe->height = 3;
  recipe->slots  = new sint16[recipe->width*recipe->height];
  sint16 inputshovel2[3] = { BLOCK_WOOD,
                              ITEM_STICK,
                              ITEM_STICK};
  recipe->output.count = 1;
  recipe->output.type  = ITEM_WOODEN_SPADE;
  recipe->output.health= 0;
  memcpy(recipe->slots,inputshovel2,recipe->width*recipe->height*sizeof(sint16));    
  recipes.push_back(recipe);


  //stone pickaxe
  recipe = new Recipe;
  recipe->width  = 3;
  recipe->height = 3;
  recipe->slots  = new sint16[recipe->width*recipe->height];
  sint16 inputpickaxe1[9] = { BLOCK_COBBLESTONE,BLOCK_COBBLESTONE,BLOCK_COBBLESTONE,
                            -1             ,ITEM_STICK,              -1,
                            -1             ,ITEM_STICK,              -1};
  recipe->output.count = 1;
  recipe->output.type  = ITEM_STONE_PICKAXE;
  recipe->output.health= 0;
  memcpy(recipe->slots,inputpickaxe1,recipe->width*recipe->height*sizeof(sint16));    
  recipes.push_back(recipe);


  //wood pickaxe
  recipe = new Recipe;
  recipe->width  = 3;
  recipe->height = 3;
  recipe->slots  = new sint16[recipe->width*recipe->height];
  sint16 inputpickaxe2[9] = {    BLOCK_WOOD,BLOCK_WOOD,BLOCK_WOOD,
                                    -1     ,ITEM_STICK,     -1,
                                    -1     ,ITEM_STICK,     -1};
  recipe->output.count = 1;
  recipe->output.type  = ITEM_WOODEN_PICKAXE;
  recipe->output.health= 0;
  memcpy(recipe->slots,inputpickaxe2,recipe->width*recipe->height*sizeof(sint16));    
  recipes.push_back(recipe);



  //stone axe
  recipe = new Recipe;
  recipe->width  = 2;
  recipe->height = 3;
  recipe->slots  = new sint16[recipe->width*recipe->height];
  sint16 inputaxe2[6] = { BLOCK_COBBLESTONE,BLOCK_COBBLESTONE,
                          BLOCK_COBBLESTONE,ITEM_STICK,
                            -1             ,ITEM_STICK};
  recipe->output.count = 1;
  recipe->output.type  = ITEM_STONE_AXE;
  recipe->output.health= 0;
  memcpy(recipe->slots,inputaxe2,recipe->width*recipe->height*sizeof(sint16));    
  recipes.push_back(recipe);


  //wooden axe
  recipe = new Recipe;
  recipe->width  = 2;
  recipe->height = 3;
  recipe->slots  = new sint16[recipe->width*recipe->height];
  sint16 inputaxe1[6] = { BLOCK_WOOD,BLOCK_WOOD,
                          BLOCK_WOOD,ITEM_STICK,
                            -1     ,ITEM_STICK};
  recipe->output.count = 1;
  recipe->output.type  = ITEM_WOODEN_AXE;
  recipe->output.health= 0;
  memcpy(recipe->slots,inputaxe1,recipe->width*recipe->height*sizeof(sint16));    
  recipes.push_back(recipe);


  //wooden Stairs
  recipe = new Recipe;
  recipe->width  = 3;
  recipe->height = 3;
  recipe->slots  = new sint16[recipe->width*recipe->height];
  sint16 inputwoodstair[9] = { BLOCK_WOOD,-1,-1,
                                BLOCK_WOOD,BLOCK_WOOD,-1,
                                BLOCK_WOOD,BLOCK_WOOD,BLOCK_WOOD};
  recipe->output.count = 4;
  recipe->output.type  = BLOCK_WOODEN_STAIRS;
  recipe->output.health= 0;
  memcpy(recipe->slots,inputwoodstair,recipe->width*recipe->height*sizeof(sint16));    
  recipes.push_back(recipe);


  //Stone Stairs
  recipe = new Recipe;
  recipe->width  = 3;
  recipe->height = 3;
  recipe->slots  = new sint16[recipe->width*recipe->height];
  sint16 inputstonestair[9] = { BLOCK_STONE,-1,-1,
                                BLOCK_STONE,BLOCK_STONE,-1,
                                BLOCK_STONE,BLOCK_STONE,BLOCK_STONE};
  recipe->output.count = 4;
  recipe->output.type  = BLOCK_COBBLESTONE_STAIRS;
  recipe->output.health= 0;
  memcpy(recipe->slots,inputstonestair,recipe->width*recipe->height*sizeof(sint16));    
  recipes.push_back(recipe);


  //stone_Slab
  recipe = new Recipe;
  recipe->width  = 3;
  recipe->height = 1;
  recipe->slots  = new sint16[recipe->width*recipe->height];
  sint16 inputstoneslab[3] = { BLOCK_STONE,BLOCK_STONE,BLOCK_STONE};
  recipe->output.count = 4;
  recipe->output.type  = 44;
  recipe->output.health= 0;
  memcpy(recipe->slots,inputstoneslab,recipe->width*recipe->height*sizeof(sint16));    
  recipes.push_back(recipe);

  //Chest
  recipe = new Recipe;
  recipe->width  = 3;
  recipe->height = 3;
  recipe->slots  = new sint16[recipe->width*recipe->height];
  sint16 inputchest[9] = { BLOCK_WOOD,BLOCK_WOOD,BLOCK_WOOD,
                              BLOCK_WOOD,   -1     ,BLOCK_WOOD,
                              BLOCK_WOOD,BLOCK_WOOD,BLOCK_WOOD};
  recipe->output.count = 1;
  recipe->output.type  = BLOCK_CHEST;
  recipe->output.health= 0;
  memcpy(recipe->slots,inputchest,recipe->width*recipe->height*sizeof(sint16));    
  recipes.push_back(recipe);


  //Torches
  recipe = new Recipe;
  recipe->width  = 1;
  recipe->height = 2;
  recipe->slots  = new sint16[recipe->width*recipe->height];
  sint16 inputtorches[2] = { ITEM_COAL,ITEM_STICK };
  recipe->output.count = 4;
  recipe->output.type  = BLOCK_TORCH;
  recipe->output.health= 0;
  memcpy(recipe->slots,inputtorches,recipe->width*recipe->height*sizeof(sint16));    
  recipes.push_back(recipe);


  //Sticks
  recipe = new Recipe;
  recipe->width  = 1;
  recipe->height = 2;
  recipe->slots  = new sint16[recipe->width*recipe->height];
  sint16 inputstick[2] = { BLOCK_WOOD,BLOCK_WOOD };
  recipe->output.count = 4;
  recipe->output.type  = ITEM_STICK;
  recipe->output.health= 0;
  memcpy(recipe->slots,inputstick,recipe->width*recipe->height*sizeof(sint16));    
  recipes.push_back(recipe);



  Recipe *wood = new Recipe;
  wood->width  = 1;
  wood->height = 1;
  wood->slots  = new sint16[1];
  sint16 inputwood[1] = { BLOCK_LOG };
  wood->output.count = 4;
  wood->output.type  = BLOCK_WOOD;
  wood->output.health= 0;
  memcpy(wood->slots,inputwood,sizeof(sint16));    
  recipes.push_back(wood);


  Recipe *furnace = new Recipe;
  furnace->width  = 3;
  furnace->height = 3;
  furnace->slots  = new sint16[9];
  sint16 input[9] = { 4, 4, 4,
                      4,-1, 4,
                      4, 4, 4 };
  furnace->output.count = 1;
  furnace->output.type  = BLOCK_FURNACE;
  furnace->output.health= 0;
  memcpy(furnace->slots,input,9*sizeof(sint16));    
  recipes.push_back(furnace);

  Recipe *workbench = new Recipe;
  workbench->width  = 2;
  workbench->height = 2;
  workbench->slots  = new sint16[4];
  sint16 inputWorkbench[4] = { 5, 5,
                                5, 5 };
                        
  workbench->output.count = 1;
  workbench->output.type  = BLOCK_WORKBENCH;
  workbench->output.health= 0;
  memcpy(workbench->slots,inputWorkbench,4*sizeof(sint16));
    
  recipes.push_back(workbench);

  Recipe *grass = new Recipe;
  grass->width  = 2;
  grass->height = 2;
  grass->slots  = new sint16[4];
  sint16 input2[4] = { 3, 3,
                        3, 3};
  grass->output.count = 1;
  grass->output.type  = BLOCK_GRASS;
  grass->output.health= 0;
  memcpy(grass->slots,input2,4*sizeof(sint16));

  recipes.push_back(grass);

}


bool Inventory::windowClick(User *user,sint8 windowID, sint16 slot, sint8 rightClick, sint16 actionNumber, sint16 itemID, sint8 itemCount,sint8 itemUses)
{  
  //Ack
  user->buffer << (sint8)PACKET_TRANSACTION << (sint8)windowID << (sint16)actionNumber << (sint8)1;

  //Mineserver::get()->screen()->log(1,"window: " + dtos(windowID) + " slot: " + dtos(slot) + " (" + dtos(actionNumber) + ") itemID: " + dtos(itemID));
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

  sChunk* chunk = NULL;
  if(windowID != 0)
  {
    chunk = Mineserver::get()->map()->chunks.GetChunk(blockToChunk(user->openInv.x),blockToChunk(user->openInv.z));

    if(chunk == NULL)
    {
      return false;
    }

    chunk->changed = true;
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

    if(otherUsers == NULL || currentInventory == NULL)
    {
      return false;
    }
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
  
  bool workbenchCrafting = false;
  bool playerCrafting    = false;

  //Empty slot and holding something
  if((itemID == -1 || (slotItem->type == itemID && slotItem->count < 64) ) && user->inventoryHolding.type != -1)
  {
    //If accessing crafting output slot
    if(slotItem->type != -1 && (windowID == WINDOW_WORKBENCH || windowID == WINDOW_PLAYER) && slot == 0)
    {
      if(user->inventoryHolding.type == slotItem->type && 64-user->inventoryHolding.count >= slotItem->count)
      {
        user->inventoryHolding.count += slotItem->count;
        if(windowID == WINDOW_WORKBENCH)
        {
          for(uint8 workbenchSlot = 1; workbenchSlot < 10; workbenchSlot++)
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
          for(uint8 playerSlot = 1; playerSlot < 5; playerSlot++)
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
      setSlot(user, WINDOW_CURSOR, 0, user->inventoryHolding.type, user->inventoryHolding.count, user->inventoryHolding.health);
    }
    else
    {
      //ToDo: Make sure we have room for the items!

      sint16 addCount = (64-slotItem->count>=user->inventoryHolding.count)?user->inventoryHolding.count:64-slotItem->count;

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
        for(uint8 workbenchSlot = 1; workbenchSlot < 10; workbenchSlot++)
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
        for(uint8 playerSlot = 1; playerSlot < 5; playerSlot++)
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

      setSlot(user, WINDOW_CURSOR, 0, user->inventoryHolding.type, user->inventoryHolding.count, user->inventoryHolding.health);
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
    
  
  //Update slot
  setSlot(user, windowID, slot, slotItem->type, slotItem->count, slotItem->health);

  //Update item on the cursor
  setSlot(user, WINDOW_CURSOR, 0, user->inventoryHolding.type, user->inventoryHolding.count, user->inventoryHolding.health);


  //Check if crafting
  if(windowID == WINDOW_WORKBENCH && slot < 10 && slot > 0 || workbenchCrafting)
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
  else if(windowID == WINDOW_PLAYER && slot < 5 && slot > 0 || playerCrafting)
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
          for(uint32 craftingPosX = 0; craftingPosX < width; craftingPosX++)
          {
            for(uint32 craftingPosY = 0; craftingPosY < height; craftingPosY++)
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

bool Inventory::setSlot(User *user, sint8 windowID, sint16 slot, sint16 itemID, sint8 count, sint16 health)
{
  //Mineserver::get()->screen()->log(1,"Setslot: " + dtos(slot) + " to " + dtos(itemID) + " (" + dtos(count) + ") health: " + dtos(health));
  user->buffer << (sint8)PACKET_SET_SLOT << (sint8)windowID << (sint16)slot   << (sint16)itemID;
  if(itemID != -1)
  {
    user->buffer << (sint8)count << (sint8)health;
  }

  return true;
}