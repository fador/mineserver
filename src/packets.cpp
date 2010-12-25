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
#include <sys/types.h>
#ifdef WIN32
  #include <conio.h>
  #include <winsock2.h>
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <string.h>
  #include <netdb.h>
  #include <netinet/tcp.h>
#endif

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
#include <functional>


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
#include "plugin.h"
#include "furnaceManager.h"
#include "mineserver.h"

#ifdef WIN32
    #define M_PI 3.141592653589793238462643
#endif
#define DEGREES_TO_RADIANS(x) ((x) / 180.0 * M_PI)
#define RADIANS_TO_DEGREES(x) ((x) / M_PI * 180.0)

//#define _DEBUG

void PacketHandler::init()
{
  packets[PACKET_KEEP_ALIVE]               = Packets(0, &PacketHandler::keep_alive);
  packets[PACKET_LOGIN_REQUEST]            = Packets(PACKET_VARIABLE_LEN, &PacketHandler::login_request);
  packets[PACKET_HANDSHAKE]                = Packets(PACKET_VARIABLE_LEN, &PacketHandler::handshake);
  packets[PACKET_CHAT_MESSAGE]             = Packets(PACKET_VARIABLE_LEN, &PacketHandler::chat_message);
  packets[PACKET_USE_ENTITY]               = Packets( 9, &PacketHandler::use_entity);
  packets[PACKET_PLAYER]                   = Packets( 1, &PacketHandler::player);
  packets[PACKET_PLAYER_POSITION]          = Packets(33, &PacketHandler::player_position);
  packets[PACKET_PLAYER_LOOK]              = Packets( 9, &PacketHandler::player_look);
  packets[PACKET_PLAYER_POSITION_AND_LOOK] = Packets(41, &PacketHandler::player_position_and_look);
  packets[PACKET_PLAYER_DIGGING]           = Packets(11, &PacketHandler::player_digging);
  packets[PACKET_PLAYER_BLOCK_PLACEMENT]   = Packets(PACKET_VARIABLE_LEN, &PacketHandler::player_block_placement);
  packets[PACKET_HOLDING_CHANGE]           = Packets( 2, &PacketHandler::holding_change);
  packets[PACKET_ARM_ANIMATION]            = Packets( 5, &PacketHandler::arm_animation);
  packets[PACKET_PICKUP_SPAWN]             = Packets(22, &PacketHandler::pickup_spawn);
  packets[PACKET_DISCONNECT]               = Packets(PACKET_VARIABLE_LEN, &PacketHandler::disconnect);
  packets[PACKET_RESPAWN]                  = Packets( 0, &PacketHandler::respawn);
  packets[PACKET_INVENTORY_CHANGE]         = Packets(PACKET_VARIABLE_LEN, &PacketHandler::inventory_change);
  packets[PACKET_INVENTORY_CLOSE]          = Packets(1, &PacketHandler::inventory_close);
  packets[PACKET_SIGN]                     = Packets(PACKET_VARIABLE_LEN, &PacketHandler::change_sign); 
  
}

int PacketHandler::change_sign(User *user)
{
  sint32 x,z;
  sint16 y;
  std::string strings[4];

  user->buffer >> x >> y >> z;

  for(int i=0;i<4;i++)
  {
    user->buffer >> strings[i];
  }
  //ToDo: Save signs!
  signData *newSign = new signData;
  newSign->x = x;
  newSign->y = y;
  newSign->z = z;
  newSign->text[0] = strings[0];
  newSign->text[1] = strings[1];
  newSign->text[2] = strings[2];
  newSign->text[3] = strings[3];

  sChunk* chunk = Mineserver::get()->map()->chunks.GetChunk(x,z);

  if(chunk != NULL)
  {
    //Check if this sign data already exists and remove
    for(uint32 i = 0; i < chunk->signs.size(); i++)
    {
      if(chunk->signs[i]->x == x && 
         chunk->signs[i]->y == y && 
         chunk->signs[i]->z == z)
      {
        //Erase existing data
        delete chunk->signs[i];
        chunk->signs.erase(chunk->signs.begin()+i);
        break;
      }
    }
    chunk->signs.push_back(newSign);

    //Send sign packet to everyone
    Packet pkt;
    pkt << (sint8)PACKET_SIGN << x << y << z;
    pkt << strings[0] << strings[1] << strings[2] << strings[3];
    user->sendAll((uint8 *)pkt.getWrite(), pkt.getWriteLen());
  }

  Mineserver::get()->screen()->log(LOG_GENERAL,"Sign: " + strings[0] + strings[1]+ strings[2]+ strings[3]);
  
  //No need to do anything
  user->buffer.removePacket();
  return PACKET_OK;
}

