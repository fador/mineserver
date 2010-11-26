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

#include <stdlib.h>
#ifdef WIN32
  #define _CRTDBG_MAP_ALLOC
  #include <crtdbg.h>
  #include <conio.h>
  #include <winsock2.h>
//  #define ZLIB_WINAPI
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <string.h>
#endif

#include <sys/types.h>
#include <fcntl.h>
#include <cstdio>
#include <deque>
#include <iostream>
#include <event.h>
#include <fstream>
#include <ctime>
#include <cmath>
#include <vector>
#include <zlib.h>
#include <stdint.h>

#include "constants.h"

#include "logger.h"

#include "sockets.h"
#include "tools.h"
#include "map.h"
#include "user.h"
#include "chat.h"
#include "config.h"
#include "nbt.h"
#include "packets.h"
#include "physics.h"

void PacketHandler::initPackets()
{

  //Len 0
  packets[PACKET_KEEP_ALIVE]               = Packets(0, &PacketHandler::keep_alive);
  //Variable len
  packets[PACKET_LOGIN_REQUEST]            = Packets(PACKET_VARIABLE_LEN, &PacketHandler::login_request);
  //Variable len
  packets[PACKET_HANDSHAKE]                = Packets(PACKET_VARIABLE_LEN, &PacketHandler::handshake);
  packets[PACKET_CHAT_MESSAGE]             = Packets(PACKET_VARIABLE_LEN,
                                                     &PacketHandler::chat_message);
  packets[PACKET_PLAYER_INVENTORY]         = Packets(PACKET_VARIABLE_LEN,
                                                     &PacketHandler::player_inventory);
  packets[PACKET_USE_ENTITY]               = Packets( 8, &PacketHandler::use_entity);
  packets[PACKET_PLAYER]                   = Packets( 1, &PacketHandler::player);
  packets[PACKET_PLAYER_POSITION]          = Packets(33, &PacketHandler::player_position);
  packets[PACKET_PLAYER_LOOK]              = Packets( 9, &PacketHandler::player_look);
  packets[PACKET_PLAYER_POSITION_AND_LOOK] = Packets(41, &PacketHandler::player_position_and_look);
  packets[PACKET_PLAYER_DIGGING]           = Packets(11, &PacketHandler::player_digging);
  packets[PACKET_PLAYER_BLOCK_PLACEMENT]   = Packets(12, &PacketHandler::player_block_placement);
  packets[PACKET_HOLDING_CHANGE]           = Packets( 6, &PacketHandler::holding_change);
  packets[PACKET_ARM_ANIMATION]            = Packets( 5, &PacketHandler::arm_animation);
  packets[PACKET_PICKUP_SPAWN]             = Packets(22, &PacketHandler::pickup_spawn);
  packets[PACKET_DISCONNECT]               = Packets(PACKET_VARIABLE_LEN,
                                                     &PacketHandler::disconnect);
  packets[PACKET_COMPLEX_ENTITIES]         = Packets(PACKET_VARIABLE_LEN,
                                                     &PacketHandler::complex_entities);

}

// Keep Alive (http://mc.kev009.com/wiki/Protocol#Keep_Alive_.280x00.29)
int PacketHandler::keep_alive(User *user)
{
  //No need to do anything
  user->buffer.removePacket();
  return PACKET_OK;
}

