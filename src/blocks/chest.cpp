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

#include "chest.h"

#include "user.h"
#include "permissions.h"
#include "nbt.h"
#include "mineserver.h"
#include "map.h"
#include "chat.h"
#include "tools.h"

void BlockChest::onStartedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  // Locksystem
  if (user->inv[36 + user->currentItemSlot()].getType() == ITEM_WOODEN_AXE)
  {
    int chunk_x = blockToChunk(x);
    int chunk_z = blockToChunk(z);

    sChunk* chunk = ServerInstance->map(map)->loadMap(chunk_x, chunk_z);

    if (chunk == NULL)
    {
      return;
    }

    NBT_Value* entityList = (*(*(chunk->nbt))["Level"])["TileEntities"];

    if (!entityList)
    {
      entityList = new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND);
      chunk->nbt->Insert("TileEntities", entityList);
    }

    if (entityList->GetType() == NBT_Value::TAG_LIST)
    {
      if (entityList->GetListType() != NBT_Value::TAG_COMPOUND)
      {
        entityList->SetType(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND);
      }

      std::vector<NBT_Value*> *entities = entityList->GetList();
      std::vector<NBT_Value*>::iterator iter = entities->begin(), end = entities->end();

      //bool done = false; // Unused variable

      for (; iter != end; iter++)
      {
        if ((**iter)["x"] == NULL || (**iter)["y"] == NULL || (**iter)["z"] == NULL ||
            (**iter)["x"]->GetType() != NBT_Value::TAG_INT ||
            (**iter)["y"]->GetType() != NBT_Value::TAG_INT ||
            (**iter)["z"]->GetType() != NBT_Value::TAG_INT)
        {
          continue;
        }

        if ((int32_t)(*(**iter)["x"]) == x && (int32_t)(*(**iter)["y"]) == y && (int32_t)(*(**iter)["z"]) == z)
        {
          int8_t locked;
          NBT_Value* nbtLockdata = (**iter)["Lockdata"];
          if (nbtLockdata != NULL)
          {
            std::string player = *(*nbtLockdata)["player"]->GetString();
            // Toggle lock if player is the owner of block
            if (player == user->nick)
            {
              locked = *(*nbtLockdata)["locked"];
              locked = (locked == 1) ? 0 : 1;
              *(*nbtLockdata)["locked"] = locked;

              if (locked == 1)
              {
                ServerInstance->chat()->sendMsg(user, MC_COLOR_RED + "Chest locked", Chat::USER);
              }
              else
              {
                ServerInstance->chat()->sendMsg(user, MC_COLOR_RED + "Chest opened", Chat::USER);
              }

            }
          }
          else
          {
            // If lockdata is missing (old chest)
            NBT_Value* nbtLock = new NBT_Value(NBT_Value::TAG_COMPOUND);
            nbtLock->Insert("player", new NBT_Value(user->nick));
            nbtLock->Insert("locked", new NBT_Value((int8_t)1));
            (*iter)->Insert("Lockdata", nbtLock);
          }
          break;
        }
      }
    }
  }
}

void BlockChest::onDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{

}

