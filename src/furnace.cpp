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

#include "furnace.h"
#include <iostream>

//#define _DEBUG

Furnace::Furnace(NBT_Value *entity)
{
  // Setup this furnace
  this->x = (sint32)(*(*entity)["x"]);
  this->y = (sint32)(*(*entity)["y"]);
  this->z = (sint32)(*(*entity)["z"]);
  //this->fuelBurningTime = (sint16)(*(*entity)["BurnTime"]);
  
  // Clean out the slots
  this->slots[SLOT_INPUT].count  = 0;
  this->slots[SLOT_INPUT].damage = 0;
  this->slots[SLOT_INPUT].id     = 0;
  this->slots[SLOT_FUEL].count   = 0;
  this->slots[SLOT_FUEL].damage  = 0;
  this->slots[SLOT_FUEL].id      = 0;
  this->slots[SLOT_OUTPUT].count = 0;
  this->slots[SLOT_OUTPUT].damage= 0;
  this->slots[SLOT_OUTPUT].id    = 0;
  
  // Set the slots to what was passed
  NBT_Value *slotList = (NBT_Value *)(*entity)["Items"];
  std::vector<NBT_Value*> *slotEntities = slotList->GetList();
  std::vector<NBT_Value*>::iterator iter = slotEntities->begin(), end = slotEntities->end();
  for( ; iter != end; iter++ )
  {
    sint8 slotNum = (sint8)(*(**iter)["Slot"]);
    slots[slotNum].count = (sint8)(*(**iter)["Count"]);
    slots[slotNum].damage = (sint16)(*(**iter)["Damage"]);
    slots[slotNum].id = (sint16)(*(**iter)["id"]);
  }
 
  // Set the cooking time based on input type (currently all smelting takes 10 secs but this gives us flexivibility in future)
  sSlot inputSlot = slots[SLOT_INPUT];
  cookingTime = 0;
  if(inputSlot.id == BLOCK_IRON_ORE)     { this->cookingTime = 10; }
  if(inputSlot.id == BLOCK_GOLD_ORE)     { this->cookingTime = 10; }
  if(inputSlot.id == BLOCK_SAND)         { this->cookingTime = 10; }
  if(inputSlot.id == BLOCK_COBBLESTONE)  { this->cookingTime = 10; }
  if(inputSlot.id == ITEM_PORK)          { this->cookingTime = 10; }
  if(inputSlot.id == ITEM_CLAY_BALLS)    { this->cookingTime = 10; }
  if(inputSlot.id == ITEM_RAW_FISH)      { this->cookingTime = 10; }
 
  // Reset our active duration
  this->fuelBurningTime = 0;
  this->activeCookDuration = 0;
  this->burning = false;
  
  // Make sure we're the right kind of block based on our current status
  this->updateBlock();
}     

void Furnace::updateBlock()
{
  // Get a pointer to this furnace's current block
  uint8 block;
  uint8 meta;
  
  // Now make sure that it's got the correct block type based on it's current status
  if(isBurningFuel() && !burning)
  {
    Map::get()->getBlock(this->x, this->y, this->z, &block, &meta);
    // Switch to burning furnace
    Map::get()->setBlock(this->x, this->y, this->z, BLOCK_BURNING_FURNACE, meta);
    Map::get()->sendBlockChange(this->x, this->y, this->z, BLOCK_BURNING_FURNACE, meta);
    this->sendToAllUsers();
    burning = true;
  }
  else if(!isBurningFuel() && burning)
  {
    Map::get()->getBlock(this->x, this->y, this->z, &block, &meta);
    // Switch to regular furnace
    Map::get()->setBlock(this->x, this->y, this->z, BLOCK_FURNACE, meta);
    Map::get()->sendBlockChange(this->x, this->y, this->z, BLOCK_FURNACE, meta);
    this->sendToAllUsers();
    burning = false;
  }
}

