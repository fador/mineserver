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
#ifdef _WIN32
#include <winsock2.h>
#else
#include <netdb.h>       // for gethostbyname()
#include <netinet/tcp.h> // for TCP constants
#endif

#include <cmath>
#include <sstream>
#include <algorithm>


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
#include "protocol.h"
#include "sockets.h"

#ifdef PROTOCOL_ENCRYPTION
#include <openssl/rsa.h>
#include <openssl/err.h>
#endif

#define _WINSOCKAPI_
#define NOMINMAX
#include <zlib.h>

void PacketHandler::init()
{
  
  packets[STATE_HANDSHAKE][PACKET_HANDSHAKE] = Packets(&PacketHandler::handshake);
  packets[STATE_LOGIN][PACKET_LOGIN_REQUEST] = Packets(&PacketHandler::login_request);
  packets[STATE_LOGIN][PACKET_ENCRYPTION_RESPONSE] = Packets(&PacketHandler::encryption_response);
  packets[STATE_STATUS][PACKET_SERVER_LIST_PING] = Packets(&PacketHandler::server_list_ping);
  packets[STATE_STATUS][PACKET_PING] = Packets(&PacketHandler::ping);
  
  packets[STATE_PLAY][PACKET_KEEP_ALIVE] = Packets(&PacketHandler::keep_alive);
  packets[STATE_PLAY][PACKET_CHAT_MESSAGE] = Packets(&PacketHandler::chat_message);
  packets[STATE_PLAY][PACKET_USE_ENTITY] = Packets(&PacketHandler::use_entity);
  packets[STATE_PLAY][PACKET_PLAYER] = Packets(&PacketHandler::player);
  packets[STATE_PLAY][PACKET_PLAYER_POSITION] = Packets(&PacketHandler::player_position);
  packets[STATE_PLAY][PACKET_PLAYER_LOOK] = Packets(&PacketHandler::player_look);
  packets[STATE_PLAY][PACKET_PLAYER_POSITION_AND_LOOK] = Packets(&PacketHandler::player_position_and_look);
  packets[STATE_PLAY][PACKET_PLAYER_DIGGING] = Packets(&PacketHandler::player_digging);
  packets[STATE_PLAY][PACKET_PLAYER_BLOCK_PLACEMENT] = Packets(&PacketHandler::player_block_placement);
  packets[STATE_PLAY][PACKET_HOLDING_CHANGE] = Packets(&PacketHandler::holding_change);
  packets[STATE_PLAY][PACKET_ANIMATION] = Packets(&PacketHandler::arm_animation);
  packets[STATE_PLAY][PACKET_PICKUP_SPAWN] = Packets(&PacketHandler::pickup_spawn);
  packets[STATE_PLAY][PACKET_DISCONNECT] = Packets(&PacketHandler::disconnect);
  packets[STATE_PLAY][PACKET_RESPAWN] = Packets(&PacketHandler::respawn);
  packets[STATE_PLAY][PACKET_INVENTORY_CHANGE] = Packets(&PacketHandler::inventory_change);
  packets[STATE_PLAY][PACKET_INVENTORY_CLOSE] = Packets(&PacketHandler::inventory_close);
  packets[STATE_PLAY][PACKET_SIGN] = Packets(&PacketHandler::change_sign);
  packets[STATE_PLAY][PACKET_TRANSACTION] = Packets(&PacketHandler::inventory_transaction);
  packets[STATE_PLAY][PACKET_ENTITY_CROUCH] = Packets(&PacketHandler::entity_crouch);
  packets[STATE_PLAY][PACKET_THUNDERBOLT] = Packets(&PacketHandler::unhandledPacket);
  packets[STATE_PLAY][PACKET_INCREMENT_STATISTICS] = Packets(&PacketHandler::unhandledPacket);
  packets[STATE_PLAY][PACKET_BLOCK_CHANGE] = Packets(&PacketHandler::block_change);
  packets[STATE_PLAY][PACKET_TAB_COMPLETE] = Packets(&PacketHandler::tab_complete);
  packets[STATE_PLAY][PACKET_CLIENT_INFO] = Packets(&PacketHandler::client_info);
  packets[STATE_PLAY][PACKET_CLIENT_STATUS] = Packets(&PacketHandler::client_status);  
  packets[STATE_PLAY][PACKET_PLUGIN_MESSAGE] = Packets(&PacketHandler::plugin_message);
  packets[STATE_PLAY][PACKET_CREATIVE_INVENTORY] = Packets(&PacketHandler::creative_inventory);
  packets[STATE_PLAY][PACKET_PLAYER_ABILITIES] = Packets(&PacketHandler::player_abilities);
}

