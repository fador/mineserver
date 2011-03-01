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

class Item
{
public:
  void setType(int16_t type);
  void setCount(int8_t count);
  void setHealth(int16_t healt);
  int16_t getType()
  {
    return type;
  }
  int8_t getCount()
  {
    return count;
  }
  int16_t getHealth()
  {
    return health;
  }
  int16_t itemHealth(int type);
  void decCount(int c = 1);
  void incHealth(int c = 1);
  void sendUpdate();
  bool ready;
private:
  int slot;
  User* player;
  int16_t type;
  int8_t count;
  int16_t health;
public:
  Item()
  {
    player = NULL;
    slot = -1;
    type   = -1;
    count  = 0;
    health = 0;
    ready = false;
  }
  Item(User* player, int slot)
  {
    this->player = player;
    this->slot = slot;
    type   = -1;
    count  = 0;
    health = 0;
    ready = false;
  }
};

struct OpenInventory
{
  int8_t type;
  int32_t x;
  int32_t y;
  int32_t z;
  Item workbench[10];
  std::vector<User*> users;
};

enum { WINDOW_CURSOR = -1, WINDOW_PLAYER = 0, WINDOW_WORKBENCH, WINDOW_CHEST, WINDOW_LARGE_CHEST, WINDOW_FURNACE };

enum { INVENTORYTYPE_CHEST = 0, INVENTORYTYPE_WORKBENCH, INVENTORYTYPE_FURNACE };

class User;

class Inventory
{
public:

  struct Recipe
  {
    Recipe() : width(0), height(0), slots(NULL) {}
    ~Recipe()
    {
    }

    int8_t width;
    int8_t height;
    std::vector<Item*> slots;
    Item output;
  };

  std::vector<Recipe*> recipes;
  bool addRecipe(int width, int height, std::vector<Item*> inputrecipe,
                 int outputCount, int16_t outputType, int16_t outputHealth);
  bool readRecipe(std::string recipeFile);

  Inventory();

  ~Inventory()
  {
    std::vector<Recipe*>::iterator it_a = recipes.begin();
    std::vector<Recipe*>::iterator it_b = recipes.end();
    for (; it_a != it_b; ++it_a)
    {
      delete *it_a;
    }
    recipes.clear();
  }

  // Open chest/workbench/furnace inventories
  std::vector<OpenInventory*> openWorkbenches;
  std::vector<OpenInventory*> openChests;
  std::vector<OpenInventory*> openFurnaces;

  bool canBeArmour(int slot, int type);
  bool onwindowOpen(User* user, int8_t type, int32_t x, int32_t y, int32_t z);
  bool onwindowClose(User* user, int8_t type, int32_t x, int32_t y, int32_t z);


  bool windowOpen(User* user, int8_t type, int32_t x, int32_t y, int32_t z);

  bool windowClick(User* user, int8_t windowID, int16_t slot, int8_t rightClick, int16_t actionNumber, int16_t itemID, int8_t itemCount, int16_t itemUses);

  bool windowClose(User* user, int8_t windowID);

  //Check inventory for space
  bool isSpace(User* user, int16_t itemID, char count);

  //Add items to inventory (pickups)
  bool addItems(User* user, int16_t itemID, int16_t count, int16_t health);

  bool doCraft(Item* slots, int8_t width, int8_t height);

  bool setSlot(User* user, int8_t windowID, int16_t slot, int16_t itemID, int8_t count, int16_t health);

  int16_t itemHealth(int16_t itemID, int8_t block, bool& rightUse);

};

#endif
