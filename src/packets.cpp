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
  packets[PACKET_LOGIN_REQUEST]            = Packets(-1, &PacketHandler::login_request);
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

  int i;

  sint32 version;
  std::string player, passwd;
  sint64 mapseed;
  sint8 dimension;


  user->buffer >> version >> player >> passwd >> mapseed >> dimension;

  if(!user->buffer)
	  return PACKET_NEED_MORE_DATA;

  user->buffer.removePacket();

  std::cout << "Player " << user->UID << " login v." << version <<" : " << player <<":"<<
  passwd << std::endl;

  // If version is not 2 or 3
  if(version != 4 && version != 3 && version != 2)
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
  char data[9] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  putSint32((uint8 *)&data[1], user->UID);
  bufferevent_write(user->buf_ev, (char *)&data[0], 9);

  //Send server time (after dawn)
  uint8 data3[9] = {0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00};
  bufferevent_write(user->buf_ev, (char *)&data3[0], 9);

  //Inventory
  uint8 data4[7+36*5];
  data4[0] = 0x05;
  putSint32(&data4[1], -1);
  data4[5] = 0;
  data4[6] = 36;
  int curpos2 = 7;
  //Send main inventory
  for(i = 0; i < 36; i++)
  {
    if(user->inv.main[i].count)
    {
      putSint16(&data4[curpos2], user->inv.main[i].type);     //Type
      curpos2       += 2;
      data4[curpos2] = user->inv.main[i].count;               //Count
      curpos2++;
      putSint16(&data4[curpos2], user->inv.main[i].health); //Health
      curpos2 += 2;
    }
    else
    {
      //Empty slot
      putSint16(&data4[curpos2], -1);
      curpos2 += 2;
    }
  }
  bufferevent_write(user->buf_ev, (char *)&data4[0], curpos2);


  //Send equipped inventory
  putSint32(&data4[1], -3);
  data4[6] = 4;
  curpos2  = 7;
  for(i = 0; i < 4; i++)
  {
    if(user->inv.equipped[i].count)
    {
      putSint16(&data4[curpos2], user->inv.equipped[i].type);     //Type
      curpos2       += 2;
      data4[curpos2] = user->inv.equipped[i].count;               //Count
      curpos2++;
      putSint16(&data4[curpos2], user->inv.equipped[i].health); //Health
      curpos2 += 2;
    }
    else
    {
      //Empty slot
      putSint16(&data4[curpos2], -1);
      curpos2 += 2;
    }
  }
  bufferevent_write(user->buf_ev, (char *)&data4[0], curpos2);

  //Send crafting inventory
  putSint32(&data4[1], -2);
  data4[6] = 4;
  curpos2  = 7;
  for(i = 0; i < 4; i++)
  {
    if(user->inv.crafting[i].count)
    {
      putSint16(&data4[curpos2], user->inv.crafting[i].type);     //Type
      curpos2       += 2;
      data4[curpos2] = user->inv.crafting[i].count;               //Count
      curpos2++;
      putSint16(&data4[curpos2], user->inv.crafting[i].health); //Health
      curpos2 += 2;
    }
    else
    {
      //Empty slot
      putSint16(&data4[curpos2], -1);
      curpos2 += 2;
    }
  }
  bufferevent_write(user->buf_ev, (char *)&data4[0], curpos2);

  // Send motd
  std::ifstream motdfs( MOTDFILE.c_str());

  std::string temp;

  while(getline( motdfs, temp ))
  {
    // If not commentline
    if(temp[0] != COMMENTPREFIX)
      Chat::get().sendMsg(user, temp, Chat::USER);
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


  //Send "On Ground" signal
  char data6[2] = {0x0A, 0x01};
  bufferevent_write(user->buf_ev, (char *)&data6[0], 2);

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
  char data2[4] = {0x02, 0x00, 0x01, '-'};
  bufferevent_write(user->buf_ev, (char *)&data2[0], 4);

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
	  sint16 item_id;
	  sint8 numberOfItems;
	  sint16 health;

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

  //If block broken
  if(status == BLOCK_STATUS_BLOCK_BROKEN)
  {
    uint8 block; uint8 meta;
    if(Map::get().getBlock(x, y, z, &block, &meta))
    {
      Map::get().sendBlockChange(x, y, z, 0, 0);
      Map::get().setBlock(x, y, z, 0, 0);

      uint8 topblock; uint8 topmeta;
      //Destroy items on top
      if(Map::get().getBlock(x, y+1, z, &topblock, &topmeta) && (topblock == BLOCK_SNOW ||
                                                                 topblock ==
                                                                 BLOCK_BROWN_MUSHROOM ||
                                                                 topblock == BLOCK_RED_MUSHROOM ||
                                                                 topblock == BLOCK_YELLOW_FLOWER ||
                                                                 topblock == BLOCK_RED_ROSE ||
                                                                 topblock == BLOCK_SAPLING))
      {
        Map::get().sendBlockChange(x, y+1, z, 0, 0);
        Map::get().setBlock(x, y+1, z, 0, 0);
        //Others than snow will spawn
        if(topblock != BLOCK_SNOW)
        {
          spawnedItem item;
          item.EID      = generateEID();
          item.health   = 0;
          item.item     = (int)topblock;
          item.count    = 1;

          item.pos.x()  = x*32;
          item.pos.y()  = (y+1)*32;
          item.pos.z()  = z*32;
          //Randomize spawn position a bit
          item.pos.x() += 5+(rand()%22);
          item.pos.z() += 5+(rand()%22);

          Map::get().sendPickupSpawn(item);
        }
      }

      if(block != BLOCK_SNOW && (int)block > 0 && (int)block < 255)
      {
        spawnedItem item;
        item.EID    = generateEID();
        item.health = 0;

        // Spawn drop according to BLOCKDROPS
        // Check propability
        if(BLOCKDROPS.count(block) && BLOCKDROPS[block].probability >= rand()%10000)
        {
          item.item  = BLOCKDROPS[block].item_id;
          item.count = BLOCKDROPS[block].count;
        }
        else
        {
          item.item  = (int)block;
          item.count = 1;
        }
        item.pos.x()  = x*32;
        item.pos.y()  = y*32;
        item.pos.z()  = z*32;
        //Randomize spawn position a bit
        item.pos.x() += 5+(rand()%22);
        item.pos.z() += 5+(rand()%22);

        // If count is greater than 0
        if(item.count > 0)
          Map::get().sendPickupSpawn(item);
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
  int curpos  = 0;
  int orig_x, orig_y, orig_z;
  bool change = false;

  sint8 y, direction;
  sint16 blockID;
  sint32 x, z;

  user->buffer >> blockID >> x >> y >> z >> direction;

  if(!user->buffer)
	  return PACKET_NEED_MORE_DATA;

  user->buffer.removePacket();

  orig_x = x; orig_y = y; orig_z = z;

  //Invalid y value
  if(y < 0) // need only to check <0, sint8 cannot be >127
  {
    //std::cout << blockID << " (" << x << "," << (int)y_orig << "," << z << ") " << direction << std::endl;
    return PACKET_OK;
  }
  //std::cout << blockID << " (" << x << "," << (int)y_orig << "," << z << ")" << direction << std::endl;

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

  // Check liquid physics
  Physics::get().checkSurrounding(vec(x, y, z));

  //If placing normal block and current block is empty
  if(blockID < 0xff && blockID != -1 && (block_direction == BLOCK_AIR ||
                                         block_direction == BLOCK_WATER ||
                                         block_direction == BLOCK_STATIONARY_WATER ||
                                         block_direction == BLOCK_LAVA ||
                                         block_direction == BLOCK_STATIONARY_LAVA))
  {
    // DO NOT place a block if block is ...
    if(block != BLOCK_WORKBENCH &&
       block != BLOCK_FURNACE &&
       block != BLOCK_BURNING_FURNACE &&
       block != BLOCK_CHEST &&
       block != BLOCK_JUKEBOX)
      change = true;
    //std::cout << "Placing over " << (int)block_direction << std::endl;
  }

  if(block == BLOCK_SNOW || block == BLOCK_TORCH) //If snow or torch, overwrite
  {
    change = true;
    x      = orig_x;
    y      = orig_y;
    z      = orig_z;
  }

  //Door status change
  if((block == BLOCK_WOODEN_DOOR) || (block == BLOCK_IRON_DOOR))
  {
    change  = true;

    blockID = block;

    //Toggle door state
    if(metadata&0x4)
      metadata &= (0x8|0x3);
    else
      metadata |= 0x4;

    uint8 metadata2, block2;

    int modifier = (metadata&0x8) ? -1 : 1;

    x = orig_x;
    y = orig_y;
    z = orig_z;

    Map::get().getBlock(x, y+modifier, z, &block2, &metadata2);
    if(block2 == block)
    {
      metadata2 = metadata;
      if(metadata&0x8)
        metadata2 &= (0x7);
      else
        metadata2 |= 0x8;

      Map::get().setBlock(x, y+modifier, z, block2, metadata2);
      Map::get().sendBlockChange(x, y+modifier, z, (char)blockID, metadata2);
    }

  }

  // Check if player is standing there
  double intX, intZ, fracX, fracZ;
  // Check Y coordinate
  if(y == user->pos.y || y-1 == user->pos.y)
  {
    fracX = std::abs(std::modf(user->pos.x, &intX));
    fracZ = std::abs(std::modf(user->pos.z, &intZ));

    // Mystics
    intX--;
    intZ--;

    // Optimized version of the code below
    if((z == intZ || (z == intZ+1 && fracZ < 0.30) || (z == intZ-1 && fracZ > 0.70)) &&
       (x == intX || (x == intX+1 && fracZ < 0.30) || (x == intX-1 && fracX > 0.70)))
      change = false;
  }

  if(blockID == BLOCK_TORCH ||
     blockID == BLOCK_REDSTONE_TORCH_OFF ||
     blockID == BLOCK_REDSTONE_TORCH_ON)
  {
    switch(direction)
    {
    case 0:
    case 1:
      metadata = 5;
      break;

    case 2:
      metadata = 4;
      break;

    case 3:
      metadata = 3;
      break;

    case 4:
      metadata = 1;
      break;

    case 5:
      metadata = 2;
      break;
    }
  }

  if(change)
  {
    Map::get().setBlock(x, y, z, (char)blockID, metadata);
    Map::get().sendBlockChange(x, y, z, (char)blockID, metadata);


    if(blockID == BLOCK_WATER || blockID == BLOCK_STATIONARY_WATER ||
       blockID == BLOCK_LAVA || blockID == BLOCK_STATIONARY_LAVA)
      Physics::get().addSimulation(vec(x, y, z));
  }
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
  uint8 holdingPackage[7];
  holdingPackage[0] = 0x10;
  putSint32(&holdingPackage[1], user->UID);
  putSint16(&holdingPackage[5], itemID);
  user->sendOthers(&holdingPackage[0], 7);

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

  uint8 animationPackage[6];
  animationPackage[0] = 0x12;
  putSint32(&animationPackage[1], user->UID);
  animationPackage[5] = animType;
  user->sendOthers(&animationPackage[0], 6);

  return PACKET_OK;
}

int PacketHandler::pickup_spawn(User *user)
{
  spawnedItem item;
  item.EID    = generateEID();
  item.health = 0;

  sint8 yaw, pitch, roll;
  sint32 eid;

  user->buffer >> eid >> (sint16&)item.item >> (sint8&)item.count ;
  user->buffer >> (sint32&)item.pos.x() >> (sint32&)item.pos.y() >> (sint32&)item.pos.z();
  user->buffer >> yaw >> pitch >> roll;

  if(!user->buffer)
	  return PACKET_NEED_MORE_DATA;

  user->buffer.removePacket();

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

//  user->buffer.erase(user->buffer.begin(), user->buffer.begin()+curpos);

  bufferevent_disable(user->buf_ev, EV_READ);
  bufferevent_free(user->buf_ev);
  
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

  unsigned int i;

  sint32 x,z;
  sint16 y, len;

  user->buffer >> x >> y >> z >> len;

  if(!user->buffer)
	  return PACKET_NEED_MORE_DATA;

  if(!user->buffer.haveData(len))
	  return PACKET_NEED_MORE_DATA;

  //ToDo: check len
  uint8 *buffer = new uint8[len];

  user->buffer.getData(buffer, len);

  user->buffer.removePacket();

  uint8 block, meta;
  Map::get().getBlock(x, y, z, &block, &meta);

  //We only handle chest for now
  if(block != BLOCK_CHEST)
  {
    delete[] buffer;
    return PACKET_OK;
  }


  //Calculate uncompressed size and allocate memory
  uLongf uncompressedSize   = ALLOCATE_NBTFILE; //buffer[len-3] + (buffer[len-2]<<8) + (buffer[len-1]<<16) + (buffer[len]<<24);
  uint8 *uncompressedBuffer = new uint8[uncompressedSize];
  
  //Initialize zstream to handle gzip format
  z_stream zstream;
  zstream.zalloc    = (alloc_func)0;
  zstream.zfree     = (free_func)0;
  zstream.opaque    = (voidpf)0;
  zstream.next_in   = buffer;
  zstream.next_out  = uncompressedBuffer;
  zstream.avail_in  = len;
  zstream.avail_out = uncompressedSize;
  zstream.total_in  = 0;
  zstream.total_out = 0;
  zstream.data_type = Z_BINARY;
  inflateInit2(&zstream, 1+MAX_WBITS);
  //Uncompress
  if(/*int state=*/inflate(&zstream, Z_FULL_FLUSH)!=Z_OK)
  {
      inflateEnd(&zstream);
  }
  //Get size
  uncompressedSize  = zstream.total_out;
  
  //Push data to NBT struct
  NBT_struct newObject;
  TAG_Compound(uncompressedBuffer, &newObject, true);

  //These are not needed anymore
  delete[] buffer;
  delete[] uncompressedBuffer;

  //Get chunk position
  int block_x = blockToChunk(x);
  int block_z = blockToChunk(z);
  uint32 chunkID;

  NBT_struct *theEntity = 0;


  //Load map
  if(Map::get().loadMap(block_x, block_z))
  {
    Map::get().posToId(block_x, block_z, &chunkID);
    NBT_struct mapData = Map::get().maps[chunkID];

    //Try to find entitylist from the chunk
    NBT_list *entitylist = get_NBT_list(&mapData, "TileEntities");

    //If list exists
    if(entitylist)
    {
      //Verify list type
      if(entitylist->tagId != TAG_COMPOUND)
      {
        //If wrong type, recreate
        freeNBT_list(entitylist);
        for(i = 0; i < mapData.lists.size(); i++)
        {
          if(mapData.lists[i].name == "TileEntities")
          {
            //Destroy old list
            freeNBT_list(&mapData.lists[i]);
            mapData.lists.erase(mapData.lists.begin()+i);
            break;
          }
        }
        
        //New list
        NBT_list newlisting;  
        newlisting.name   = "TileEntities";
        newlisting.tagId  = TAG_COMPOUND;
        newlisting.length = 0;
        mapData.lists.push_back(newlisting);

        entitylist = get_NBT_list(&mapData, "TileEntities");
      }

      NBT_struct **entities = (NBT_struct **)entitylist->items;
      bool entityExists     = false;
      int existingID        = -1;

      //Search for mathing entity in the list
      for(int i = 0; i < entitylist->length; i++)
      {
        NBT_struct *entity = entities[i];
        std::string id;

        //Get ID
        if(get_NBT_value(entity, "id", &id))
        {
          int entity_x, entity_y, entity_z;
          if(!get_NBT_value(entity, "x", &entity_x) ||
             !get_NBT_value(entity, "y", &entity_y) ||
             !get_NBT_value(entity, "z", &entity_z))
          {
            continue;
          }

          //Check for mathing blocktype and ID
          if(block == BLOCK_CHEST && id == "Chest")
          {
            if(x == entity_x && y == entity_y && z == entity_z)
            {
              entityExists = true;
              theEntity    = entity;
              existingID   = i;
              break;
            }
          }
        }
      } //End For entitylist

      //Generate struct
      theEntity = new NBT_struct;
      NBT_value value;

      //Push ID
      value.type  = TAG_STRING;
      value.name  = "id";
      std::string *name =  new std::string;
      value.value = (void *)name;
      *(std::string *)value.value = "Chest";
      theEntity->values.push_back(value);

      //Position
      value.type  = TAG_INT;
      value.name  = "x";
      value.value = (void *)new int;
      *(int *)value.value = x;
      theEntity->values.push_back(value);

      value.name  = "y";
      value.value = (void *)new int;
      *(int *)value.value = y;
      theEntity->values.push_back(value);

      value.name  = "z";
      value.value = (void *)new int;
      *(int *)value.value = z;
      theEntity->values.push_back(value);

      //Put special chest items
      if(block == BLOCK_CHEST)
      {
        NBT_list *newlist = get_NBT_list(&newObject, "Items");
        if(!newlist)
        {
          //std::cout << "Items not found!" << std::endl;
          return PACKET_OK;
        }
        NBT_list itemlist;
        itemlist.name   = "Items";
        itemlist.tagId  = TAG_COMPOUND;
        itemlist.length = newlist->length;
        itemlist.items  = (void **)new NBT_struct *[itemlist.length];

        NBT_struct **structlist = (NBT_struct **)itemlist.items;
        for(int i = 0; i < itemlist.length; i++)
        {
          structlist[i]        = new NBT_struct;
          char type_char;
          sint16 type_sint16;

          //Generate struct
          value.type             = TAG_BYTE;
          value.name             = "Count";          
          get_NBT_value((NBT_struct *)((NBT_struct **)newlist->items)[i], "Count", &type_char);
          value.value            = (void *)new char;
          *(char *)value.value   = type_char;
          structlist[i]->values.push_back(value);

          value.type             = TAG_BYTE;
          value.name             = "Slot";
          get_NBT_value((NBT_struct *)((NBT_struct **)newlist->items)[i], "Slot", &type_char);
          value.value            = (void *)new char;
          *(char *)value.value   = type_char;
          structlist[i]->values.push_back(value);

          value.type             = TAG_SHORT;
          value.name             = "Damage";          
          get_NBT_value((NBT_struct *)((NBT_struct **)newlist->items)[i], "Damage", &type_sint16);
          value.value            = (void *)new sint16;
          *(sint16 *)value.value = type_sint16;
          structlist[i]->values.push_back(value);

          value.type             = TAG_SHORT;
          value.name             = "id";
          get_NBT_value((NBT_struct *)((NBT_struct **)newlist->items)[i], "id", &type_sint16);
          value.value            = (void *)new sint16;
          *(sint16 *)value.value = type_sint16;
          structlist[i]->values.push_back(value);
        }

        theEntity->lists.push_back(itemlist);
      }
      
      //If entity doesn't exist in the list, resize the list to fit it in
      if(!entityExists)
      {
        
        //ToDo: try this!
        NBT_struct **newlist = new NBT_struct *[entitylist->length+1];
        NBT_struct **oldlist = (NBT_struct **)entitylist->items;
        uint8 *structbuffer  = new uint8[ALLOCATE_NBTFILE];        
        for(int i = 0; i < entitylist->length; i++)
        {
          newlist[i] = new NBT_struct;
          dumpNBT_struct(oldlist[i],structbuffer);
          TAG_Compound(structbuffer, newlist[i],true);
          freeNBT_struct(oldlist[i]);
          oldlist[i] = NULL;
        }
        
        delete [] structbuffer;
        entitylist->length++;
        entitylist->items = (void **)newlist;
        delete [] (NBT_struct **)oldlist;
        newlist[entitylist->length-1] = theEntity;
        
      }
      //If item exists, replace the old with the new
      else
      {
        //Destroy old entitylist
        NBT_struct **oldlist = (NBT_struct **)entitylist->items;
        freeNBT_struct(oldlist[existingID]);
        //Replace with the new
        oldlist[existingID]  = theEntity;
      }

      //Mark chunk as changed
      Map::get().mapChanged[chunkID] = true;
      
    } //If entity exists
  } //If loaded map

  //Send complex entity packet to others
  if(theEntity)
  {
    uint8 *structdump = new uint8[ALLOCATE_NBTFILE];
    uint8 *packetData = new uint8[ALLOCATE_NBTFILE];
    int dumped        = dumpNBT_struct(theEntity, structdump);
    uLongf written    = ALLOCATE_NBTFILE;

    z_stream zstream2;
    zstream2.zalloc    = Z_NULL;
    zstream2.zfree     = Z_NULL;
    zstream2.opaque    = Z_NULL;
    zstream2.next_out  = &packetData[13];
    zstream2.next_in   = structdump;
    zstream2.avail_in  = dumped;
    zstream2.avail_out = written;
    zstream2.total_out = 0;
    zstream2.total_in  = 0;
    deflateInit2(&zstream2, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15+MAX_WBITS, 8,
                 Z_DEFAULT_STRATEGY);

    if(int state=deflate(&zstream2,Z_FULL_FLUSH)!=Z_OK)
    {
      std::cout << "Error in deflate: " << state << std::endl;
      deflateEnd(&zstream2);
    }
    else
    {
      written = zstream2.total_out;
      packetData[0] = 0x3b; //Complex Entities
      putSint32(&packetData[1],x);
      putSint16(&packetData[5],y);
      putSint32(&packetData[7],z);
      putSint16(&packetData[11], (sint16)written);
      user->sendAll((uint8 *)&packetData[0], 13+written);
    }

    delete [] packetData;
    delete [] structdump;
  }

  return PACKET_OK;
}


int PacketHandler::use_entity(User *user)
{
	sint32 userID,target;
	user->buffer >> userID >> target;
	if(!user->buffer)
		return PACKET_NEED_MORE_DATA;

	user->buffer.removePacket();

	return PACKET_OK;
}