int PacketHandler::unhandledPacket(User* user)
{

  return PACKET_OK;
}

int PacketHandler::plugin_message(User* user)
{

  if (!user->buffer.haveData(4))
  {
    return PACKET_NEED_MORE_DATA;
  }

  
  std::string channel;
  int16_t datalen;

  user->buffer >> channel;

  if (!user->buffer || !user->buffer.haveData(2))
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->buffer >> datalen;

  if (!user->buffer.haveData(datalen))
  {
    return PACKET_NEED_MORE_DATA;
  }

  for(int i = 0; i < datalen; i++)
  {
    int8_t byte;
    user->buffer >> byte;
  }

  LOG2(INFO, "Plugin message: "+channel);


  return PACKET_OK;
}


#ifdef PROTOCOL_ENCRYPTION
int PacketHandler::encryption_response(User* user)
{

  MS_VarInt secretLen, verifyLen;
  std::string secret,verify;
  std::string decryptedSecret(' ', 16);

  user->buffer >> secretLen;

  if (!user->buffer.haveData((int)secretLen))
  {
    return PACKET_NEED_MORE_DATA;
  }

  for(int i = 0; i < secretLen; i++)
  {
    int8_t byte;
    user->buffer >> byte;
    secret.push_back(byte);
  }

  user->buffer >> verifyLen;


  for(int i = 0; i < (int)verifyLen; i++)
  {
    int8_t byte;
    user->buffer >> byte;
    verify.push_back(byte);
  }

  if (!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }
  
  //Those should be around 128 bytes
  if((int)verifyLen > 1023 || (int)secretLen > 1023)
  {
    user->kick("Invalid verify/secret size");
    return PACKET_OK;
  }

  
  uint8_t buffer[1024];
  memset(buffer, 0, 1024);
  //Decrypt the verification bytes
  int ret = RSA_private_decrypt((int)verifyLen,(const uint8_t *)verify.c_str(),buffer,ServerInstance->rsa,RSA_PKCS1_PADDING);
  //Check they match with the ones sent
  if(ret != 4 || std::string((char *)buffer) != ServerInstance->encryptionBytes)
  {
    user->kick("Decryption failed");
    return PACKET_OK;
  }

  //Decrypt secret sent by the client and store
  memset(buffer, 0, 1024);
  ret = RSA_private_decrypt((int)secretLen,(const uint8_t *)secret.c_str(),buffer,ServerInstance->rsa,RSA_PKCS1_PADDING);
  user->secret = std::string((char *)buffer, ret);
  //We're going crypted!
  user->initCipher();
  
  
  if(!ServerInstance->config()->bData("system.user_validation"))
  {
    //Response
    user->crypted = true;
    user->sendLoginInfo();
  }
  else
  {
    pthread_t validation_thread;
    Mineserver::userValidation* valid = new Mineserver::userValidation;
    valid->user = user;
    valid->UID = user->UID;
    pthread_create(&validation_thread,NULL,user_validation_thread,(void*)valid);
  }
  

  return PACKET_OK;
}
#endif

int PacketHandler::client_status(User* user)
{
  int8_t payload;

  user->buffer >> payload;
  


  //0: Initial spawn, 1: Respawn after death
  LOG2(INFO, "client_status.");
  if(payload == 0 && user->crypted)
  {
    LOG2(INFO, "Sending login info..");
    user->sendLoginInfo();
  }
  //player respawns
  if(payload == 1)
  {
    user->dropInventory();
    user->respawn();
  }

  return PACKET_OK;
}