// Login request (http://mc.kev009.com/wiki/Protocol#Login_Request_.280x01.29)
int PacketHandler::login_request(User *user)
{
  //Check that we have enought data in the buffer
  if(!user->buffer.haveData(12))
    return PACKET_NEED_MORE_DATA;

  sint32 version;
  std::string player, passwd;
  sint64 mapseed;
  sint8 dimension;

  user->buffer >> version >> player >> passwd >> mapseed >> dimension;

  if(!user->buffer)
    return PACKET_NEED_MORE_DATA;

  user->buffer.removePacket();

  std::cout << "Player " << user->UID << " login v." << version <<" : " << player <<":"<< passwd << std::endl;

  // If version is not 2 or 3
  if(version != 5)
  {
    user->kick(Conf::get().sValue("wrong_protocol_message"));
    return PACKET_OK;
  }

  // If userlimit is reached
  if((int)Users.size() >= Conf::get().iValue("userlimit"))
  {
    user->kick(Conf::get().sValue("server_full_message"));
    return PACKET_OK;
  }

  user->changeNick(player);

  //Load user data
  user->loadData();

  //Login OK package
  user->buffer << (sint8)PACKET_LOGIN_RESPONSE 
    << (sint32)user->UID << std::string("") << std::string("") << (sint64)0 << (sint8)0;

  //Send server time (after dawn)
  user->buffer << (sint8)PACKET_TIME_UPDATE << (sint64)Map::get().mapTime;

  //Inventory
  for(sint32 invType=-1; invType != -4; invType--)
  {
    Item *inventory = NULL;
  sint16 inventoryCount = 0;

  if(invType == -1)
  {
    inventory = user->inv.main;
    inventoryCount = 36;
  }
  else if(invType == -2)
  {
    inventory = user->inv.equipped;
    inventoryCount = 4;
  }
  else if(invType == -3)
  {
    inventory = user->inv.crafting;
    inventoryCount = 4;
  }
  user->buffer << (sint8)PACKET_PLAYER_INVENTORY << invType << inventoryCount;

  for(int i=0; i<inventoryCount; i++)
  {
    if(inventory[i].count)
    {
      user->buffer << (sint16)inventory[i].type << (sint8)inventory[i].count << (sint16)inventory[i].health;
    }
    else
    {
      user->buffer << (sint16)-1;
    }
  }
  }

  // Send motd
  std::ifstream motdfs( MOTDFILE.c_str());

  std::string temp;

  while(getline( motdfs, temp ))
  {
    // If not commentline
    if(temp[0] != COMMENTPREFIX)
  {
      user->buffer << (sint8)PACKET_CHAT_MESSAGE << temp;
  }
  }
  motdfs.close();

  //Teleport player
  user->teleport(user->pos.x, user->pos.y+2, user->pos.z);

  //Put nearby chunks to queue
  for(int x = -user->viewDistance; x <= user->viewDistance; x++)
  {
    for(int z = -user->viewDistance; z <= user->viewDistance; z++)
    {
      user->addQueue((sint32)user->pos.x/16+x, (sint32)user->pos.z/16+z);
    }
  }
  // Push chunks to user
  user->pushMap();

  //Spawn this user to others
  user->spawnUser((sint32)user->pos.x*32, ((sint32)user->pos.y+2)*32, (sint32)user->pos.z*32);
  //Spawn other users for connected user
  user->spawnOthers();

  user->logged = true;

  Chat::get().sendMsg(user, player+" connected!", Chat::ALL);

  return PACKET_OK;
}

int PacketHandler::handshake(User *user)
{
  if(!user->buffer.haveData(3))
    return PACKET_NEED_MORE_DATA;

  std::string player;

  user->buffer >> player;

  //Check for data
  if(!user->buffer)
    return PACKET_NEED_MORE_DATA;

  user->buffer.removePacket();

  //Remove package from buffer
  std::cout << "Handshake player: " << player << std::endl;

  //Send handshake package
  user->buffer << (sint8)PACKET_HANDSHAKE << std::string("-");

  return PACKET_OK;
}

int PacketHandler::chat_message(User *user)
{
  // Wait for length-short. HEHE
  if(!user->buffer.haveData(2))
    return PACKET_NEED_MORE_DATA;

  std::string msg;

  user->buffer >> msg;

  if(!user->buffer)
    return PACKET_NEED_MORE_DATA;

  user->buffer.removePacket();

  Chat::get().handleMsg( user, msg );

  return PACKET_OK;
}

