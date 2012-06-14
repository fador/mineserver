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

#include <cstdlib>

#include "plant.h"
#include "mineserver.h"
#include "config.h"
#include "map.h"


std::vector<PlantBlockPtr> growingPlants;

BlockPlant::BlockPlant()
{
  grass_timeout = ServerInstance->config()->iData("mapgen.grassrate");
  crop_timeout = ServerInstance->config()->iData("mapgen.croprate");
  cactus_timeout = ServerInstance->config()->iData("mapgen.cactusrate");
  reed_timeout = ServerInstance->config()->iData("mapgen.reedrate");
  cactus_max = ServerInstance->config()->iData("mapgen.cactusmax");
  reed_max = ServerInstance->config()->iData("mapgen.reedmax");
}

void BlockPlant::onStartedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
 
}

void BlockPlant::onDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{

}

void BlockPlant::onStoppedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{

}

void BlockPlant::addBlock(PlantBlockPtr p2)
{
  for (size_t i = 0; i < growingPlants.size(); ++i)
  {
    PlantBlockPtr p = growingPlants[i];
    if (p->x == p2->x && p->y == p2->y && p->z == p2->z && p->map == p2->map)
    {
      return;
    }
  }
  growingPlants.push_back(p2);
}

void BlockPlant::remBlock(PlantBlockPtr p)
{
  growingPlants.erase(std::remove(growingPlants.begin(), growingPlants.end(), p), growingPlants.end());
}

void BlockPlant::remBlock(int x, int y, int z, int map)
{
  growingPlants.erase(std::remove_if(growingPlants.begin(), growingPlants.end(), PlantBlockFinder(x,y,z,map)), growingPlants.end());
}

void BlockPlant::addBlock(int x, int y, int z, int map)
{
  uint8_t block, meta;
  ServerInstance->map(map)->getBlock(x, y, z, &block, &meta);

  const int b = block;

  if (b == BLOCK_GRASS || b == BLOCK_DIRT || b == BLOCK_CROPS || b == BLOCK_REED || b == BLOCK_CACTUS)
  {
    addBlock(PlantBlockPtr(new PlantBlock(x, y, z, map, 0)));
  }
}

void BlockPlant::addBlocks(int x, int y, int z, int map)
{
  addBlock(x - 1, y, z, map);
  addBlock(x + 1, y, z, map);
  addBlock(x, y - 1, z, map);
  addBlock(x, y + 1, z, map);
  addBlock(x, y, z - 1, map);
  addBlock(x, y, z + 1, map);
  addBlock(x - 1, y, z - 1, map);
  addBlock(x - 1, y, z + 1, map);
  addBlock(x + 1, y, z - 1, map);
  addBlock(x + 1, y, z + 1, map);
  addBlock(x + 1, y + 1, z, map);
  addBlock(x - 1, y + 1, z, map);
  addBlock(x, y + 1, z + 1, map);
  addBlock(x, y + 1, z - 1, map);
  addBlock(x + 1, y - 1, z, map);
  addBlock(x - 1, y - 1, z, map);
  addBlock(x, y - 1, z + 1, map);
  addBlock(x, y - 1, z - 1, map);

}

