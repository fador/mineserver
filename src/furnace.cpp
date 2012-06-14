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

#include <iostream>

#include "constants.h"
#include "furnace.h"
#include "mineserver.h"
#include "map.h"
#include "tools.h"
#include "config.h"
#include "protocol.h"

Creation createList[2258];
bool configIsRead = false;

Creation::Creation()
{
  output = -1;
  meta = 0;
  count = 0;
}

Furnace::Furnace(furnaceDataPtr data)
  : m_data(data)
{
  uint8_t block;
  uint8_t meta;

  ServerInstance->map(m_data->map)->getBlock(m_data->x, m_data->y, m_data->z, &block, &meta);

  if (!configIsRead)
  {
    readConfig();
    configIsRead = true;
  }

  // Check if this is a burning block
  if (block == BLOCK_BURNING_FURNACE)
  {
    m_burning = true;
  }
  else
  {
    m_burning = false;
  }

  // Make sure we're the right kind of block based on our current status
  updateBlock();
}

void Furnace::updateItems()
{
  if (!hasValidIngredient())
  {
    m_data->cookTime = 0;
  }
  else
  {

  }
}

void Furnace::updateBlock()
{
  // Get a pointer to this furnace's current block
  uint8_t block;
  uint8_t meta;

  // Now make sure that it's got the correct block type based on it's current status
  if (isBurningFuel() && !m_burning)
  {
    ServerInstance->map(m_data->map)->getBlock(m_data->x, m_data->y, m_data->z, &block, &meta);
    // Switch to burning furnace
    ServerInstance->map(m_data->map)->setBlock(m_data->x, m_data->y, m_data->z, BLOCK_BURNING_FURNACE, meta);
    ServerInstance->map(m_data->map)->sendBlockChange(m_data->x, m_data->y, m_data->z, BLOCK_BURNING_FURNACE, meta);
    sendToAllUsers();
    m_burning = true;
  }
  else if (!isBurningFuel() && m_burning)
  {
    ServerInstance->map(m_data->map)->getBlock(m_data->x, m_data->y, m_data->z, &block, &meta);
    // Switch to regular furnace
    ServerInstance->map(m_data->map)->setBlock(m_data->x, m_data->y, m_data->z, BLOCK_FURNACE, meta);
    ServerInstance->map(m_data->map)->sendBlockChange(m_data->x, m_data->y, m_data->z, BLOCK_FURNACE, meta);
    sendToAllUsers();
    m_burning = false;
  }
}

