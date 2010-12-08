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

#include "chest.h"
#include <iostream>
#include "../chat.h"
#include "../nbt.h"
#include "../map.h"
#include "../logger.h"
#include "../tools.h"

void BlockChest::onStartedDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{
  if(user->currentItem() == ITEM_WOODEN_AXE)
  {
    uint32 mapId;
    
    int block_x = blockToChunk(x);
    int block_z = blockToChunk(z);
    
    Map::get()->posToId(block_x, block_z, &mapId);
    
    NBT_Value *entityList = (*(*(Map::get()->maps[mapId].nbt))["Level"])["TileEntities"];

    if(!entityList)
    {
      entityList = new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND);
      Map::get()->maps[mapId].nbt->Insert("TileEntities", entityList);
    }

    if(entityList->GetType() == NBT_Value::TAG_LIST)
    {
      if(entityList->GetListType() != NBT_Value::TAG_COMPOUND)
      {
        entityList->SetType(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND);
      }

      std::vector<NBT_Value*> *entities = entityList->GetList();
      std::vector<NBT_Value*>::iterator iter = entities->begin(), end = entities->end();

      bool done = false;

      for( ; iter != end; iter++ )
      {
        if((**iter)["x"] == NULL || (**iter)["y"] == NULL || (**iter)["z"] == NULL ||
           (**iter)["x"]->GetType() != NBT_Value::TAG_INT ||
           (**iter)["y"]->GetType() != NBT_Value::TAG_INT ||
           (**iter)["z"]->GetType() != NBT_Value::TAG_INT)
        {
          continue;
        }

        if((sint32)(*(**iter)["x"]) == x && (sint32)(*(**iter)["y"]) == y && (sint32)(*(**iter)["z"]) == z)
        {
          sint8 locked;
          NBT_Value *nbtLockdata = (**iter)["Lockdata"];
          if(nbtLockdata != NULL)
          {
            std::string player = *(*nbtLockdata)["player"]->GetString();
            // Toggle lock if player is the owner of block
            if(player == user->nick)
            {
              locked = *(*nbtLockdata)["locked"];
              locked = (locked==1)?0:1;
              *(*nbtLockdata)["locked"] = locked;
            }
          } 
          else 
          {
            // If lockdata is missing (old chest)
            NBT_Value *nbtLock = new NBT_Value(NBT_Value::TAG_COMPOUND);
            nbtLock->Insert("player", new NBT_Value(user->nick));
            nbtLock->Insert("locked", new NBT_Value((sint8)1));
            (*iter)->Insert("Lockdata", nbtLock);
            
            locked = 1;
          }
          std::string msg;
          if(locked == 1)
          {
            msg = "locked";
          }
          else
          {
            msg = "opened";
          }
          Chat::get()->sendMsg(user, COLOR_RED + "Chest " + msg, Chat::USER);

          break;
        }
      }
    }
  }
}

void BlockChest::onDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

void BlockChest::onStoppedDigging(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

void BlockChest::onBroken(User* user, sint8 status, sint32 x, sint8 y, sint32 z, sint8 direction)
{
  
}

void BlockChest::onNeighbourBroken(User* user, sint8 oldblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

void BlockChest::onPlace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
  uint8 oldblock;
  uint8 oldmeta;

  if (!Map::get()->getBlock(x, y, z, &oldblock, &oldmeta))
    return;

  /* Check block below allows blocks placed on top */
  if (!this->isBlockStackable(oldblock))
    return;

  /* move the x,y,z coords dependent upon placement direction */
  if (!this->translateDirection(&x,&y,&z,direction))
    return;

  if (this->isUserOnBlock(x,y,z))
     return;

  if (!this->isBlockEmpty(x,y,z))
     return;

  direction = user->relativeToBlock(x, y, z);

  NBT_Value *val = new NBT_Value(NBT_Value::TAG_COMPOUND);
  val->Insert("id", new NBT_Value("Chest"));
  val->Insert("x", new NBT_Value((sint32)x));
  val->Insert("y", new NBT_Value((sint32)y));
  val->Insert("z", new NBT_Value((sint32)z));
  
  NBT_Value *nbtItems = new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND);
  val->Insert("Items", nbtItems);
  
  Map::get()->setBlock(x, y, z, (char)newblock, direction);
  Map::get()->sendBlockChange(x, y, z, (char)newblock, direction);
  
  Map::get()->setComplexEntity(user, x, y, z, val);
}

void BlockChest::onNeighbourPlace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{

}

void BlockChest::onReplace(User* user, sint8 newblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
  
}

void BlockChest::onNeighbourMove(User* user, sint8 oldblock, sint32 x, sint8 y, sint32 z, sint8 direction)
{
  
}