void Furnace::smelt()
{
  // Check if we're cooking
  if(this->isCooking()) {
    
    // Convert where applicable
    sSlot inputSlot  = slots[SLOT_INPUT];
    sSlot fuelSlot   = slots[SLOT_FUEL];
    sSlot outputSlot = slots[SLOT_OUTPUT];
    sint32 creationID = 0;
    if(inputSlot.id == BLOCK_IRON_ORE)    { creationID = ITEM_IRON_INGOT; }
    if(inputSlot.id == BLOCK_GOLD_ORE)    { creationID = ITEM_GOLD_INGOT; }
    if(inputSlot.id == BLOCK_SAND)        { creationID = BLOCK_GLASS; }
    if(inputSlot.id == BLOCK_COBBLESTONE) { creationID = BLOCK_STONE; }
    if(inputSlot.id == ITEM_PORK)         { creationID = ITEM_GRILLED_PORK; }
    if(inputSlot.id == ITEM_CLAY_BALLS)   { creationID = ITEM_CLAY_BRICK; }
    if(inputSlot.id == ITEM_RAW_FISH)     { creationID = ITEM_COOKED_FISH; }
        
    // Update other params if we actually converted
    if(creationID != 0) {
      
      // Ok - now check if the current output slot contains the same stuff
      if(outputSlot.id != creationID) {
        // No so overwrite it
        outputSlot.id = creationID;
        outputSlot.count = 0;
      }      
      
      // Increment output and decrememnt the input source
      outputSlot.count++;
      inputSlot.count--;
      outputSlot.damage = inputSlot.damage;
      
      // Update the slots
      this->slots[SLOT_INPUT]  = inputSlot;
      this->slots[SLOT_FUEL]   = fuelSlot;
      this->slots[SLOT_OUTPUT] = outputSlot;
    }
  }
  
  // Reset our active cook durations
  this->activeCookDuration = 0;
}
bool Furnace::isBurningFuel()
{
  // Check if this furnace is currently burning
  if(this->fuelBurningTime > 0) { 
      return true;
    }
    else {
      return false;
    }
}
bool Furnace::isCooking()
{
  // If we're burning fuel and have valid ingredients, we're cooking!
  if(this->isBurningFuel() && this->hasValidIngredient())
    return true;
  else
    return false;
}
bool Furnace::hasValidIngredient() 
{
  // Check that we have a valid input type  
  sSlot slot = slots[SLOT_INPUT];
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
  if(slots[SLOT_FUEL].count == 0)
    return;
    
  // Increment the fuel burning time based on fuel type
  // http://www.minecraftwiki.net/wiki/Furnace#Fuel_efficiency
  sSlot fuelSlot = slots[SLOT_FUEL];
  this->fuelBurningTime = 0;
  if(fuelSlot.id == ITEM_COAL)           { this->fuelBurningTime += 80; }
  if(fuelSlot.id == BLOCK_WOOD)          { this->fuelBurningTime += 15; }
  if(fuelSlot.id == ITEM_STICK)          { this->fuelBurningTime += 5; }
  if(fuelSlot.id == BLOCK_LOG)           { this->fuelBurningTime += 15; }
  if(fuelSlot.id == BLOCK_WORKBENCH)     { this->fuelBurningTime += 15; }
  if(fuelSlot.id == BLOCK_CHEST)         { this->fuelBurningTime += 15; }
  if(fuelSlot.id == BLOCK_BOOKSHELF)     { this->fuelBurningTime += 15; }
  if(fuelSlot.id == BLOCK_JUKEBOX)       { this->fuelBurningTime += 15; }
  if(fuelSlot.id == BLOCK_FENCE)         { this->fuelBurningTime += 15; }
  if(fuelSlot.id == BLOCK_WOODEN_STAIRS) { this->fuelBurningTime += 15; }
  if(fuelSlot.id == ITEM_LAVA_BUCKET)    { this->fuelBurningTime += 1000; }
    
  // Now decrement the fuel & reset
  slots[SLOT_FUEL].count--;
  
  // Update our block type if need be
  this->updateBlock();
}
sint16 Furnace::burnTime() 
{
  // Just return the number of secs we're burning for
  return (sint16)this->fuelBurningTime;
}
sint16 Furnace::cookTime() 
{
  // Express cook time as a fraction of total cooking time
  sint16 tempCookTime = (sint16)((200.0f / this->cookingTime) * this->activeCookDuration);
  if(tempCookTime < 0)
    tempCookTime = 0;
  return tempCookTime;
}
NBT_Value* Furnace::getSlotEntity(sint8 slotNumber) 
{
  // Return null of we don't have anything in this slot
  if(slots[slotNumber].count == 0)
    return NULL;

  // Create a new slot NBT entity and add it's data
  NBT_Value* slot = new NBT_Value(NBT_Value::TAG_COMPOUND);
  slot->Insert("Count", new NBT_Value(slots[slotNumber].count));
  slot->Insert("Damage", new NBT_Value(slots[slotNumber].damage));
  slot->Insert("Slot", new NBT_Value(slotNumber));
  slot->Insert("id", new NBT_Value(slots[slotNumber].id));

  return slot;
}
void Furnace::sendToAllUsers() 
{
  // Create a new compound tag and set it's direct properties
  NBT_Value* newEntity = new NBT_Value(NBT_Value::TAG_COMPOUND);
  newEntity->Insert("BurnTime", new NBT_Value(this->burnTime()));
  newEntity->Insert("CookTime", new NBT_Value(this->cookTime()));
  newEntity->Insert("id", new NBT_Value("Furnace"));
  newEntity->Insert("x", new NBT_Value(this->x));
  newEntity->Insert("y", new NBT_Value(this->y));
  newEntity->Insert("z", new NBT_Value(this->z));
  
  // Add our 3 child compounds for each slot that contains something
  NBT_Value *slotList = new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND);
  for(int i = 0; i <= 2; i++) {
    NBT_Value *slot = getSlotEntity(i);
    if(slot != NULL)
      slotList->GetList()->push_back(slot);
  }
  newEntity->Insert("Items", slotList);
  
  // Write the entity data into a parent Compound
  std::vector<uint8> buffer;
  buffer.push_back(NBT_Value::TAG_COMPOUND);
  buffer.push_back(0);
  buffer.push_back(0);
  newEntity->Write(buffer);
  buffer.push_back(0);
  buffer.push_back(0);

  // Compress the data
  uint8 *compressedData = new uint8[ALLOCATE_NBTFILE];
  z_stream zstream;
  zstream.zalloc = Z_NULL;
  zstream.zfree = Z_NULL;
  zstream.opaque = Z_NULL;
  zstream.next_out = compressedData;
  zstream.next_in = &buffer[0];
  zstream.avail_in = buffer.size();
  zstream.avail_out = ALLOCATE_NBTFILE;
  zstream.total_out = 0;
  zstream.total_in = 0;
  deflateInit2(&zstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15+MAX_WBITS, 8, Z_DEFAULT_STRATEGY);

  // Gzip the data
  if(int state=deflate(&zstream, Z_FULL_FLUSH)!=Z_OK) {
    std::cout << "Error in deflate: " << state << std::endl;            
  }
  deflateEnd(&zstream);  
  
  // Create a new packet to send back to client
  Packet pkt;
  pkt << (sint8)PACKET_COMPLEX_ENTITIES  << this->x << (sint16)this->y << this->z << (sint16)zstream.total_out;
  pkt.addToWrite(compressedData, zstream.total_out);
  delete [] compressedData;
  
  // Tell all users about this guy
  User::sendAll((uint8*)pkt.getWrite(), pkt.getWriteLen());
  
  #ifdef _DEBUG
    std::cout << "Furnace entity data: " << std::endl;       
    newEntity->Print();
  #endif
  
  // Update our map with this guy
  Map::get()->setComplexEntity(x, y, z, newEntity);
  
}

