/*
   Copyright (c) 2013, The Mineserver Project
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

#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include "extern.h"
#include "config.h"
#include "constants.h"
#include "mob.h"
#include "tools.h"
#include "utf8.h"
#include "inventory.h"
#include "mineserver.h"
#include "logger.h"

/* This file introduces a basic abstraction over raw protocol packets format.
 * This is needed for varuios protocol updates - we need to change the raw format
 * only in one place. 
 * The implementation is totally in header to be inlined and optimized out.
 *
 * Maybe we need to add a lot of asserts there?
 */

class Protocol
{
  public:

    #ifdef PROTOCOL_ENCRYPTION
    static Packet encryptionRequest()
    {
      Packet ret;
      ret << MS_VarInt((uint32_t)PACKET_OUT_ENCRYPTION_REQUEST) << ServerInstance->serverID 
      << MS_VarInt(ServerInstance->publicKey.size());
      ret.addToWrite((uint8_t*)ServerInstance->publicKey.c_str(),ServerInstance->publicKey.size());
      ret << MS_VarInt(ServerInstance->encryptionBytes.size());
      ret.addToWrite((uint8_t*)ServerInstance->encryptionBytes.c_str(),ServerInstance->encryptionBytes.size());
      return ret;
    }
    #endif

    static Packet openWindow(uint8_t windowId, std::string windowType, std::string title_json, int8_t numberOfSlots, int32_t EID = 0)
    {
      Packet ret;
      ret << MS_VarInt((uint32_t)PACKET_OUT_OPEN_WINDOW) << windowId << windowType << title_json << numberOfSlots;
      if (windowType == INVENTORYTYPE_HORSE)
        ret << EID;
      return ret;
    }

    static Packet windowProperty(uint8_t windowId, int16_t property, int16_t value)
    {
      Packet ret;
      ret << MS_VarInt((uint32_t)PACKET_OUT_WINDOW_PROPERTY) << windowId << property << value;
      return ret;
    }

    static Packet animation(int eid, int aid)
    {
      Packet ret;
      ret << MS_VarInt((uint32_t)PACKET_OUT_ANIMATION) << MS_VarInt((uint32_t)eid) << (int8_t)aid;
      return ret;
    }

    static Packet updateSign(int32_t x, int32_t y, int32_t z, 
                             std::string line1, std::string line2, std::string line3, std::string line4)
    {
      Packet ret;      
      ret << MS_VarInt((uint32_t)PACKET_OUT_UPDATE_SIGN) 
        << (uint64_t)((((uint64_t)x & 0x3FFFFFF) << 38) | (((uint64_t)y & 0xFFF) << 26) | ((uint64_t)z & 0x3FFFFFF))
        << line1 << line2 << line3 << line4;
      return ret;
    }

    static Packet entityStatus(int eid, int aid)
    {
      Packet ret;
      ret << (int8_t)PACKET_OUT_ENTITY_STATUS << (int32_t)eid << (int8_t)aid;
      return ret;
    }

    static Packet entityMetadata(int eid, MetaData& metadata)
    {
      Packet ret;
      ret << (int8_t)PACKET_OUT_ENTITY_METADATA << (int32_t)eid << metadata;
      return ret;
    }

    static Packet entityVelocity(int eid, int16_t vel_x, int16_t vel_y,int16_t vel_z)
    {
      Packet ret;
      ret << (int8_t)PACKET_OUT_ENTITY_VELOCITY << (int32_t)eid << vel_x << vel_y << vel_z;
      return ret;
    }

    static Packet slot(Item &item)
    {
      Packet ret;
      ret << (int16_t)item.getType();
      if(item.getType() != -1) {
        ret << (int16_t)item.getCount() << (int16_t)item.getHealth();
        // ToDo: item extra data
        /*
        if(item_data != NULL) {
          ret << item_data_size;
          ret.addToWrite(item_data, item_data_size);
        } else {
          ret << (int16_t)-1;
        }
        */
        ret << (int16_t)-1;
      }
      return ret;
    }