void Furnace::smelt()
{
  // Check if we're cooking
  if (isCooking())
  {
    // Convert where applicable
    Item* inputSlot  = &slots()[SLOT_INPUT];
    Item* fuelSlot   = &slots()[SLOT_FUEL];
    Item* outputSlot = &slots()[SLOT_OUTPUT];
    int32_t creationID = createList[inputSlot->getType()].output;

    // Update other params if we actually converted
    if (creationID != -1 && outputSlot->getCount() != 64)
    {
      // Check if the outputSlot is empty
      if (outputSlot->getType() == -1)
      {
        outputSlot->setType(creationID);
        outputSlot->setCount(1);
        outputSlot->setHealth(createList[inputSlot->getType()].meta);
        inputSlot->setCount(inputSlot->getCount() - 1);
        m_data->cookTime = 0;
      }

      // Ok - now check if the current output slot contains the same stuff
      if (outputSlot->getType() == creationID && m_data->cookTime != 0)
      {
        // Increment output and decrememnt the input source
        outputSlot->setCount(outputSlot->getCount() + createList[inputSlot->getType()].count);
        inputSlot->setCount(inputSlot->getCount() - 1);
        outputSlot->setHealth(createList[inputSlot->getType()].meta);
        m_data->cookTime = 0;

        if (inputSlot->getCount() == 0)
        {
          *inputSlot = Item();
        }
      }
    }
  }
}
bool Furnace::isBurningFuel()
{
  // Check if this furnace is currently burning
  if (m_data->burnTime > 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}
bool Furnace::isCooking()
{
  // If we're burning fuel and have valid ingredients, we're cooking!
  if (isBurningFuel() && hasValidIngredient())
  {
    return true;
  }
  else
  {
    return false;
  }
}
bool Furnace::hasValidIngredient()
{
  // Check that we have a valid input type
  Item* slot = &slots()[SLOT_INPUT];
  if (slot->getType() < 0)
  {
    return false;
  }
  if (createList[slot->getType()].output != -1)
  {
    return true;
  }
  return false;
}
void Furnace::consumeFuel()
{
  // Check that we have fuel
  if (slots()[SLOT_FUEL].getCount() == 0)
  {
    return;
  }

  // Increment the fuel burning time based on fuel type
  // http://www.minecraftwiki.net/wiki/Furnace#Fuel_efficiency
  Item* fuelSlot = &slots()[SLOT_FUEL];

  uint16_t fuelTime = 0;
  switch (fuelSlot->getType())
  {
  case ITEM_COAL:
    fuelTime = 80;
    break;
  case BLOCK_PLANK:
    fuelTime = 15;
    break;
  case ITEM_STICK:
    fuelTime = 5;
    break;
  case BLOCK_WOOD:
    fuelTime = 15;
    break;
  case BLOCK_WORKBENCH:
    fuelTime = 15;
    break;
  case BLOCK_CHEST:
    fuelTime = 15;
    break;
  case BLOCK_BOOKSHELF:
    fuelTime = 15;
    break;
  case BLOCK_JUKEBOX:
    fuelTime = 15;
    break;
  case BLOCK_FENCE:
    fuelTime = 15;
    break;
  case BLOCK_WOODEN_STAIRS:
    fuelTime = 15;
    break;
  case ITEM_LAVA_BUCKET:
    fuelTime = 1000;
    break;
  default:
    break;
  }

  if (fuelTime > 0)
  {
    m_data->burnTime += fuelTime;
    // Now decrement the fuel & reset
    fuelSlot->setCount(fuelSlot->getCount() - 1);
    if (fuelSlot->getCount() == 0)
    {
      *fuelSlot = Item();
    }
  }

  // Update our block type if need be
  updateBlock();
}
int16_t Furnace::burnTime()
{
  return m_data->burnTime;
}
int16_t Furnace::cookTime()
{
  return 10;
}

void Furnace::sendToAllUsers()
{


  enum { PROGRESS_ARROW = 0, PROGRESS_FIRE = 1 };
  //ToDo: send changes to all with this furnace opened

  std::vector<OpenInvPtr>& inv = ServerInstance->inventory()->openFurnaces;

  for (size_t openinv = 0; openinv < inv.size(); ++openinv)
  {
    if (inv[openinv]->x == m_data->x &&
        inv[openinv]->y == m_data->y &&
        inv[openinv]->z == m_data->z)
    {
      for (size_t user = 0; user < inv[openinv]->users.size(); ++user)
      {
        for (size_t j = 0; j < 3; ++j)
        {
          if (m_data->items[j].getType() != -1)
          {
            Item& item = m_data->items[j];
            inv[openinv]->users[user]->buffer << Protocol::setSlotHeader(WINDOW_FURNACE, j)
                                              << Protocol::slot(item.getType(), item.getCount(), item.getHealth());
          }
        }

        inv[openinv]->users[user]->buffer << (int8_t)PACKET_PROGRESS_BAR << (int8_t)WINDOW_FURNACE << (int16_t)PROGRESS_ARROW << (int16_t)(m_data->cookTime * 18);
        inv[openinv]->users[user]->buffer << (int8_t)PACKET_PROGRESS_BAR << (int8_t)WINDOW_FURNACE << (int16_t)PROGRESS_FIRE  << (int16_t)(m_data->burnTime * 3);
      }

      break;
    }
  }

}

void readConfig()
{
  const std::string key = "furnace.items";
  if (ServerInstance->config()->has(key) && ServerInstance->config()->type(key) == CONFIG_NODE_LIST)
  {
    std::list<std::string> tmp = ServerInstance->config()->mData(key)->keys();
    for (std::list<std::string>::const_iterator it = tmp.begin(); it != tmp.end(); ++it)
    {
      int input = ServerInstance->config()->iData(key + "." + *it + ".in");
      createList[input].output = ServerInstance->config()->iData(key + "." + *it + ".out");
      createList[input].meta = ServerInstance->config()->iData(key + "." + *it + ".meta");
      createList[input].count = ServerInstance->config()->iData(key + "." + *it + ".count");
    }
  }
}
