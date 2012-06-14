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


#include <fstream>
#include <stdio.h>
#include <stdlib.h>

#include "inventory.h"
#include "protocol.h"
#include "constants.h"
#include "map.h"
#include "user.h"
#include "mineserver.h"
#include "furnaceManager.h"
#include "logger.h"
#include <sstream>


void Item::sendUpdate()
{
  if (player != NULL && ready)
  {
    if (slot == player->curItem + 36)
    {
      Packet pkt;
      pkt << (int8_t)PACKET_ENTITY_EQUIPMENT << (int32_t)player->UID
          << (int16_t)0 << (int16_t)type << (int16_t) health;
      player->sendAll(pkt);
    }
    if (slot >= 5 && slot <= 8)
    {
      Packet pkt;
      pkt << (int8_t)PACKET_ENTITY_EQUIPMENT << (int32_t)player->UID
          << (int16_t)(5 - (slot - 4)) << (int16_t)type << (int16_t) 0;
      player->sendAll(pkt);
    }
    int window = 0;
    int t_slot = slot;
    if (slot == -1)
    {
      window = -1;
      t_slot = 0;
    }
    player->buffer << Protocol::setSlotHeader(window, t_slot)
                   << Protocol::slot(type, count, health);
  }
  // Cases where we're changing items in chests, furnaces etc?
}

void Item::setType(int16_t type)
{
  this->type = type;
  if (type == -1)
  {
    count = 0;
    health = 0;
  }

  sendUpdate();
}

void Item::setCount(int8_t count)
{
  this->count = count;
  if (count < 1)
  {
    setType(-1);
    return;
  }
  sendUpdate();
}

void Item::setHealth(int16_t health)
{
  bool rightUse;
  if (health <= 0)
  {
    this->health = health;
    return;
  }
  int healthMax = itemHealth(type);
  if (health > healthMax && healthMax > 0)
  {
    type = -1;
    count = 0;
    this->health = 0;
    sendUpdate();
  }
  else
  {
    this->health = health;
    sendUpdate();
  }
}

void Item::decCount(int c)
{
  count -= c;
  if (count < 1)
  {
    setType(-1);
    return;
  }
  sendUpdate();
}

void Item::incHealth(int c)
{
  int healthMax = itemHealth(type);
  health += c;
  if (health > healthMax && healthMax > 0)
  {
    setType(-1);
  }
  sendUpdate();
}

int16_t Item::itemHealth(int item)
{
  int16_t health = 0;
  switch (type)
  {
  case ITEM_GOLD_AXE:
  case ITEM_GOLD_PICKAXE:
  case ITEM_GOLD_HOE:
  case ITEM_GOLD_SPADE:
  case ITEM_GOLD_SWORD:
    health = 33;
    break;
  case ITEM_WOODEN_AXE:
  case ITEM_WOODEN_PICKAXE:
  case ITEM_WOODEN_HOE:
  case ITEM_WOODEN_SPADE:
  case ITEM_WOODEN_SWORD:
    health = 60;
    break;
  case ITEM_STONE_AXE:
  case ITEM_STONE_PICKAXE:
  case ITEM_STONE_HOE:
  case ITEM_STONE_SPADE:
  case ITEM_STONE_SWORD:
  case ITEM_BOW:
    health = 132;
    break;
  case ITEM_IRON_AXE:
  case ITEM_IRON_PICKAXE:
  case ITEM_IRON_HOE:
  case ITEM_IRON_SPADE:
  case ITEM_IRON_SWORD:
    health = 251;
    break;
  case ITEM_DIAMOND_AXE:
  case ITEM_DIAMOND_PICKAXE:
  case ITEM_DIAMOND_HOE:
  case ITEM_DIAMOND_SPADE:
  case ITEM_DIAMOND_SWORD:
    health = 1562;
    break;

  default:
    health = 0;
  }
  return health;
}


void Inventory::getEnabledRecipes(std::vector<std::string>& receiptFiles, const std::string& cfg)
{
  std::ifstream ifs(cfg.c_str());

  if (ifs.fail())
  {
    ifs.close();
    return;
  }

  std::string suffix = ".recipe";
  std::string temp;
  std::string text;
  while (getline(ifs, temp))
  {
    //If empty line
    if (temp.size() == 0)
    {
      continue;
    }

    // If commentline -> skip to next
    if (temp[0] == COMMENTPREFIX)
    {
      continue;
    }

    receiptFiles.push_back(temp + suffix);
  }
  ifs.close();
}

