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

#include "furnaceManager.h"
#include "furnace.h"
#include "mineserver.h"
#include "logger.h"
#include "tools.h"

void FurnaceManager::update()
{
  // Loop thru all the furnaces
  for (FurnaceContainer::iterator it = m_activeFurnaces.begin(); it != m_activeFurnaces.end(); )
  {
    // Get a pointer to this furnace
    FurnacePtr currentFurnace = *it;

    // If we're burning, decrememnt the fuel
    if (currentFurnace->isBurningFuel())
    {
      currentFurnace->setFuelBurningTime(currentFurnace->fuelBurningTime() - 1);
    }
    // Now that we've decremented, if we're no longer burning fuel but still have stuff to cook, consume fuel
    if (!currentFurnace->isBurningFuel() && currentFurnace->hasValidIngredient())
    {
      currentFurnace->consumeFuel();
    }

    // If we're cooking, increment the activity and check if we're ready to smelt the output
    if (currentFurnace->isCooking())
    {
      currentFurnace->setCookingTime(currentFurnace->cookingTime() + 1);
      if (currentFurnace->cookingTime() >= currentFurnace->cookTime())
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
    if (!currentFurnace->isBurningFuel())
    {
      it = m_activeFurnaces.erase(it);
    }
    else
    {
      ++it;
    }
  }
}

void removeFurnace(furnaceDataPtr data)
{
  ServerInstance->furnaceManager()->removeFurnace(data);
}

struct FurnaceDataFinder
{
  FurnaceDataFinder(furnaceDataPtr d) : data(d) { }
  furnaceDataPtr data;
  inline bool operator()(FurnacePtr f) { return (f->x() == data->x && f->y() == data->y && f->z() == data->z); }
};

void FurnaceManager::removeFurnace(furnaceDataPtr data)
{
  m_activeFurnaces.remove_if(FurnaceDataFinder(data));
}

void FurnaceManager::handleActivity(furnaceDataPtr data)
{
  FurnaceContainer::iterator it = std::find_if(m_activeFurnaces.begin(), m_activeFurnaces.end(), FurnaceDataFinder(data));
  const bool found = !(it == m_activeFurnaces.end());
  FurnacePtr furnace = found ? *it : FurnacePtr(new Furnace(data));

  if (found) furnace->updateItems();

  // Check if this furnace is active
  if ((furnace->isBurningFuel() || furnace->slots()[SLOT_FUEL].getCount() > 0) &&  furnace->hasValidIngredient())
  {
    if (!found) m_activeFurnaces.push_front(furnace);
  }
  else
  {
    if (found)  m_activeFurnaces.erase(it);
  }
}