int PacketHandler::player_inventory(User *user)
{
  if(!user->buffer.haveData(14))
    return PACKET_NEED_MORE_DATA;

  int i=0;
  sint32 type;
  sint16 count;

  user->buffer >> type >> count;

  if(!user->buffer)
    return PACKET_NEED_MORE_DATA;

  int items   = 0;
  Item *slots = NULL;

  items = count;

  switch(type)
  {
  //Main inventory
  case -1:
    //items = 36;
    memset(user->inv.main, 0, sizeof(Item)*36);
    slots = (Item *)&user->inv.main;
    break;

  //Equipped armour
  case -3:
    //items = 4;
    memset(user->inv.equipped, 0, sizeof(Item)*4);
    slots = (Item *)&user->inv.equipped;
    break;

  //Crafting slots
  case -2:
    //items = 4;
    memset(user->inv.crafting, 0, sizeof(Item)*4);
    slots = (Item *)&user->inv.crafting;
    break;
  }

  for(i = 0; i < items; i++)
  {
    sint16 item_id = 0;
    sint8 numberOfItems = 0;
    sint16 health = 0;

    user->buffer >> item_id;

    if(!user->buffer)
      return PACKET_NEED_MORE_DATA;

    if(item_id != -1)
    {
      user->buffer >> numberOfItems >> health;

      if(!user->buffer)
        return PACKET_NEED_MORE_DATA;

      
      slots[i].type   = item_id;
      slots[i].count  = numberOfItems;
      slots[i].health = health;
    }
  }

  user->buffer.removePacket();

  return PACKET_OK;
}

int PacketHandler::player(User *user)
{
  //OnGround packet
  sint8 onground;
  user->buffer >> onground;
  if(!user->buffer)
    return PACKET_NEED_MORE_DATA;
  user->buffer.removePacket();
  return PACKET_OK;
}

int PacketHandler::player_position(User *user)
{
  double x, y, stance, z;
  sint8 onground;

  user->buffer >> x >> y >> stance >> z >> onground;

  if(!user->buffer)
    return PACKET_NEED_MORE_DATA;

  user->updatePos(x, y, z, stance);
  user->buffer.removePacket();

  return PACKET_OK;
}

int PacketHandler::player_look(User *user)
{
  float yaw, pitch;
  sint8 onground;

  user->buffer >> yaw >> pitch >> onground;
  
  if(!user->buffer)
    return PACKET_NEED_MORE_DATA;

  user->updateLook(yaw, pitch);

  user->buffer.removePacket();

  return PACKET_OK;
}

int PacketHandler::player_position_and_look(User *user)
{
  double x, y, stance, z;
  float yaw, pitch;
  sint8 onground;

  user->buffer >> x >> y >> stance >> z 
        >> yaw >> pitch >> onground;

  if(!user->buffer)
    return PACKET_NEED_MORE_DATA;

  //Update user data
  user->updatePos(x, y, z, stance);
  user->updateLook(yaw, pitch);

  user->buffer.removePacket();

  return PACKET_OK;
}