int PacketHandler::creative_inventory(User *user)
{
    /// TODO: use this somewhere!
    if (!user->buffer.haveData(4))
        return PACKET_NEED_MORE_DATA;

    int16_t slot, itemID;

    user->buffer>>slot>>itemID;

    if((uint16_t)itemID == 0xffff) return PACKET_OK;

    if (!user->buffer.haveData(5))
        return PACKET_NEED_MORE_DATA;

    int8_t count;
    int16_t meta;

    user->buffer >> count >> meta;


    int16_t enchantment_data_len;
    user->buffer >> enchantment_data_len;

    if(enchantment_data_len != 0xffff) {
        LOG2(INFO, "Got enchantment data, ignoring...");
    }

    Item& it = user->inv[slot];

    it.setType(itemID);
    it.setCount(count);
    it.setHealth(meta);

  
    return PACKET_OK;
}

int PacketHandler::player_abilities(User *user)
{
    /// TODO: use this somewhere!
    int8_t flags, fspeed, wspeed;

    user->buffer >> flags >> fspeed >> wspeed;

    return PACKET_OK;
}

int PacketHandler::client_info(User* user)
{
  // Wait for length-short. HEHE
  if (!user->buffer.haveData(2))
  {
    return PACKET_NEED_MORE_DATA;
  }

  std::string locale;
  int8_t viewDistance,chatFlags,difficulty,showCape;

  user->buffer >> locale;

  if (!user->buffer || !user->buffer.haveData(4))
  {
    return PACKET_NEED_MORE_DATA;
  }

  user->buffer >> viewDistance >> chatFlags >> difficulty >> showCape;



  //ToDo: Do something with the values


  return PACKET_OK;
}


int PacketHandler::tab_complete(User* user)
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


  //ToDo: autocomplete!
  user->buffer << (int8_t)PACKET_TAB_COMPLETE << " ";

  return PACKET_OK;
}

int PacketHandler::entity_crouch(User* user)
{
  int32_t EID;
  int8_t action;
  int32_t horseJumpBoost; // range: 0 -> 100; (Notch Logic)

  MetaData meta;
  MetaDataElemByte *element;

  user->buffer >> EID >> action >> horseJumpBoost;
  Packet pkt;
  bool packetData = false;

  //ToDo: handle other actions
  switch(action)
  {
  //Crouch
  case 1:    
    element = new MetaDataElemByte(0,0x02);
    meta.set(element);
    pkt << Protocol::animation(user->UID, 104) << Protocol::entityMetadata(user->UID,meta);
    packetData = true;
    break;
    //Uncrouch
  case 2:
    element = new MetaDataElemByte(0,0x00);
    meta.set(element);
    pkt << Protocol::animation(user->UID, 105) << Protocol::entityMetadata(user->UID,meta);
    packetData = true;
    break;
  default:
    break;
  }
  
  if(packetData)
  {
    sChunk* chunk = ServerInstance->map(user->pos.map)->getChunk(blockToChunk((int32_t)user->pos.x), blockToChunk((int32_t)user->pos.z));
    if (chunk != NULL)
    {
      chunk->sendPacket(pkt);
    }
  }


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

  return PACKET_OK;
}


int PacketHandler::inventory_close(User* user)
{
  int8_t windowID;

  user->buffer >> windowID;

  ServerInstance->inventory()->windowClose(user, windowID);


  return PACKET_OK;
}


int PacketHandler::inventory_transaction(User* user)
{
  int8_t windowID;
  int16_t action;
  int8_t accepted;

  user->buffer >> windowID >> action >> accepted;

  //No need to do anything

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
    //if(Item::isEnchantable(itemID)) {
    int16_t enchantment_data_len;
    user->buffer >> enchantment_data_len;
    if(enchantment_data_len >= 0) {
      LOG2(INFO, "Got enchantment data, ignoring...");
    }
    //}
  }

  ServerInstance->inventory()->windowClick(user, windowID, slot, rightClick, actionNumber, itemID, itemCount, itemUses, shift);


  return PACKET_OK;
}