void BlockChest::onStoppedDigging(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{

}

bool BlockChest::onBroken(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  uint8_t block;
  uint8_t meta;

  if (!ServerInstance->map(map)->getBlock(x, y, z, &block, &meta))
  {
    return true;
  }

  bool destroy = false;

  int chunk_x = blockToChunk(x);
  int chunk_z = blockToChunk(z);

  sChunk* chunk = ServerInstance->map(map)->loadMap(chunk_x, chunk_z);

  if (chunk == NULL)
  {
    return true;
  }

  for (uint32_t i = 0; i < chunk->chests.size(); i++)
  {
    if (chunk->chests[i]->x() == x &&
        chunk->chests[i]->y() == y &&
        chunk->chests[i]->z() == z)
    {
      if(!chunk->chests[i]->large())
      {
        // clean up a small chest
        int32_t item_i = 26;
        for(int32_t item_i = 26; 0 <= item_i; item_i--)
        {
          if ((*chunk->chests[i]->items())[(size_t)item_i]->getType() != -1)
          {
            ServerInstance->map(map)->createPickupSpawn(chunk->chests[i]->x(),
                chunk->chests[i]->y(),
                chunk->chests[i]->z(),
                (*chunk->chests[i]->items())[(size_t)item_i]->getType(),
                (*chunk->chests[i]->items())[(size_t)item_i]->getCount(),
                (*chunk->chests[i]->items())[(size_t)item_i]->getHealth(),
                NULL);
          }
          chunk->chests[i]->items()->pop_back();
        }
      } else {
        // size a large chest down
        for(uint32_t item_i = 53; 27-3 <= item_i; item_i--)
        {
          if ((*chunk->chests[i]->items())[item_i]->getType() != -1)
          {
            ServerInstance->map(map)->createPickupSpawn(chunk->chests[i]->x(),
                chunk->chests[i]->y(),
                chunk->chests[i]->z(),
                (*chunk->chests[i]->items())[item_i]->getType(),
                (*chunk->chests[i]->items())[item_i]->getCount(),
                (*chunk->chests[i]->items())[item_i]->getHealth(),
                NULL);
          }
          chunk->chests[i]->items()->pop_back();
        }
      }
      chunk->chests.erase(chunk->chests.begin() + i);

      break;
    }
  }

  ServerInstance->map(map)->sendBlockChange(x, y, z, BLOCK_AIR, 0);
  ServerInstance->map(map)->setBlock(x, y, z, BLOCK_AIR, 0);
  this->spawnBlockItem(x, y, z, map, block);
  return false;

}

void BlockChest::onNeighbourBroken(User* user, int16_t oldblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
}

bool BlockChest::onPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
  uint8_t oldblock;
  uint8_t oldmeta;

  if (!ServerInstance->map(map)->getBlock(x, y, z, &oldblock, &oldmeta))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

  /* Check block below allows blocks placed on top */
  if (!this->isBlockStackable(oldblock))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

  /* move the x,y,z coords dependent upon placement direction */
  if (!this->translateDirection(&x, &y, &z, map, direction))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

  if (this->isUserOnBlock(x, y, z, map))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

  if (!this->isBlockEmpty(x, y, z, map))
  {
    revertBlock(user, x, y, z, map);
    return true;
  }

  // if there is a large chest around --> block
  {
    chestDataPtr _connectedChest;
    if(findConnectedChest(x, y, z, map, _connectedChest))
    {
      if(_connectedChest->large())
      {
        revertBlock(user, x, y, z, map);
        return true;
      }
    }
  }

  direction = user->relativeToBlock(x, y, z);

  //// Fix orientation
  //switch (direction)
  //{
  //case BLOCK_EAST:
  //  direction = BLOCK_SOUTH;
  //  break;
  //case BLOCK_BOTTOM:
  //  direction = BLOCK_EAST;
  //  break;
  //case BLOCK_NORTH:
  //  direction = BLOCK_NORTH;
  //  break;
  //case BLOCK_SOUTH:
  //  direction = BLOCK_BOTTOM;
  //  break;
  //}

  int32_t connectedChestX, connectedChestZ;
  if(findConnectedChest(x, y, z, map, &connectedChestX, &connectedChestZ))
  {
    // create a new chest and connect it to another chest --> large chest
    chestDataPtr connectedChest;
    if(getChestByCoordinates(connectedChestX, y, connectedChestZ, map, connectedChest))
    {
      chestDataPtr newchest;
      if(!getChestByCoordinates(x, y, z, map, newchest))
      {
        sChunk* chunk = ServerInstance->map(map)->getChunk(blockToChunk(x), blockToChunk(z));
        if(chunk != NULL)
        {
          newchest = chestDataPtr(new chestData);
          newchest->items(connectedChest->items());
          newchest->x(x);
          newchest->y(y);
          newchest->z(z);
          chunk->chests.push_back(newchest);
        }
      } else {
        newchest->large(true);
      }
      connectedChest->large(true);
    }
  } else {
    // create a new (small) chest
    chestDataPtr newchest(new chestData);
    sChunk* chunk = ServerInstance->map(map)->getChunk(blockToChunk(x), blockToChunk(z));
    if(chunk != NULL)
    {
      newchest->x(x);
      newchest->y(y);
      newchest->z(z);
      chunk->chests.push_back(newchest);
    }
  }

  ServerInstance->map(map)->setBlock(x, y, z, (char)newblock, direction);
  ServerInstance->map(map)->sendBlockChange(x, y, z, (char)newblock, direction);
  return false;
}