int PacketHandler::player_digging(User *user)
{
  sint8 status,y;
  sint32 x,z;
  sint8 direction;

  user->buffer >> status >> x >> y >> z >> direction;

  if(!user->buffer)
    return PACKET_NEED_MORE_DATA;

  user->buffer.removePacket();

  //When player starts diggins
  if(status == BLOCK_STATUS_STARTED_DIGGING)
  {
    uint8 block,metadata;
    Map::get().getBlock(x, y, z, &block, &metadata);
    // Door status change  
    if (block == BLOCK_WOODEN_DOOR || 
        block == BLOCK_IRON_DOOR)
    {
      
      // Toggle door state
      if (metadata & 0x4)
      {
        metadata &= (0x8 | 0x3);
      }
      else
      {
        metadata |= 0x4;
      }

      uint8 metadata2, block2;

      int modifier = (metadata & 0x8) ? -1 : 1;

      Map::get().setBlock(x, y, z, block, metadata);
      Map::get().sendBlockChange(x, y, z, (char)block, metadata);  

      Map::get().getBlock(x, y + modifier, z, &block2, &metadata2);

      if (block2 == block)
      {
        metadata2 = metadata;
      
        if(metadata & 0x8)
          metadata2 &= 0x7;
        else
          metadata2 |= 0x8;

        Map::get().setBlock(x, y + modifier, z, block2, metadata2);
        Map::get().sendBlockChange(x, y + modifier, z, (char)block, metadata2);     
      
      }
      return PACKET_OK;
    }

  }


  //If block broken
  else if(status == BLOCK_STATUS_BLOCK_BROKEN)
  {
    uint8 block; uint8 meta;
    if(Map::get().getBlock(x, y, z, &block, &meta))
    {
      Map::get().sendBlockChange(x, y, z, 0, 0);
      Map::get().setBlock(x, y, z, 0, 0);

      uint8 topblock; uint8 topmeta;

      // Destroy items on sides
      if(Map::get().getBlock(x+1, y, z, &topblock, &topmeta) && (topblock == BLOCK_TORCH && topmeta == BLOCK_NORTH))
      {
         Map::get().sendBlockChange(x+1, y, z, 0, 0);
         Map::get().setBlock(x+1, y, z, 0, 0);
         Map::get().createPickupSpawn(x+1, y, z, topblock, 1);
      }

      if(Map::get().getBlock(x-1, y, z, &topblock, &topmeta) && (topblock == BLOCK_TORCH && topmeta == BLOCK_SOUTH))
      {
         Map::get().sendBlockChange(x-1, y, z, 0, 0);
         Map::get().setBlock(x-1, y, z, 0, 0);
         Map::get().createPickupSpawn(x-1, y, z, topblock, 1);
      }

      if(Map::get().getBlock(x, y, z+1, &topblock, &topmeta) && (topblock == BLOCK_TORCH && topmeta == BLOCK_EAST))
      {
         Map::get().sendBlockChange(x, y, z+1, 0, 0);
         Map::get().setBlock(x, y, z+1, 0, 0);
         Map::get().createPickupSpawn(x, y, z+1, topblock, 1);
      }

      if(Map::get().getBlock(x, y, z-1, &topblock, &topmeta) && (topblock == BLOCK_TORCH && topmeta == BLOCK_WEST))
      {
         Map::get().sendBlockChange(x, y, z-1, 0, 0);
         Map::get().setBlock(x, y, z-1, 0, 0);
         Map::get().createPickupSpawn(x, y, z-1, topblock, 1);
      }

      //Destroy items on top
      if(Map::get().getBlock(x, y+1, z, &topblock, &topmeta) && (topblock == BLOCK_SNOW ||
                                                                 topblock ==
                                                                 BLOCK_BROWN_MUSHROOM ||
                                                                 topblock == BLOCK_RED_MUSHROOM ||
                                                                 topblock == BLOCK_YELLOW_FLOWER ||
                                                                 topblock == BLOCK_RED_ROSE ||
                                                                 topblock == BLOCK_SAPLING ||
                                                                 (topblock == BLOCK_TORCH && topmeta == BLOCK_TOP)))
      {
        Map::get().sendBlockChange(x, y+1, z, 0, 0);
        Map::get().setBlock(x, y+1, z, 0, 0);
        //Others than snow will spawn
        if(topblock != BLOCK_SNOW)
        {
          Map::get().createPickupSpawn(x, y+1, z, topblock, 1);
        }
      }

      if(block != BLOCK_SNOW && (int)block > 0 && (int)block < 255)
      {
        spawnedItem item;
        bool spawnItem = false;
        
        // Spawn drop according to BLOCKDROPS
        // Check probability
        if(BLOCKDROPS.count(block) && BLOCKDROPS[block].probability >= rand() % 10000)
        {
          item.item  = BLOCKDROPS[block].item_id;
          item.count = BLOCKDROPS[block].count;
          spawnItem = true;
        }
        else if(!BLOCKDROPS.count(block) || !BLOCKDROPS[block].exclusive)
        {
          item.item  = (int)block;
          item.count = 1;
          spawnItem = true;
        }
        
        if (spawnItem)
        {

          item.EID    = generateEID();
          item.health = 0;
          
          item.pos.x()  = x * 32;
          item.pos.y()  = y * 32;
          item.pos.z()  = z * 32;
          
          //Randomize spawn position a bit
          item.pos.x() += 5 + (rand() % 22);
          item.pos.z() += 5 + (rand() % 22);

          // If count is greater than 0
          if(item.count > 0)
            Map::get().sendPickupSpawn(item);
        }
      }

      // Check liquid physics
      Physics::get().checkSurrounding(vec(x, y, z));


      // Block physics for BLOCK_GRAVEL and BLOCK_SAND and BLOCK_SNOW
      while(Map::get().getBlock(x, y+1, z, &topblock, &topmeta) && (topblock == BLOCK_GRAVEL ||
                                                                    topblock == BLOCK_SAND ||
                                                                    topblock == BLOCK_SNOW ||
                                                                    topblock ==
                                                                    BLOCK_BROWN_MUSHROOM ||
                                                                    topblock ==
                                                                    BLOCK_RED_MUSHROOM ||
                                                                    topblock ==
                                                                    BLOCK_YELLOW_FLOWER ||
                                                                    topblock == BLOCK_RED_ROSE ||
                                                                    topblock == BLOCK_SAPLING))
      {
        // Destroy original block
        Map::get().sendBlockChange(x, y+1, z, 0, 0);
        Map::get().setBlock(x, y+1, z, 0, 0);

        Map::get().setBlock(x, y, z, topblock, topmeta);
        Map::get().sendBlockChange(x, y, z, topblock, topmeta);

        y++;
      }
    }
  }
  return PACKET_OK;
}