    static Packet setSlot(int8_t window_id, int16_t slotId, Item& item)
    {
      Packet ret;
      ret << MS_VarInt((uint32_t)PACKET_OUT_SET_SLOT) << window_id << slotId;
      ret << Protocol::slot(item);
      return ret;
    }

    static Packet entityEquipment(int32_t eid, int16_t slot, Item& item)
    {
      Packet ret;
      ret << MS_VarInt((uint32_t)PACKET_OUT_ENTITY_EQUIPMENT) << MS_VarInt((uint32_t)eid) << (int16_t)slot
          << Protocol::slot(item);
      return ret;
    }

    static Packet mobSpawn(int eid, int8_t type, double x, double y, double z, int yaw, int pitch, int head_yaw, MetaData& metadata)
    {
      // Warning! This converts absolute double coordinates to absolute integer coordinates!
      Packet ret;
      ret << (int8_t)PACKET_OUT_SPAWN_MOB << (int32_t)eid << (int8_t)type
          << (int32_t)(x * 32) << (int32_t)(y * 32) << (int32_t)(z * 32)
          << (int8_t)yaw << (int8_t)pitch << (int8_t)head_yaw << (int16_t)0 
          << (int16_t)0 << (int16_t)0 << (int8_t)0 << (int8_t)0 << metadata;
      return ret;
    }

    static Packet mobSpawn(Mob& mob)
    {
      return Protocol::mobSpawn(mob.UID, mob.type, mob.x, mob.y, mob.z, mob.yaw, mob.pitch, mob.head_yaw, mob.metadata);
    }

    static Packet destroyEntity(int eid)
    {
      Packet ret;
      ret << (int8_t)PACKET_OUT_DESTROY_ENTITIES << (int8_t)1 << (int32_t)eid;
      return ret;
    }

    static Packet entityTeleport(uint32_t eid, double x, double y, double z, float yaw, float pitch)
    {
      Packet ret;
      ret << MS_VarInt((uint32_t)PACKET_OUT_ENTITY_TELEPORT) << MS_VarInt((uint32_t)eid)
          << (int32_t)(x * 32) << (int32_t)(y * 32) << (int32_t)(z * 32)
          << (int8_t)angleToByte(yaw) << (int8_t)angleToByte(pitch);
      return ret;
    }
    static Packet entityRelativeMove(int eid, int8_t x, int8_t y, int8_t z)
    {
      Packet ret;
      ret << (int8_t)PACKET_OUT_ENTITY_RELATIVE_MOVE << (int32_t)eid << (int8_t)x << (int8_t)y << (int8_t)z;
      return ret;
    }

    static Packet entityTeleport(uint32_t eid, const position& pos)
    {
      return Protocol::entityTeleport(eid, pos.x, pos.y, pos.z, pos.yaw, pos.pitch);
    }

    static Packet namedSoundEffect(std::string name, int32_t x, int32_t y, int32_t z, float volume, int8_t pitch)
    {
      Packet ret;
      ret << (int8_t)PACKET_OUT_SOUND_EFFECT << name << x << y << z << volume << pitch;
      return ret;
    }

    static Packet entityHeadLook(int eid, int head_yaw)
    {
      Packet ret;
      ret << (int8_t)PACKET_OUT_ENTITY_HEAD_LOOK << eid << (int8_t)head_yaw;
      return ret;
    }

    static Packet attachEntity(int32_t entity, int32_t vehicle, bool leash = 0)
    {
      Packet ret;
      ret << (int8_t)PACKET_OUT_ATTACH_ENTITY << (int32_t)entity << (int32_t)vehicle << (int8_t)leash;
      return ret;
    }

    static Packet entityLook(int eid, int yaw, int pitch)
    {
      Packet ret;
      ret << (int8_t)PACKET_OUT_ENTITY_LOOK << (int32_t)eid << (int8_t)yaw << (int8_t)pitch;
      return ret;
    }

    static Packet entityLook(int eid, float yaw, float pitch)
    {
      return entityLook(eid, angleToByte(yaw), angleToByte(pitch));
    }

