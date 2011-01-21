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

#ifndef _INVENTORY_H_
#define _INVENTORY_H_

#include <stdint.h>
#include <vector>

class User;

struct Item
{
  int16_t type;
  int8_t count;
  int16_t health;
  Item()
  {
    type   = -1;
    count  = 0;
    health = 0;
  }
};

struct OpenInventory
{
  int8_t type;
  int32_t x;
  int32_t y;
  int32_t z;
  Item workbench[10];
  std::vector<User *> users;  
};

enum { WINDOW_CURSOR = -1, WINDOW_PLAYER = 0, WINDOW_WORKBENCH, WINDOW_CHEST, WINDOW_LARGE_CHEST, WINDOW_FURNACE };

enum { INVENTORYTYPE_CHEST = 0,INVENTORYTYPE_WORKBENCH, INVENTORYTYPE_FURNACE };

class User;

class Inventory
{
public:

  struct Recipe
  {
    Recipe() : width(0),height(0),slots(NULL) {}
    ~Recipe()
    {
      delete [] slots;
    }

    int8_t width;
    int8_t height;
    int16_t *slots;
    Item output;
  };
  
  std::vector<Recipe*> recipes;
  bool addRecipe(int width, int height, int16_t* inputrecipe, int outputCount, 
                 int16_t outputType, int16_t outputHealth);
  bool readRecipe(std::string recipeFile);

  Inventory();

  ~Inventory()
  {
    std::vector<Recipe*>::iterator it_a = recipes.begin();
    std::vector<Recipe*>::iterator it_b = recipes.end();
    for(;it_a!=it_b;++it_a)
    {
      delete *it_a;
    }
    recipes.clear();
  }

  // Open chest/workbench/furnace inventories
  std::vector<OpenInventory *> openWorkbenches;
  std::vector<OpenInventory *> openChests;
  std::vector<OpenInventory *> openFurnaces;

  bool onwindowOpen(User *user,int8_t type, int32_t x, int32_t y, int32_t z);
  bool onwindowClose(User *user,int8_t type,int32_t x, int32_t y, int32_t z);


  bool windowOpen(User *user, int8_t type, int32_t x, int32_t y, int32_t z);

  bool windowClick(User *user,int8_t windowID, int16_t slot, int8_t rightClick, int16_t actionNumber, int16_t itemID, int8_t itemCount,int16_t itemUses);

  bool windowClose(User *user,int8_t windowID);
    
  //Check inventory for space
  bool isSpace(User *user, int16_t itemID, char count);

  //Add items to inventory (pickups)
  bool addItems(User *user, int16_t itemID, int16_t count, int16_t health);

  bool doCraft(Item *slots, int8_t width, int8_t height);

  bool setSlot(User *user, int8_t windowID, int16_t slot, int16_t itemID, int8_t count, int16_t health);

  int16_t itemHealth(int16_t itemID, int8_t block, bool &rightUse);

};

#endif
