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

Furnace::Furnace(NBT_Value* entity, uint8_t blockType)
{
  // Setup this furnace
  m_x = (int32_t)(*(*entity)["x"]);
  m_y = (int32_t)(*(*entity)["y"]);
  m_z = (int32_t)(*(*entity)["z"]);
  //m_fuelBurningTime = (int16_t)(*(*entity)["BurnTime"]);

  // Clean out the slots
  m_slots[SLOT_INPUT].count  = 0;
  m_slots[SLOT_INPUT].damage = 0;
  m_slots[SLOT_INPUT].id     = 0;
  m_slots[SLOT_FUEL].count   = 0;
  m_slots[SLOT_FUEL].damage  = 0;
  m_slots[SLOT_FUEL].id      = 0;
  m_slots[SLOT_OUTPUT].count = 0;
  m_slots[SLOT_OUTPUT].damage= 0;
  m_slots[SLOT_OUTPUT].id    = 0;


  // Set the cooking time based on input type (currently all smelting takes 10 secs but this gives us flexivibility in future)
  Slot inputSlot = m_slots[SLOT_INPUT];
  m_cookingTime = 0;
  if(inputSlot.id == BLOCK_IRON_ORE)     { m_cookingTime = 10; }
  if(inputSlot.id == BLOCK_GOLD_ORE)     { m_cookingTime = 10; }
  if(inputSlot.id == BLOCK_SAND)         { m_cookingTime = 10; }
  if(inputSlot.id == BLOCK_COBBLESTONE)  { m_cookingTime = 10; }
  if(inputSlot.id == ITEM_PORK)          { m_cookingTime = 10; }
  if(inputSlot.id == ITEM_CLAY_BALLS)    { m_cookingTime = 10; }
  if(inputSlot.id == ITEM_RAW_FISH)      { m_cookingTime = 10; }

  // Reset our active duration
  m_fuelBurningTime = 0;
  m_activeCookDuration = 0;

  // Check if this is a burning block
  if(blockType == BLOCK_BURNING_FURNACE)
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

void Furnace::updateBlock()
{
  // Get a pointer to this furnace's current block
  uint8_t block;
  uint8_t meta;

  // Now make sure that it's got the correct block type based on it's current status
  if(isBurningFuel() && !m_burning)
  {
    Mineserver::get()->map(map)->getBlock(m_x, m_y, m_z, &block, &meta);
    // Switch to burning furnace
    Mineserver::get()->map(map)->setBlock(m_x, m_y, m_z, BLOCK_BURNING_FURNACE, meta);
    Mineserver::get()->map(map)->sendBlockChange(m_x, m_y, m_z, BLOCK_BURNING_FURNACE, meta);
    sendToAllUsers();
    m_burning = true;
  }
  else if(!isBurningFuel() && m_burning)
  {
    Mineserver::get()->map(map)->getBlock(m_x, m_y, m_z, &block, &meta);
    // Switch to regular furnace
    Mineserver::get()->map(map)->setBlock(m_x, m_y, m_z, BLOCK_FURNACE, meta);
    Mineserver::get()->map(map)->sendBlockChange(m_x, m_y, m_z, BLOCK_FURNACE, meta);
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
    Slot inputSlot  = m_slots[SLOT_INPUT];
    Slot fuelSlot   = m_slots[SLOT_FUEL];
    Slot outputSlot = m_slots[SLOT_OUTPUT];
    int32_t creationID = 0;
    if(inputSlot.id == BLOCK_IRON_ORE)    { creationID = ITEM_IRON_INGOT; }
    if(inputSlot.id == BLOCK_GOLD_ORE)    { creationID = ITEM_GOLD_INGOT; }
    if(inputSlot.id == BLOCK_SAND)        { creationID = BLOCK_GLASS; }
    if(inputSlot.id == BLOCK_COBBLESTONE) { creationID = BLOCK_STONE; }
    if(inputSlot.id == ITEM_PORK)         { creationID = ITEM_GRILLED_PORK; }
    if(inputSlot.id == ITEM_CLAY_BALLS)   { creationID = ITEM_CLAY_BRICK; }
    if(inputSlot.id == ITEM_RAW_FISH)     { creationID = ITEM_COOKED_FISH; }

    // Update other params if we actually converted
    if(creationID != 0)
    {
      // Ok - now check if the current output slot contains the same stuff
      if(outputSlot.id != creationID)
      {
        // No so overwrite it
        outputSlot.id = creationID;
        outputSlot.count = 0;
      }

      // Increment output and decrememnt the input source
      outputSlot.count++;
      inputSlot.count--;
      outputSlot.damage = inputSlot.damage;

      // Bounds check all
      if(outputSlot.count > 64)
      {
        outputSlot.count = 64;
      }

      if(inputSlot.count < 0)
      {
        inputSlot.count = 0;
      }

      // Update the m_slots
      m_slots[SLOT_INPUT]  = inputSlot;
      m_slots[SLOT_FUEL]   = fuelSlot;
      m_slots[SLOT_OUTPUT] = outputSlot;
    }
  }

  // Reset our active cook durations
  m_activeCookDuration = 0;
}
bool Furnace::isBurningFuel()
{
  // Check if this furnace is currently burning
  if(m_fuelBurningTime > 0)
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
  Slot slot = m_slots[SLOT_INPUT];
  if((slot.count != 0) &&
    (
     (slot.id == BLOCK_IRON_ORE) ||
     (slot.id == BLOCK_GOLD_ORE) ||
     (slot.id == BLOCK_SAND) ||
     (slot.id == BLOCK_COBBLESTONE) ||
     (slot.id == ITEM_PORK) ||
     (slot.id == ITEM_CLAY_BALLS) ||
     (slot.id == ITEM_RAW_FISH))
     ){
      return true;
    }
    else {
      return false;
    }
}
void Furnace::consumeFuel()
{
  // Check that we have fuel
  if(m_slots[SLOT_FUEL].count == 0)
    return;

  // Increment the fuel burning time based on fuel type
  // http://www.minecraftwiki.net/wiki/Furnace#Fuel_efficiency
  Slot fuelSlot = m_slots[SLOT_FUEL];
  m_initialBurningTime = 0;
  if(fuelSlot.id == ITEM_COAL)           { m_initialBurningTime += 80; }
  if(fuelSlot.id == BLOCK_PLANK)          { m_initialBurningTime += 15; }
  if(fuelSlot.id == ITEM_STICK)          { m_initialBurningTime += 5; }
  if(fuelSlot.id == BLOCK_WOOD)           { m_initialBurningTime += 15; }
  if(fuelSlot.id == BLOCK_WORKBENCH)     { m_initialBurningTime += 15; }
  if(fuelSlot.id == BLOCK_CHEST)         { m_initialBurningTime += 15; }
  if(fuelSlot.id == BLOCK_BOOKSHELF)     { m_initialBurningTime += 15; }
  if(fuelSlot.id == BLOCK_JUKEBOX)       { m_initialBurningTime += 15; }
  if(fuelSlot.id == BLOCK_FENCE)         { m_initialBurningTime += 15; }
  if(fuelSlot.id == BLOCK_WOODEN_STAIRS) { m_initialBurningTime += 15; }
  if(fuelSlot.id == ITEM_LAVA_BUCKET)    { m_initialBurningTime += 1000; }
  m_fuelBurningTime += m_initialBurningTime;

  // Now decrement the fuel & reset
  m_slots[SLOT_FUEL].count--;
  if(m_slots[SLOT_FUEL].count < 0)
    m_slots[SLOT_FUEL].count = 0;

  // Update our block type if need be
  updateBlock();
}
int16_t Furnace::burnTime()
{
  int16_t fuelBurningTime = (int16_t)((200.0f / m_initialBurningTime) * m_fuelBurningTime);
  if(fuelBurningTime < 0)
    fuelBurningTime = 0;
  return fuelBurningTime;
}
int16_t Furnace::cookTime()
{
  // Express cook time as a fraction of total cooking time
  int16_t tempCookTime = (int16_t)((200.0f / m_cookingTime) * m_activeCookDuration);
  if(tempCookTime < 0)
  {
    tempCookTime = 0;
  }
  return tempCookTime;
}

NBT_Value* Furnace::getSlotEntity(int8_t slotNumber)
{
  // Return null of we don't have anything in this slot
  if(m_slots[slotNumber].count == 0)
  {
    return NULL;
  }

  // Create a new slot NBT entity and add it's data
  NBT_Value* slot = new NBT_Value(NBT_Value::TAG_COMPOUND);
  slot->Insert("Count", new NBT_Value(m_slots[slotNumber].count));
  slot->Insert("Damage", new NBT_Value(m_slots[slotNumber].damage));
  slot->Insert("Slot", new NBT_Value(slotNumber));
  slot->Insert("id", new NBT_Value(m_slots[slotNumber].id));

  return slot;
}
void Furnace::sendToAllUsers()
{
 
}