    static Packet entityRelativeMove(int eid, double dx, double dy, double dz)
    {
      Packet ret;
      ret << (int8_t)PACKET_OUT_ENTITY_RELATIVE_MOVE << (int32_t)eid
          << (int8_t)(dx * 32) << (int8_t)(dy * 32) << (int8_t)(dz * 32);
      return ret;
    }
    
    static Packet entityLookRelativeMove(int eid, double dx, double dy, double dz, int yaw, int pitch)
    {
      Packet ret;
      ret << (int8_t)PACKET_OUT_ENTITY_LOOK_RELATIVE_MOVE << (int32_t)eid
          << (int8_t)(dx * 32) << (int8_t)(dy * 32) << (int8_t)(dz * 32)
          << (int8_t)yaw << (int8_t)pitch;
      return ret;
    }

    static Packet setCompression(int threshold)
    {
      Packet ret;
      ret << MS_VarInt((uint32_t)PACKET_OUT_SET_COMPRESSION) << MS_VarInt((uint32_t)threshold);
      return ret;
    }

    static Packet loginSuccess(std::string uuid, std::string username)
    {
      Packet ret;
      ret << MS_VarInt((uint32_t)PACKET_OUT_LOGIN_SUCCESS) << uuid
          << username;
      return ret;
    }

    //ToDo: remember gamemode for players
    static Packet joinGame(int eid, int8_t gamemode = 0)
    {

      Packet ret;
      ret << MS_VarInt((uint32_t)PACKET_OUT_JOIN_GAME) << (int32_t)eid
          << (int8_t)gamemode << (int8_t)0 /* dimension */
          << (int8_t)2 /*difficulty */ << (int8_t)ServerInstance->config()->iData("system.user_limit")
          << std::string("default") << (int8_t)0; /* reduced debug info */
      return ret;
    }

    static Packet playerAbilities(uint8_t flags, float flying_speed, float walking_speed)
    {

      Packet ret;
      ret << MS_VarInt((uint32_t)PACKET_OUT_PLAYER_ABILITIES) << flags
          << flying_speed << walking_speed;
      return ret;
    }

    static Packet spawnPosition(int x, int y, int z)
    {
      Packet ret;
      ret << MS_VarInt((uint32_t)PACKET_OUT_SPAWN_POSITION) << 
      (uint64_t)((((uint64_t)x & 0x3FFFFFF) << 38) | (((uint64_t)y & 0xFFF) << 26) | ((uint64_t)z & 0x3FFFFFF));
      return ret;
    }

    //ToDo: use age of the world field somewhere
    static Packet timeUpdate(int64_t time, int64_t  ageOfTheWorld = 0)
    {
      Packet ret;
      ret << MS_VarInt((uint32_t)PACKET_OUT_TIME_UPDATE) << (int64_t)ageOfTheWorld << (int64_t)time;
      return ret;
    }

    static Packet disconnect(std::string msg)
    {
      Packet ret;
      ret << MS_VarInt((uint32_t)PACKET_OUT_DISCONNECT) << msg;
      return ret;
    }

    static Packet spawnPlayer(int eid, uint8_t *uuid, std::string &nick, float health, double x, double y, double z, float yaw, float pitch, int16_t item)
    {
      Packet ret;
      ret << MS_VarInt((uint32_t)PACKET_OUT_SPAWN_PLAYER) << MS_VarInt((uint32_t)eid);
      for (int i = 0; i < 16; i++) ret << uuid[i];
      ret << (int32_t)(x*32)<<(int32_t)(y * 32) << (int32_t)(z * 32)
          << (int8_t)angleToByte(yaw) << (int8_t)angleToByte(pitch)
          << (int16_t)item
          << (uint8_t)((4<<5) | 2) // Metadata, type 4 = string, index 6 = name tag
          << nick
          << (uint8_t)((3<<5) | 6) // Metadata, type 3 = float, index 6 = health
          << health
          << (int8_t)127;
      return ret;
    }