void BlockChest::onNeighbourPlace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{

}

void BlockChest::onReplace(User* user, int16_t newblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{

}

void BlockChest::onNeighbourMove(User* user, int16_t oldblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction)
{
}

bool BlockChest::onInteract(User* user, int32_t x, int8_t y, int32_t z, int map)
{
  //ToDo: check for large chest!
  sChunk* chunk = ServerInstance->map(map)->getChunk(blockToChunk(x), blockToChunk(z));
  if(chunk == NULL)
  {
    return false;
  }

  chestDataPtr _chestData;

  for(uint32_t i = 0; i < chunk->chests.size(); i++)
  {
    if((chunk->chests[i]->y() == y) && (chunk->chests[i]->x() == x) && (chunk->chests[i]->z() == z))
    {
      _chestData = chunk->chests[i];
      break;
    }
  }
  if(_chestData != NULL)
  {
    ServerInstance->inventory()->windowOpen(user, (_chestData->large() ? WINDOW_LARGE_CHEST : WINDOW_CHEST), x, y, z);
    return true;
  } else {
    return false;
  }
}

bool BlockChest::findConnectedChest(int32_t x, int8_t y, int32_t z, int map, int32_t* chest_x, int32_t* chest_z)
{
  uint8_t blocktype, blockmeta;

  *chest_x = x - 1;
  *chest_z = z;
  ServerInstance->map(map)->getBlock(*chest_x, y, *chest_z, &blocktype, &blockmeta);
  if(blocktype == BLOCK_CHEST)
    return true;

  *chest_x = x + 1;
  ServerInstance->map(map)->getBlock(*chest_x, y, *chest_z, &blocktype, &blockmeta);
  if(blocktype == BLOCK_CHEST)
    return true;

  *chest_x = x;
  *chest_z = z - 1;
  ServerInstance->map(map)->getBlock(*chest_x, y, *chest_z, &blocktype, &blockmeta);
  if(blocktype == BLOCK_CHEST)
    return true;

  *chest_x = x;
  *chest_z = z + 1;
  ServerInstance->map(map)->getBlock(*chest_x, y, *chest_z, &blocktype, &blockmeta);
  if(blocktype == BLOCK_CHEST)
    return true;

  return false;
}

bool BlockChest::findConnectedChest(int32_t x, int8_t y, int32_t z, int map, chestDataPtr& chest)
{
  int32_t connectedX, connectedZ;
  if(findConnectedChest(x, y, z, map, &connectedX, &connectedZ))
  {
    return getChestByCoordinates(connectedX, y, connectedZ, map, chest);
  } else {
    return false;
  }
}

bool BlockChest::getChestByCoordinates(int32_t x, int8_t y, int32_t z, int map, chestDataPtr& chest)
{
  sChunk* chunk = ServerInstance->map(map)->getChunk(blockToChunk(x), blockToChunk(z));
  for(size_t i = 0; i < chunk->chests.size(); i++)
  {
    if((chunk->chests[i]->x() == x)
      && (chunk->chests[i]->y() == y)
      && (chunk->chests[i]->z() == z))
    {
      chest = chunk->chests[i];
      return true;
    }
  }
  return false;
}
