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

Furnace::Furnace(furnaceData *data_)
{

  data = data_;
  uint8_t block;
  uint8_t meta;
  Mineserver::get()->map(data->map)->getBlock(data->x, data->y, data->z, &block, &meta);

  // Check if this is a burning block
  if(block == BLOCK_BURNING_FURNACE)
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
  if(!hasValidIngredient())
  {
    data->cookTime = 0;
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
  if(isBurningFuel() && !m_burning)
  {
    Mineserver::get()->map(data->map)->getBlock(data->x, data->y, data->z, &block, &meta);
    // Switch to burning furnace
    Mineserver::get()->map(data->map)->setBlock(data->x, data->y, data->z, BLOCK_BURNING_FURNACE, meta);
    Mineserver::get()->map(data->map)->sendBlockChange(data->x, data->y, data->z, BLOCK_BURNING_FURNACE, meta);
    sendToAllUsers();
    m_burning = true;
  }
  else if(!isBurningFuel() && m_burning)
  {
    Mineserver::get()->map(data->map)->getBlock(data->x, data->y, data->z, &block, &meta);
    // Switch to regular furnace
    Mineserver::get()->map(data->map)->setBlock(data->x, data->y, data->z, BLOCK_FURNACE, meta);
    Mineserver::get()->map(data->map)->sendBlockChange(data->x, data->y, data->z, BLOCK_FURNACE, meta);
    sendToAllUsers();
    m_burning = false;
  }
}

void Furnace::smelt()
{
  // Check if we're cooking
  if(isCooking())
  {
    // Convert where applicable
    Item* inputSlot  = &slots()[SLOT_INPUT];
    Item* fuelSlot   = &slots()[SLOT_FUEL];
    Item* outputSlot = &slots()[SLOT_OUTPUT];
    int32_t creationID = -1;
    if(inputSlot->type == BLOCK_IRON_ORE)    { creationID = ITEM_IRON_INGOT; }
    if(inputSlot->type == BLOCK_GOLD_ORE)    { creationID = ITEM_GOLD_INGOT; }
    if(inputSlot->type == BLOCK_SAND)        { creationID = BLOCK_GLASS; }
    if(inputSlot->type == BLOCK_COBBLESTONE) { creationID = BLOCK_STONE; }
    if(inputSlot->type == ITEM_PORK)         { creationID = ITEM_GRILLED_PORK; }
    if(inputSlot->type == ITEM_CLAY_BALLS)   { creationID = ITEM_CLAY_BRICK; }
    if(inputSlot->type == ITEM_RAW_FISH)     { creationID = ITEM_COOKED_FISH; }

    // Update other params if we actually converted
    if(creationID != -1 && outputSlot->count != 64)
    {
      // Check if the outputSlot is empty
      if(outputSlot->type == -1)
      {
        outputSlot->type = creationID;
        outputSlot->count = 0;
      }

      // Ok - now check if the current output slot contains the same stuff
      if(outputSlot->type == creationID)
      {
        // Increment output and decrememnt the input source
        outputSlot->count++;
        inputSlot->count--;
        outputSlot->health = inputSlot->health;
        data->cookTime = 0;

        if(inputSlot->count == 0)
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
  if(data->burnTime > 0)
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
  if(isBurningFuel() && hasValidIngredient())
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
  if((slot->count != 0) &&
    (
     (slot->type == BLOCK_IRON_ORE)    ||
     (slot->type == BLOCK_GOLD_ORE)    ||
     (slot->type == BLOCK_SAND)        ||
     (slot->type == BLOCK_COBBLESTONE) ||
     (slot->type == ITEM_PORK)         ||
     (slot->type == ITEM_CLAY_BALLS)   ||
     (slot->type == ITEM_RAW_FISH))
     )
  {
    return true;
  }
  else
  {
    return false;
  }
}
void Furnace::consumeFuel()
{
  // Check that we have fuel
  if(slots()[SLOT_FUEL].count == 0)
    return;

  // Increment the fuel burning time based on fuel type
  // http://www.minecraftwiki.net/wiki/Furnace#Fuel_efficiency
  Item *fuelSlot = &slots()[SLOT_FUEL];

  uint16_t fuelTime = 0;
  switch(fuelSlot->type)
  {
    case ITEM_COAL:           fuelTime = 80;   break;
    case BLOCK_PLANK:         fuelTime = 15;   break;
    case ITEM_STICK:          fuelTime = 5;    break;
    case BLOCK_WOOD:          fuelTime = 15;   break;
    case BLOCK_WORKBENCH:     fuelTime = 15;   break;
    case BLOCK_CHEST:         fuelTime = 15;   break;
    case BLOCK_BOOKSHELF:     fuelTime = 15;   break;
    case BLOCK_JUKEBOX:       fuelTime = 15;   break;
    case BLOCK_FENCE:         fuelTime = 15;   break;
    case BLOCK_WOODEN_STAIRS: fuelTime = 15;   break;
    case ITEM_LAVA_BUCKET:    fuelTime = 1000; break;
    default: break;
  }

  if(fuelTime > 0)
  {
    data->burnTime += fuelTime;
    // Now decrement the fuel & reset
    fuelSlot->count--;
    if (fuelSlot->count == 0)
    {
      *fuelSlot = Item();
    }
  }

  // Update our block type if need be
  updateBlock();
}
int16_t Furnace::burnTime()
{
  return data->burnTime;
}
int16_t Furnace::cookTime()
{
  return 10;
}

void Furnace::sendToAllUsers()
{


  enum { PROGRESS_ARROW = 0, PROGRESS_FIRE = 1 };
  //ToDo: send changes to all with this furnace opened

  std::vector<OpenInventory*>* inv = &Mineserver::get()->inventory()->openFurnaces;
      
  for(uint32_t openinv = 0; openinv < inv->size(); openinv ++)
  {
    if((*inv)[openinv]->x == data->x &&
       (*inv)[openinv]->y == data->y &&
       (*inv)[openinv]->z == data->z)
    {
      for(uint32_t user = 0; user < (*inv)[openinv]->users.size(); user ++)
      {
        for(int j = 0; j < 3; j++)
        {
          if(data->items[j].type != -1)
          {
            (*inv)[openinv]->users[user]->buffer << (int8_t)PACKET_SET_SLOT << (int8_t)WINDOW_FURNACE << (int16_t)j << (int16_t)data->items[j].type 
                          << (int8_t)(data->items[j].count) << (int16_t)data->items[j].health;
          }
        }
        
        (*inv)[openinv]->users[user]->buffer << (int8_t)PACKET_PROGRESS_BAR << (int8_t)WINDOW_FURNACE << (int16_t)PROGRESS_ARROW << (int16_t)(data->cookTime*18);
        (*inv)[openinv]->users[user]->buffer << (int8_t)PACKET_PROGRESS_BAR << (int8_t)WINDOW_FURNACE << (int16_t)PROGRESS_FIRE  << (int16_t)(data->burnTime*3);
      }

      break;
    }
  }

}