int PacketHandler::inventory_close(User *user)
{
  sint8 a;

  user->buffer >> a;

  //ToDo: check user->inventoryHolding for any items and spawn if it exist

  //Mineserver::get()->screen()->log(LOG_GENERAL,"Packet 0x65: " + dtos(a));
  
  //No need to do anything
  user->buffer.removePacket();
  return PACKET_OK;
}

//
int PacketHandler::inventory_change(User *user)
{
  if(!user->buffer.haveData(10))
  {
    return PACKET_NEED_MORE_DATA;
  }
  sint8 from_type;
  sint16 from_slot;
  sint8 to_type;
  sint16 to_slot;
  sint16 itemID;
  sint8 item_count;
  sint8 item_health;

  user->buffer >> from_type >> from_slot >> to_type >> to_slot >> itemID;
  if(itemID != -1)
  {
    if(!user->buffer.haveData(2))
    {
      return PACKET_NEED_MORE_DATA;
    }
    user->buffer >> item_count >> item_health;
  }
  if(from_type == 0) //Player inventory
  {
    Item slotItem;
    if(from_slot<5)
    {
      slotItem=user->inv.crafting[from_slot-1];
    }
    else if(from_slot<9)
    {
      slotItem=user->inv.equipped[from_slot-5];
    }
    else
    {
      slotItem=user->inv.main[from_slot-9];
    }
    //Empty slot and holding something
    if((itemID == -1 || (slotItem.type == itemID && slotItem.count < 64) ) && user->inventoryHolding.type != 0)
    {
      sint16 addCount=(64-slotItem.count>=user->inventoryHolding.count)?user->inventoryHolding.count:64-slotItem.count;
      if(to_type == 1)
      {
        addCount = 1;
      }
      if(from_slot<5)
      {
        user->inv.crafting[from_slot-1].count  += addCount;
        user->inv.crafting[from_slot-1].health = user->inventoryHolding.health;
        user->inv.crafting[from_slot-1].type   = user->inventoryHolding.type;
      }
      else if(from_slot<9)
      {
        user->inv.equipped[from_slot-5].count  += addCount;
        user->inv.equipped[from_slot-5].health = user->inventoryHolding.health;
        user->inv.equipped[from_slot-5].type   = user->inventoryHolding.type;
      }
      else
      {
        user->inv.main[from_slot-9].count  += addCount;
        user->inv.main[from_slot-9].health = user->inventoryHolding.health;
        user->inv.main[from_slot-9].type   = user->inventoryHolding.type;
      }
      if(to_type == 1)
      {
        user->inventoryHolding.count--;
        if(user->inventoryHolding.count == 0)
        {
          user->inventoryHolding.type  = -1;
          user->inventoryHolding.health= 0;
        }
      }
      else
      {
        user->inventoryHolding.count -=addCount;
        if(user->inventoryHolding.count == 0)
        {
          user->inventoryHolding.type  = -1;
          user->inventoryHolding.health= 0;
        }
      }
    }
    else if(user->inventoryHolding.type == -1)
    {
      user->inventoryHolding.type   = itemID;
      user->inventoryHolding.health = item_health;
      user->inventoryHolding.count  = item_count;
      if(to_type == 1)
      {
        user->inventoryHolding.count  -= item_count>>1;
      }

      if(from_slot<5)
      {
        user->inv.crafting[from_slot-1].count  -= user->inventoryHolding.count;
        if(user->inv.crafting[from_slot-1].count == 0)
        {
          user->inv.crafting[from_slot-1].health = 0;
          user->inv.crafting[from_slot-1].count  = 0;
        }
      }
      else if(from_slot<9)
      {
        user->inv.equipped[from_slot-5].count  -= user->inventoryHolding.count;
        if(user->inv.equipped[from_slot-5].count == 0)
        {
          user->inv.equipped[from_slot-5].health = 0;
          user->inv.equipped[from_slot-5].count  = 0;
        }
      }
      else
      {
        user->inv.main[from_slot-9].count  -= user->inventoryHolding.count;
        if(user->inv.main[from_slot-9].count == 0)
        {
          user->inv.main[from_slot-9].health = 0;
          user->inv.main[from_slot-9].count  = 0;
        }
      }
    }
  }
  //Mineserver::get()->screen()->log(LOG_GENERAL,"Packet 0x66: " + dtos(from_type) + "," + dtos(from_slot) + "," + dtos(to_type) +  "," + dtos(to_slot) + "," + dtos(itemID) + "," + dtos(item_count) + "," + dtos(item_health));

  //No need to do anything
  user->buffer.removePacket();
  return PACKET_OK;
}

// Keep Alive (http://mc.kev009.com/wiki/Protocol#Keep_Alive_.280x00.29)
int PacketHandler::keep_alive(User *user)
{
  //No need to do anything
  user->buffer.removePacket();
  return PACKET_OK;
}

