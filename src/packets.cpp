/*
   Copyright (c) 2012, The Mineserver Project
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

#include <sys/types.h>
#ifdef WIN32
#define NOMINMAX
#include <winsock2.h>
#else
#include <netdb.h>       // for gethostbyname()
#include <netinet/tcp.h> // for TCP constants
#endif

#include <cmath>
#include <sstream>

#include "chat.h"
#include "config.h"
#include "constants.h"
#include "furnaceManager.h"
#include "inventory.h"
#include "logger.h"
#include "map.h"
#include "mineserver.h"
#include "nbt.h"
#include "packets.h"
#include "physics.h"
#include "plugin.h"
#include "sockets.h"
#include "tools.h"
#include "user.h"
#include "blocks/basic.h"
#include "blocks/default.h"
#include "blocks/note.h"
#include "items/itembasic.h"
#include "mob.h"
#include "utf8.h"

void PacketHandler::init()
{
  packets[PACKET_KEEP_ALIVE]               = Packets(0, &PacketHandler::keep_alive);
  packets[PACKET_LOGIN_REQUEST]            = Packets(PACKET_VARIABLE_LEN, &PacketHandler::login_request);
  packets[PACKET_HANDSHAKE]                = Packets(PACKET_VARIABLE_LEN, &PacketHandler::handshake);
  packets[PACKET_CHAT_MESSAGE]             = Packets(PACKET_VARIABLE_LEN, &PacketHandler::chat_message);
  packets[PACKET_USE_ENTITY]               = Packets(9, &PacketHandler::use_entity);
  packets[PACKET_PLAYER]                   = Packets(1, &PacketHandler::player);
  packets[PACKET_PLAYER_POSITION]          = Packets(33, &PacketHandler::player_position);
  packets[PACKET_PLAYER_LOOK]              = Packets(9, &PacketHandler::player_look);
  packets[PACKET_PLAYER_POSITION_AND_LOOK] = Packets(41, &PacketHandler::player_position_and_look);
  packets[PACKET_PLAYER_DIGGING]           = Packets(11, &PacketHandler::player_digging);
  packets[PACKET_PLAYER_BLOCK_PLACEMENT]   = Packets(PACKET_VARIABLE_LEN, &PacketHandler::player_block_placement);
  packets[PACKET_HOLDING_CHANGE]           = Packets(2, &PacketHandler::holding_change);
  packets[PACKET_ARM_ANIMATION]            = Packets(5, &PacketHandler::arm_animation);
  packets[PACKET_PICKUP_SPAWN]             = Packets(22, &PacketHandler::pickup_spawn);
  packets[PACKET_DISCONNECT]               = Packets(PACKET_VARIABLE_LEN, &PacketHandler::disconnect);
  packets[PACKET_RESPAWN]                  = Packets(PACKET_VARIABLE_LEN, &PacketHandler::respawn);
  packets[PACKET_INVENTORY_CHANGE]         = Packets(PACKET_VARIABLE_LEN, &PacketHandler::inventory_change);
  packets[PACKET_INVENTORY_CLOSE]          = Packets(1, &PacketHandler::inventory_close);
  packets[PACKET_SIGN]                     = Packets(PACKET_VARIABLE_LEN, &PacketHandler::change_sign);
  packets[PACKET_TRANSACTION]              = Packets(4, &PacketHandler::inventory_transaction);
  packets[PACKET_ENTITY_CROUCH]            = Packets(5, &PacketHandler::entity_crouch);
  packets[PACKET_WEATHER]                  = Packets(18, &PacketHandler::unhandledPacket);
  packets[PACKET_INCREMENT_STATISTICS]     = Packets(6, &PacketHandler::unhandledPacket);
  packets[PACKET_PING]                     = Packets(0, &PacketHandler::ping);
  packets[PACKET_BLOCK_CHANGE]             = Packets(11, &PacketHandler::block_change);
  
  
}

int PacketHandler::unhandledPacket(User* user)
{
  user->buffer.removePacket();
  return PACKET_OK;
}

int PacketHandler::entity_crouch(User* user)
{
  int32_t EID;
  int8_t action;

  user->buffer >> EID >> action;

  //ToDo: inform other players
  //LOG2(INFO, "Entity action: EID: " + dtos(EID) +" Action: " +dtos(action));

  user->buffer.removePacket();
  return PACKET_OK;
}

int PacketHandler::change_sign(User* user)
{
  if (!user->buffer.haveData(16))
  {
    return PACKET_NEED_MORE_DATA;
  }
  int32_t x, z;
  int16_t y;
  std::string strings1, strings2, strings3, strings4;

  user->buffer >> x >> y >> z;

  if (!user->buffer.haveData(8))
  {
    return PACKET_NEED_MORE_DATA;
  }
  user->buffer >> strings1;
  if (!user->buffer.haveData(6))
  {
    return PACKET_NEED_MORE_DATA;
  }
  user->buffer >> strings2;
  if (!user->buffer.haveData(4))
  {
    return PACKET_NEED_MORE_DATA;
  }
  user->buffer >> strings3;
  if (!user->buffer.haveData(2))
  {
    return PACKET_NEED_MORE_DATA;
  }
  user->buffer >> strings4;

  //ToDo: Save signs!
  signDataPtr newSign(new signData);
  newSign->x = x;
  newSign->y = y;
  newSign->z = z;
  newSign->text1 = strings1;
  newSign->text2 = strings2;
  newSign->text3 = strings3;
  newSign->text4 = strings4;

  sChunk* chunk = ServerInstance->map(user->pos.map)->getChunk(blockToChunk(x), blockToChunk(z));

  if (chunk != NULL)
  {
    //Check if this sign data already exists and remove
    chunk->signs.erase(std::remove_if(chunk->signs.begin(), chunk->signs.end(), DataFinder<signData>(x,y,z)), chunk->signs.end());

    // Insert new sign
    chunk->signs.push_back(newSign);

    //Send sign packet to everyone
    Packet pkt;
    pkt << (int8_t)PACKET_SIGN << x << y << z;
    pkt << strings1 << strings2 << strings3 << strings4;
    user->sendAll(pkt);
  }

  LOG2(INFO, "Sign: " + strings1 + strings2 + strings3 + strings4);

  //No need to do anything
  user->buffer.removePacket();
  return PACKET_OK;
}


int PacketHandler::inventory_close(User* user)
{
  int8_t windowID;

  user->buffer >> windowID;

  ServerInstance->inventory()->windowClose(user, windowID);

  user->buffer.removePacket();
  return PACKET_OK;
}


int PacketHandler::inventory_transaction(User* user)
{
  int8_t windowID;
  int16_t action;
  int8_t accepted;

  user->buffer >> windowID >> action >> accepted;

  //No need to do anything
  user->buffer.removePacket();
  return PACKET_OK;
}

int PacketHandler::inventory_change(User* user)
{
  if (!user->buffer.haveData(10))
  {
    return PACKET_NEED_MORE_DATA;
  }
  int8_t windowID = 0;
  int16_t slot = 0;
  int8_t rightClick = 0;
  int16_t actionNumber = 0;
  int8_t shift = 0;
  int16_t itemID = 0;
  int8_t itemCount = 0;
  int16_t itemUses  = 0;

  user->buffer >> windowID >> slot >> rightClick >> actionNumber >> shift >> itemID;
  if (itemID != -1)
  {
    if (!user->buffer.haveData(2))
    {
      return PACKET_NEED_MORE_DATA;
    }
    user->buffer >> itemCount >> itemUses;
    if(Item::isEnchantable(itemID)) {
      int16_t enchantment_data_len;
      user->buffer >> enchantment_data_len;
      if(enchantment_data_len >= 0) {
        LOG2(INFO, "Got enchantment data, ignoring...");
      }
    }
  }

  ServerInstance->inventory()->windowClick(user, windowID, slot, rightClick, actionNumber, itemID, itemCount, itemUses, shift);

  //No need to do anything
  user->buffer.removePacket();
  return PACKET_OK;
}

// Keep Alive (http://mc.kev009.com/wiki/Protocol#Keep_Alive_.280x00.29)
int PacketHandler::keep_alive(User* user)
{
  /*
  if (!user->buffer.haveData(4))
  {
    return PACKET_NEED_MORE_DATA;
  }
  int32_t pingTime;
  user->buffer >> pingTime;
  */
  //No need to do anything
  user->buffer.removePacket();
  return PACKET_OK;
}

