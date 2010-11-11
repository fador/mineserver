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

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>

#include "logger.h"
#include "constants.h"
#include "config.h"
#include "user.h"

#include "physics.h"

Physics &Physics::get()
{
  static Physics instance;
  return instance;
}

// Physics loop
bool Physics::update()
{
  // Check if needs to be updated
  if(simList.empty())
    return true;
    
  // Iterate each simulation
  std::vector<Sim>::iterator simIt = simList.begin();
  while(simIt != simList.end())
  {
    // Does not check type yet.. water only
    
    // Blocks
    uint8 block, meta;
    for(std::vector<SimBlock>::iterator it = simIt->blocks.begin(), it!=simIt->blocks.end(), it++)
    {
      // If below is free to fall
      if(!Map::get().getBlock(it->x, it->y-1, it->z, &block, &meta))
      {
        // Set new fallblock there
        block = BLOCK_WATER;
        meta = M_FALLING;
        Map::get().setBlock(x,y-1,z, block, meta);
        Map::get().sendBlockChange(x,y-1,z, block, meta);
        
        // Change simulation-block to current block
        it->y--;
        it->id = BLOCK_WATER;
        it->meta = M_FALLING;
        
      }
    }
    
  
    simIt++;
  }
    
  
}

// Add world simulation
bool Physics::addSimulation(int x, int y, int z)
{
  uint8 block; uint8 meta;        
  Map::get().getBlock(x,y,z, &block, &meta);  
  
  // Simulating only water for now..
  simList.push_back(Sim(TYPE_WATER, Simblock temp(block, x, y, z, meta)));
}

