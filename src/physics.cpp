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
#ifdef WIN32
    #define _CRTDBG_MAP_ALLOC
    #include <stdlib.h>
    #include <crtdbg.h>
    #include <conio.h>
    #include <winsock2.h>
typedef  int socklen_t;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>

#include "logger.h"
#include "constants.h"
#include "config.h"
#include "user.h"
#include "map.h"

#include "physics.h"

Physics &Physics::get()
{
  static Physics instance;
  return instance;
}

// Physics loop
bool Physics::update()
{
  if(!enabled)
    return true;
  // Check if needs to be updated
  if(simList.empty())
    return true;
    
  std::cout << "Simulating " << simList.size() << " items!" << std::endl;
  // Iterate each simulation
  for(sint32 simIt = (sint32)(simList.size()-1);simIt>=0;simIt--)
  {
    // Blocks
    uint8 block, meta;

    //Water simulation
    if(simList[simIt].type == TYPE_WATER)
    {
      sint32 it=0;
      //for(sint32 it=simList[simIt].blocks.size()-1; it>=0; it--)
      {
        int x=simList[simIt].blocks[it].x;
        int y=simList[simIt].blocks[it].y;
        int z=simList[simIt].blocks[it].z;

        // If below is free to fall
        if(Map::get().getBlock(x, y-1, z, &block, &meta)
           && (block == BLOCK_AIR || block == BLOCK_WATER || block == BLOCK_STATIONARY_WATER) )
        {
          // Set new fallblock there
          block = BLOCK_WATER;
          meta = M_FALLING;
          Map::get().setBlock(x,y-1,z, block, meta);
          Map::get().sendBlockChange(x,y-1,z, block, meta);        
          // Change simulation-block to current block
          simList[simIt].blocks[it].y--;
          simList[simIt].blocks[it].id = BLOCK_WATER;
          simList[simIt].blocks[it].meta = M_FALLING;        
        }
        //Else if spreading to sides
        //If water level is at minimum, dont simulate anymore
        else if((meta&M7)!=M7)
        {
          for(int i=0;i<4;i++)
          {
            int x_local=x;
            int y_local=y;
            int z_local=z;
            switch(i)
            {
              case 0: x_local++; break;
              case 1: x_local--; break;
              case 2: z_local++; break;
              case 3: z_local--; break;
            }

            if(Map::get().getBlock(x_local, y_local, z_local, &block, &meta)
               && (block == BLOCK_AIR || block == BLOCK_WATER || block == BLOCK_STATIONARY_WATER) )
            {
              //Decrease water level each turn
              if(block == BLOCK_AIR || meta>(simList[simIt].blocks[it].meta&0x07)+1)
              {
                meta=(simList[simIt].blocks[it].meta&0x07)+1;
                Map::get().setBlock(x_local, y_local, z_local, BLOCK_WATER, meta);
                Map::get().sendBlockChange(x_local, y_local, z_local, BLOCK_WATER, meta);
                if(meta < M7)
                {
                  addSimulation(x_local, y_local, z_local);
                }
              }

            }            
          } // End for i=0:3

          //Remove this block from simulation
          simList.erase(simList.begin()+simIt);
        } 
        //Water level at minimum
        else
        {
          //Todo: Check if source has drained

          //Remove this block from simulation
          simList.erase(simList.begin()+simIt);
        }
        
      }
      
    }
  }
    
  return true;
}

// Add world simulation
bool Physics::addSimulation(int x, int y, int z)
{
  if(!enabled)
    return true;
  uint8 block; uint8 meta;        
  Map::get().getBlock(x,y,z, &block, &meta);  
  
  // Simulating water
  if(block == BLOCK_WATER || block == BLOCK_STATIONARY_WATER)
  {
    simList.push_back(Sim(TYPE_WATER, SimBlock(block, x, y, z, meta)));
    return true;
  }
  // Simulating lava
  else if(block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA)
  {    
    simList.push_back(Sim(TYPE_LAVA, SimBlock(block, x, y, z, meta)));
    return true;
  }

  return false;
}



bool Physics::checkSurrounding(int x, int y, int z)
{
  if(!enabled)
    return true;
  uint8 block; uint8 meta;
  
  for(int i=0;i<5;i++)
  {
    int x_local=x;
    int y_local=y;
    int z_local=z;
    switch(i)
    {
      case 0: y_local++; break;
      case 1: x_local++; break;
      case 2: x_local--; break;
      case 3: z_local++; break;
      case 4: z_local--; break;
    }

    //Add liquid blocks to simulation if they are affected by breaking a block
    if(Map::get().getBlock(x_local,y_local,z_local, &block, &meta) &&
       (block == BLOCK_WATER || block == BLOCK_STATIONARY_WATER ||
        block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA ))
    {
      addSimulation(x_local,y_local,z_local);
    }
  }

  return true;

}