//Source: http://wiki.linuxquestions.org/wiki/Connecting_a_socket_in_C
int socket_connect(char* host, int port)
{
  struct hostent* hp;
  struct sockaddr_in addr;
  int on = 1, sock;

  if ((hp = gethostbyname(host)) == NULL)
  {
    return 0;
  }

  memmove(&addr.sin_addr, hp->h_addr, hp->h_length);
  addr.sin_port = htons(port);
  addr.sin_family = AF_INET;
  sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  struct timeval tv;
  tv.tv_sec = 2;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval));
  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(struct timeval));

  setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(int));

  if (sock == -1)
  {
    return 0;
  }

  if (connect(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1)
  {
    return 0;
  }

  return sock;
}

// Login request (http://mc.kev009.com/wiki/Protocol#Login_Request_.280x01.29)
int PacketHandler::login_request(User* user)
{
  //Check that we have enough data in the buffer
  if (!user->buffer.haveData(18))
    {
      return PACKET_NEED_MORE_DATA;
    }

  int32_t version;
  std::string player, passwd;
  //int64_t mapseed;
  uint32_t param_1 = 0, param_2 = 0;
  int8_t dimension = 0, param_3 = 0, param_4 = 0;

  // As of version 1.5, the password is no longer sent (at least for non-authenticated mode)
  //user->buffer >> version >> player >> passwd >> mapseed >> dimension;
  user->buffer >> version >> player >> passwd /*>> mapseed*/ >> param_1 >> param_2 >> dimension >> param_3 >> param_4;

  if (!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->buffer.removePacket();

  while (User::byNick(player) != NULL)
  {
    player.append("_");
  }

  LOG(INFO, "Packets", "Player " + dtos(user->UID) + " login v." + dtos(version) + " : " + player + ":" + passwd);

  user->nick = player;

  // If version is not the current version
  if (version != PROTOCOL_VERSION)
  {
    user->kick(ServerInstance->config()->sData("strings.wrong_protocol"));
    return PACKET_OK;
  }

  // If userlimit is reached
  if ((int)User::all().size() > ServerInstance->config()->iData("system.user_limit"))
  {
    user->kick(ServerInstance->config()->sData("strings.server_full"));
    return PACKET_OK;
  }



  // Check if we're to do user validation
  if (ServerInstance->config()->bData("system.user_validation") == true)
  {
    std::string url = "/game/checkserver.jsp?user=" + player + "&serverId=" + hash(player);
    LOG(INFO, "Packets", "Validating " + player + " against minecraft.net: ");

    std::string http_request = "GET " + url + " HTTP/1.1\r\n"
                               + "Host: www.minecraft.net\r\n"
                               + "Connection: close\r\n\r\n";

    int fd = socket_connect((char*)"50.16.200.224", 80);
    if (fd)
    {
#ifdef WIN32
      send(fd, http_request.c_str(), http_request.length(), NULL);
#else
      write(fd, http_request.c_str(), http_request.length());
#endif

#define BUFFER_SIZE 1024
      char* buffer = new char[BUFFER_SIZE];
      std::string stringbuffer;

#ifdef WIN32
      while (int received = recv(fd, buffer, BUFFER_SIZE - 1, NULL) != 0)
      {
#else
      while (read(fd, buffer, BUFFER_SIZE - 1) != 0)
      {
#endif
        stringbuffer += std::string(buffer);
      }
      delete[] buffer;
#ifdef WIN32
      closesocket(fd);
#else
      close(fd);
#endif

      bool allow_access = false;
      //No response data, timeout
      if (stringbuffer.size() == 0 && ServerInstance->config()->bData("system.allow_connect_on_auth_timeout"))
      {
        LOG(INFO, "Packets", "  Auth skipped on timeout ");
        allow_access = true;
      }

      if (allow_access || (stringbuffer.size() >= 3 && stringbuffer.find("\r\n\r\nYES", 0) != std::string::npos))
      {
        LOG(INFO, "Packets", "  Verified!");

        char* kickMessage = NULL;
        if ((static_cast<Hook2<bool, const char*, char**>*>(ServerInstance->plugin()->getHook("PlayerLoginPre")))->doUntilFalse(player.c_str(), &kickMessage))
        {
          user->kick(std::string(kickMessage));
        }
        else
        {
          user->sendLoginInfo();
          (static_cast<Hook1<bool, const char*>*>(ServerInstance->plugin()->getHook("PlayerLoginPost")))->doAll(player.c_str());
        }
      }
      else
      {
        LOG(INFO, "Packets", "  Failed"  + stringbuffer.substr(stringbuffer.size() - 3));
        user->kick("Failed to verify username!");
      }
    }
    else
    {
      LOG(INFO, "Packets", "  Failed");
      user->kick("Failed to verify username!");
    }

    return PACKET_OK;
  }


  char* kickMessage = NULL;
  if ((static_cast<Hook2<bool, const char*, char**>*>(ServerInstance->plugin()->getHook("PlayerLoginPre")))->doUntilFalse(player.c_str(), &kickMessage))
  {
    user->kick(std::string(kickMessage));
  }
  else
  {
    user->sendLoginInfo();
    (static_cast<Hook1<bool, const char*>*>(ServerInstance->plugin()->getHook("PlayerLoginPost")))->doAll(player.c_str());
  }

  return PACKET_OK;
}

int PacketHandler::handshake(User* user)
{
  if (!user->buffer.haveData(3))
  {
    return PACKET_NEED_MORE_DATA;
  }

  std::string player;

  user->buffer >> player;

  // Check for data
  if (!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  // Remove package from buffer
  user->buffer.removePacket();

  // Check whether we're to validate against minecraft.net
  if (ServerInstance->config()->bData("system.user_validation") == true)
  {
    // Send the unique hash for this player to prompt the client to go to minecraft.net to validate
    LOG(INFO, "Packets", "Handshake: Giving player " + player + " their minecraft.net hash of: " + hash(player));
    user->buffer << (int8_t)PACKET_HANDSHAKE << hash(player);
  }
  else
  {
    // Send "no validation or password needed" validation
    LOG(INFO, "Packets", "Handshake: No validation required for player " + player + ".");
    user->buffer << (int8_t)PACKET_HANDSHAKE << std::string("-");
  }
  // TODO: Add support for prompting user for Server password (once client supports it)

  return PACKET_OK;
}

int PacketHandler::chat_message(User* user)
{
  // Wait for length-short. HEHE
  if (!user->buffer.haveData(2))
  {
    return PACKET_NEED_MORE_DATA;
  }

  std::string msg;

  user->buffer >> msg;

  if (!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->buffer.removePacket();

  ServerInstance->chat()->handleMsg(user, msg);

  return PACKET_OK;
}

int PacketHandler::player(User* user)
{
  //OnGround packet
  int8_t onground;
  user->buffer >> onground;
  if (!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }
  user->buffer.removePacket();
  return PACKET_OK;
}

int PacketHandler::player_position(User* user)
{
  double x, y, stance, z;
  int8_t onground;

  user->buffer >> x >> y >> stance >> z >> onground;

  if (!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->updatePos(x, y, z, stance);
  user->buffer.removePacket();

  return PACKET_OK;
}

int PacketHandler::player_look(User* user)
{
  float yaw, pitch;
  int8_t onground;

  user->buffer >> yaw >> pitch >> onground;

  if (!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->updateLook(yaw, pitch);

  user->buffer.removePacket();

  return PACKET_OK;
}

int PacketHandler::player_position_and_look(User* user)
{
  double x, y, stance, z;
  float yaw, pitch;
  int8_t onground;

  user->buffer >> x >> y >> stance >> z
               >> yaw >> pitch >> onground;

  if (!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  //Update user data
  user->updatePos(x, y, z, stance);
  user->updateLook(yaw, pitch);

  user->buffer.removePacket();

  return PACKET_OK;
}

int PacketHandler::player_digging(User* user)
{
  int8_t status;
  int32_t x;
  int8_t  y;
  int32_t z;
  int8_t direction;
  uint8_t block;
  uint8_t meta;
  BlockBasicPtr blockcb;
  BlockDefault blockD;


  user->buffer >> status >> x >> y >> z >> direction;

  if (!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->buffer.removePacket();

  if (!ServerInstance->map(user->pos.map)->getBlock(x, y, z, &block, &meta))
  {
    blockD.revertBlock(user, x, y, z, user->pos.map);
    return PACKET_OK;
  }

  // Blocks that break with first hit
  if (status == BLOCK_STATUS_STARTED_DIGGING &&
      (block == BLOCK_SNOW || block == BLOCK_REED || block == BLOCK_TORCH
    || block == BLOCK_REDSTONE_WIRE || block == BLOCK_RED_ROSE || block == BLOCK_YELLOW_FLOWER 
    || block == BLOCK_BROWN_MUSHROOM || block == BLOCK_RED_MUSHROOM 
    || block == BLOCK_REDSTONE_TORCH_OFF || block == BLOCK_REDSTONE_TORCH_ON))
  {
    status = BLOCK_STATUS_BLOCK_BROKEN;
  }

  switch (status)
  {
  case BLOCK_STATUS_STARTED_DIGGING:
  {
    (static_cast<Hook5<bool, const char*, int32_t, int8_t, int32_t, int8_t>*>(ServerInstance->plugin()->getHook("PlayerDiggingStarted")))->doAll(user->nick.c_str(), x, y, z, direction);

    for (uint32_t i = 0 ; i < ServerInstance->plugin()->getBlockCB().size(); i++)
    {
      blockcb = ServerInstance->plugin()->getBlockCB()[i];
      if (blockcb != NULL && blockcb->affectedBlock(block))
      {
        blockcb->onStartedDigging(user, status, x, y, z, user->pos.map, direction);
      }
    }
    break;
  }
  /*
  case BLOCK_STATUS_DIGGING:
  {
    (static_cast<Hook5<bool, const char*, int32_t, int8_t, int32_t, int8_t>*>(ServerInstance->plugin()->getHook("PlayerDigging")))->doAll(user->nick.c_str(), x, y, z, direction);
    (static_cast<Hook4<bool, const char*, int32_t, int8_t, int32_t>*>(ServerInstance->plugin()->getHook("PlayerDigging")))->doAll(user->nick.c_str(), x, y, z);
    for (uint32_t i = 0 ; i < ServerInstance->plugin()->getBlockCB().size(); i++)
    {
      blockcb = ServerInstance->plugin()->getBlockCB()[i];
      if (blockcb != NULL && blockcb->affectedBlock(block))
      {
        blockcb->onDigging(user, status, x, y, z, user->pos.map, direction);
      }
    }

    break;
  }
  */
  /*    case BLOCK_STATUS_STOPPED_DIGGING:
      {
        (static_cast<Hook5<bool,const char*,int32_t,int8_t,int32_t,int8_t>*>(ServerInstance->plugin()->getHook("PlayerDiggingStopped")))->doAll(user->nick.c_str(), x, y, z, direction);
        for(uint32_t i =0 ; i<ServerInstance->plugin()->getBlockCB().size(); i++)
        {
          blockcb = ServerInstance->plugin()->getBlockCB()[i];
          if(blockcb!=NULL && blockcb->affectedBlock(block))
          {
            blockcb->onStoppedDigging(user,status, x,y,z,user->pos.map,direction);
          }
        }
        break;
      }*/
  case BLOCK_STATUS_BLOCK_BROKEN:
  {
    //Player tool usage calculation etc

    bool rightUse;
    int16_t itemSlot = user->currentItemSlot() + 36;
    int16_t itemHealth = ServerInstance->inventory()->itemHealth(user->inv[itemSlot].getType(), block, rightUse);
    if (itemHealth > 0)
    {
      user->inv[itemSlot].incHealth();
      if (!rightUse)
      {
        user->inv[itemSlot].incHealth();
      }

      if (itemHealth <= user->inv[itemSlot].getHealth())
      {
        user->inv[itemSlot].decCount();

        if (user->inv[itemSlot].getCount() == 0)
        {
          user->inv[itemSlot].setHealth(0);
          user->inv[itemSlot].setType(-1);
        }
      }
      ServerInstance->inventory()->setSlot(user, WINDOW_PLAYER, itemSlot, user->inv[itemSlot].getType(),
                                              user->inv[itemSlot].getCount(), user->inv[itemSlot].getHealth());
    }

    if ((static_cast<Hook4<bool, const char*, int32_t, int8_t, int32_t>*>(ServerInstance->plugin()->getHook("BlockBreakPre")))->doUntilFalse(user->nick.c_str(), x, y, z))
    {
      blockD.revertBlock(user, x, y, z, user->pos.map);
      return PACKET_OK;
    }

    (static_cast<Hook4<bool, const char*, int32_t, int8_t, int32_t>*>(ServerInstance->plugin()->getHook("BlockBreakPost")))->doAll(user->nick.c_str(), x, y, z);

    for (uint32_t i = 0 ; i < ServerInstance->plugin()->getBlockCB().size(); i++)
    {
      blockcb = ServerInstance->plugin()->getBlockCB()[i];
      if (blockcb != NULL && blockcb->affectedBlock(block))
      {
        if (blockcb->onBroken(user, status, x, y, z, user->pos.map, direction))
        {
          // Do not break
          return PACKET_OK;
        }
        else
        {
          break;
        }
      }
    }

    /* notify neighbour blocks of the broken block */
    status = block;
    if (ServerInstance->map(user->pos.map)->getBlock(x + 1, y, z, &block, &meta) && block != BLOCK_AIR)
    {
      (static_cast<Hook7<bool, const char*, int32_t, int8_t, int32_t, int32_t, int8_t, int32_t>*>(ServerInstance->plugin()->getHook("BlockNeighbourBreak")))->doAll(user->nick.c_str(), x + 1, y, z, x, y, z);
      for (uint32_t i = 0 ; i < ServerInstance->plugin()->getBlockCB().size(); i++)
      {
        blockcb = ServerInstance->plugin()->getBlockCB()[i];
        if (blockcb != NULL && (blockcb->affectedBlock(status) || blockcb->affectedBlock(block)))
        {
          blockcb->onNeighbourBroken(user, status, x + 1, y, z, user->pos.map, BLOCK_SOUTH);
        }
      }

    }

    if (ServerInstance->map(user->pos.map)->getBlock(x - 1, y, z, &block, &meta) && block != BLOCK_AIR)
    {
      (static_cast<Hook7<bool, const char*, int32_t, int8_t, int32_t, int32_t, int8_t, int32_t>*>(ServerInstance->plugin()->getHook("BlockNeighbourBreak")))->doAll(user->nick.c_str(), x - 1, y, z, x, y, z);
      for (uint32_t i = 0 ; i < ServerInstance->plugin()->getBlockCB().size(); i++)
      {
        blockcb = ServerInstance->plugin()->getBlockCB()[i];
        if (blockcb != NULL && (blockcb->affectedBlock(status) || blockcb->affectedBlock(block)))
        {
          blockcb->onNeighbourBroken(user, status, x - 1, y, z, user->pos.map, BLOCK_NORTH);
        }
      }

    }

    if (ServerInstance->map(user->pos.map)->getBlock(x, y + 1, z, &block, &meta) && block != BLOCK_AIR)
    {
      (static_cast<Hook7<bool, const char*, int32_t, int8_t, int32_t, int32_t, int8_t, int32_t>*>(ServerInstance->plugin()->getHook("BlockNeighbourBreak")))->doAll(user->nick.c_str(), x, y + 1, z, x, y, z);
      for (uint32_t i = 0 ; i < ServerInstance->plugin()->getBlockCB().size(); i++)
      {
        blockcb = ServerInstance->plugin()->getBlockCB()[i];
        if (blockcb != NULL && (blockcb->affectedBlock(status) || blockcb->affectedBlock(block)))
        {
          blockcb->onNeighbourBroken(user, status, x, y + 1, z, user->pos.map, BLOCK_TOP);
        }
      }

    }

    if (ServerInstance->map(user->pos.map)->getBlock(x, y - 1, z, &block, &meta) && block != BLOCK_AIR)
    {
      (static_cast<Hook7<bool, const char*, int32_t, int8_t, int32_t, int32_t, int8_t, int32_t>*>(ServerInstance->plugin()->getHook("BlockNeighbourBreak")))->doAll(user->nick.c_str(), x, y - 1, z, x, y, z);
      for (uint32_t i = 0 ; i < ServerInstance->plugin()->getBlockCB().size(); i++)
      {
        blockcb = ServerInstance->plugin()->getBlockCB()[i];
        if (blockcb != NULL && (blockcb->affectedBlock(status) || blockcb->affectedBlock(block)))
        {
          blockcb->onNeighbourBroken(user, status, x, y - 1, z, user->pos.map, BLOCK_BOTTOM);
        }
      }

    }

    if (ServerInstance->map(user->pos.map)->getBlock(x, y, z + 1, &block, &meta) && block != BLOCK_AIR)
    {
      (static_cast<Hook7<bool, const char*, int32_t, int8_t, int32_t, int32_t, int8_t, int32_t>*>(ServerInstance->plugin()->getHook("BlockNeighbourBreak")))->doAll(user->nick.c_str(), x, y, z + 1, x, y, z);
      for (uint32_t i = 0 ; i < ServerInstance->plugin()->getBlockCB().size(); i++)
      {
        blockcb = ServerInstance->plugin()->getBlockCB()[i];
        if (blockcb != NULL && (blockcb->affectedBlock(status) || blockcb->affectedBlock(block)))
        {
          blockcb->onNeighbourBroken(user, status, x, y, z + 1, user->pos.map, BLOCK_WEST);
        }
      }

    }

    if (ServerInstance->map(user->pos.map)->getBlock(x, y, z - 1, &block, &meta) && block != BLOCK_AIR)
    {
      (static_cast<Hook7<bool, const char*, int32_t, int8_t, int32_t, int32_t, int8_t, int32_t>*>(ServerInstance->plugin()->getHook("BlockNeighbourBreak")))->doAll(user->nick.c_str(), x, y, z - 1, x, y, z);
      for (uint32_t i = 0 ; i < ServerInstance->plugin()->getBlockCB().size(); i++)
      {
        blockcb = ServerInstance->plugin()->getBlockCB()[i];
        if (blockcb != NULL && (blockcb->affectedBlock(status) || blockcb->affectedBlock(block)))
        {
          blockcb->onNeighbourBroken(user, status, x, y, z - 1, user->pos.map, BLOCK_EAST);
        }
      }

    }

    break;
  }
  case BLOCK_STATUS_PICKUP_SPAWN:
  {
    //ToDo: handle
#define itemSlot (36+user->currentItemSlot())
    if (user->inv[itemSlot].getType() > 0)
    {
      ServerInstance->map(user->pos.map)->createPickupSpawn(user->pos.x, user->pos.y, user->pos.z, user->inv[itemSlot].getType(), 1, user->inv[itemSlot].getHealth(), user);

      user->inv[itemSlot].decCount();
    }
    break;
#undef itemSlot
  }

  }

  return PACKET_OK;
}



int PacketHandler::player_block_placement(User* user)
{
  if (!user->buffer.haveData(12))
  {
    return PACKET_NEED_MORE_DATA;
  }
  int8_t y = 0, direction = 0;
  int16_t newblock = 0;
  int32_t x, z = 0;
  /* replacement of block */
  uint8_t oldblock = 0;
  uint8_t metadata = 0;
  /* neighbour blocks */
  uint8_t block = 0;
  uint8_t meta  = 0;
  int8_t count  = 0;
  int16_t health = 0;
  BlockBasicPtr blockcb;
  BlockDefault blockD;


  user->buffer >> x >> y >> z >> direction >> newblock;

  if (newblock >= 0)
  {
    if (!user->buffer.haveData(2))
    {
      return PACKET_NEED_MORE_DATA;
    }
    user->buffer >> count >> health;
    if(Item::isEnchantable(newblock)) {
      int16_t enchantment_data_len;
      user->buffer >> enchantment_data_len;
      if(enchantment_data_len >= 0) {
        LOG2(INFO, "Got enchantment data, ignoring...");
      }
    }
  }
  user->buffer.removePacket();


  ItemBasicPtr itemcb;
  if (x == -1 && y == -1 && z == -1)
  {
    // Right clicked without pointing at a tile
    Item* item = &(user->inv[user->curItem + 36]);
    if ((static_cast<Hook6<bool, const char*, int32_t, int8_t, int32_t, int16_t, int8_t>*>(ServerInstance->plugin()->getHook("ItemRightClickPre")))->doUntilFalse(user->nick.c_str(), x, y, z, item->getType(), direction))
    {
      return PACKET_OK;
    }


    for (uint32_t i = 0 ; i < ServerInstance->plugin()->getItemCB().size(); i++)
    {
      itemcb = ServerInstance->plugin()->getItemCB()[i];
      if (itemcb != NULL && itemcb->affectedItem(newblock))
      {
        itemcb->onRightClick(user, item);
      }
    }
    return PACKET_OK;
  }

  if (!ServerInstance->map(user->pos.map)->getBlock(x, y, z, &oldblock, &metadata))
  {
    blockD.revertBlock(user, x, y, z, user->pos.map);
    return PACKET_OK;
  }

  /* Protocol docs say this should be what interacting is. */
  if (oldblock != BLOCK_AIR)
  {
    (static_cast<Hook4<bool, const char*, int32_t, int8_t, int32_t>*>(ServerInstance->plugin()->getHook("PlayerBlockInteract")))->doAll(user->nick.c_str(), x, y, z);
    for (uint32_t i = 0 ; i < ServerInstance->plugin()->getBlockCB().size(); i++)
    {
      blockcb = ServerInstance->plugin()->getBlockCB()[i];
      if (blockcb != NULL && blockcb->affectedBlock(oldblock))
      {
        //This should actually make the boolean do something. Maybe.
        if (blockcb->onInteract(user, x, y, z, user->pos.map))
        {
          blockD.revertBlock(user, x, y, z, user->pos.map);
          return PACKET_OK;
        }
        else
        {
          break;
        }
      }
    }
  }
  bool foundFromInventory = false;

#define INV_TASKBAR_START 36
  if (user->inv[INV_TASKBAR_START + user->currentItemSlot()].getType() == newblock && newblock != -1)
  {
    foundFromInventory = true;
  }
#undef INV_TASKBAR_START


  // TODO: Handle processing of
  if (direction == -1 || !foundFromInventory)
  {
    blockD.revertBlock(user, x, y, z, user->pos.map);
    return PACKET_OK;
  }

  // Minecart testing!!
  if (newblock == ITEM_MINECART && ServerInstance->map(user->pos.map)->getBlock(x, y, z, &oldblock, &metadata))
  {
    if (oldblock != BLOCK_MINECART_TRACKS)
    {
      return PACKET_OK;
    }

    LOG(INFO, "Packets", "Spawn minecart");
    int32_t EID = Mineserver::generateEID();
    Packet pkt;
    // MINECART
    pkt << (int8_t)PACKET_ADD_OBJECT << (int32_t)EID << (int8_t)10 << (int32_t)(x * 32 + 16) << (int32_t)(y * 32) << (int32_t)(z * 32 + 16);
    user->sendAll(pkt);
  }

  if (newblock == -1 && newblock != ITEM_SIGN)
  {
#ifdef DEBUG
    LOG(DEBUG, "Packets", "ignoring: " + dtos(newblock));
#endif
    return PACKET_OK;
  }

  if (y < 0)
  {
    return PACKET_OK;
  }

#ifdef DEBUG
  LOG(DEBUG, "Packets", "Block_placement: " + dtos(newblock) + " (" + dtos(x) + "," + dtos((int)y) + "," + dtos(z) + ") dir: " + dtos((int)direction));
#endif

  if (direction)
  {
    direction = 6 - direction;
  }

  //if (ServerInstance->map()->getBlock(x, y, z, &oldblock, &metadata))
  {
    uint8_t oldblocktop;
    uint8_t metadatatop;
    int8_t check_y = y;
    int32_t check_x = x;
    int32_t check_z = z;

    /* client doesn't give us the correct block for lava and water, check block above */
    switch (direction)
    {
    case BLOCK_BOTTOM:
      check_y--;
      break;
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

    if (ServerInstance->map(user->pos.map)->getBlock(check_x, check_y, check_z, &oldblocktop, &metadatatop) &&
        (oldblocktop == BLOCK_LAVA || oldblocktop == BLOCK_STATIONARY_LAVA ||
         oldblocktop == BLOCK_WATER || oldblocktop == BLOCK_STATIONARY_WATER))
    {
      /* block above needs replacing rather then the block sent by the client */

      // TODO: Does this require some form of recursion for multiple water/lava blocks?

      for (uint32_t i = 0 ; i < ServerInstance->plugin()->getBlockCB().size(); i++)
      {
        blockcb = ServerInstance->plugin()->getBlockCB()[i];
        if (blockcb != NULL && blockcb->affectedBlock(newblock))
        {
          blockcb->onReplace(user, newblock, check_x, check_y, check_z, user->pos.map, direction);
        }
      }

      if ((static_cast<Hook6<bool, const char*, int32_t, int8_t, int32_t, int16_t, int16_t>*>(ServerInstance->plugin()->getHook("BlockReplacePre")))->doUntilFalse(user->nick.c_str(), check_x, check_y, check_z, oldblock, newblock))
      {
        blockD.revertBlock(user, x, y, z, user->pos.map);
        return PACKET_OK;
      }
      (static_cast<Hook6<bool, const char*, int32_t, int8_t, int32_t, int16_t, int16_t>*>(ServerInstance->plugin()->getHook("BlockReplacePost")))->doAll(user->nick.c_str(), check_x, check_y, check_z, oldblock, newblock);
    }
    else
    {
      /*      for(int i =0 ; i<ServerInstance->plugin()->getBlockCB().size(); i++)
            {
              blockcb = ServerInstance->plugin()->getBlockCB()[i];
              if(blockcb!=NULL && blockcb->affectedBlock(newblock))
              {
                blockcb->onReplace(user, newblock,check_x,check_y,check_z,user->pos.map,direction);
              }
            }*/

      if ((static_cast<Hook6<bool, const char*, int32_t, int8_t, int32_t, int16_t, int16_t>*>(ServerInstance->plugin()->getHook("BlockReplacePre")))->doUntilFalse(user->nick.c_str(), x, y, z, oldblock, newblock))
      {
        blockD.revertBlock(user, x, y, z, user->pos.map);
        return PACKET_OK;
      }
      (static_cast<Hook6<bool, const char*, int32_t, int8_t, int32_t, int16_t, int16_t>*>(ServerInstance->plugin()->getHook("BlockReplacePost")))->doAll(user->nick.c_str(), x, y, z, oldblock, newblock);
    }

    if ((static_cast<Hook6<bool, const char*, int32_t, int8_t, int32_t, int16_t, int8_t>*>(ServerInstance->plugin()->getHook("BlockPlacePre")))->doUntilFalse(user->nick.c_str(), x, y, z, newblock, direction))
    {
      blockD.revertBlock(user, x, y, z, user->pos.map);
      return PACKET_OK;
    }

    /* We pass the newblock to the newblock's onPlace callback because
    the callback doesn't know what type of block we're placing. Instead
    the callback's job is to describe the behaviour when placing the
    block down, not to place any specifically block itself. */
    for (uint32_t i = 0 ; i < ServerInstance->plugin()->getBlockCB().size(); i++)
    {
      blockcb = ServerInstance->plugin()->getBlockCB()[i];
      if (blockcb != NULL && blockcb->affectedBlock(newblock))
      {
        if (blockcb->onPlace(user, newblock, x, y, z, user->pos.map, direction))
        {
          return PACKET_OK;
        }
        else
        {
          break;
        }
      }
    }
    (static_cast<Hook6<bool, const char*, int32_t, int8_t, int32_t, int16_t, int8_t>*>(ServerInstance->plugin()->getHook("BlockPlacePost")))->doAll(user->nick.c_str(), x, y, z, newblock, direction);

    /* notify neighbour blocks of the placed block */
    if (ServerInstance->map(user->pos.map)->getBlock(x + 1, y, z, &block, &meta) && block != BLOCK_AIR)
    {
      for (uint32_t i = 0 ; i < ServerInstance->plugin()->getBlockCB().size(); i++)
      {
        blockcb = ServerInstance->plugin()->getBlockCB()[i];
        if (blockcb != NULL && blockcb->affectedBlock(newblock))
        {
          blockcb->onNeighbourPlace(user, newblock, x + 1, y, z, user->pos.map, direction);
        }
      }

      (static_cast<Hook4<bool, const char*, int32_t, int8_t, int32_t>*>(ServerInstance->plugin()->getHook("BlockNeighbourPlace")))->doAll(user->nick.c_str(), x + 1, y, z);
    }

    if (ServerInstance->map(user->pos.map)->getBlock(x - 1, y, z, &block, &meta) && block != BLOCK_AIR)
    {
      for (uint32_t i = 0 ; i < ServerInstance->plugin()->getBlockCB().size(); i++)
      {
        blockcb = ServerInstance->plugin()->getBlockCB()[i];
        if (blockcb != NULL && blockcb->affectedBlock(newblock))
        {
          blockcb->onNeighbourPlace(user, newblock, x - 1, y, z, user->pos.map, direction);
        }
      }
      (static_cast<Hook4<bool, const char*, int32_t, int8_t, int32_t>*>(ServerInstance->plugin()->getHook("BlockNeighbourPlace")))->doAll(user->nick.c_str(), x - 1, y, z);
    }

    if (ServerInstance->map(user->pos.map)->getBlock(x, y + 1, z, &block, &meta) && block != BLOCK_AIR)
    {
      for (uint32_t i = 0 ; i < ServerInstance->plugin()->getBlockCB().size(); i++)
      {
        blockcb = ServerInstance->plugin()->getBlockCB()[i];
        if (blockcb != NULL && blockcb->affectedBlock(newblock))
        {
          blockcb->onNeighbourPlace(user, newblock, x, y + 1, z, user->pos.map, direction);
        }
      }
      (static_cast<Hook4<bool, const char*, int32_t, int8_t, int32_t>*>(ServerInstance->plugin()->getHook("BlockNeighbourPlace")))->doAll(user->nick.c_str(), x, y + 1, z);
    }

    if (ServerInstance->map(user->pos.map)->getBlock(x, y - 1, z, &block, &meta) && block != BLOCK_AIR)
    {
      for (uint32_t i = 0 ; i < ServerInstance->plugin()->getBlockCB().size(); i++)
      {
        blockcb = ServerInstance->plugin()->getBlockCB()[i];
        if (blockcb != NULL && blockcb->affectedBlock(newblock))
        {
          blockcb->onNeighbourPlace(user, newblock, x, y - 1, z, user->pos.map, direction);
        }
      }
      (static_cast<Hook4<bool, const char*, int32_t, int8_t, int32_t>*>(ServerInstance->plugin()->getHook("BlockNeighbourPlace")))->doAll(user->nick.c_str(), x, y - 1, z);
    }

    if (ServerInstance->map(user->pos.map)->getBlock(x, y, z + 1, &block, &meta) && block != BLOCK_AIR)
    {
      for (uint32_t i = 0 ; i < ServerInstance->plugin()->getBlockCB().size(); i++)
      {
        blockcb = ServerInstance->plugin()->getBlockCB()[i];
        if (blockcb != NULL && blockcb->affectedBlock(newblock))
        {
          blockcb->onNeighbourPlace(user, newblock, x, y, z + 1, user->pos.map, direction);
        }
      }
      (static_cast<Hook4<bool, const char*, int32_t, int8_t, int32_t>*>(ServerInstance->plugin()->getHook("BlockNeighbourPlace")))->doAll(user->nick.c_str(), x, y, z + 1);
    }

    if (ServerInstance->map(user->pos.map)->getBlock(x, y, z - 1, &block, &meta) && block != BLOCK_AIR)
    {
      for (uint32_t i = 0 ; i < ServerInstance->plugin()->getBlockCB().size(); i++)
      {
        blockcb = ServerInstance->plugin()->getBlockCB()[i];
        if (blockcb != NULL && blockcb->affectedBlock(newblock))
        {
          blockcb->onNeighbourPlace(user, newblock, x, y, z - 1, user->pos.map, direction);
        }
      }
      (static_cast<Hook4<bool, const char*, int32_t, int8_t, int32_t>*>(ServerInstance->plugin()->getHook("BlockNeighbourPlace")))->doAll(user->nick.c_str(), x, y, z - 1);
    }
  }
  // Now we're sure we're using it, lets remove from inventory!
#define INV_TASKBAR_START 36
  if (user->inv[INV_TASKBAR_START + user->currentItemSlot()].getType() == newblock && newblock != -1)
  {
    //if(newblock<256)
    {
      // It's a block
      user->inv[INV_TASKBAR_START + user->currentItemSlot()].decCount();
    }
  }
#undef INV_TASKBAR_START



  /* TODO: Should be removed from here. Only needed for liquid related blocks? */
  ServerInstance->physics(user->pos.map)->checkSurrounding(vec(x, y, z));
  return PACKET_OK;
}

int PacketHandler::holding_change(User* user)
{
  int16_t itemSlot;
  user->buffer >> itemSlot;

  if (!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->buffer.removePacket();

  user->curItem = itemSlot;

  //Send holding change to others
  Packet pkt;
  pkt << (int8_t)PACKET_ENTITY_EQUIPMENT << (int32_t)user->UID << (int16_t)0 << (int16_t)user->inv[itemSlot + 36].getType() << (int16_t)user->inv[itemSlot + 36].getHealth();
  user->sendOthers(pkt);

  // Set current itemID to user
  user->setCurrentItemSlot(itemSlot);
  return PACKET_OK;
}

int PacketHandler::arm_animation(User* user)
{
  int32_t userID;
  int8_t animType;

  user->buffer >> userID >> animType;

  if (!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->buffer.removePacket();

  Packet pkt;
  pkt << (int8_t)PACKET_ARM_ANIMATION << (int32_t)user->UID << animType;
  user->sendOthers(pkt);

  (static_cast<Hook1<bool, const char*>*>(ServerInstance->plugin()->getHook("PlayerArmSwing")))->doAll(user->nick.c_str());

  return PACKET_OK;
}

int PacketHandler::pickup_spawn(User* user)
{
  //uint32_t curpos = 4; //warning: unused variable ‘curpos’
  spawnedItem item;

  item.health = 0;

  int8_t yaw, pitch, roll;

  user->buffer >> (int32_t&)item.EID;

  user->buffer >> (int16_t&)item.item >> (int8_t&)item.count >> (int16_t&)item.health;
  user->buffer >> (int32_t&)item.pos.x() >> (int32_t&)item.pos.y() >> (int32_t&)item.pos.z();
  user->buffer >> yaw >> pitch >> roll;

  if (!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->buffer.removePacket();

  item.EID       = Mineserver::generateEID();

  item.spawnedBy = user->UID;
  item.spawnedAt = time(NULL);

  // Modify the position of the dropped item so that it appears in front of user instead of under user
  int distanceToThrow = 64;
  float angle = DEGREES_TO_RADIANS(user->pos.yaw + 45);     // For some reason, yaw seems to be off to the left by 45 degrees from where you're actually looking?
  int x = int(cos(angle) * distanceToThrow - sin(angle) * distanceToThrow);
  int z = int(sin(angle) * distanceToThrow + cos(angle) * distanceToThrow);
  item.pos += vec(x, 0, z);

  ServerInstance->map(user->pos.map)->sendPickupSpawn(item);

  return PACKET_OK;
}

int PacketHandler::disconnect(User* user)
{
  if (!user->buffer.haveData(2))
  {
    return PACKET_NEED_MORE_DATA;
  }

  std::string msg;
  user->buffer >> msg;

  if (!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->buffer.removePacket();
  
  LOG(INFO, "Packets", "Disconnect: " + msg);

  return PACKET_OK;
}

int PacketHandler::use_entity(User* user)
{
  int32_t userID, target;
  int8_t leftClick;


  user->buffer >> userID >> target >> leftClick;

  if (!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->buffer.removePacket();

  if (!leftClick)
  {
    // right clicks: interaction, attaching, ...
    for (size_t i = 0; i < ServerInstance->mobs()->getMobCount(); i++)
    {
      if (ServerInstance->mobs()->getMobByID(i)->UID == (uint32_t)target)
      {
        (static_cast<Hook2<bool, const char* ,int32_t>*>(ServerInstance->plugin()->getHook("interact")))->doAll(user->nick.c_str(), (int32_t)ServerInstance->mobs()->getMobByTarget(target));
        //make a callback
        return PACKET_OK;
      }
    }

    // No? Try to attach.
    Packet pkt;
    //Attach
    if (user->attachedTo == 0)
    {
      pkt << (int8_t)PACKET_ATTACH_ENTITY << (int32_t)user->UID << (int32_t)target;
      user->attachedTo = target;
    }
    //Detach
    else
    {
      pkt << (int8_t)PACKET_ATTACH_ENTITY << (int32_t)user->UID << (int32_t) - 1;
      user->attachedTo = 0;
    }
    user->sendAll(pkt);
    return PACKET_OK;

  }
  else
  {
    // left clicks: fighhht!
    if (ServerInstance->m_pvp_enabled)
    {
      //This is used when punching users, mobs or other entities
      for (std::set<User*>::const_iterator it = ServerInstance->users().begin(); it != ServerInstance->users().end(); ++it)
      {
        if ((*it)->UID == (uint32_t)target)
        {
          (*it)->health--;
          (*it)->sethealth((*it)->health);

          if ((*it)->health <= 0)
          {
            Packet pkt;
            pkt << (int8_t)PACKET_DEATH_ANIMATION << (int32_t)(*it)->UID << (int8_t)3;
            (*it)->sendOthers(pkt);
          }
          break;
        }
      }
    }
    for (size_t i = 0; i < ServerInstance->mobs()->getMobCount(); i++)
    {
      if (ServerInstance->mobs()->getMobByID(i)->UID == (uint32_t)target)
      {
        //int h = ServerInstance->mobs()->getMobByID(i)->health - 1;
        //ServerInstance->mobs()->getMobByID(i)->sethealth(h);
        (static_cast<Hook2<bool, const char* ,int32_t>*>(ServerInstance->plugin()->getHook("gotAttacked")))->doAll(user->nick.c_str(),(int32_t)ServerInstance->mobs()->getMobByTarget(target));
        //make a callback
        break;
      }
    }
  }

  return PACKET_OK;
}


// Keep Alive (http://mc.kev009.com/wiki/Protocol)
int PacketHandler::ping(User* user)
{
  //Reply with server info
  user->buffer.removePacket();

  std::ostringstream line;
  line << ServerInstance->config()->sData("system.server_name") << "§"
       << ServerInstance->getLoggedUsersCount() << "§"
       << ServerInstance->config()->iData("system.user_limit");
  user->kick(line.str());

  return PACKET_OK;
}


int PacketHandler::respawn(User* user)
{
  if (!user->buffer.haveData(10))
  {
    return PACKET_NEED_MORE_DATA;
  }
  int32_t dimension;
  int8_t difficulty,creative;
  int16_t height;
  std::string level_type;

  user->buffer >> dimension >> difficulty >> creative >> height >> level_type;
  user->dropInventory();
  user->respawn();
  user->buffer.removePacket();
  return PACKET_OK;
}


//Sent when right clicking blocks without placing, interact?
int PacketHandler::block_change(User* user)
{
  int32_t x,z;
  int8_t y,type,meta;

  user->buffer >> x >> y >> z >> type >> meta;
  //printf("block change %d:%d:%d type %d meta %d\r\n",x,y,z, type,meta);
  user->buffer.removePacket();
  return PACKET_OK;
}




// Shift operators for Packet class
Packet& Packet::operator<<(int8_t val)
{
  m_writeBuffer.push_back(val);
  return *this;
}

Packet& Packet::operator>>(int8_t& val)
{
  if (haveData(1))
  {
    val = m_readBuffer[m_readPos++];
  }
  return *this;
}

Packet& Packet::operator<<(int16_t val)
{
  uint16_t nval = htons(val);
  addToWrite(reinterpret_cast<const uint8_t*>(&nval), sizeof(nval));
  return *this;
}

Packet& Packet::operator>>(int16_t& val)
{
  if (haveData(2))
  {
    int16_t res;
    uint8_t* p = reinterpret_cast<uint8_t*>(&res);
    for (size_t i = 0; i < sizeof(res); ++i) *p++ = m_readBuffer[m_readPos++];
    val = ntohs(res);
  }
  return *this;
}

Packet& Packet::operator<<(int32_t val)
{
  uint32_t nval = htonl(val);
  addToWrite(reinterpret_cast<const uint8_t*>(&nval), sizeof(nval));
  return *this;
}

Packet& Packet::operator>>(int32_t& val)
{
  if (haveData(4))
  {
    int32_t res;
    uint8_t* p = reinterpret_cast<uint8_t*>(&res);
    for (size_t i = 0; i < sizeof(res); ++i) *p++ = m_readBuffer[m_readPos++];
    val = ntohl(res);
  }
  return *this;
}

Packet& Packet::operator<<(int64_t val)
{
  uint64_t nval = ntohll(val);
  addToWrite(reinterpret_cast<const uint8_t*>(&nval), sizeof(nval));
  return *this;
}

Packet& Packet::operator>>(int64_t& val)
{
  if (haveData(8))
  {
    int64_t res;
    uint8_t* p = reinterpret_cast<uint8_t*>(&res);
    for (size_t i = 0; i < sizeof(res); ++i) *p++ = m_readBuffer[m_readPos++];
    val = ntohll(val);
  }
  return *this;
}

Packet& Packet::operator<<(float val)
{
  uint32_t nval;
  memcpy(&nval, &val, 4);
  nval = htonl(nval);
  addToWrite(reinterpret_cast<const uint8_t*>(&nval), sizeof(nval));
  return *this;
}

Packet& Packet::operator>>(float& val)
{
  if (haveData(4))
  {
    uint32_t res;
    uint8_t* p = reinterpret_cast<uint8_t*>(&res);
    for (size_t i = 0; i < sizeof(res); ++i) *p++ = m_readBuffer[m_readPos++];
    uint32_t ival = ntohl(res);
    memcpy(&val, &ival, 4);
  }
  return *this;
}

Packet& Packet::operator<<(double val)
{
  uint64_t nval;
  memcpy(&nval, &val, 8);
  nval = ntohll(nval);
  addToWrite(reinterpret_cast<const uint8_t*>(&nval), sizeof(nval));
  return *this;
}

Packet& Packet::operator>>(double& val)
{
  if (haveData(8))
  {
    uint64_t res;
    uint8_t* p = reinterpret_cast<uint8_t*>(&res);
    for (size_t i = 0; i < sizeof(res); ++i) *p++ = m_readBuffer[m_readPos++];
    uint64_t ival = ntohll(res);
    memcpy(&val, &ival, 8);
  }
  return *this;
}

Packet& Packet::operator<<(const std::string& str)
{
  std::vector<uint16_t> result;
  makeUCS2MessageFromUTF8(str, result);

  uint16_t lenval = htons(result.size());
  addToWrite(reinterpret_cast<const uint8_t*>(&lenval), 2);

  for (size_t i = 0;  i < result.size(); ++i)
  {
    uint16_t character = htons(result[i]);
    addToWrite(reinterpret_cast<const uint8_t*>(&character), 2);
  }

  return *this;
}

Packet& Packet::operator>>(std::string& str)
{
  uint16_t lenval;
  if (haveData(2))
  {
    uint16_t lenval = 0;
    *this >> (int16_t&)lenval;

    if (lenval && haveData(2 * lenval)) // We ASSUME that every character takes 2 bytes. DANGEROUS.
    {
      unsigned char buf[2];
      t_codepoint ccp;

      for (size_t i = 0;  i < lenval; ++i)
      {
        buf[0] = m_readBuffer[m_readPos++];
        buf[1] = m_readBuffer[m_readPos++];

        codepointToUTF8(((unsigned int)(buf[0]) << 8) | ((unsigned int)(buf[1])), &ccp);
        str += std::string(ccp.c);
      }
    }
  }
  return *this;
}

Packet& Packet::operator<<(const Packet& other)
{
  addToWrite(other);
  return *this;
}

// writeString and readString provide the old, 8-bit string features.

void Packet::writeString(const std::string& str)
{
  uint16_t lenval = htons(str.size());
  addToWrite(reinterpret_cast<const uint8_t*>(&lenval), 2);
  addToWrite(reinterpret_cast<const uint8_t*>(str.data()), str.length());
}

std::string Packet::readString()
{
  std::string str;

  if (haveData(2))
  {
    uint16_t lenval = 0;
    *this >> (int16_t)lenval;

    if (lenval && haveData(lenval))
    {
      for (size_t i = 0; i < lenval; ++i)
        str += m_readBuffer[m_readPos++];
    }
  }

  return str;
}