Inventory::Inventory(const std::string& path, const std::string& suffix, const std::string& cfg)
{
  std::vector<std::string> receiptFiles;
  getEnabledRecipes( receiptFiles, cfg);

  for (unsigned int i = 0; i < receiptFiles.size(); i++)
  {
    readRecipe(path + '/' + receiptFiles[i]);
  }
}

bool Inventory::addRecipe(int width, int height, std::vector<ItemPtr> inputrecipe, int outputCount, int16_t outputType, int16_t outputHealth)
{
  RecipePtr recipe(new Recipe);

  recipe->width  = width;
  recipe->height = height;
  recipe->output.setCount(outputCount);
  recipe->output.setType(outputType);
  recipe->output.setHealth(outputHealth);
  recipe->slots = inputrecipe;

  recipes.push_back(recipe);

  return true;
}

bool Inventory::readRecipe(const std::string& recipeFile)
{
  std::ifstream ifs(recipeFile.c_str());

  if (ifs.fail())
  {
    LOG2(ERROR, "Could not find: " + recipeFile);
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
  std::vector<ItemPtr> recipetable;
  std::string text;
  while (getline(ifs, temp))
  {
    //If empty line
    if (temp.empty())
    {
      continue;
    }

    // If commentline -> skip to next
    if (temp[0] == COMMENTPREFIX)
    {
      continue;
    }

    // Init vars
    del = 0;
    line.clear();

    // Process line
    while (!temp.empty())
    {
      // Remove white spaces
      while (temp[0] == ' ')
      {
        temp = temp.substr(1);
      }

      // Split words
      del = temp.find(' ');
      if (del > -1)
      {
        line.push_back(temp.substr(0, del));
        temp = temp.substr(del + 1);
      }
      else
      {
        line.push_back(temp);
        break;
      }
    }

    // Begin recipe
    if (line.size() == 1 && line[0] == "<-")
    {
      readingRecipe = true;
      continue;
    }
    // Begin recipe
    if (line.size() == 1 && line[0] == "->")
    {
      readingRecipe = false;
      continue;
    }

    if (readingRecipe)
    {
      for (unsigned int i = 0; i < line.size(); i++)
      {
        std::string data(line[i]);
        ItemPtr item(new Item);
        item->setCount(1);
        item->setHealth(-1);
        int location = data.find("x");
        if (location > -1)
        {
          // Quantity before ID
          item->setCount(atoi(data.substr(0, location).c_str()));
          data = data.substr(location + 1, std::string::npos);
        }
        location = data.find(":");
        if (location > -1)
        {
          // Meta after ID
          item->setHealth(atoi(data.substr(location + 1, std::string::npos).c_str()));
          data = data.substr(0, location);
        }
        item->setType(atoi(data.c_str()));
        recipetable.push_back(item);
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

  addRecipe(width, height, recipetable, outCount, outType, outHealth);

  return true;
}

bool Inventory::canBeArmour(int slot, int type)
{
  if (slot == 5)
  {
    // Helmet slot. Lots of fun here
    if (ServerInstance->m_only_helmets)
    {
      switch (type)
      {
      case ITEM_LEATHER_HELMET:
      case ITEM_CHAINMAIL_HELMET:
      case ITEM_IRON_HELMET:
      case ITEM_DIAMOND_HELMET:
      case ITEM_GOLD_HELMET:
        return true;
        break;
      }
      return false;
    }
    else
    {
      return true;
    }
  }
  else if (slot == 6)
  {
    switch (type)
    {
    case ITEM_LEATHER_CHESTPLATE:
    case ITEM_CHAINMAIL_CHESTPLATE:
    case ITEM_IRON_CHESTPLATE:
    case ITEM_DIAMOND_CHESTPLATE:
    case ITEM_GOLD_CHESTPLATE:
      return true;
      break;
    }
    return false;
  }
  else if (slot == 7)
  {
    switch (type)
    {
    case ITEM_LEATHER_LEGGINGS:
    case ITEM_CHAINMAIL_LEGGINGS:
    case ITEM_IRON_LEGGINGS:
    case ITEM_DIAMOND_LEGGINGS:
    case ITEM_GOLD_LEGGINGS:
      return true;
      break;
    }
    return false;
  }
  else if (slot == 8)
  {
    switch (type)
    {
    case ITEM_LEATHER_BOOTS:
    case ITEM_CHAINMAIL_BOOTS:
    case ITEM_IRON_BOOTS:
    case ITEM_DIAMOND_BOOTS:
    case ITEM_GOLD_BOOTS:
      return true;
      break;
    }
    return false;
  }
  LOG2(WARNING, "Unknown armour slot.");
  return false;
}


bool Inventory::windowClick(User* user, int8_t windowID, int16_t slot, int8_t rightClick, int16_t actionNumber, int16_t itemID, int8_t itemCount, int16_t itemUses, int8_t shift)
{
  //Ack
  user->buffer << (int8_t)PACKET_TRANSACTION << (int8_t)windowID << (int16_t)actionNumber << (int8_t)1;

  //Click outside the window
  if (slot == -999)
  {
    if (user->inventoryHolding.getType() != -1)
    {
      ServerInstance->map(user->pos.map)->createPickupSpawn((int)user->pos.x, (int)user->pos.y, (int)user->pos.z,
          user->inventoryHolding.getType(), user->inventoryHolding.getCount(),
          user->inventoryHolding.getHealth(), user);
      user->inventoryHolding.setType(-1);
    }
    return true;
  }

  if (!user->isOpenInv && windowID != 0)
  {
    return false;
  }

  sChunk* chunk = NULL;
  if (windowID != 0)
  {
    chunk = ServerInstance->map(user->pos.map)->getChunk(blockToChunk(user->openInv.x), blockToChunk(user->openInv.z));

    if (chunk == NULL)
    {
      return false;
    }

    chunk->changed = true;
  }

  std::vector<User*>* otherUsers = NULL;
  OpenInvPtr currentInventory;

  if (windowID != WINDOW_PLAYER)
  {
    std::vector<OpenInvPtr>* pinv = NULL;
    switch (user->openInv.type)
    {
    case WINDOW_CHEST:
    case WINDOW_LARGE_CHEST:
      pinv = &openChests;
      break;
    case WINDOW_FURNACE:
      pinv = &openFurnaces;
      break;
    case WINDOW_WORKBENCH:
      pinv = &openWorkbenches;
      break;
    }

    std::vector<OpenInvPtr>& inv = *pinv;

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
  }

  Item* slotItem = NULL;
  furnaceDataPtr tempFurnace;

  switch (windowID)
  {
    //Player inventory
  case WINDOW_PLAYER:
    slotItem = &user->inv[slot];
    break;
  case WINDOW_CHEST:
    if (slot > 26)
    {
      slotItem = &user->inv[slot - 18];
    }
    else
    {
      for (uint32_t i = 0; i < chunk->chests.size(); i ++)
      {
        if (chunk->chests[i]->x() == user->openInv.x &&
            chunk->chests[i]->y() == user->openInv.y &&
            chunk->chests[i]->z() == user->openInv.z)
        {
          slotItem = (*chunk->chests[i]->items())[slot].get();
          break;
        }
      }
      //Create chest data if it doesn't exist
      if (slotItem == NULL)
      {
        chestDataPtr newChest(new chestData);
        newChest->x(user->openInv.x);
        newChest->y(user->openInv.y);
        newChest->z(user->openInv.z);
        chunk->chests.push_back(newChest);
        slotItem = (*newChest->items())[slot].get();
      }
    }
    break;
  case WINDOW_LARGE_CHEST:
    if (slot > 54)
    {
      slotItem = &user->inv[slot - 45];
    }
    else
    {
      //ToDo: Handle large chest
      for (uint32_t i = 0; i < chunk->chests.size(); i++)
      {
        //if(!chunk->chests[i]->large())
        //  continue;

        if(chunk->chests[i]->x() == user->openInv.x &&
          chunk->chests[i]->y() == user->openInv.y &&
          chunk->chests[i]->z() == user->openInv.z)
        {
          slotItem = (*chunk->chests[i]->items())[slot].get();
          break;
        }
      }
      if(slotItem == NULL)
      {
        chestDataPtr newChest(new chestData);
        newChest->x(user->openInv.x);
        newChest->y(user->openInv.y);
        newChest->z(user->openInv.z);
        newChest->large(true);
        chunk->chests.push_back(newChest);
        slotItem = (*newChest->items())[slot].get();
      }
    }
    break;
  case WINDOW_FURNACE:
    if (slot >= 3)
    {
      slotItem = &user->inv[slot + 6];
    }
    else
    {
      for (uint32_t i = 0; i < chunk->furnaces.size(); i ++)
      {
        if (chunk->furnaces[i]->x == user->openInv.x &&
            chunk->furnaces[i]->y == user->openInv.y &&
            chunk->furnaces[i]->z == user->openInv.z)
        {
          slotItem = &chunk->furnaces[i]->items[slot];
          tempFurnace = chunk->furnaces[i];
        }
      }
      //Create furnace data if it doesn't exist
      if (slotItem == NULL)
      {
        furnaceDataPtr newFurnace(new furnaceData);
        newFurnace->x = user->openInv.x;
        newFurnace->y = user->openInv.y;
        newFurnace->z = user->openInv.z;
        newFurnace->burnTime = 0;
        newFurnace->cookTime = 0;
        chunk->furnaces.push_back(newFurnace);
        slotItem = &newFurnace->items[slot];
        tempFurnace = newFurnace;
      }
    }
    break;
  case WINDOW_WORKBENCH:
    if (slot > 9)
    {
      slotItem = &user->inv[slot - 1];
    }
    else
    {
      slotItem = &currentInventory->workbench[slot];
    }
    break;
  }

  bool workbenchCrafting = false;
  bool playerCrafting    = false;

  if (windowID == WINDOW_PLAYER && slot >= 5 && slot <= 8)
  {
    // Armour slots are a strange case. Only a quantity of one should be allowed, so this must be checked for.
    if (slotItem->getType() == -1 && user->inventoryHolding.getType() > 0)
    {
      if (canBeArmour(slot, user->inventoryHolding.getType()))
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
      if (canBeArmour(slot, user->inventoryHolding.getType()))
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
    setSlot(user, WINDOW_CURSOR, 0, user->inventoryHolding.getType(), user->inventoryHolding.getCount(), user->inventoryHolding.getHealth());
    return false;
  }


  //Empty slot and holding something
  if ((slotItem->getType() == -1 || (slotItem->getType() == user->inventoryHolding.getType() && slotItem->getHealth() == user->inventoryHolding.getHealth() && slotItem->getCount() < 64)) && user->inventoryHolding.getType() != -1)
  {
    //If accessing crafting output slot
    if (slotItem->getType() != -1 && (windowID == WINDOW_WORKBENCH || windowID == WINDOW_PLAYER) && slot == 0)
    {
      if (user->inventoryHolding.getType() == slotItem->getType() && 64 - user->inventoryHolding.getCount() >= slotItem->getCount())
      {
        user->inventoryHolding.decCount(-slotItem->getCount());
        if (windowID == WINDOW_WORKBENCH)
        {
          for (uint8_t workbenchSlot = 1; workbenchSlot < 10; workbenchSlot++)
          {
            if (currentInventory->workbench[workbenchSlot].getType() != -1)
            {
              currentInventory->workbench[workbenchSlot].decCount();
              setSlot(user, windowID, workbenchSlot, currentInventory->workbench[workbenchSlot].getType(),
                      currentInventory->workbench[workbenchSlot].getCount(),
                      currentInventory->workbench[workbenchSlot].getHealth());
            }
          }
          workbenchCrafting = true;
        }
        else
        {
          for (uint8_t playerSlot = 1; playerSlot < 5; playerSlot++)
          {
            if (user->inv[playerSlot].getType() != -1)
            {
              user->inv[playerSlot].decCount();
              setSlot(user, windowID, playerSlot, user->inv[playerSlot].getType(),
                      user->inv[playerSlot].getCount(),
                      user->inv[playerSlot].getHealth());
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
      if ((windowID != WINDOW_WORKBENCH && windowID != WINDOW_PLAYER) || slot != 0)
      {
        int16_t addCount = (64 - slotItem->getCount() >= user->inventoryHolding.getCount()) ? user->inventoryHolding.getCount() : 64 - slotItem->getCount();

        slotItem->decCount(0 - ((rightClick) ? 1 : addCount));
        slotItem->setHealth(user->inventoryHolding.getHealth());
        slotItem->setType(user->inventoryHolding.getType());

        user->inventoryHolding.decCount((rightClick) ? 1 : addCount);
      }
    }

  }
  else if (user->inventoryHolding.getType() == -1)
  {
    //If accessing crafting output slot, remove from input!
    if (slotItem->getType() != -1 && (windowID == WINDOW_WORKBENCH || windowID == WINDOW_PLAYER) && slot == 0)
    {
      user->inventoryHolding.setType(slotItem->getType());
      user->inventoryHolding.setCount(slotItem->getCount());
      user->inventoryHolding.setHealth(slotItem->getHealth());

      if (windowID == WINDOW_WORKBENCH)
      {
        for (uint8_t workbenchSlot = 1; workbenchSlot < 10; workbenchSlot++)
        {
          if (currentInventory->workbench[workbenchSlot].getType() != -1)
          {
            currentInventory->workbench[workbenchSlot].decCount();

            setSlot(user, windowID, workbenchSlot, currentInventory->workbench[workbenchSlot].getType(),
                    currentInventory->workbench[workbenchSlot].getCount(),
                    currentInventory->workbench[workbenchSlot].getHealth());
          }
        }
        workbenchCrafting = true;
      }
      else
      {
        for (uint8_t playerSlot = 1; playerSlot < 5; playerSlot++)
        {
          if (user->inv[playerSlot].getType() != -1)
          {
            user->inv[playerSlot].decCount();
          }
        }
        playerCrafting = true;
      }
    }
    else
    {
      user->inventoryHolding.setType(slotItem->getType());
      user->inventoryHolding.setHealth(slotItem->getHealth());
      user->inventoryHolding.setCount(slotItem->getCount());
      if (rightClick == 1)
      {
        user->inventoryHolding.decCount(slotItem->getCount() >> 1);
      }

      slotItem->decCount(user->inventoryHolding.getCount());
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
    if ((windowID != WINDOW_WORKBENCH && windowID != WINDOW_PLAYER) || slot != 0)
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
  setSlot(user, windowID, slot, slotItem->getType(), slotItem->getCount(), slotItem->getHealth());

  //Update item on the cursor
  //TODO Shift klick
  /*
  if(shift != 0)
  {
  }
  else*/
  setSlot(user, WINDOW_CURSOR, 0, user->inventoryHolding.getType(), user->inventoryHolding.getCount(), user->inventoryHolding.getHealth());


  //Check if crafting
  if ((windowID == WINDOW_WORKBENCH && slot < 10 && slot > 0) || workbenchCrafting)
  {
    if (doCraft(currentInventory->workbench, 3, 3))
    {
      setSlot(user, windowID, 0, currentInventory->workbench[0].getType(), currentInventory->workbench[0].getCount(), currentInventory->workbench[0].getHealth());
    }
    else
    {
      currentInventory->workbench[0].setType(-1);
      setSlot(user, windowID, 0, -1, 0, 0);
    }
  }
  else if ((windowID == WINDOW_PLAYER && slot < 5 && slot > 0) || playerCrafting)
  {
    if (doCraft(user->inv, 2, 2))
    {
      setSlot(user, windowID, 0, user->inv[0].getType(), user->inv[0].getCount(), user->inv[0].getHealth());
    }
    else
    {
      user->inv[0].setType(-1);
      setSlot(user, windowID, 0, -1, 0, 0);
    }
  }
  //If handling the "fuel" slot
  else if (windowID == WINDOW_FURNACE && (slot == 1 || slot == 0))
  {
    tempFurnace->map = user->pos.map;
    ServerInstance->furnaceManager()->handleActivity(tempFurnace);
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

  if(!updateInventory(user, windowID))
     return false;
  return true;
}


bool Inventory::windowOpen(User* user, int8_t type, int32_t x, int32_t y, int32_t z)
{
  sChunk* chunk = ServerInstance->map(user->pos.map)->getChunk(blockToChunk(x), blockToChunk(z));
  if (chunk == NULL)
  {
    return false;
  }

  onwindowOpen(user, type, x, y, z);

  switch (type)
  {
  case WINDOW_CHEST:
  case WINDOW_LARGE_CHEST:
    {
      chestDataPtr _chestData;
      for (uint32_t i = 0; i < chunk->chests.size(); i++)
      {
        if ((chunk->chests[i]->x() == x)
          && (chunk->chests[i]->y() == y)
          && (chunk->chests[i]->z() == z) )
        {
          _chestData = chunk->chests[i];
          break;
        }
      }
      if(_chestData == NULL)
        break;

      user->buffer << (int8_t)PACKET_OPEN_WINDOW << (int8_t)type << (int8_t)INVENTORYTYPE_CHEST;
      if(_chestData->large())
      {
        user->buffer << std::string("Large chest");
      } else {
        user->buffer << std::string("Chest");
      }
      user->buffer << (int8_t)(_chestData->size()); // size.. not a very good idea. lets just hope this will only return 27 or 54

      for (size_t j = 0; j < _chestData->size(); j++)
      {
        if ((*_chestData->items())[j]->getType() != -1)
        {
          Packet packet = Protocol::setSlotHeader(type, j);
          ItemPtr item = (*_chestData->items())[j];
          packet << Protocol::slot(item->getType(), item->getCount(), item->getHealth());
          user->buffer << packet;
        }
      }
    }
    break;

  case WINDOW_WORKBENCH:
    user->buffer << (int8_t)PACKET_OPEN_WINDOW << (int8_t)WINDOW_WORKBENCH  << (int8_t)INVENTORYTYPE_WORKBENCH;
    user->buffer << std::string("Workbench") << (int8_t)0;

    for (uint32_t i = 0; i < openWorkbenches.size(); i++)
    {
      if (openWorkbenches[i]->x == user->openInv.x &&
          openWorkbenches[i]->y == user->openInv.y &&
          openWorkbenches[i]->z == user->openInv.z)
      {
        for (int j = 0; j < 10; j++)
        {
          if (openWorkbenches[i]->workbench[j].getType() != -1)
          {
            Packet packet = Protocol::setSlotHeader(WINDOW_WORKBENCH, j);
            packet << Protocol::slot(openWorkbenches[i]->workbench[j].getType(),
            openWorkbenches[i]->workbench[j].getCount(),
            openWorkbenches[i]->workbench[j].getHealth());
            user->buffer << packet;
          }
        }
        break;
      }
    }
    break;
  case WINDOW_FURNACE:

    user->buffer << (int8_t)PACKET_OPEN_WINDOW << (int8_t)WINDOW_FURNACE  << (int8_t)INVENTORYTYPE_FURNACE;
    user->buffer << std::string("Furnace") << (int8_t)0;

    for (uint32_t i = 0; i < chunk->furnaces.size(); i++)
    {
      if (chunk->furnaces[i]->x == x && chunk->furnaces[i]->y == y && chunk->furnaces[i]->z == z)
      {
        for (int j = 0; j < 3; j++)
        {
          if (chunk->furnaces[i]->items[j].getType() != -1)
          {
            Packet packet = Protocol::setSlotHeader(WINDOW_FURNACE, j);
            Item& item = chunk->furnaces[i]->items[j];
            packet << Protocol::slot(item.getType(), item.getCount(), item.getHealth());
          }
        }
        user->buffer << (int8_t)PACKET_PROGRESS_BAR << (int8_t)WINDOW_FURNACE << (int16_t)0 << (int16_t)(chunk->furnaces[i]->cookTime * 18);
        user->buffer << (int8_t)PACKET_PROGRESS_BAR << (int8_t)WINDOW_FURNACE << (int16_t)1 << (int16_t)(chunk->furnaces[i]->burnTime * 3);
        break;
      }
    }
    break;
  }

  return true;
}

bool Inventory::isSpace(User* user, int16_t itemID, char count)
{
  int leftToFit = count;
  for (uint8_t i = 0; i < 36; i++)
  {
    Item* slot = &user->inv[i + 9];
    if (slot->getType() == -1)
    {
      return true;
    }

    if (slot->getType() == itemID)
    {
      if (64 - slot->getCount() >= leftToFit)
      {
        return true;
      }
      else if (64 - slot->getCount() > 0)
      {
        leftToFit -= 64 - slot->getCount();
      }
    }
  }
  return false;
}


bool Inventory::addItems(User* user, int16_t itemID, int16_t count, int16_t health)
{
  bool checkingTaskbar = true;

  for (uint8_t i = 36 - 9; i < 36 - 9 || checkingTaskbar; i++)
  {
    //First, the "task bar"
    if (i == 36)
    {
      checkingTaskbar = false;
      i = 0;
    }

    //The main slots are in range 9-44
    Item* slot = &user->inv[i + 9];

    //If slot empty, put item there
    if (slot->getType() == -1)
    {
      slot->setType(itemID);
      slot->setCount(count);
      slot->setHealth(health);
      break;
    }

    //If same item type
    if (slot->getType() == itemID)
    {
      if (slot->getHealth() == health)
      {
        //Put to the stack
        if (64 - slot->getCount() >= count)
        {
          slot->setType(itemID);
          slot->decCount(-count);
          break;
        }
        //Put some of the items to this stack and continue searching for space
        else if (64 - slot->getCount() > 0)
        {
          slot->setType(itemID);
          count -= 64 - slot->getCount();
          slot->setCount(64);
        }
      }
    }
  }

  return true;
}

bool Inventory::windowClose(User* user, int8_t windowID)
{
  //If still holding something, dump the items to ground
  if (user->inventoryHolding.getType() != -1)
  {
    ServerInstance->map(user->pos.map)->createPickupSpawn((int)user->pos.x, (int)user->pos.y, (int)user->pos.z,
        user->inventoryHolding.getType(), user->inventoryHolding.getCount(),
        user->inventoryHolding.getHealth(), user);
    user->inventoryHolding.setType(-1);
  }

  if (user->isOpenInv)
  {
    onwindowClose(user, user->openInv.type, user->openInv.x, user->openInv.y, user->openInv.z);
  }

  return true;
}

bool Inventory::onwindowOpen(User* user, int8_t type, int32_t x, int32_t y, int32_t z)
{

  if (user->isOpenInv)
  {
    onwindowClose(user, user->openInv.type, user->openInv.x, user->openInv.y, user->openInv.z);
  }

  std::vector<OpenInvPtr>* pinv;
  switch (type)
  {
  case WINDOW_CHEST:
  case WINDOW_LARGE_CHEST:
    pinv = &openChests;
    break;
  case WINDOW_FURNACE:
    pinv = &openFurnaces;
    break;
  case WINDOW_WORKBENCH:
    pinv = &openWorkbenches;
    break;
  }

  std::vector<OpenInvPtr>& inv = *pinv;

  for (size_t i = 0; i < inv.size(); ++i)
  {
    if (inv[i]->x == user->openInv.x &&
        inv[i]->y == user->openInv.y &&
        inv[i]->z == user->openInv.z)
    {
      inv[i]->users.push_back(user);
      user->isOpenInv = true;
      return true;
    }
  }

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

  return true;
}

bool Inventory::onwindowClose(User* user, int8_t type, int32_t x, int32_t y, int32_t z)
{
  std::vector<OpenInvPtr>* pinv = NULL;

  switch (type)
  {
  case WINDOW_CHEST:
  case WINDOW_LARGE_CHEST:
    pinv = &openChests;
    break;
  case WINDOW_FURNACE:
    pinv = &openFurnaces;
    break;
  case WINDOW_WORKBENCH:
    pinv = &openWorkbenches;
    break;
  default:
    return false;
  }

  std::vector<OpenInvPtr>& inv = *pinv;

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
            //Dump stuff to ground if workbench and no other users
            if (type == WINDOW_WORKBENCH)
            {
              for (uint32_t slotNumber = 1; slotNumber < 10; ++slotNumber)
              {
                if (inv[i]->workbench[slotNumber].getType() != -1)
                {
                  ServerInstance->map(user->pos.map)->createPickupSpawn((int)user->pos.x, (int)user->pos.y, (int)user->pos.z,
                      inv[i]->workbench[slotNumber].getType(), inv[i]->workbench[slotNumber].getCount(),
                      inv[i]->workbench[slotNumber].getHealth(), user);
                }
              }
            }
            inv.erase(inv.begin() + i);
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




bool Inventory::doCraft(Item* slots, int8_t width, int8_t height)
{
  for (uint32_t i = 0; i < recipes.size(); i++)
  {
    //Skip if recipe doesn't fit
    if (width < recipes[i]->width || height < recipes[i]->height)
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
        for (int32_t recipePosX = 0; recipePosX < recipes[i]->width; recipePosX++)
        {
          for (int32_t recipePosY = 0; recipePosY < recipes[i]->height; recipePosY++)
          {
            if (slots[(recipePosY + offsetY)*width + recipePosX + 1 + offsetX].getType() != recipes[i]->slots[recipePosY * recipes[i]->width + recipePosX]->getType())
            {
              mismatch = true;
              break;
            }
            if (recipes[i]->slots[recipePosY * recipes[i]->width + recipePosX]->getHealth() != -1)
            {
              if (slots[(recipePosY + offsetY)*width + recipePosX + 1 + offsetX].getHealth() != recipes[i]->slots[recipePosY * recipes[i]->width + recipePosX]->getHealth())
              {
                mismatch = true;
                break;
              }
            }
            if (slots[(recipePosY + offsetY)*width + recipePosX + 1 + offsetX].getCount() < recipes[i]->slots[recipePosY * recipes[i]->width + recipePosX]->getCount())
            {
              mismatch = true;
              break;
            }
          }
        }

        //Found match!
        if (!mismatch)
        {
          //Check that other areas are empty!
          bool foundItem = false;
          for (int32_t craftingPosX = 0; craftingPosX < width; craftingPosX++)
          {
            for (int32_t craftingPosY = 0; craftingPosY < height; craftingPosY++)
            {
              //If not inside the recipe boundaries
              if (craftingPosX < offsetX || craftingPosX >= offsetX + recipes[i]->width ||
                  craftingPosY < offsetY || craftingPosY >= offsetY + recipes[i]->height)
              {
                if (slots[(craftingPosY)*width + craftingPosX + 1].getType() != -1)
                {
                  foundItem = true;
                  break;
                }
              }
            }
          }
          if (!foundItem)
          {
            slots[0] = recipes[i]->output;
            return true;
          }
        }

        offsetX++;
      }
      while (offsetX <= width - recipes[i]->width);

      offsetY++;
    }
    while (offsetY <= height - recipes[i]->height);
  }

  return false;
}

bool Inventory::setSlot(User* user, int8_t windowID, int16_t slot, int16_t itemID, int8_t count, int16_t health)
{
  //ServerInstance->logger()->log(1,"Setslot: " + dtos(slot) + " to " + dtos(itemID) + " (" + dtos(count) + ") health: " + dtos(health));
  user->buffer << Protocol::setSlotHeader(windowID, slot) << Protocol::slot(itemID, count, health);
  return true;
}

bool Inventory::updateInventory(User* user, int8_t windowID)
{
   if(!this->onupdateinventory(user, windowID))
      return false;
   for(int i = 0; i < 44; i++)
      user->inv[i].sendUpdate();
   return true;
}

bool Inventory::onupdateinventory(User* user, int8_t windowID)
{
   return true;
}

int16_t Inventory::itemHealth(int16_t itemID, int8_t block, bool& rightUse)
{
  int16_t health = 0;
  rightUse = false;
  switch (itemID)
  {
  case ITEM_GOLD_AXE:
  case ITEM_GOLD_PICKAXE:
  case ITEM_GOLD_HOE:
  case ITEM_GOLD_SPADE:
    health = 33;
    break;
  case ITEM_WOODEN_AXE:
  case ITEM_WOODEN_PICKAXE:
  case ITEM_WOODEN_HOE:
  case ITEM_WOODEN_SPADE:
    health = 60;
    break;
  case ITEM_STONE_AXE:
  case ITEM_STONE_PICKAXE:
  case ITEM_STONE_HOE:
  case ITEM_STONE_SPADE:
  case ITEM_BOW:
    health = 132;
    break;
  case ITEM_IRON_AXE:
  case ITEM_IRON_PICKAXE:
  case ITEM_IRON_HOE:
  case ITEM_IRON_SPADE:
    health = 251;
    break;
  case ITEM_DIAMOND_AXE:
  case ITEM_DIAMOND_PICKAXE:
  case ITEM_DIAMOND_HOE:
  case ITEM_DIAMOND_SPADE:
    health = 1562;
    break;

  default:
    health = 0;
  }


  switch (itemID)
  {
  case ITEM_WOODEN_AXE:
  case ITEM_GOLD_AXE:
  case ITEM_STONE_AXE:
  case ITEM_IRON_AXE:
  case ITEM_DIAMOND_AXE:
    if (block == BLOCK_WOOD || block == BLOCK_PLANK)
    {
      rightUse = true;
    }
    break;

  case ITEM_WOODEN_PICKAXE:
  case ITEM_STONE_PICKAXE:
  case ITEM_GOLD_PICKAXE:
  case ITEM_IRON_PICKAXE:
  case ITEM_DIAMOND_PICKAXE:
    switch (block)
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
    switch (block)
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
