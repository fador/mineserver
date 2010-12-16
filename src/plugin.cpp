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

#include "plugin.h"
#include "logger.h"
#include "blocks/default.h"
#include "blocks/falling.h"
#include "blocks/torch.h"
#include "blocks/plant.h"
#include "blocks/snow.h"
#include "blocks/liquid.h"
#include "blocks/fire.h"
#include "blocks/stair.h"
#include "blocks/door.h"
#include "blocks/sign.h"
#include "blocks/tracks.h"
#include "blocks/chest.h"

Plugin* Plugin::m_plugin;

void Plugin::free()
{
   if (m_plugin)
   {
      delete m_plugin;
      m_plugin = NULL;
   }
}

void Plugin::init()
{
   // Set default behaviours 
   Callback call;
   /* FIXME: must remember to delete any memory we create here upon server stop */
   
   BlockDefault* defaultblock = new BlockDefault();
   call.add("onBroken", Function::from_method<BlockDefault, &BlockDefault::onBroken>(defaultblock));
   call.add("onPlace", Function::from_method<BlockDefault, &BlockDefault::onPlace>(defaultblock));
   setBlockCallback(BLOCK_STONE, call);
   setBlockCallback(BLOCK_GRASS, call);
   setBlockCallback(BLOCK_DIRT, call);
   setBlockCallback(BLOCK_COBBLESTONE, call);
   setBlockCallback(BLOCK_WOOD, call);
   setBlockCallback(BLOCK_LOG, call);
   setBlockCallback(BLOCK_SOIL, call);
   setBlockCallback(BLOCK_GLASS, call);
   /* cloth */
   setBlockCallback(BLOCK_RED_CLOTH, call);
   setBlockCallback(BLOCK_ORANGE_CLOTH, call);
   setBlockCallback(BLOCK_YELLOW_CLOTH, call);
   setBlockCallback(BLOCK_LIME_CLOTH, call);
   setBlockCallback(BLOCK_GREEN_CLOTH, call);
   setBlockCallback(BLOCK_AQUA_GREEN_CLOTH, call);
   setBlockCallback(BLOCK_CYAN_CLOTH, call);
   setBlockCallback(BLOCK_BLUE_CLOTH, call);
   setBlockCallback(BLOCK_PURPLE_CLOTH, call);
   setBlockCallback(BLOCK_INDIGO_CLOTH, call);
   setBlockCallback(BLOCK_VIOLET_CLOTH, call);
   setBlockCallback(BLOCK_MAGENTA_CLOTH, call);
   setBlockCallback(BLOCK_PINK_CLOTH, call);
   setBlockCallback(BLOCK_BLACK_CLOTH, call);
   setBlockCallback(BLOCK_GRAY_CLOTH, call);
   setBlockCallback(BLOCK_WHITE_CLOTH, call);
   /* metals */
   setBlockCallback(BLOCK_GOLD_BLOCK, call);
   setBlockCallback(BLOCK_IRON_BLOCK, call);
   
   setBlockCallback(BLOCK_DOUBLE_STEP, call);
   setBlockCallback(BLOCK_STEP, call);
   setBlockCallback(BLOCK_BRICK, call);
   setBlockCallback(BLOCK_BOOKSHELF, call);
   setBlockCallback(BLOCK_MOSSY_COBBLESTONE, call);
   setBlockCallback(BLOCK_OBSIDIAN, call);
   setBlockCallback(BLOCK_MOB_SPAWNER, call);
   setBlockCallback(BLOCK_DIAMOND_BLOCK, call);
   setBlockCallback(BLOCK_PUMPKIN, call);
   setBlockCallback(BLOCK_CLAY, call);
   setBlockCallback(BLOCK_NETHERSTONE, call);
   setBlockCallback(BLOCK_LIGHTSTONE, call);
   setBlockCallback(BLOCK_JACK_O_LANTERN, call);
   setBlockCallback(BLOCK_JUKEBOX, call);
   setBlockCallback(BLOCK_FENCE, call);
   setBlockCallback(BLOCK_GOLD_ORE, call);
   setBlockCallback(BLOCK_IRON_ORE, call);
   setBlockCallback(BLOCK_COAL_ORE, call);
   setBlockCallback(BLOCK_DIAMOND_ORE, call);
   setBlockCallback(BLOCK_GLOWING_REDSTONE_ORE, call);
   setBlockCallback(BLOCK_REDSTONE_ORE, call);

   /* Falling blocks (sand, etc) */
   call.reset();
   BlockFalling* fallingblock = new BlockFalling();
   call.add("onBroken", Function::from_method<BlockDefault, &BlockDefault::onBroken>(defaultblock));
   call.add("onPlace", Function::from_method<BlockFalling, &BlockFalling::onPlace>(fallingblock));
   call.add("onNeighbourBroken", Function::from_method<BlockFalling, &BlockFalling::onNeighbourBroken>(fallingblock));
   call.add("onNeighbourMove", Function::from_method<BlockFalling, &BlockFalling::onNeighbourMove>(fallingblock));
   setBlockCallback(BLOCK_SAND, call);
   setBlockCallback(BLOCK_SLOW_SAND, call);
   setBlockCallback(BLOCK_GRAVEL, call);
   
   /* Torches */
   call.reset();
   BlockTorch* torchblock = new BlockTorch();
   call.add("onBroken", Function::from_method<BlockDefault, &BlockDefault::onBroken>(defaultblock));
   call.add("onPlace", Function::from_method<BlockTorch, &BlockTorch::onPlace>(torchblock));
   call.add("onNeighbourBroken", Function::from_method<BlockTorch, &BlockTorch::onNeighbourBroken>(torchblock));
   call.add("onReplace", Function::from_method<BlockDefault, &BlockDefault::onReplace>(defaultblock));
   call.add("onNeighbourMove", Function::from_method<BlockTorch, &BlockTorch::onNeighbourMove>(torchblock));
   setBlockCallback(BLOCK_TORCH, call);
   setBlockCallback(BLOCK_REDSTONE_TORCH_OFF, call);
   setBlockCallback(BLOCK_REDSTONE_TORCH_ON, call);
   
   /* ladders */
   call.reset();
   call.add("onBroken", Function::from_method<BlockDefault, &BlockDefault::onBroken>(defaultblock));
   call.add("onPlace", Function::from_method<BlockDefault, &BlockDefault::onPlace>(defaultblock));
   call.add("onNeighbourBroken", Function::from_method<BlockTorch, &BlockTorch::onNeighbourBroken>(torchblock));
   setBlockCallback(BLOCK_LADDER, call);

   /* Plants */
   call.reset();
   BlockPlant* plantblock = new BlockPlant();
   call.add("onBroken", Function::from_method<BlockDefault, &BlockDefault::onBroken>(defaultblock));
   call.add("onPlace", Function::from_method<BlockPlant, &BlockPlant::onPlace>(plantblock));
   call.add("onNeighbourBroken", Function::from_method<BlockPlant, &BlockPlant::onNeighbourBroken>(plantblock));
   call.add("onReplace", Function::from_method<BlockDefault, &BlockDefault::onReplace>(defaultblock));
   setBlockCallback(BLOCK_YELLOW_FLOWER, call);
   setBlockCallback(BLOCK_RED_ROSE, call);
   setBlockCallback(BLOCK_BROWN_MUSHROOM, call);
   setBlockCallback(BLOCK_RED_MUSHROOM, call);
   setBlockCallback(BLOCK_CROPS, call);
   setBlockCallback(BLOCK_CACTUS, call);
   setBlockCallback(BLOCK_REED, call);
   setBlockCallback(BLOCK_SAPLING, call);
   
   /* Snow */
   call.reset();
   BlockSnow* snowblock = new BlockSnow();
   call.add("onNeighbourBroken", Function::from_method<BlockSnow, &BlockSnow::onNeighbourBroken>(snowblock));
   call.add("onPlace", Function::from_method<BlockDefault, &BlockDefault::onPlace>(defaultblock));
   setBlockCallback(BLOCK_SNOW, call);

   /* Lava and Water */
   call.reset();
   BlockLiquid* liquidblock = new BlockLiquid();
   call.add("onPlace", Function::from_method<BlockLiquid, &BlockLiquid::onPlace>(liquidblock));
   call.add("onNeighbourBroken", Function::from_method<BlockLiquid, &BlockLiquid::onNeighbourBroken>(liquidblock));
   call.add("onReplace", Function::from_method<BlockLiquid, &BlockLiquid::onReplace>(liquidblock));
   setBlockCallback(BLOCK_WATER, call);
   setBlockCallback(BLOCK_STATIONARY_WATER, call);
   setBlockCallback(BLOCK_LAVA, call);
   setBlockCallback(BLOCK_STATIONARY_LAVA, call);
   setBlockCallback(ITEM_WATER_BUCKET, call);
   setBlockCallback(ITEM_LAVA_BUCKET, call);

   /* Fire */
   call.reset();
   BlockFire* fireblock = new BlockFire();
   call.add("onPlace", Function::from_method<BlockFire, &BlockFire::onPlace>(fireblock));
   setBlockCallback(BLOCK_FIRE, call);
   setBlockCallback(ITEM_FLINT_AND_STEEL, call);

   /* Stairs */
   call.reset();
   BlockStair* stairblock = new BlockStair();
   call.add("onBroken", Function::from_method<BlockDefault, &BlockDefault::onBroken>(defaultblock));
   call.add("onPlace", Function::from_method<BlockDefault, &BlockDefault::onPlace>(defaultblock));
   call.add("onNeighbourBroken", Function::from_method<BlockStair, &BlockStair::onNeighbourBroken>(stairblock));
   setBlockCallback(BLOCK_WOODEN_STAIRS, call);
   setBlockCallback(BLOCK_COBBLESTONE_STAIRS, call);
   
   /* TNT */
   call.reset();
   call.add("onPlace", Function::from_method<BlockDefault, &BlockDefault::onPlace>(defaultblock));
   call.add("onBroken", Function::from_method<BlockDefault, &BlockDefault::onBroken>(defaultblock));
   /* TODO: Currently works like glass. Explosion is not implemented yet. */
   setBlockCallback(BLOCK_TNT, call);
   
   /* Workbench and furnace */
   call.reset();
   call.add("onBroken", Function::from_method<BlockDefault, &BlockDefault::onBroken>(defaultblock));
   call.add("onPlace", Function::from_method<BlockDefault, &BlockDefault::onPlace>(defaultblock));
   setBlockCallback(BLOCK_WORKBENCH, call);
   setBlockCallback(BLOCK_FURNACE, call);
   /* TODO: Needs this? BLOCK_BURNING_FURNACE */
  
   /* Chests */
   call.reset();
   BlockChest* chestblock = new BlockChest();
   call.add("onBroken", Function::from_method<BlockDefault, &BlockDefault::onBroken>(defaultblock));
   call.add("onPlace", Function::from_method<BlockChest, &BlockChest::onPlace>(chestblock));
   call.add("onStartedDigging", Function::from_method<BlockChest, &BlockChest::onStartedDigging>(chestblock));
   setBlockCallback(BLOCK_CHEST, call);

   /* Doors */
   call.reset();
   BlockDoor* doorblock = new BlockDoor();
   call.add("onStartedDigging", Function::from_method<BlockDoor, &BlockDoor::onStartedDigging>(doorblock));
   call.add("onBroken", Function::from_method<BlockDefault, &BlockDefault::onBroken>(defaultblock));
   call.add("onPlace", Function::from_method<BlockDoor, &BlockDoor::onPlace>(doorblock));
   setBlockCallback(BLOCK_WOODEN_DOOR, call);
   setBlockCallback(BLOCK_IRON_DOOR, call);
   setBlockCallback(ITEM_WOODEN_DOOR, call);
   setBlockCallback(ITEM_IRON_DOOR, call);

   /* leaves */
   call.reset();
   call.add("onBroken", Function::from_method<BlockDefault, &BlockDefault::onBroken>(defaultblock));
   setBlockCallback(BLOCK_LEAVES, call);

   /* signs */
   call.reset();
   BlockSign* signblock = new BlockSign();
   call.add("onBroken", Function::from_method<BlockDefault, &BlockDefault::onBroken>(defaultblock));
   call.add("onNeighbourBroken", Function::from_method<BlockTorch, &BlockTorch::onNeighbourBroken>(torchblock));
   call.add("onPlace", Function::from_method<BlockSign, &BlockSign::onPlace>(signblock));
   setBlockCallback(BLOCK_WALL_SIGN, call);
   setBlockCallback(BLOCK_SIGN_POST, call);
   setBlockCallback(ITEM_SIGN, call);
   
   /* minecart tracks */
   call.reset();
   BlockTracks* tracksblock = new BlockTracks();
   call.add("onBroken", Function::from_method<BlockDefault, &BlockDefault::onBroken>(defaultblock));
   call.add("onPlace", Function::from_method<BlockTracks, &BlockTracks::onPlace>(tracksblock));
   call.add("onNeighbourBroken", Function::from_method<BlockTracks, &BlockTracks::onNeighbourBroken>(tracksblock));
//   call.add("onNeighbourPlace", Function::from_method<BlockTracks, &BlockTracks::onNeighbourPlace>(tracksblock));
//   call.add("onNeighbourMove", Function::from_method<BlockTracks, &BlockTracks::onNeighbourMove>(tracksblock));
   setBlockCallback(BLOCK_MINECART_TRACKS, call);


  /* TODO: Unimplemented */
  /* BLOCK_SPONGE */
  /* BLOCK_REDSTONE_WIRE */
  /* BLOCK_PORTAL */
  /* BLOCK_LEVER, BLOCK_STONE_BUTTON */
  /* BLOCK_WOODEN_PRESSURE_PLATE, BLOCK_STONE_PRESSURE_PLATE */
  /* BLOCK_ICE */
  /* BLOCK_SNOW_BLOCK */
}

void Plugin::setBlockCallback(const int type, Callback call)
{
   if (getBlockCallback(type))
   {
      LOG("Block type set more then once.");
      removeBlockCallback(type);
   }
   blockevents.insert(std::pair<int, Callback>(type, call));
}

Callback* Plugin::getBlockCallback(const int type)
{
   Callbacks::iterator iter = blockevents.find(type);

   if (iter == blockevents.end())
      return NULL;

   return &(*iter).second;
}

bool Plugin::runBlockCallback(const int type, const std::string name, const Function::invoker_type function)
{
   Callbacks::iterator iter = blockevents.find(type);

   if (iter == blockevents.end())
      return false;

   return (*iter).second.run(name, function);
}

bool Plugin::removeBlockCallback(const int type)
{
   Callbacks::iterator iter = blockevents.find(type);

   if (iter == blockevents.end())
      return false;

   blockevents.erase(iter);
   return true;
}