int PacketHandler::player_block_placement(User *user)
{
  sint8 y, oy, direction;
  sint16 blockID;
  sint32 x, z, ox, oz;

  user->buffer >> blockID >> x >> y >> z >> direction;

  if(!user->buffer)
    return PACKET_NEED_MORE_DATA;

  user->buffer.removePacket();
  
  ox = x;
  oy = y;
  oz = z;
  
  // TODO: Handle processing of 
  if(direction == -1)
    return PACKET_OK;
    
  if(y < 0)
    return PACKET_OK;
    
  uint8 block;
  uint8 metadata;
  Map::get().getBlock(x, y, z, &block, &metadata);
  
  switch(direction)
  {
    case 0: y--; break;
    case 1: y++; break;
    case 2: z--; break;
    case 3: z++; break;
    case 4: x--; break;
    case 5: x++; break;
  }
  
  uint8 block_direction;
  uint8 metadata_direction;
  Map::get().getBlock(x, y, z, &block_direction, &metadata_direction);
  
  uint8 block_bottom;
  uint8 metadata_bottom;
  Map::get().getBlock(x, y - 1, z, &block_bottom, &metadata_bottom);
  
  Physics::get().checkSurrounding(vec(x, y, z));
  
  
  // If the "placing-on" block is a block that you cannot place blocks on
  
  if (block == BLOCK_WORKBENCH       ||
      block == BLOCK_FURNACE         ||
      block == BLOCK_BURNING_FURNACE ||
      block == BLOCK_CHEST           ||
      block == BLOCK_JUKEBOX         ||
      block == BLOCK_TORCH)
    return PACKET_OK;    
  
  // Door status change  
  if (block == BLOCK_WOODEN_DOOR || 
      block == BLOCK_IRON_DOOR)
  {
    blockID = block;
    // Toggle door state
    if (metadata & 0x4)
    {
      metadata &= (0x8 | 0x3);
    }
    else
    {
      metadata |= 0x4;
    }

    uint8 metadata2, block2;

    int modifier = (metadata & 0x8) ? -1 : 1;

    x = ox;
    y = oy;
    z = oz;

    Map::get().setBlock(x, y, z, block, metadata);
    Map::get().sendBlockChange(x, y, z, (char)blockID, metadata);  

    Map::get().getBlock(x, y + modifier, z, &block2, &metadata2);

    if (block2 == block)
    {
      metadata2 = metadata;
      
      if(metadata & 0x8)
        metadata2 &= 0x7;
      else
        metadata2 |= 0x8;

      Map::get().setBlock(x, y + modifier, z, block2, metadata2);
      Map::get().sendBlockChange(x, y + modifier, z, (char)blockID, metadata2);     
      
    }
    return PACKET_OK;
  }


  // If the block is invalid  
  if (blockID > 0xFF || blockID == -1)
    return PACKET_OK;
    
    
  // Can't place fire on/in water
  
  if (blockID == BLOCK_FIRE                     &&
        (block_bottom == BLOCK_WATER            ||
         block_bottom == BLOCK_STATIONARY_WATER ||
         block == BLOCK_WATER                   ||
         block == BLOCK_STATIONARY_WATER))
    return PACKET_OK;
    
    
  // Check if the directed block is replace-able
  
  //Blocks that drop items once replaced
  if (block_direction != BLOCK_TORCH && 
      block_direction != BLOCK_REDSTONE_TORCH_OFF &&
      block_direction != BLOCK_REDSTONE_TORCH_ON &&
      block_direction != BLOCK_BROWN_MUSHROOM &&
      block_direction != BLOCK_RED_MUSHROOM &&
      block_direction != BLOCK_YELLOW_FLOWER &&
      block_direction != BLOCK_RED_ROSE &&
      block_direction != BLOCK_SAPLING)
  {
    //Blocks that are simply replaced
    if (block_direction != BLOCK_AIR &&
        block_direction != BLOCK_WATER &&
        block_direction != BLOCK_STATIONARY_WATER &&
        block_direction != BLOCK_LAVA &&
        block_direction != BLOCK_STATIONARY_LAVA &&
        block_direction != BLOCK_SNOW &&
        block_direction != BLOCK_FIRE)
      {
        // It's not an overwritable block
        return PACKET_OK;
      }
  }
  else
  {
    // Drop the item representing the block
    //TODO: Drop the item
  }


  // Overwrite over these blocks (Target block)
 
  if (block == BLOCK_SNOW || 
      block == BLOCK_FIRE ||
      block == BLOCK_TORCH || 
      block == BLOCK_REDSTONE_TORCH_OFF ||
      block == BLOCK_REDSTONE_TORCH_ON ||
      block == BLOCK_BROWN_MUSHROOM ||
      block == BLOCK_RED_MUSHROOM ||
      block == BLOCK_YELLOW_FLOWER ||
      block == BLOCK_RED_ROSE ||
      block == BLOCK_SAPLING)
  {
    x = ox;
    y = oy;
    z = oz;
  }
  
  
  
  // Jack-O-Lantern
  
  if (blockID == BLOCK_JACK_O_LANTERN)
  {
    // Where the visage face
    // -Z -> East   0x0
    // +X -> South  0x1
    // +Z -> West   0x2
    // -X -> North  0x3
    // Anything else, no visage

    // We place according to the player's position

    double diffX = x - user->pos.x;
    double diffZ = z - user->pos.z;
    
    if (std::abs(diffX) > std::abs(diffZ))
    {
      // We compare on the x axis
      
      if (diffX > 0)
        metadata = 0x3;
      else
        metadata = 0x1;
    }
    else
    {
      // We compare on the z axis
      
      if (diffZ > 0)
        metadata = 0x0;
      else
        metadata = 0x2;
    }
  }
  
  
  // If the block is stairs, place them in the right direction
  
  if (blockID == BLOCK_WOODEN_STAIRS ||
      blockID == BLOCK_COBBLESTONE_STAIRS)
  {
    // Where the stairs ascend
    // +X -> South  0x0
    // -X -> North  0x1
    // +Z -> West   0x2
    // -Z -> East   0x3
    
    //TODO: Check the surrounding for other stairs and allign it with them.

    // We cannot place stairs over a weak block
    if (block_bottom == BLOCK_TORCH ||
        block_bottom == BLOCK_REDSTONE_TORCH_OFF ||
        block_bottom == BLOCK_REDSTONE_TORCH_ON ||
        block_bottom == BLOCK_BROWN_MUSHROOM ||
        block_bottom == BLOCK_RED_MUSHROOM ||
        block_bottom == BLOCK_YELLOW_FLOWER ||
        block_bottom == BLOCK_RED_ROSE ||
        block_bottom == BLOCK_SAPLING ||
        block_bottom == BLOCK_AIR ||
        block_bottom == BLOCK_WATER ||
        block_bottom == BLOCK_STATIONARY_WATER ||
        block_bottom == BLOCK_LAVA ||
        block_bottom == BLOCK_STATIONARY_LAVA ||
        block_bottom == BLOCK_SNOW ||
        block_bottom == BLOCK_FIRE)
      return PACKET_OK;

    if (y == oy && (x != ox || y != oy || z != oz))
    {
      // We place according to the target block

      if (x < ox)
        metadata = 0x0;
      else if (x > ox)
        metadata = 0x1;
      else if (z < oz)
        metadata = 0x2;
      else
        metadata = 0x3;
    }
    else
    {
      // We place according to the player's position

      double diffX = x - user->pos.x;
      double diffZ = z - user->pos.z;
      
      if (std::abs(diffX) > std::abs(diffZ))
      {
        // We compare on the x axis
        
        if (diffX > 0)
          metadata = 0x0;
        else
          metadata = 0x1;
      }
      else
      {
        // We compare on the z axis
        
        if (diffZ > 0)
          metadata = 0x2;
        else
          metadata = 0x3;
      }
    }
  }

  // We can place saplings only on dirt or grass
  
  if (blockID == BLOCK_SAPLING &&
        (block != BLOCK_GRASS ||
        block != BLOCK_DIRT))
    return PACKET_OK;
  
  
  // Check block placement
  
  if (blockID != BLOCK_TORCH && 
      blockID != BLOCK_REDSTONE_TORCH_OFF &&
      blockID != BLOCK_REDSTONE_TORCH_ON &&
      blockID != BLOCK_AIR &&
      blockID != BLOCK_WATER &&
      blockID != BLOCK_STATIONARY_WATER &&
      blockID != BLOCK_LAVA &&
      blockID != BLOCK_STATIONARY_LAVA &&
      blockID != BLOCK_BROWN_MUSHROOM &&
      blockID != BLOCK_RED_MUSHROOM &&
      blockID != BLOCK_YELLOW_FLOWER &&
      blockID != BLOCK_RED_ROSE &&
      blockID != BLOCK_SAPLING &&
      blockID != BLOCK_FIRE &&
      blockID != BLOCK_REDSTONE_WIRE &&
      blockID != BLOCK_SIGN_POST &&
      blockID != BLOCK_LADDER &&
      blockID != BLOCK_MINECART_TRACKS &&
      blockID != BLOCK_WALL_SIGN &&
      blockID != BLOCK_STONE_PRESSURE_PLATE &&
      blockID != BLOCK_WOODEN_PRESSURE_PLATE &&
      blockID != BLOCK_STONE_BUTTON &&
      blockID != BLOCK_PORTAL)
  {
    double diffX = x - user->pos.x;
    double diffY = y - user->pos.y;
    double diffZ = z - user->pos.z;

    //std::cout << user->pos.x << ", " << user->pos.y << ", " << user->pos.z << "  =>  " << diffX << ", " << diffY << ", " << diffZ << std::endl;
    
    //TODO: Check for doors and fences

    // We check Y, X then Z
    if (diffY > -0.9 && diffY < 1.3 && 
        diffX > -1.3 && diffX < 0.3 && 
        diffZ > -1.3 && diffZ < 0.3)
      return PACKET_OK;
  }
  
  
  // Set the direction for the block
  
  if(blockID == BLOCK_TORCH ||
     blockID == BLOCK_REDSTONE_TORCH_OFF ||
     blockID == BLOCK_REDSTONE_TORCH_ON)
  {
    metadata = 0;
    if (direction)
       metadata = 6 - direction;
  }
  
  
  // Proceed to change the block
  
  Map::get().setBlock(x, y, z, (char)blockID, metadata);
  Map::get().sendBlockChange(x, y, z, (char)blockID, metadata);

  if (blockID == BLOCK_WATER || 
      blockID == BLOCK_STATIONARY_WATER ||
      blockID == BLOCK_LAVA || 
      blockID == BLOCK_STATIONARY_LAVA)
    Physics::get().addSimulation(vec(x, y, z));

  return PACKET_OK;
}