void BlockPlant::timer200()
{
  for (int i = growingPlants.size() - 1; i >= 0; i--)
  {
    PlantBlockPtr p = growingPlants[i];
    uint8_t block, meta, sky, light;

    ServerInstance->map(p->map)->getBlock(p->x, p->y, p->z, &block, &meta);
    ServerInstance->map(p->map)->getLight(p->x, p->y, p->z, &light, &sky);

    p->count++;

    if (p->count > grass_timeout * 5 && (block == BLOCK_DIRT || block == BLOCK_GRASS))
    {
      uint8_t block2, meta2;
      ServerInstance->map(p->map)->getBlock(p->x, p->y + 1, p->z, &block2, &meta2);
      if (block2 == BLOCK_AIR || block2 == BLOCK_SAPLING ||
          block2 == BLOCK_LEAVES || block2 == BLOCK_GLASS ||
          block2 == BLOCK_BROWN_MUSHROOM || block2 == BLOCK_RED_MUSHROOM ||
          block2 == BLOCK_YELLOW_FLOWER || block2 == BLOCK_RED_ROSE ||
          block2 == BLOCK_TORCH || block2 == BLOCK_FIRE ||
          block2 == BLOCK_SAPLING || block2 == BLOCK_SIGN_POST ||
          block2 == BLOCK_WOODEN_DOOR || block2 == BLOCK_LADDER ||
          block2 == BLOCK_WALL_SIGN || block2 == BLOCK_LEVER ||
          block2 == BLOCK_IRON_DOOR || block2 == BLOCK_REDSTONE_TORCH_OFF ||
          block2 == BLOCK_REDSTONE_TORCH_ON || block2 == BLOCK_STONE_BUTTON ||
          block2 == BLOCK_SNOW)
      {
        // The grass can grow
        if (block == BLOCK_DIRT)
        {
          if (light > 4 || sky > 3)
          {
            ServerInstance->map(p->map)->sendBlockChange(p->x, p->y, p->z, (char)BLOCK_GRASS, 0);
            ServerInstance->map(p->map)->setBlock(p->x, p->y, p->z, (char)BLOCK_GRASS, 0);
            addBlocks(p->x, p->y, p->z, p->map);
          }
        }
      }
      else
      {
        if (block == BLOCK_GRASS)
        {
          ServerInstance->map(p->map)->sendBlockChange(p->x, p->y, p->z, (char)BLOCK_DIRT, 0);
          ServerInstance->map(p->map)->setBlock(p->x, p->y, p->z, (char)BLOCK_DIRT, 0);
          addBlocks(p->x, p->y, p->z, p->map);
        }
      }

      remBlock(p);
      continue;
    }
    if (p->count > crop_timeout * 5 && block == BLOCK_CROPS)
    {
      p->count = 0;
      if (light > 7 || sky > 3)
      {
        if (meta < 7)
        {
          meta ++;
        }
        else
        {
          remBlock(p);
          continue;
        }
        ServerInstance->map(p->map)->sendBlockChange(p->x, p->y, p->z, (char)BLOCK_CROPS, meta);
        ServerInstance->map(p->map)->setBlock(p->x, p->y, p->z, (char)BLOCK_CROPS, meta);
      }
    }
    if (p->count > cactus_timeout * 5 && block == BLOCK_CACTUS)
    {
      uint8_t block, meta;
      if (!ServerInstance->map(p->map)->getBlock(p->x, p->y + 1, p->z, &block, &meta))
      {
        remBlock(p);
        continue;
      }
      if (block != BLOCK_AIR)
      {
        remBlock(p);
        continue;
      }
      p->count = 0;
      for (int i = 0 ; i < cactus_max; i++)
      {
        ServerInstance->map(p->map)->getBlock(p->x, p->y - i, p->z, &block, &meta);
        if (block != BLOCK_CACTUS && block != BLOCK_SAND)
        {
          onBroken(NULL, 0, p->x, p->y, p->z, p->map, 0);
          remBlock(p);
          i = cactus_max;
        }
        if (block == BLOCK_SAND)
        {
          ServerInstance->map(p->map)->sendBlockChange(p->x, p->y + 1, p->z, (char)BLOCK_CACTUS, 0);
          ServerInstance->map(p->map)->setBlock(p->x, p->y + 1, p->z, (char)BLOCK_CACTUS, 0);
          addBlocks(p->x, p->y, p->z, p->map);
          break;
        }
      }
    }
    if (p->count > reed_timeout * 5 && (block == BLOCK_REED))
    {
      uint8_t block, meta;
      if (!ServerInstance->map(p->map)->getBlock(p->x, p->y + 1, p->z, &block, &meta))
      {
        remBlock(p);
        continue;
      }
      if (block != BLOCK_AIR)
      {
        remBlock(p);
        continue;
      }
      p->count = 0;
      for (int i = 0 ; i < reed_max; i++)
      {
        ServerInstance->map(p->map)->getBlock(p->x, p->y - i, p->z, &block, &meta);
        if (block != BLOCK_REED && block != BLOCK_GRASS && block != BLOCK_DIRT)
        {
          onBroken(NULL, 0, p->x, p->y, p->z, p->map, 0);
          remBlock(p);
          i = reed_max;
        }
        if (block == BLOCK_GRASS || block == BLOCK_DIRT)
        {
          ServerInstance->map(p->map)->sendBlockChange(p->x, p->y + 1, p->z, (char)BLOCK_REED, 0);
          ServerInstance->map(p->map)->setBlock(p->x, p->y + 1, p->z, (char)BLOCK_REED, 0);
          addBlocks(p->x, p->y, p->z, p->map);
          break;
        }
      }
    }

  }
}


bool BlockPlant::onBroken(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  uint8_t block, meta;
  ServerInstance->map(map)->getBlock(x, y, z, &block, &meta);
  ServerInstance->map(map)->sendBlockChange(x, y, z, BLOCK_AIR, 0);
  ServerInstance->map(map)->setBlock(x, y, z, BLOCK_AIR, 0);
  remBlock(x, y, z, map);
  if (block == BLOCK_CROPS && meta == 7)
  {
    ServerInstance->map(map)->createPickupSpawn(x, y + 1, z, ITEM_WHEAT, 1, 0, NULL);
  }
  else if (block == BLOCK_CROPS)
  {
    ServerInstance->map(map)->createPickupSpawn(x, y + 1, z, ITEM_SEEDS, 1, 0, NULL);
  }
  else if (block == BLOCK_REED)
  {
    ServerInstance->map(map)->createPickupSpawn(x, y + 1, z, ITEM_REED, 1, 0, NULL);
  }
  else
  {
    this->spawnBlockItem(x, y, z, map, block);
  }
  ServerInstance->map(map)->getBlock(x, y + 1, z, &block, &meta);
  if (isPlant(block))
  {
    onBroken(user, status, x, y + 1, z, map, direction);
  }
  return false;
}

