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

#include "furnaceManager.h"
#include "furnace.h"
#include <iostream>

//#define _DEBUG

FurnaceManager* FurnaceManager::_instance;

FurnaceManager::FurnaceManager()
{
}

void FurnaceManager::update()
{

  // Bail if we don't have any furnaces
  if(m_activeFurnaces.size() == 0)
  {
    return;
  }

#ifdef _DEBUG
  Screen::get()->log("Checking Furnaces: " + dtos(m_activeFurnaces.size()) + " active furnaces.");
#endif

  // Loop thru all the furnaces
  for(unsigned int index = 0; index < m_activeFurnaces.size(); index++)
  {
    // Get a pointer to this furnace
    Furnace* currentFurnace = (Furnace*)m_activeFurnaces[index];

    // If we're burning, decrememnt the fuel
    if(currentFurnace->isBurningFuel())
    {
      currentFurnace->setFuelBurningTime(currentFurnace->fuelBurningTime() - 1);
    }
    // Now that we've decremented, if we're no longer burning fuel but still have stuff to cook, consume fuel
    if (!currentFurnace->isBurningFuel() && currentFurnace->hasValidIngredient())
    {
      currentFurnace->consumeFuel();
    }

    // If we're cooking, increment the activity and check if we're ready to smelt the output
    if(currentFurnace->isCooking())
    {
      currentFurnace->setActiveCookDuration(currentFurnace->activeCookDuration() + 1);
      if(currentFurnace->activeCookDuration() >= currentFurnace->cookingTime())
      {
        // Finished cooking time, so create the output
        currentFurnace->smelt();
      }
    }

    // Update all clients
    currentFurnace->sendToAllUsers();

    // Update it's block style
    currentFurnace->updateBlock();

    // Remove this furnace from the list once it stops burning it's current fuel
    if(!currentFurnace->isBurningFuel())
    {
      delete m_activeFurnaces[index];
      m_activeFurnaces.erase(m_activeFurnaces.begin() + index);
    }
  }
}

void FurnaceManager::handleActivity(NBT_Value *entity, uint8 blockType)
{
  // Create a furnace
  Furnace* furnace = new Furnace(entity, blockType);

  // Loop thru all active furnaces, to see if this one is here
  for(unsigned int index = 0; index < m_activeFurnaces.size(); index++)
  {
    Furnace* currentFurnace = (Furnace*)m_activeFurnaces[index];
    if(currentFurnace->x() == furnace->x() && currentFurnace->y() == furnace->y() && currentFurnace->z() == furnace->z())
    {
      // Preserve the current burning time
      furnace->setFuelBurningTime(currentFurnace->fuelBurningTime());
      furnace->setActiveCookDuration(currentFurnace->activeCookDuration());
      // Now delete it (we'll add back later if it's active)
      delete m_activeFurnaces[index];
      m_activeFurnaces.erase(m_activeFurnaces.begin() + index);
    }
  }

  // Check if this furnace is active
  if(furnace->isBurningFuel() || furnace->slots()[SLOT_FUEL].count > 0)
  {
    m_activeFurnaces.push_back(furnace);
  }
  else
  {
    delete furnace;
    furnace = NULL;
  }

  // Let everyone know about this furnace
  if(furnace)
  {
    furnace->sendToAllUsers();
  }
}

void FurnaceManager::free()
{
   if(_instance)
   {
      delete _instance;
      _instance = 0;
   }
}