int PacketHandler::holding_change(User *user)
{
  sint32 entityID;
  sint16 itemID;
  user->buffer >> entityID >> itemID;

  if(!user->buffer)
    return PACKET_NEED_MORE_DATA;

  user->buffer.removePacket();

  //Send holding change to others
  Packet pkt;
  pkt << (sint8)PACKET_HOLDING_CHANGE << (sint32)user->UID << itemID;
  user->sendOthers((uint8*)pkt.getWrite(), pkt.getWriteLen());

  return PACKET_OK;
}

int PacketHandler::arm_animation(User *user)
{
  sint32 userID;
  sint8 animType;
  
  user->buffer >> userID >> animType;

  if(!user->buffer)
    return PACKET_NEED_MORE_DATA;

  user->buffer.removePacket();

  Packet pkt;
  pkt << (sint8)PACKET_ARM_ANIMATION << (sint32)user->UID << animType;
  user->sendOthers((uint8*)pkt.getWrite(), pkt.getWriteLen());

  return PACKET_OK;
}

int PacketHandler::pickup_spawn(User *user)
{
  uint32 curpos = 4;
  spawnedItem item;
  
  item.health = 0;

  sint8 yaw, pitch, roll;

  user->buffer >> (sint32&)item.EID;
  
  user->buffer >> (sint16&)item.item >> (sint8&)item.count ;
  user->buffer >> (sint32&)item.pos.x() >> (sint32&)item.pos.y() >> (sint32&)item.pos.z();
  user->buffer >> yaw >> pitch >> roll;

  if(!user->buffer)
    return PACKET_NEED_MORE_DATA;

  //Client sends multiple packets with same EID, check for recent spawns
  for(int i=0;i<10;i++)
  {
    if(user->recentSpawn[i] == item.EID)
    {
      return PACKET_OK;
    }
  }

  //Put this EID in the next slot
  user->recentSpawn[user->recentSpawnPos++] = item.EID;
  if(user->recentSpawnPos==10) user->recentSpawnPos=0;

  item.EID    = generateEID();

  item.spawnedBy = user->UID;
  Map::get().sendPickupSpawn(item);

  return PACKET_OK;
}

