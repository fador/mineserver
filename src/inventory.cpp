/*
   Copyright (c) 2013, The Mineserver Project
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
#include "plugin.h"
#include <sstream>


void Item::sendUpdate()
{
  if (player != NULL && ready)
  {
    if (slot == player->curItem + 36)
    {
      Packet pkt = Protocol::entityEquipment(player->UID, 0, *this);
      player->sendOthers(pkt);
    }
    if (slot >= 5 && slot <= 8)
    {
      Packet pkt = Protocol::entityEquipment(player->UID,(5 - (slot - 4)), *this);
      player->sendOthers(pkt);
    }
    
    int window = 0;
    int t_slot = slot;
    if (slot == -1)
    {
      window = -1;
      t_slot = 0;
    }
    player->writePacket(Protocol::setSlot(window, t_slot, *this));
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

void Item::setData(NBT_Value* _data)
{
  if (data) delete data;
  data = _data;
}

void Item::setHealth(int16_t health)
{
  //bool rightUse;
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


bool Inventory::windowClick(User* user, int8_t windowID, int16_t slot, int8_t button, int16_t actionNumber, int16_t itemID, int8_t itemCount, int16_t itemUses, int8_t mode)
{
  for (auto invCb : ServerInstance->plugin()->getInventoryCB())
  {    
    if (invCb != NULL && invCb->affected(windowID))
    {
      if (invCb->onwindowClick(user, windowID, slot, button, actionNumber, itemID, itemCount, itemUses, mode))
      {
        return true;
      }
    }
  }
  return false;
}

bool Inventory::windowClose(User* user, int8_t type, int32_t x, int32_t y, int32_t z)
{
  for (auto invCb : ServerInstance->plugin()->getInventoryCB())
  {    
    if (invCb != NULL && invCb->affected(type))
    {
      if (invCb->onwindowClose(user, type, x, y, z))
      {
        return true;
      }
    }
  }
}

bool Inventory::windowOpen(User* user, int8_t type, int32_t x, int32_t y, int32_t z)
{
  // Close inventory if one already open
  if (user->isOpenInv)
  {
    windowClose(user, user->openInv.type, user->openInv.x, user->openInv.y, user->openInv.z);
  }

  for (auto invCb : ServerInstance->plugin()->getInventoryCB())
  {    
    if (invCb != NULL && invCb->affected(type))
    {
      if (invCb->onwindowOpen(user, type, x, y, z))
      {
        return true;
      }
    }
  }

  return false;
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
  int checkdir=1;

  //First loop and check if we can pile this up with existing blocks
  for (uint8_t i = 36 - 9; i >= 9 || checkingTaskbar; i+=checkdir)
  {
    //First, the "task bar"
    if (i == 36)
    {
      checkingTaskbar = false;
      i = 35-9;
      checkdir=-1;
    }

    //The main slots are in range 9-44
    Item* slot = &user->inv[i + 9];

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
          count--;
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

  checkdir=1;

  //If more items
  if(count)
  {
    //Check for empty slots
    for (uint8_t i = 36 - 9; i >= 9 || checkingTaskbar; i+=checkdir)
    {
      //First, the "task bar"
      if (i == 36)
      {
        checkingTaskbar = false;
        i = 35-9;
        checkdir=-1;
      }

      //The main slots are in range 9-44
      Item* slot = &user->inv[i + 9];

      //If slot empty, put item there
      if (slot->getType() == -1)
      {
        slot->setType(itemID);
        slot->setCount(int8_t(count));
        slot->setHealth(health);
        break;
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
    windowClose(user, user->openInv.type, user->openInv.x, user->openInv.y, user->openInv.z);
  }

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

bool Inventory::setSlot(User* user, int8_t windowID, int16_t slot, Item* item)
{
  user->writePacket(Protocol::setSlot(windowID, slot, *item));
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
    if (block == BLOCK_LOG || block == BLOCK_PLANK)
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