//Source: http://wiki.linuxquestions.org/wiki/Connecting_a_socket_in_C
int socket_connect(char *host, int port)
{
  struct hostent *hp;
  struct sockaddr_in addr;
  int on = 1, sock;     

  if((hp = gethostbyname(host)) == NULL)
  {
      return 0;
  }

  memmove(&addr.sin_addr, hp->h_addr, hp->h_length);
  addr.sin_port = htons(port);
  addr.sin_family = AF_INET;
  sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  struct timeval tv;
  tv.tv_sec = 2;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,(const char *)&tv,sizeof(struct timeval));
  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO,(const char *)&tv,sizeof(struct timeval));

  setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(int));

  if (sock == -1)
  {
    return 0;
  }

  if (connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
  {
    return 0;
  }

  return sock;
}

// Login request (http://mc.kev009.com/wiki/Protocol#Login_Request_.280x01.29)
int PacketHandler::login_request(User *user)
{
  //Check that we have enought data in the buffer
  if(!user->buffer.haveData(12))
  {
    return PACKET_NEED_MORE_DATA;
  }

  sint32 version;
  std::string player, passwd;
  sint64 mapseed;
  sint8 dimension;

  user->buffer >> version >> player >> passwd >> mapseed >> dimension;

  if(!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->buffer.removePacket();

  Mineserver::get()->screen()->log("Player " + dtos(user->UID) + " login v." + dtos(version) + " : " + player + ":" + passwd);

  user->nick = player;

  // If version is not 2 or 3
  if(version != PROTOCOL_VERSION)
  {
    user->kick(Mineserver::get()->conf()->sValue("wrong_protocol_message"));
    return PACKET_OK;
  }

  // If userlimit is reached
  if((int)User::all().size() >= Mineserver::get()->conf()->iValue("user_limit"))
  {
    user->kick(Mineserver::get()->conf()->sValue("server_full_message"));
    return PACKET_OK;
  }

  std::string kickMessage;
  if ((static_cast<Hook2<bool,User*,std::string*>*>(Mineserver::get()->plugin()->getHook("LoginPre")))->doUntilFalse(user, &kickMessage))
  {
    user->kick(kickMessage);
  }
  else
  {
    (static_cast<Hook1<void,User*>*>(Mineserver::get()->plugin()->getHook("LoginPost")))->doAll(user);
    user->sendLoginInfo();
  }

  return PACKET_OK;
}

int PacketHandler::handshake(User *user)
{
  if(!user->buffer.haveData(3))
    return PACKET_NEED_MORE_DATA;

  std::string player;

  user->buffer >> player;

  // Check for data
  if(!user->buffer)
    return PACKET_NEED_MORE_DATA;

  // Remove package from buffer
  user->buffer.removePacket();

  // Check whether we're to validate against minecraft.net
  if(Mineserver::get()->conf()->bValue("user_validation") == true)
  {
    // Send the unique hash for this player to prompt the client to go to minecraft.net to validate
    Mineserver::get()->screen()->log("Handshake: Giving player "+player+" their minecraft.net hash of: " + hash(player));
    user->buffer << (sint8)PACKET_HANDSHAKE << hash(player);
  }
  else
  {
    // Send "no validation or password needed" validation
    Mineserver::get()->screen()->log("Handshake: No validation required for player "+player+".");
    user->buffer << (sint8)PACKET_HANDSHAKE << std::string("-");
  }
  // TODO: Add support for prompting user for Server password (once client supports it)

  return PACKET_OK;
}

int PacketHandler::chat_message(User *user)
{
  // Wait for length-short. HEHE
  if(!user->buffer.haveData(2))
  {
    return PACKET_NEED_MORE_DATA;
  }

  std::string msg;

  user->buffer >> msg;

  if(!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->buffer.removePacket();

  Mineserver::get()->chat()->handleMsg(user, msg);

  return PACKET_OK;
}

int PacketHandler::player(User *user)
{
  //OnGround packet
  sint8 onground;
  user->buffer >> onground;
  if(!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }
  user->buffer.removePacket();
  return PACKET_OK;
}

int PacketHandler::player_position(User *user)
{
  double x, y, stance, z;
  sint8 onground;

  user->buffer >> x >> y >> stance >> z >> onground;

  if(!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

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
  {
    return PACKET_NEED_MORE_DATA;
  }

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
  {
    return PACKET_NEED_MORE_DATA;
  }

  //Update user data
  user->updatePos(x, y, z, stance);
  user->updateLook(yaw, pitch);

  user->buffer.removePacket();

  return PACKET_OK;
}

int PacketHandler::player_digging(User *user)
{
  sint8 status;
  sint32 x;
  sint8  y;
  sint32 z;
  sint8 direction;
  uint8 block;
  uint8 meta;

  user->buffer >> status >> x >> y >> z >> direction;

  if(!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->buffer.removePacket();

  if(!Mineserver::get()->map()->getBlock(x, y, z, &block, &meta))
  {
    return PACKET_OK;
  }

  Function::invoker_type inv(user, status, x, y, z, direction);

  switch(status)
  {
    case BLOCK_STATUS_STARTED_DIGGING:
    {
      (static_cast<Hook4<void,User*,sint32,sint8,sint32>*>(Mineserver::get()->plugin()->getHook("DiggingStarted")))->doAll(user, x, y, z);
      Mineserver::get()->plugin()->runBlockCallback(block, "onStartedDigging", inv);
      break;
    }
    case BLOCK_STATUS_DIGGING:
    {
      (static_cast<Hook4<void,User*,sint32,sint8,sint32>*>(Mineserver::get()->plugin()->getHook("Digging")))->doAll(user, x, y, z);
      Mineserver::get()->plugin()->runBlockCallback(block, "onDigging", inv);
      break;
    }
    case BLOCK_STATUS_STOPPED_DIGGING:
    {
      (static_cast<Hook4<void,User*,sint32,sint8,sint32>*>(Mineserver::get()->plugin()->getHook("DiggingStopped")))->doAll(user, x, y, z);
      Mineserver::get()->plugin()->runBlockCallback(block, "onStoppedDigging", inv);
      break;
    }
    case BLOCK_STATUS_BLOCK_BROKEN:
    {
      //ToDo: Fix, made for testing purposes!
      if(user->inv.main[27+user->currentItemSlot()].type == ITEM_WOODEN_SPADE)
         user->inv.main[27+user->currentItemSlot()].health++;

      if ((static_cast<Hook4<bool,User*,sint32,sint8,sint32>*>(Mineserver::get()->plugin()->getHook("BlockBreakPre")))->doUntilFalse(user, x, y, z))
      {
        return PACKET_OK;
      }

      (static_cast<Hook4<void,User*,sint32,sint8,sint32>*>(Mineserver::get()->plugin()->getHook("BlockBreakPost")))->doAll(user, x, y, z);
      Mineserver::get()->plugin()->runBlockCallback(block, "onBroken", inv);

      /* notify neighbour blocks of the broken block */
      status = block;
      if (Mineserver::get()->map()->getBlock(x+1, y, z, &block, &meta) && block != BLOCK_AIR)
      {
        (static_cast<Hook7<bool,User*,sint32,sint8,sint32, sint32, sint8, sint32>*>(Mineserver::get()->plugin()->getHook("BlockNeighbourBreak")))->doAll(user, x+1, y, z, x, y, z);
        inv = Function::invoker_type(user, status, x+1, y, z, BLOCK_SOUTH);
        Mineserver::get()->plugin()->runBlockCallback(block, "onNeighbourBroken", inv);
      }

      if (Mineserver::get()->map()->getBlock(x-1, y, z, &block, &meta) && block != BLOCK_AIR)
      {
        (static_cast<Hook7<bool,User*,sint32,sint8,sint32, sint32, sint8, sint32>*>(Mineserver::get()->plugin()->getHook("BlockNeighbourBreak")))->doAll(user, x-1, y, z, x, y, z);
        inv = Function::invoker_type(user, status, x-1, y, z, BLOCK_NORTH);
        Mineserver::get()->plugin()->runBlockCallback(block, "onNeighbourBroken", inv);
      }

      if (Mineserver::get()->map()->getBlock(x, y+1, z, &block, &meta) && block != BLOCK_AIR)
      {
        (static_cast<Hook7<bool,User*,sint32,sint8,sint32, sint32, sint8, sint32>*>(Mineserver::get()->plugin()->getHook("BlockNeighbourBreak")))->doAll(user, x, y+1, z, x, y, z);
        inv = Function::invoker_type(user, status, x, y+1, z, BLOCK_TOP);
        Mineserver::get()->plugin()->runBlockCallback(block, "onNeighbourBroken", inv);
      }

      if (Mineserver::get()->map()->getBlock(x, y-1, z, &block, &meta) && block != BLOCK_AIR)
      {
        (static_cast<Hook7<bool,User*,sint32,sint8,sint32, sint32, sint8, sint32>*>(Mineserver::get()->plugin()->getHook("BlockNeighbourBreak")))->doAll(user, x, y-1, z, x, y, z);
        inv = Function::invoker_type(user, status, x, y-1, z, BLOCK_BOTTOM);
        Mineserver::get()->plugin()->runBlockCallback(block, "onNeighbourBroken", inv);
      }

      if (Mineserver::get()->map()->getBlock(x, y, z+1, &block, &meta) && block != BLOCK_AIR)
      {
        (static_cast<Hook7<bool,User*,sint32,sint8,sint32, sint32, sint8, sint32>*>(Mineserver::get()->plugin()->getHook("BlockNeighbourBreak")))->doAll(user, x, y, z+1, x, y, z);
        inv = Function::invoker_type(user, status, x, y, z+1, BLOCK_WEST);
        Mineserver::get()->plugin()->runBlockCallback(block, "onNeighbourBroken", inv);
      }

      if (Mineserver::get()->map()->getBlock(x, y, z-1, &block, &meta) && block != BLOCK_AIR)
      {
        (static_cast<Hook7<bool,User*,sint32,sint8,sint32, sint32, sint8, sint32>*>(Mineserver::get()->plugin()->getHook("BlockNeighbourBreak")))->doAll(user, x, y, z-1, x, y, z);
        inv = Function::invoker_type(user, status, x, y, z-1, BLOCK_EAST);
        Mineserver::get()->plugin()->runBlockCallback(block, "onNeighbourBroken", inv);
      }

      break;
    }
    case BLOCK_STATUS_PICKUP_SPAWN:
    {
      //ToDo: handle
      if(user->inv.main[27+user->currentItemSlot()].type > 0)
      {
        Mineserver::get()->map()->createPickupSpawn(user->pos.x, user->pos.y,user->pos.z,user->inv.main[27+user->currentItemSlot()].type,1,user->inv.main[27+user->currentItemSlot()].health,user);

        user->inv.main[27+user->currentItemSlot()].count--;
        if(user->inv.main[27+user->currentItemSlot()].count == 0)
        {
          user->inv.main[27+user->currentItemSlot()].type   = -1;
          user->inv.main[27+user->currentItemSlot()].health = 0;
        }

        user->buffer << (sint8)PACKET_SET_SLOT << (sint8)0 << (sint16)(36+user->currentItemSlot()) 
                     << (sint16)user->inv.main[27+user->currentItemSlot()].type 
                     << (sint8)user->inv.main[27+user->currentItemSlot()].count 
                     << (sint8)user->inv.main[27+user->currentItemSlot()].health;
      }
      break;
    }
    
  }

  return PACKET_OK;
}

int PacketHandler::player_block_placement(User *user)
{
  if(!user->buffer.haveData(12))
  {
    return PACKET_NEED_MORE_DATA;
  }
  sint8 y, direction;
  sint16 newblock;
  sint32 x, z;
  /* replacement of block */
  uint8 oldblock;
  uint8 metadata;
  /* neighbour blocks */
  uint8 block;
  uint8 meta;
  sint8 count,health;

  user->buffer >> x >> y >> z >> direction >> newblock;

  if(newblock >= 0)
  {
    if(!user->buffer.haveData(2))
    {
      return PACKET_NEED_MORE_DATA;
    }
    user->buffer >> count >> health;
  }
  user->buffer.removePacket();


  if (!Mineserver::get()->map()->getBlock(x, y, z, &oldblock, &metadata))
  {
    return PACKET_OK;
  }
  
  if(oldblock == BLOCK_CHEST)
  {
    user->buffer << (sint8)PACKET_OPEN_WINDOW << (sint8)1  << (sint8)0 << std::string("Chest") << (sint8)28;

    sChunk* chunk = Mineserver::get()->map()->chunks.GetChunk(x,z);

    if(chunk == NULL)
    {
      return PACKET_OK;
    }

    for(uint32 i = 0;i < chunk->chests.size(); i++)
    {
      if(chunk->chests[i]->x == x && chunk->chests[i]->z == z)
      {
        for(int i = 0;i < 28; i++)
        {
          if(chunk->chests[i]->items[i].type != -1)
          {
            user->buffer << (sint8)PACKET_SET_SLOT << (sint8)1 << (sint16)(i) << (sint16)chunk->chests[i]->items[i].type 
                         << (sint8)(chunk->chests[i]->items[i].count) << (sint8)chunk->chests[i]->items[i].health;
          }
        }
      }
    }

    return PACKET_OK;
  }

  if(oldblock == BLOCK_FURNACE || oldblock == BLOCK_BURNING_FURNACE)
  {
    sChunk* chunk = Mineserver::get()->map()->chunks.GetChunk(x,z);

    user->buffer << (sint8)PACKET_OPEN_WINDOW << (sint8)3  << (sint8)2 << std::string("Furnace") << (sint8)0;


    if(chunk == NULL)
    {
      return PACKET_OK;
    }

    for(uint32 i = 0;i < chunk->furnaces.size(); i++)
    {
      if(chunk->furnaces[i]->x == x && chunk->furnaces[i]->z == z)
      {
        for(int i = 0; i < 3; i++)
        {
          if(chunk->furnaces[i]->items[i].type != -1)
          {
            user->buffer << (sint8)PACKET_SET_SLOT << (sint8)3 << (sint16)(i) << (sint16)chunk->furnaces[i]->items[i].type 
                         << (sint8)(chunk->furnaces[i]->items[i].count) << (sint8)chunk->furnaces[i]->items[i].health;
          }
        }
      }
    }

    return PACKET_OK;
  }

  if(oldblock == BLOCK_WORKBENCH)
  {
    sChunk* chunk = Mineserver::get()->map()->chunks.GetChunk(x,z);
    user->buffer << (sint8)PACKET_OPEN_WINDOW << (sint8)2  << (sint8)1 << std::string("Workbench") << (sint8)0;


    if(chunk == NULL)
    {
      return PACKET_OK;
    }

    //ToDo: send inventory
    return PACKET_OK;
  }



  bool foundFromInventory = false;

  if(user->inv.main[27+user->currentItemSlot()].type == newblock)
  {
    //Are we really placing this?
    user->inv.main[27+user->currentItemSlot()].count--;
    if(user->inv.main[27+user->currentItemSlot()].count == 0)
    {
      user->inv.main[27+user->currentItemSlot()].type   = 0;
      user->inv.main[27+user->currentItemSlot()].health = 0;
      //ToDo: add holding change packet.
    }
    foundFromInventory = true;
  }


  // TODO: Handle processing of
  if(direction == -1 || !foundFromInventory)
  {
    return PACKET_OK;
  }

  // Minecart testing!!
  if(newblock == ITEM_MINECART && Mineserver::get()->map()->getBlock(x, y, z, &oldblock, &metadata))
  {
    if (oldblock != BLOCK_MINECART_TRACKS)
    {
      return PACKET_OK;
    }

    Mineserver::get()->screen()->log("Spawn minecart");
    sint32 EID=generateEID();
    Packet pkt;
    // MINECART
    pkt << PACKET_ADD_OBJECT << (sint32)EID << (sint8)10 << (sint32)(x*32+16) << (sint32)(y*32) << (sint32)(z*32+16);
    user->sendAll((uint8 *)pkt.getWrite(), pkt.getWriteLen());
  }

  if (newblock == -1 && newblock != ITEM_SIGN)
  {
     Mineserver::get()->screen()->log("ignoring: "+newblock);
     return PACKET_OK;
  }

  if(y < 0)
  {
    return PACKET_OK;
  }


#ifdef _DEBUG
  Mineserver::get()->screen()->log("Block_placement: "+dtos(newblock)+" ("+dtos(x)+","+dtos((int)y)+","+dtos(z)+") dir: "+dtos((int)direction));
#endif

  if (direction)
  {
    direction = 6-direction;
  }

  Callback callback;
  Function event;
  Function::invoker_type inv(user, newblock, x, y, z, direction);

  //if (Mineserver::get()->map()->getBlock(x, y, z, &oldblock, &metadata))
  {
    uint8 oldblocktop;
    uint8 metadatatop;
    sint8 check_y = y;
    sint32 check_x = x;
    sint32 check_z = z;

    /* client doesn't give us the correct block for lava and water, check block above */
    switch(direction)
    {
    case BLOCK_TOP:
      check_y++;
      break;
    case BLOCK_NORTH:
      check_x++;
      break;
    case BLOCK_SOUTH:
      check_x--;
      break;
    case BLOCK_EAST:
      check_z++;
      break;
    case BLOCK_WEST:
      check_z--;
      break;
    default:
      break;
    }

    if (Mineserver::get()->map()->getBlock(check_x, check_y, check_z, &oldblocktop, &metadatatop) && (oldblocktop == BLOCK_LAVA || oldblocktop == BLOCK_STATIONARY_LAVA || oldblocktop == BLOCK_WATER || oldblocktop == BLOCK_STATIONARY_WATER))
    {
      /* block above needs replacing rather then the block sent by the client */
      inv = Function::invoker_type(user, newblock, check_x, check_y, check_z, direction);
      Mineserver::get()->plugin()->runBlockCallback(oldblocktop, "onReplace", inv);
      if ((static_cast<Hook6<bool,User*,sint32,sint8,sint32,uint8,uint8>*>(Mineserver::get()->plugin()->getHook("BlockReplacePre")))->doUntilFalse(user, check_x, check_y, check_z, oldblock, newblock))
      {
        return PACKET_OK;
      }
      (static_cast<Hook6<void,User*,sint32,sint8,sint32,uint8,uint8>*>(Mineserver::get()->plugin()->getHook("BlockReplacePost")))->doAll(user, check_x, check_y, check_z, oldblock, newblock);
    }
    else
    {
      inv = Function::invoker_type(user, newblock, x, y, z, direction);
      Mineserver::get()->plugin()->runBlockCallback(oldblock, "onReplace", inv);
      if ((static_cast<Hook6<bool,User*,sint32,sint8,sint32,uint8,uint8>*>(Mineserver::get()->plugin()->getHook("BlockReplacePre")))->doUntilFalse(user, x, y, z, oldblock, newblock))
      {
        return PACKET_OK;
      }
      (static_cast<Hook6<void,User*,sint32,sint8,sint32,uint8,uint8>*>(Mineserver::get()->plugin()->getHook("BlockReplacePost")))->doAll(user, x, y, z, oldblock, newblock);
    }

    if ((static_cast<Hook5<bool,User*,sint32,sint8,sint32,uint8>*>(Mineserver::get()->plugin()->getHook("BlockPlacePre")))->doUntilFalse(user, x, y, z, newblock))
    {
      return PACKET_OK;
    }

    /* We pass the newblock to the newblock's onPlace callback because
    the callback doesn't know what type of block we're placing. Instead
    the callback's job is to describe the behaviour when placing the
    block down, not to place any specifically block itself. */
    inv = Function::invoker_type(user, newblock, x, y, z, direction);
    Mineserver::get()->plugin()->runBlockCallback(newblock, "onPlace", inv);

    (static_cast<Hook5<void,User*,sint32,sint8,sint32,uint8>*>(Mineserver::get()->plugin()->getHook("BlockPlacePost")))->doAll(user, x, y, z, newblock);

    /* notify neighbour blocks of the placed block */
    if (Mineserver::get()->map()->getBlock(x+1, y, z, &block, &meta) && block != BLOCK_AIR)
    {
      inv = Function::invoker_type(user, newblock, x+1, y, z, BLOCK_SOUTH);
      Mineserver::get()->plugin()->runBlockCallback(block, "onNeighbourPlace", inv);
      (static_cast<Hook4<void,User*,sint32,sint8,sint32>*>(Mineserver::get()->plugin()->getHook("BlockNeighbourPlace")))->doAll(user, x+1, y, z);
    }

    if (Mineserver::get()->map()->getBlock(x-1, y, z, &block, &meta) && block != BLOCK_AIR)
    {
      inv = Function::invoker_type(user, newblock, x-1, y, z, BLOCK_NORTH);
      Mineserver::get()->plugin()->runBlockCallback(block, "onNeighbourPlace", inv);
      (static_cast<Hook4<void,User*,sint32,sint8,sint32>*>(Mineserver::get()->plugin()->getHook("BlockNeighbourPlace")))->doAll(user, x-1, y, z);
    }

    if (Mineserver::get()->map()->getBlock(x, y+1, z, &block, &meta) && block != BLOCK_AIR)
    {
      inv = Function::invoker_type(user, newblock, x, y+1, z, BLOCK_TOP);
      Mineserver::get()->plugin()->runBlockCallback(block, "onNeighbourPlace", inv);
      (static_cast<Hook4<void,User*,sint32,sint8,sint32>*>(Mineserver::get()->plugin()->getHook("BlockNeighbourPlace")))->doAll(user, x, y+1, z);
    }

    if (Mineserver::get()->map()->getBlock(x, y-1, z, &block, &meta) && block != BLOCK_AIR)
    {
      inv = Function::invoker_type(user, newblock, x, y-1, z, BLOCK_BOTTOM);
      Mineserver::get()->plugin()->runBlockCallback(block, "onNeighbourPlace", inv);
      (static_cast<Hook4<void,User*,sint32,sint8,sint32>*>(Mineserver::get()->plugin()->getHook("BlockNeighbourPlace")))->doAll(user, x, y-1, z);
    }

    if (Mineserver::get()->map()->getBlock(x, y, z+1, &block, &meta) && block != BLOCK_AIR)
    {
      inv = Function::invoker_type(user, newblock, x, y, z+1, BLOCK_WEST);
      Mineserver::get()->plugin()->runBlockCallback(block, "onNeighbourPlace", inv);
      (static_cast<Hook4<void,User*,sint32,sint8,sint32>*>(Mineserver::get()->plugin()->getHook("BlockNeighbourPlace")))->doAll(user, x, y, z+1);
    }

    if (Mineserver::get()->map()->getBlock(x, y, z-1, &block, &meta) && block != BLOCK_AIR)
    {
      inv = Function::invoker_type(user, newblock, x, y, z-1, BLOCK_EAST);
      Mineserver::get()->plugin()->runBlockCallback(block, "onNeighbourPlace", inv);
      (static_cast<Hook4<void,User*,sint32,sint8,sint32>*>(Mineserver::get()->plugin()->getHook("BlockNeighbourPlace")))->doAll(user, x, y, z-1);
    }
  }

  /* TODO: Should be removed from here. Only needed for liquid related blocks? */
  Mineserver::get()->physics()->checkSurrounding(vec(x, y, z));
  return PACKET_OK;
}

int PacketHandler::holding_change(User *user)
{
  sint16 itemSlot;
  user->buffer >> itemSlot;

  if(!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->buffer.removePacket();

  user->curItem = itemSlot;

  //Send holding change to others
  Packet pkt;
  pkt << (sint8)PACKET_ENTITY_EQUIPMENT << (sint32)user->UID << (sint16)0 << (sint16)user->inv.main[itemSlot+27].type;
  user->sendOthers((uint8*)pkt.getWrite(), pkt.getWriteLen());

  // Set current itemID to user
  user->setCurrentItemSlot(itemSlot);

  return PACKET_OK;
}

int PacketHandler::arm_animation(User *user)
{
  sint32 userID;
  sint8 animType;

  user->buffer >> userID >> animType;

  if(!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->buffer.removePacket();

  Packet pkt;
  pkt << (sint8)PACKET_ARM_ANIMATION << (sint32)user->UID << animType;
  user->sendOthers((uint8*)pkt.getWrite(), pkt.getWriteLen());

  return PACKET_OK;
}

int PacketHandler::pickup_spawn(User *user)
{
  //uint32 curpos = 4; //warning: unused variable ‘curpos’
  spawnedItem item;

  item.health = 0;

  sint8 yaw, pitch, roll;

  user->buffer >> (sint32&)item.EID;

  user->buffer >> (sint16&)item.item >> (sint8&)item.count ;
  user->buffer >> (sint32&)item.pos.x() >> (sint32&)item.pos.y() >> (sint32&)item.pos.z();
  user->buffer >> yaw >> pitch >> roll;

  if(!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->buffer.removePacket();

  item.EID       = generateEID();

  item.spawnedBy = user->UID;
  item.spawnedAt = time(NULL);

  // Modify the position of the dropped item so that it appears in front of user instead of under user
  int distanceToThrow = 64;
  float angle = DEGREES_TO_RADIANS(user->pos.yaw + 45);     // For some reason, yaw seems to be off to the left by 45 degrees from where you're actually looking?
  int x = int(cos(angle) * distanceToThrow - sin(angle) * distanceToThrow);
  int z = int(sin(angle) * distanceToThrow + cos(angle) * distanceToThrow);
  item.pos += vec(x, 0, z);

  Mineserver::get()->map()->sendPickupSpawn(item);

  return PACKET_OK;
}

int PacketHandler::disconnect(User *user)
{
  if(!user->buffer.haveData(2))
    return PACKET_NEED_MORE_DATA;

  std::string msg;
  user->buffer >> msg;

  if(!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->buffer.removePacket();

  Mineserver::get()->screen()->log("Disconnect: " + msg);

  event_del(user->GetEvent());

  #ifdef WIN32
  closesocket(user->fd);
  #else
  close(user->fd);
  #endif

  delete user;

  return PACKET_OK;
}


int PacketHandler::use_entity(User *user)
{
  sint32 userID, target;
  sint8 targetType;

  user->buffer >> userID >> target >> targetType;

  if (!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->buffer.removePacket();

  if(targetType != 1)
  {

    Packet pkt;
    //Attach
    if(user->attachedTo == 0)
    {
      pkt << PACKET_ATTACH_ENTITY << (sint32)user->UID << (sint32)target;
      user->attachedTo = target;
    }
    //Detach
    else
    {
      pkt << PACKET_ATTACH_ENTITY << (sint32)user->UID << (sint32)-1;
      user->attachedTo = 0;
    }
    user->sendAll((uint8*)pkt.getWrite(), pkt.getWriteLen());
    return PACKET_OK;
  }

  //This is used when punching users
  for(uint32 i = 0; i < User::all().size(); i++)
  {
    if(User::all()[i]->UID == (uint32)target)
    {
      User::all()[i]->health--;
      User::all()[i]->sethealth(User::all()[i]->health);

      if(User::all()[i]->health <= 0)
      {
        Packet pkt;
        pkt << PACKET_DEATH_ANIMATION << (sint32)User::all()[i]->UID << (sint8)3;
        User::all()[i]->sendOthers((uint8*)pkt.getWrite(), pkt.getWriteLen());
      }
      break;
    }
  }

  return PACKET_OK;
}

int PacketHandler::respawn(User *user)
{
  user->dropInventory();
  user->respawn();
  user->teleport(Mineserver::get()->map()->spawnPos.x(), Mineserver::get()->map()->spawnPos.y() + 2, Mineserver::get()->map()->spawnPos.z());
  user->buffer.removePacket();
  return PACKET_OK;
}