void BlockPlant::onNeighbourBroken(User* user, int16_t oldblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  //   uint8_t block,meta,up_block,up_meta;
  //   if (!ServerInstance->map(map)->getBlock(x, y, z, &block, &meta))
  //     return;
  //   if (!ServerInstance->map(map)->getBlock(x, y-1, z,&up_block, &up_meta))
  //     return;
  //
  //   if (direction == BLOCK_TOP && isPlant(up_block))
  //   {
  //      // Break plant and spawn plant item
  //      ServerInstance->map(map)->sendBlockChange(x, y, z, BLOCK_AIR, 0);
  //      ServerInstance->map(map)->setBlock(x, y, z, BLOCK_AIR, 0);
  //      this->spawnBlockItem(x, y, z, map, block);
  //   }
}

bool BlockPlant::isPlant(int num)
{
  return (num == BLOCK_CROPS || num == BLOCK_CACTUS || num == BLOCK_YELLOW_FLOWER || num == BLOCK_RED_ROSE || num == BLOCK_REED || num == BLOCK_SAPLING || num == BLOCK_RED_MUSHROOM || num == BLOCK_BROWN_MUSHROOM);
}

bool BlockPlant::onPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  uint8_t oldblock;
  uint8_t oldmeta;

  /* move the x,y,z coords dependent upon placement direction */
  if (!this->translateDirection(&x, &y, &z, map, direction))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

  if (!ServerInstance->map(map)->getBlock(x, y - 1, z, &oldblock, &oldmeta))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

  if (newblock != BLOCK_DIRT && newblock != BLOCK_SOIL && newblock != BLOCK_GRASS)
  {
    if (this->isBlockEmpty(x, y - 1, z, map) || !this->isBlockEmpty(x, y, z, map))
    {
      revertBlock(user, x, y, z, map);
      return true;
    }

    if (!this->isBlockStackable(oldblock))
    {
      revertBlock(user, x, y, z, map);
      return true;
    }
  }
  else
  {
    if (this->isUserOnBlock(x, y, z, map))
    {
      revertBlock(user, x, y, z, map);
      return true;
    }
  }

  if ((newblock == BLOCK_REED || newblock == ITEM_REED) && (oldblock == BLOCK_GRASS || oldblock == BLOCK_DIRT))
  {
    // TODO : Check for water
    ServerInstance->map(map)->sendBlockChange(x, y, z, BLOCK_REED, 0);
    ServerInstance->map(map)->setBlock(x, y, z, BLOCK_REED, 0);
    addBlock(x, y, z, map);
    return false;
  }

  if (newblock == BLOCK_CACTUS && oldblock != BLOCK_SAND)
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

  if ((newblock == BLOCK_YELLOW_FLOWER  ||
       newblock == BLOCK_RED_ROSE) && (oldblock != BLOCK_DIRT &&
                                       oldblock != BLOCK_GRASS))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

  if ((newblock == ITEM_SEEDS || newblock == BLOCK_CROPS) && (oldblock == BLOCK_SOIL))
  {
    ServerInstance->map(map)->sendBlockChange(x, y, z, BLOCK_CROPS, 0);
    ServerInstance->map(map)->setBlock(x, y, z, BLOCK_CROPS, 0);
    addBlock(x, y, z, map);
    return false;
  }

  if ((newblock == ITEM_SEEDS || newblock == BLOCK_CROPS))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

  if (newblock > 255 && (oldblock == BLOCK_DIRT  || oldblock == BLOCK_GRASS))
  {
    // Hoe on dirt = Soil
    ServerInstance->map(map)->sendBlockChange(x, y - 1, z, BLOCK_SOIL, 0);
    ServerInstance->map(map)->setBlock(x, y - 1, z, BLOCK_SOIL, 0);
    if (SEEDS_CHANCE >= rand() % 10000)
    {
      ServerInstance->map(map)->createPickupSpawn(x, y + 1, z, ITEM_SEEDS, 1, 0, NULL);
    }
    return true;
  }

  if (newblock > 255)
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

  if ((newblock == BLOCK_BROWN_MUSHROOM || newblock == BLOCK_RED_MUSHROOM)
      && oldblock != BLOCK_DIRT)
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

  if (newblock == BLOCK_SAPLING)
  {
    ServerInstance->map(map)->addSapling(user, x, y, z);
  }
  else
  {
    ServerInstance->map(map)->sendBlockChange(x, y, z, newblock, 0);
    ServerInstance->map(map)->setBlock(x, y, z, newblock, 0);
    addBlocks(x, y, z, map);
  }
  return false;
}

void BlockPlant::onNeighbourPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  addBlocks(x, y, z, map);
}

void BlockPlant::onReplace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{

}