// Keep Alive (http://mc.kev009.com/wiki/Protocol#Keep_Alive_.280x00.29)
int PacketHandler::keep_alive(User* user)
{
  //No need to do anything

  return PACKET_OK;
}

// Login request
int PacketHandler::login_request(User* user)
{

  std::string player;

  user->buffer >> player;

  // Check for data
  if (!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }


  user->nick = player;

  char* kickMessage = NULL;
  runCallbackUntilFalse("PlayerLoginPre",player.c_str(), &kickMessage);
  if (callbackReturnValue)
  {
    user->kick(std::string(kickMessage));
  }
  else
  {
    //We can skip the protocol encryption
    if(!ServerInstance->config()->bData("system.protocol_encryption"))
    {
      user->sendLoginInfo();
    }
    else
    {
      user->buffer.writePacket(Protocol::encryptionRequest(), user->compression);
    }
    runAllCallback("PlayerLoginPost",player.c_str());
  }

  return PACKET_OK;
}

int PacketHandler::handshake(User* user)
{
  std::string host;
  MS_VarInt version;
  int16_t port;
  MS_VarInt nextState;

  user->buffer >> version >> host >> port >> nextState;

  // Check for data
  if (!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }

  LOG(INFO, "Packets", "Player " + dtos(user->UID) + " login v." + dtos(static_cast<int64_t>(version)) + " : ");


  // If version is not the current version
  if (static_cast<int64_t>(version) != PROTOCOL_VERSION)
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

  // Move to the next state
  if (nextState >= 0 && nextState < 4)
    user->gameState = nextState;
  
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



  return PACKET_OK;
}

