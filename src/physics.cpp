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

