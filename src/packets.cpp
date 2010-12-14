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

#ifdef WIN32
    #define M_PI 3.141592653589793238462643
#endif
#define DEGREES_TO_RADIANS(x) ((x) / 180.0 * M_PI)
#define RADIANS_TO_DEGREES(x) ((x) / M_PI * 180.0)

//#define _DEBUG

PacketHandler* PacketHandler::mPacketHandler;

void PacketHandler::free()
{
   if (mPacketHandler)
   {
      delete mPacketHandler;
      mPacketHandler = 0;
   }
}

void PacketHandler::init()
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
  packets[PACKET_USE_ENTITY]               = Packets( 9, &PacketHandler::use_entity);
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
  packets[PACKET_RESPAWN]                  = Packets( 0, &PacketHandler::respawn);

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

  memmove(&addr.sin_addr,hp->h_addr,  hp->h_length);
  addr.sin_port = htons(port);
  addr.sin_family = AF_INET;
  sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  struct timeval tv;
  tv.tv_sec = 2;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,(const char *)&tv,sizeof(struct timeval));
  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO,(const char *)&tv,sizeof(struct timeval));

  setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(int));
  if(sock == -1)
  {
    return 0;
  }
  if(connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
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
    return PACKET_NEED_MORE_DATA;

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

  Screen::get()->log("Player " + dtos(user->UID) + " login v." + dtos(version) + " : " + player + ":" + passwd);
  
  user->temp_nick=player;

  // If version is not 2 or 3
  if(version != PROTOCOL_VERSION)
  {
    user->kick(Conf::get()->sValue("wrong_protocol_message"));
    return PACKET_OK;
  }

    // If userlimit is reached
  if((int)User::all().size() >= Conf::get()->iValue("user_limit"))
  {
    user->kick(Conf::get()->sValue("server_full_message"));
    return PACKET_OK;
  }

  // Check if user is on the whitelist
  // But first, is it enabled?
  if(Conf::get()->bValue("use_whitelist") == true)
  {
    if(user->checkWhitelist(player))
    {
      user->kick(Conf::get()->sValue("default_whitelist_message"));
      return PACKET_OK;
    }
  }

  // If user is banned
  if(user->checkBanned(player))
  {
    user->kick(Conf::get()->sValue("default_banned_message"));
    return PACKET_OK;
  }


  // Check if we're to do user validation
  if(Conf::get()->bValue("user_validation") == true)
  {    
    std::string url = "/game/checkserver.jsp?user=" + player + "&serverId=" + hash(player);
    Screen::get()->log("Validating " + player + " against minecraft.net: ");

    std::string http_request ="GET " + url + " HTTP/1.1\r\n"
                             +"Host: www.minecraft.net\r\n"
                             +"Connection: close\r\n\r\n";

    int fd=socket_connect((char*)"www.minecraft.net", 80);
    if(fd)
    {
      #ifdef WIN32
      send(fd, http_request.c_str(), http_request.length(),NULL);
      #else
      write(fd, http_request.c_str(), http_request.length());
      #endif

      #define BUFFER_SIZE 1024
      char *buffer = new char[BUFFER_SIZE];
      std::string stringbuffer;

      #ifdef WIN32
      while(int received=recv(fd, buffer, BUFFER_SIZE - 1, NULL) != 0)
      {
      #else
      while(read(fd, buffer, BUFFER_SIZE - 1) != 0)
      {
      #endif
        stringbuffer+=std::string(buffer);
      }
      delete [] buffer;
      #ifdef WIN32
      closesocket(fd);
      #else
      close(fd);
      #endif

      bool allow_access = false;
      //No response data, timeout
      if(stringbuffer.size() == 0 && Conf::get()->bValue("allow_connect_on_auth_timeout"))
      {
        Screen::get()->log("  Auth skipped on timeout ");
        allow_access = true;
      }
      
      if(allow_access || (stringbuffer.size()>=3 && stringbuffer.find("\r\n\r\nYES",0) != std::string::npos))
      {
        Screen::get()->log("  Verified!");
        user->sendLoginInfo();
      }
      else
      {
        Screen::get()->log("  Failed"  + stringbuffer.substr(stringbuffer.size()-3));
        user->kick("Failed to verify username!");
      }
    }
    else
    {
      Screen::get()->log("  Failed");
      user->kick("Failed to verify username!");
    }

    return PACKET_OK;
  }
  user->sendLoginInfo();

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
  if(Conf::get()->bValue("user_validation") == true)
  {
    // Send the unique hash for this player to prompt the client to go to minecraft.net to validate
    Screen::get()->log("Handshake: Requesting minecraft.net validation for player: " + player + " " + hash(player));
    user->buffer << (sint8)PACKET_HANDSHAKE << hash(player);
  }
  else
  {
    // Send "no validation or password needed" validation
    Screen::get()->log("Handshake: No validation for player: " + player);
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

  Chat::get()->handleMsg( user, msg );

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
  case -2:
    //items = 4;
    memset(user->inv.equipped, 0, sizeof(Item)*4);
    slots = (Item *)&user->inv.equipped;
    break;

  //Crafting slots
  case -3:
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
    {
      return PACKET_NEED_MORE_DATA;
    }

    if(item_id != -1)
    {
      user->buffer >> numberOfItems >> health;

      if(!user->buffer)
      {
        return PACKET_NEED_MORE_DATA;
      }


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
  sint8 status,y;
  sint32 x,z;
  sint8 direction;
  uint8 block;
  uint8 meta;
  user->buffer >> status >> x >> y >> z >> direction;

  if(!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->buffer.removePacket();

  if(!Map::get()->getBlock(x, y, z, &block, &meta))
  {
    return PACKET_OK;
  }

  Function::invoker_type inv(user, status, x, y, z, direction);

  switch(status)
  {
     case BLOCK_STATUS_STARTED_DIGGING:
       Plugin::get()->runBlockCallback(block, "onStartedDigging", inv);
     break;
     case BLOCK_STATUS_DIGGING:
       Plugin::get()->runBlockCallback(block, "onDigging", inv);
     break;
     case BLOCK_STATUS_STOPPED_DIGGING:
       Plugin::get()->runBlockCallback(block, "onStoppedDigging", inv);
     break;
     case BLOCK_STATUS_BLOCK_BROKEN:
       Plugin::get()->runBlockCallback(block, "onBroken", inv);

       /* notify neighbour blocks of the broken block */
       status = block;
       if (Map::get()->getBlock(x+1, y, z, &block, &meta) && block != BLOCK_AIR)
       {
          inv = Function::invoker_type(user, status, x+1, y, z, BLOCK_SOUTH);
          Plugin::get()->runBlockCallback(block, "onNeighbourBroken", inv);
       }

       if (Map::get()->getBlock(x-1, y, z, &block, &meta) && block != BLOCK_AIR)
       {
          inv = Function::invoker_type(user, status, x-1, y, z, BLOCK_NORTH);
          Plugin::get()->runBlockCallback(block, "onNeighbourBroken", inv);
       }

       if (Map::get()->getBlock(x, y+1, z, &block, &meta) && block != BLOCK_AIR)
       {
          inv = Function::invoker_type(user, status, x, y+1, z, BLOCK_TOP);
          Plugin::get()->runBlockCallback(block, "onNeighbourBroken", inv);
       }

       if (Map::get()->getBlock(x, y-1, z, &block, &meta) && block != BLOCK_AIR)
       {
          inv = Function::invoker_type(user, status, x, y-1, z, BLOCK_BOTTOM);
          Plugin::get()->runBlockCallback(block, "onNeighbourBroken", inv);
       }

       if (Map::get()->getBlock(x, y, z+1, &block, &meta) && block != BLOCK_AIR)
       {
          inv = Function::invoker_type(user, status, x, y, z+1, BLOCK_WEST);
          Plugin::get()->runBlockCallback(block, "onNeighbourBroken", inv);
       }

       if (Map::get()->getBlock(x, y, z-1, &block, &meta) && block != BLOCK_AIR)
       {
          inv = Function::invoker_type(user, status, x, y, z-1, BLOCK_EAST);
          Plugin::get()->runBlockCallback(block, "onNeighbourBroken", inv);
       }
     break;
  }
  return PACKET_OK;
}

int PacketHandler::player_block_placement(User *user)
{
  sint8 y, direction;
  sint16 newblock;
  sint32 x, z;
  /* replacement of block */
  uint8 oldblock;
  uint8 metadata;
  /* neighbour blocks */
  uint8 block;
  uint8 meta;

  user->buffer >> newblock >> x >> y >> z >> direction;

  if(!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->buffer.removePacket();

  // TODO: Handle processing of
  if(direction == -1)
  {
    return PACKET_OK;
  }

  //Minecart testing!!
  if(newblock == ITEM_MINECART && Map::get()->getBlock(x, y, z, &oldblock, &metadata))
  {
    if(oldblock != BLOCK_MINECART_TRACKS) return PACKET_OK;
    Screen::get()->log("Spawn minecart");
    sint32 EID=generateEID();
    Packet pkt;
    //                                              MINECART
    pkt << PACKET_ADD_OBJECT << (sint32)EID <<  (sint8)10       << (sint32)(x*32+16) << (sint32)(y*32) << (sint32)(z*32+16);
    user->sendAll((uint8 *)pkt.getWrite(), pkt.getWriteLen());
  }

  if (newblock == -1 && newblock != ITEM_SIGN)
  {
     Screen::get()->log("ignoring:" + newblock);
     return PACKET_OK;
  }

  if(y < 0)
  {
    return PACKET_OK;
  }

  #ifdef _DEBUG
    Screen::get()->log("Block_placement: " + newblock + " (" + dtos(x) + "," + dtos((int)y) + "," + dtos(z) + ") dir: " + dtos((int)direction);
  #endif

  if (direction)
  {
    direction = 6-direction;
  }

  Callback callback;
  Function event;
  Function::invoker_type inv(user, newblock, x, y, z, direction);

  if (Map::get()->getBlock(x, y, z, &oldblock, &metadata))
  {
     uint8 oldblocktop;
     uint8 metadatatop;
     sint8 check_y = y;
     sint32 check_x = x;
     sint32 check_z = z;

     /* client doesn't give us the correct block for lava
        and water, check block above */
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

     if (Map::get()->getBlock(check_x, check_y, check_z, &oldblocktop, &metadatatop)
         && (oldblocktop == BLOCK_LAVA || oldblocktop == BLOCK_STATIONARY_LAVA
         || oldblocktop == BLOCK_WATER || oldblocktop == BLOCK_STATIONARY_WATER))
     {
       /* block above needs replacing rather then the block send by the client */
       inv = Function::invoker_type(user, newblock, check_x, check_y, check_z, direction);
       Plugin::get()->runBlockCallback(oldblocktop, "onReplace", inv);
     }
     else
     {
       inv = Function::invoker_type(user, newblock, x, y, z, direction);
       Plugin::get()->runBlockCallback(oldblock, "onReplace", inv);
     }

     /* We pass the newblock to the newblock's onPlace callback because
     the callback doesn't know what type of block we're placing. Instead
     the callback's job is to describe the behaviour when placing the
     block down, not to place any specifically block itself. */
     inv = Function::invoker_type(user, newblock, x, y, z, direction);
     Plugin::get()->runBlockCallback(newblock, "onPlace", inv);

     /* notify neighbour blocks of the placed block */
     if (Map::get()->getBlock(x+1, y, z, &block, &meta) && block != BLOCK_AIR)
     {
        inv = Function::invoker_type(user, newblock, x+1, y, z, BLOCK_SOUTH);
        Plugin::get()->runBlockCallback(block, "onNeighbourPlace", inv);
     }

     if (Map::get()->getBlock(x-1, y, z, &block, &meta) && block != BLOCK_AIR)
     {
        inv = Function::invoker_type(user, newblock, x-1, y, z, BLOCK_NORTH);
        Plugin::get()->runBlockCallback(block, "onNeighbourPlace", inv);
     }

     if (Map::get()->getBlock(x, y+1, z, &block, &meta) && block != BLOCK_AIR)
     {
        inv = Function::invoker_type(user, newblock, x, y+1, z, BLOCK_TOP);
        Plugin::get()->runBlockCallback(block, "onNeighbourPlace", inv);
     }

     if (Map::get()->getBlock(x, y-1, z, &block, &meta) && block != BLOCK_AIR)
     {
        inv = Function::invoker_type(user, newblock, x, y-1, z, BLOCK_BOTTOM);
        Plugin::get()->runBlockCallback(block, "onNeighbourPlace", inv);
     }

     if (Map::get()->getBlock(x, y, z+1, &block, &meta) && block != BLOCK_AIR)
     {
        inv = Function::invoker_type(user, newblock, x, y, z+1, BLOCK_WEST);
        Plugin::get()->runBlockCallback(block, "onNeighbourPlace", inv);
     }

     if (Map::get()->getBlock(x, y, z-1, &block, &meta) && block != BLOCK_AIR)
     {
        inv = Function::invoker_type(user, newblock, x, y, z-1, BLOCK_EAST);
        Plugin::get()->runBlockCallback(block, "onNeighbourPlace", inv);
     }
  }
  /* TODO: Should be removed from here. Only needed for liquid related blocks? */
  Physics::get()->checkSurrounding(vec(x, y, z));
  return PACKET_OK;
}

int PacketHandler::holding_change(User *user)
{
  sint32 entityID;
  sint16 itemID;
  user->buffer >> entityID >> itemID;

  if(!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->buffer.removePacket();

  user->curItem = itemID;

  //Send holding change to others
  Packet pkt;
  pkt << (sint8)PACKET_HOLDING_CHANGE << (sint32)user->UID << itemID;
  user->sendOthers((uint8*)pkt.getWrite(), pkt.getWriteLen());

  // Set current itemID to user
  user->setCurrentItem(itemID);

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

  // Modify the position of the dropped item so that it appears in front of user instead of under user
  int distanceToThrow = 64;
  float angle = DEGREES_TO_RADIANS(user->pos.yaw + 45);     // For some reason, yaw seems to be off to the left by 45 degrees from where you're actually looking?
  int x = int(cos(angle) * distanceToThrow - sin(angle) * distanceToThrow);
  int z = int(sin(angle) * distanceToThrow + cos(angle) * distanceToThrow);
  item.pos += vec(x, 0, z);

  Map::get()->sendPickupSpawn(item);

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

  Screen::get()->log("Disconnect: " + msg);

  event_del(user->GetEvent());

  #ifdef WIN32
  closesocket(user->fd);
  #else
  close(user->fd);
  #endif

  delete user;

  return PACKET_OK;
}

int PacketHandler::complex_entities(User *user)
{
  if(!user->buffer.haveData(12))
  {
    return PACKET_NEED_MORE_DATA;
  }

  sint32 x,z;
  sint16 len,y;


  user->buffer >> x >> y >> z >> len;

  if(!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  if(!user->buffer.haveData(len))
  {
    return PACKET_NEED_MORE_DATA;
  }

  uint8 *buffer = new uint8[len];

  user->buffer.getData(buffer, len);

  user->buffer.removePacket();

  uint8 block, meta;
  Map::get()->getBlock(x, y, z, &block, &meta);

  //We only handle chest for now
  if(block != BLOCK_CHEST && block != BLOCK_FURNACE && block != BLOCK_BURNING_FURNACE &&
     block != BLOCK_SIGN_POST && block != BLOCK_WALL_SIGN)
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
  if(inflate(&zstream, Z_FULL_FLUSH) != Z_STREAM_END)
  {
    Screen::get()->log(LOG_ERROR, "Error in inflate!");
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
  Screen::get()->log("Complex entity at (" + dtos(x) + "," + dtos(y) + "," + dtos(z) + ")");
  entity->Print();
#endif

  // Check if this is a Furnace and handle if so
  if(block == BLOCK_FURNACE || block == BLOCK_BURNING_FURNACE)
  {
    FurnaceManager::get()->handleActivity(entity, block);
  }
  else
  {
    Map::get()->setComplexEntity(user, x, y, z, entity);
  }

  delete [] buffer;

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
  user->teleport(Map::get()->spawnPos.x(), Map::get()->spawnPos.y() + 2, Map::get()->spawnPos.z());
  user->buffer.removePacket();
  return PACKET_OK;
}