int PacketHandler::player_digging(User* user)
{
  int8_t status;
  int32_t x;
  int16_t  y;
  int8_t temp_y;
  int32_t z;
  int8_t direction;
  uint8_t block;
  uint8_t meta;
  BlockBasicPtr blockcb;
  BlockDefault blockD;

  user->buffer >> status >> x >> temp_y >> z >> direction;
  y = (uint8_t)temp_y;

  if (!user->buffer)
  {
    return PACKET_NEED_MORE_DATA;
  }



  if (!ServerInstance->map(user->pos.map)->getBlock(x, y, z, &block, &meta))
  {
    blockD.revertBlock(user, x, y, z, user->pos.map);
    return PACKET_OK;
  }

  // Blocks that break with first hit
  if(status == BLOCK_STATUS_STARTED_DIGGING)
  {
      if( user->creative || (block == BLOCK_SNOW || block == BLOCK_REED || block == BLOCK_TORCH
                             || block == BLOCK_REDSTONE_WIRE || block == BLOCK_RED_ROSE || block == BLOCK_YELLOW_FLOWER
                             || block == BLOCK_BROWN_MUSHROOM || block == BLOCK_RED_MUSHROOM
                             || block == BLOCK_REDSTONE_TORCH_OFF || block == BLOCK_REDSTONE_TORCH_ON))
      {
          status = BLOCK_STATUS_BLOCK_BROKEN;
      }
  }

  switch (status)
  {
  case BLOCK_STATUS_STARTED_DIGGING:
  {
    runAllCallback("PlayerDiggingStarted",user->nick.c_str(), x, y, z, direction);

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

    runCallbackUntilFalse("BlockBreakPre",user->nick.c_str(), x, y, z);
    if (callbackReturnValue)
    {
      blockD.revertBlock(user, x, y, z, user->pos.map);
      return PACKET_OK;
    }

    runAllCallback("BlockBreakPost",user->nick.c_str(), x, y, z);

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
      runAllCallback("BlockNeighbourBreak",user->nick.c_str(), x + 1, y, z, x, int8_t(y), z);
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
      runAllCallback("BlockNeighbourBreak",user->nick.c_str(), x - 1, y, z, x, int8_t(y), z);
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
      runAllCallback("BlockNeighbourBreak",user->nick.c_str(), x, y + 1, z, x, int8_t(y), z);
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
      runAllCallback("BlockNeighbourBreak",user->nick.c_str(), x, y - 1, z, x, int8_t(y), z);
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
      runAllCallback("BlockNeighbourBreak",user->nick.c_str(), x, y, z + 1, x, int8_t(y), z);
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
      runAllCallback("BlockNeighbourBreak",user->nick.c_str(), x, y, z - 1, x, int8_t(y), z);
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
      ServerInstance->map(user->pos.map)->createPickupSpawn(int(user->pos.x), int(user->pos.y), int(user->pos.z), int(user->inv[itemSlot].getType()), 1, int(user->inv[itemSlot].getHealth()), user);

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
  int16_t y = 0;
  int8_t temp_y = 0;
  int8_t direction = 0;
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
  int16_t slotLen;
  int8_t posx,posy,posz;

  user->buffer >> x >> temp_y >> z >> direction >> newblock;
  if(newblock != -1)
  {
    int8_t count;
    int16_t damage;
    user->buffer >> count >> damage;
    user->buffer >> slotLen;
    if(slotLen != -1)
    {
      if (!user->buffer.haveData(slotLen+3))
      {
        return PACKET_NEED_MORE_DATA;
      }
      uint8_t *buf = new uint8_t[slotLen];
      for(int i = 0; i < slotLen; i++)
      {
        int8_t data;
        user->buffer >> data;
        buf[i] = data;
      }
      //Do something with the slot data
      delete[] buf;
    }
  }

  user->buffer  >> posx >> posy >> posz;
  //newblock;
  y = (uint8_t)temp_y;



  ItemBasicPtr itemcb;
  if (direction == -1 && x == -1 && y == 255 && z == -1)
  {
    // Right clicked without pointing at a tile
    Item* item = &(user->inv[user->curItem + 36]);
    runCallbackUntilFalse("ItemRightClickPre",user->nick.c_str(), x, y, z, item->getType(), direction);
    if (callbackReturnValue)
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
    runAllCallback("PlayerBlockInteract",user->nick.c_str(), x, y, z);
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
    
    int32_t EID = Mineserver::generateEID();
    Packet pkt;
    // MINECART
    //10 == minecart
    pkt << Protocol::addObject(EID,10,x,y,z,0);
    
    user->sendAll(pkt);

    ServerInstance->map(user->pos.map)->minecarts.push_back(MinecartData(EID,vec(x*32+16,y*32+16,z*32+16),vec(0,0,0),microTime()));
    //ToDo: Store
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
    int16_t check_y = y;
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

      runCallbackUntilFalse("BlockReplacePre",user->nick.c_str(), check_x, check_y, check_z, oldblock, newblock);
      if (callbackReturnValue)
      {
        blockD.revertBlock(user, x, y, z, user->pos.map);
        return PACKET_OK;
      }
      runAllCallback("BlockReplacePost",user->nick.c_str(), check_x, check_y, check_z, oldblock, newblock);
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

      runCallbackUntilFalse("BlockReplacePre",user->nick.c_str(), x, y, z, oldblock, newblock);
      if (callbackReturnValue)
      {
        blockD.revertBlock(user, x, y, z, user->pos.map);
        return PACKET_OK;
      }
      runAllCallback("BlockReplacePost",user->nick.c_str(), x, y, z, oldblock, newblock);
    }

    runCallbackUntilFalse("BlockPlacePre",user->nick.c_str(), x, y, z, newblock, direction);
    if(callbackReturnValue)
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
    runAllCallback("BlockPlacePost",user->nick.c_str(), x, y, z, newblock, direction);

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

      runAllCallback("BlockNeighbourPlace",user->nick.c_str(), x + 1, y, z);
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
      runAllCallback("BlockNeighbourPlace",user->nick.c_str(), x - 1, y, z);
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
      runAllCallback("BlockNeighbourPlace",user->nick.c_str(), x, y + 1, z);
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
      runAllCallback("BlockNeighbourPlace",user->nick.c_str(), x, y - 1, z);
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
      runAllCallback("BlockNeighbourPlace",user->nick.c_str(), x, y, z + 1);
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
      runAllCallback("BlockNeighbourPlace",user->nick.c_str(), x, y, z - 1);
    }
  }
  // Now we're sure we're using it, lets remove from inventory!
#define INV_TASKBAR_START 36
  if (user->inv[INV_TASKBAR_START + user->currentItemSlot()].getType() == newblock && newblock != -1)
  {
    //if(newblock<256)
    {
      // It's a block
      if(!user->creative)
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



  user->curItem = itemSlot;

  //Send holding change to others
  Packet pkt;
  pkt << (int8_t)PACKET_ENTITY_EQUIPMENT << (int32_t)user->UID << (int16_t)0 << Protocol::slot(user->inv[itemSlot + 36].getType(),1,user->inv[itemSlot + 36].getHealth());
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



  Packet pkt = Protocol::animation(user->UID,animType);
  user->sendOthers(pkt);

  runAllCallback("PlayerArmSwing",user->nick.c_str());

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



  item.EID       = Mineserver::generateEID();

  item.spawnedBy = user->UID;
  item.spawnedAt = time(NULL);

  // Modify the position of the dropped item so that it appears in front of user instead of under user
  int distanceToThrow = 64;
  float angle = float(DEGREES_TO_RADIANS(user->pos.yaw + 45));     // For some reason, yaw seems to be off to the left by 45 degrees from where you're actually looking?
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



  if (!leftClick)
  {
    // right clicks: interaction, attaching, ...
    for (size_t i = 0; i < ServerInstance->mobs()->getMobCount(); i++)
    {
      if (ServerInstance->mobs()->getMobByID(i)->UID == (uint32_t)target)
      {
        runAllCallback("interact",user->nick.c_str(), (int32_t)ServerInstance->mobs()->getMobByTarget(target));
        //make a callback
        return PACKET_OK;
      }
    }

    // No? Try to attach.
    Packet pkt;
    //Attach
    if (user->attachedTo == 0)
    {
      pkt << Protocol::attachEntity(user->UID,target);
      user->attachedTo = target;
      for (size_t i = 0; i < ServerInstance->map(user->pos.map)->minecarts.size(); i++)
      {
        if(ServerInstance->map(user->pos.map)->minecarts[i].EID == target)
        {
          ServerInstance->map(user->pos.map)->minecarts[i].speed = vec(64,0,0);
        }
      }
    }
    //Detach
    else
    {
      for (size_t i = 0; i < ServerInstance->map(user->pos.map)->minecarts.size(); i++)
      {
        if(ServerInstance->map(user->pos.map)->minecarts[i].EID == user->attachedTo)
        {
          ServerInstance->map(user->pos.map)->minecarts[i].speed = vec(0,0,0);
        }
      }
      pkt << Protocol::attachEntity(user->UID,-1);
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
            pkt << (int8_t)PACKET_ENTITY_STATUS << (int32_t)(*it)->UID << (int8_t)3;
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
        runAllCallback("gotAttacked",user->nick.c_str(),(int32_t)ServerInstance->mobs()->getMobByTarget(target));
        //make a callback
        break;
      }
    }
  }

  return PACKET_OK;
}


// Serverlist ping (http://wiki.vg/Server_List_Ping)
int PacketHandler::server_list_ping(User* user)
{  
  //Reply with server info
  std::string line;
  Packet pkt;

  line = "{ \"version\": {\"name\": \""+MINECRAFT_VERSION+"\", \"protocol\": "+my_itoa(PROTOCOL_VERSION)+" },\n"+
        "\"players\": {"+
          "\"max\": "+my_itoa(ServerInstance->config()->iData("system.user_limit"))+","+
          "\"online\": "+my_itoa(ServerInstance->getLoggedUsersCount())+
        " },"+
        "\"description\": {\"text\": \""+ServerInstance->config()->sData("system.server_name")+"\" }"+
      "}";

  LOG2(DEBUG, line);
  pkt << (uint8_t)0 << line;

  user->buffer.writePacket(pkt, user->compression);

  return PACKET_OK;
}

int PacketHandler::ping(User* user)
{

  uint64_t value;
  user->buffer >> value; 
  
  std::string line;
  Packet pkt;

  pkt << (uint8_t)1 << value;

  user->buffer.writePacket(pkt, user->compression);

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

  return PACKET_OK;
}


//Sent when right clicking blocks without placing, interact?
int PacketHandler::block_change(User* user)
{
  int32_t x,z;
  int8_t y,type,meta;

  user->buffer >> x >> y >> z >> type >> meta;
  //printf("block change %d:%d:%d type %d meta %d\r\n",x,y,z, type,meta);

  return PACKET_OK;
}


Packet& Packet::operator<<(const std::string& str)
{
  writeString(str);
  return *this;
}

Packet& Packet::operator>>(std::string& str)
{
  str = readString();
  return *this;
}

Packet& Packet::operator<<(const MS_VarInt& varint)
{
  writeVarInt(static_cast<int64_t>(varint));
  return *this;
}
Packet& Packet::operator>>(MS_VarInt& varint)
{
  varint.val = readVarInt();
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
  MS_VarInt len;
  len.val = str.size();
  *this << len;
  addToWrite(reinterpret_cast<const uint8_t*>(str.data()), str.length());
}

std::string Packet::readString()
{
  std::string str;

  if (haveData(1))
  {
    MS_VarInt lenval;
    *this >> lenval;
    uint16_t len = (uint16_t)static_cast<int64_t>(lenval);

    if (len && haveData(len))
    {
      for (size_t i = 0; i < uint16_t(len); ++i)
        str += m_readBuffer[m_readPos++];
    }
  }

  return str;
}

void Packet::writeVarInt(int64_t varint)
{
  do{
    uint8_t byte = 0;
    if(varint > 128)
      byte |= 0x80;

    byte |= (varint & 0x7F);

    write(&byte, 1);

    varint = varint >> 7;
  }while(varint != 0);
}

int64_t Packet::readVarInt()
{
  int64_t ret = 0;

  int byte_idx = 0;

  uint8_t byte;
  do{
    if(read(&byte, 1) != 1)
      // silent fail
      return 0;
    ret += (byte & 0x7F) << (7*byte_idx++);
  }while(byte & 0x80);

  return ret;
}


void Packet::writePacket(const Packet& p, uint16_t compression)
  {
    MS_VarInt datalen;
    datalen.val = p.m_writeBuffer.end()-p.m_writeBuffer.begin();

    if (compression)
    {
      if (datalen > compression) 
      {
        uint8_t* tempBuf = new uint8_t[(uint32_t)datalen+16];
        uint8_t* inBuf = new uint8_t[(uint32_t)datalen];

        // Read the writebuffer to a continuous buffer
        read(inBuf,(uint32_t)datalen);

        // Compress the buffer data
        uLongf written = (uint32_t)datalen+16;
        compress(tempBuf, &written, inBuf, (uint32_t)datalen);

        // We have to get the length of the uncompressed data
        Packet pkt_len;
        pkt_len << datalen;
        
        // Compressed packet length is the uncompressed length varint len + compressed data len
        MS_VarInt compressedLen;
        compressedLen.val = pkt_len.m_writeBuffer.size() + written;

        // Write the total packet length, uncompressed length and the compressed data
        Packet packetOut;
        packetOut << compressedLen << datalen;
        packetOut.write(tempBuf, written);
        addToWrite(packetOut);
        delete[] tempBuf;
        delete[] inBuf;
      }
      else
      {
        // If data is uncompressed, set uncompressed size to "0"
        datalen.val ++;
        *this << datalen;
        datalen.val = 0;
        *this << datalen;
        addToWrite(p);      
      }
    }
    else {
      // Write without the compress field
      MS_VarInt len;
      len.val = p.m_writeBuffer.end()-p.m_writeBuffer.begin();
      *this << len;
      addToWrite(p);
    }
  }