    static Packet spawnPlayer(int eid, uint8_t *uuid, std::string &nick, float health, position& pos, int16_t item){
      Packet ret;
      ret << MS_VarInt((uint32_t)PACKET_OUT_SPAWN_PLAYER) << MS_VarInt((uint32_t)eid);
      for (int i = 0; i < 16; i++) ret << uuid[i];
      ret << (int32_t)(pos.x*32)<<(int32_t)(pos.y * 32) << (int32_t)(pos.z * 32)
          << (int8_t)angleToByte(pos.yaw) << (int8_t)angleToByte(pos.pitch)
          << (int16_t)item
          << (uint8_t)((4<<5) | 2) // Metadata, type 4 = string, index 6 = name tag
          << nick
          << (uint8_t)((3<<5) | 6) // Metadata, type 3 = float, index 6 = health
          << health
          << (int8_t)127;
      return ret;
    }

    static Packet spawnObject(int eid, int8_t object, int32_t x, int32_t y, int32_t z, int objectData, int16_t speed_x = 0, int16_t speed_y = 0,int16_t speed_z = 0, int8_t yaw = 0, int8_t pitch = 0)
    {
      Packet  pkt;  
      pkt //<< (int8_t)PACKET_OUT_ENTITY << (int32_t)eid
          << (int8_t)PACKET_OUT_SPAWN_OBJECT << (int32_t)eid << (int8_t)object << x << y << z 
          << yaw << pitch
          << (int32_t)objectData;
      if(objectData)
      {
        pkt << (int16_t)speed_x << (int16_t)speed_y << (int16_t)speed_z;
      }
      return pkt;
    }

    static Packet collectItem(int itemEid, int eid)
    {
      Packet ret;
      ret << (int8_t)PACKET_OUT_COLLECT_ITEM << (int32_t)itemEid << (int32_t)eid;
      return ret;
    }

    static Packet blockAction(int32_t x, int16_t y, int32_t z, int8_t byte1, int8_t byte2, int16_t blockid)
    {
      Packet ret;
      ret << (int8_t)PACKET_OUT_BLOCK_ACTION << x << y << z << byte1 << byte2 << blockid;
      return ret;
    }

    static Packet playerPositionAndLook(double x, double y, double z, float yaw, float pitch, uint8_t flags)
    {
      Packet ret;
      ret << MS_VarInt((uint32_t)PACKET_OUT_PLAYER_POSITION_AND_LOOK) << x << y << z << yaw << pitch << flags;
      return ret;
    }

    static Packet client_status(int32_t world = 0, int8_t difficulty=1,int8_t creative_mode=0, int16_t world_height=256, std::string level_type="default")
    {
      Packet ret;
      ret << (int8_t)PACKET_OUT_client_status << world << difficulty << creative_mode << world_height << level_type;
      return ret;
    }

    static Packet updateHealth(int health, int food=15)
    {
      Packet ret;
      ret << (int8_t)PACKET_OUT_UPDATE_HEALTH << (float)health << (int16_t)food << (float)5.0;
      return ret;
    }

    static Packet keepalive(int32_t time)
    {
      Packet ret;
      ret << MS_VarInt((uint32_t)PACKET_OUT_KEEP_ALIVE) << MS_VarInt((uint32_t)time);
      return ret;
    }

    static Packet playerlist()
    {
      Packet ret;

      //ToDo: Add admin/moderator coloring
      for (std::set<User*>::const_iterator it = ServerInstance->users().begin(); it != ServerInstance->users().end(); ++it)
      { 
        if((*it)->nick.length() > 0)
        {
          ret << (int8_t)PACKET_OUT_PLAYER_LIST_ITEM << (*it)->nick << (int8_t)1 << (int16_t)0;
        }
      }      
      return ret;
    }
    static Packet gameMode(int8_t reason, float value){
      Packet ret;
      ret<< MS_VarInt((uint32_t)PACKET_OUT_CHANGE_GAME_STATE) << reason << value;
      return ret;
    }

    static Packet chatMsg(const std::string& msg)
    {
      Packet ret;
      ret << MS_VarInt((uint32_t)PACKET_OUT_CHAT_MESSAGE) << msg << (uint8_t)0;
      return ret;
    }
};
#endif