int PacketHandler::disconnect(User *user)
{
  if(!user->buffer.haveData(2))
    return PACKET_NEED_MORE_DATA;

  std::string msg;
  user->buffer >> msg;

  if(!user->buffer)
    return PACKET_NEED_MORE_DATA;

  user->buffer.removePacket();

  std::cout << "Disconnect: " << msg << std::endl;

  event_del(user->GetEvent());
  
  #ifdef WIN32
  closesocket(user->fd);
  #else
  close(user->fd);
  #endif
  
  remUser(user->fd);

  
  return PACKET_OK;
}

int PacketHandler::complex_entities(User *user)
{
  if(!user->buffer.haveData(12))
    return PACKET_NEED_MORE_DATA;

  sint32 x,z;
  sint16 len,y;


  user->buffer >> x >> y >> z >> len;

  if(!user->buffer)
    return PACKET_NEED_MORE_DATA;

  if(!user->buffer.haveData(len))
    return PACKET_NEED_MORE_DATA;

  uint8 *buffer = new uint8[len];

  user->buffer.getData(buffer, len);

  user->buffer.removePacket();

  uint8 block, meta;
  Map::get().getBlock(x, y, z, &block, &meta);

  //We only handle chest for now
  if(block != BLOCK_CHEST || block != BLOCK_FURNACE || block != BLOCK_SIGN_POST || block != BLOCK_WALL_SIGN)
  {
    delete[] buffer;
    return PACKET_OK;
  }


  //Initialize zstream to handle gzip format
  z_stream zstream;
  zstream.zalloc    = (alloc_func)0;
  zstream.zfree     = (free_func)0;
  zstream.opaque    = (voidpf)0;
  zstream.next_in   = buffer;
  zstream.next_out  = 0;
  zstream.avail_in  = len;
  zstream.avail_out = 0;
  zstream.total_in  = 0;
  zstream.total_out = 0;
  zstream.data_type = Z_BINARY;
  inflateInit2(&zstream, 16+MAX_WBITS);

  uLongf uncompressedSize   = ALLOCATE_NBTFILE;
  uint8 *uncompressedBuffer = new uint8[uncompressedSize];

  zstream.avail_out = uncompressedSize;
  zstream.next_out = uncompressedBuffer;
  
  //Uncompress
  if(inflate(&zstream, Z_FULL_FLUSH)!=Z_STREAM_END)
  {
    std::cout << "Error in inflate!" << std::endl;
    delete[] buffer;
    return PACKET_OK;
  }

  inflateEnd(&zstream);

  //Get size
  uncompressedSize  = zstream.total_out;
  
  uint8 *ptr = uncompressedBuffer + 3; // skip blank compound
  int remaining = uncompressedSize;

  NBT_Value *entity = new NBT_Value(NBT_Value::TAG_COMPOUND, &ptr, remaining);

#ifdef _DEBUG
  std::cout << "Complex entity at (" << x << "," << y << "," << z << ")" << std::endl;
  entity->Print();
#endif

  Map::get().setComplexEntity(x, y, z, entity);

  delete [] buffer;

  return PACKET_OK;
}


int PacketHandler::use_entity(User *user)
{
  sint32 userID,target;
  user->buffer >> userID >> target;
  if(!user->buffer)
    return PACKET_NEED_MORE_DATA;

  return PACKET_OK;
}
