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
    static Packet encryptionRequest();
    #endif

    static Packet openWindow(uint8_t windowId, std::string windowType, std::string title_json, int8_t numberOfSlots, int32_t EID = 0);

    static Packet windowProperty(uint8_t windowId, int16_t property, int16_t value);

    static Packet confirmTransaction(uint8_t windowId, int16_t action_number, int8_t accepted);

    static Packet animation(int eid, int aid);

    static Packet updateSign(int32_t x, int32_t y, int32_t z, 
                             std::string line1, std::string line2, std::string line3, std::string line4);

    static Packet openSignEditor(int32_t x, int32_t y, int32_t z);

    static Packet entityStatus(int eid, int aid);

    static Packet entityMetadata(int eid, MetaData& metadata);

    static Packet entityVelocity(int eid, int16_t vel_x, int16_t vel_y,int16_t vel_z);
    static Packet slot(const Item &item);

    static Packet setSlot(int8_t window_id, int16_t slotId, Item& item);

    static Packet entityEquipment(int32_t eid, int16_t slot, Item& item);

    static Packet mobSpawn(int eid, int8_t type, double x, double y, double z, int yaw, int pitch, int head_yaw, MetaData& metadata);

    static Packet mobSpawn(Mob& mob);

    static Packet destroyEntity(uint32_t eid);

    static Packet entityTeleport(uint32_t eid, double x, double y, double z, float yaw, float pitch);
    static Packet entityRelativeMove(uint32_t eid, int8_t x, int8_t y, int8_t z, int8_t onGround);

    static Packet entityTeleport(uint32_t eid, const position& pos);

    static Packet namedSoundEffect(std::string name, int32_t x, int32_t y, int32_t z, float volume, int8_t pitch);

    static Packet entityHeadLook(int eid, int head_yaw);

    static Packet attachEntity(int32_t entity, int32_t vehicle, bool leash = 0);

    static Packet entityLook(int eid, int yaw, int pitch);

    static Packet entityLook(int eid, float yaw, float pitch);

    static Packet entityRelativeMove(uint32_t eid, double dx, double dy, double dz, int8_t onGround);
    
    static Packet entityLookRelativeMove(int eid, double dx, double dy, double dz, int yaw, int pitch);

    static Packet setCompression(int threshold);

    static Packet loginSuccess(std::string uuid, std::string username);

    //ToDo: remember gamemode for players
    static Packet joinGame(int eid, int8_t gamemode = 0);

    static Packet playerAbilities(uint8_t flags, float flying_speed, float walking_speed);

    static Packet spawnPosition(int x, int y, int z);

    //ToDo: use age of the world field somewhere
    static Packet timeUpdate(int64_t time, int64_t  ageOfTheWorld = 0);
    static Packet disconnect(std::string msg);

    static Packet spawnPlayer(int eid, uint8_t *uuid, std::string &nick, float health, double x, double y, double z, float yaw, float pitch, int16_t item);

    static Packet spawnPlayer(int eid, uint8_t *uuid, std::string &nick, float health, position& pos, int16_t item);

    static Packet PlayerListItemAddSingle(uint8_t *uuid, std::string &nick, int8_t gamemode, int ping);

    static Packet PlayerListItemRemoveSingle(uint8_t *uuid);

    static Packet spawnObject(int eid, int8_t object_type, int32_t x, int32_t y, int32_t z, int objectData, int16_t speed_x = 0, int16_t speed_y = 0,int16_t speed_z = 0, float yaw = 0.0, float pitch = 0.0);

    static Packet entity(int eid);

    static Packet collectItem(int itemEid, int eid);

    static Packet blockAction(int32_t x, int16_t y, int32_t z, int8_t byte1, int8_t byte2, int16_t blockid);

    static Packet playerPositionAndLook(double x, double y, double z, float yaw, float pitch, uint8_t flags);

    static Packet respawn(int32_t dimension = 0, int8_t difficulty=1,int8_t game_mode=0, std::string level_type="default");

    static Packet updateHealth(int health, int food=15);

    static Packet keepalive(int32_t time);

    static Packet playerlist();
    static Packet gameMode(int8_t reason, float value);

    static Packet chatMsg(const std::string& msg);

    static Packet blockChange(int32_t x, int32_t y, int32_t z, int16_t blocktype, int8_t meta);

    static Packet tabComplete(const std::vector<std::string> &msgs);

    static Packet explosion(float x, float y, float z, float radius, std::vector<vec>& record, float velocity_x, float velocity_y, float velocity_z);

    static Packet windowItems(uint8_t windowID, std::vector<Item>& slots);

    static Packet chunkDataUnload(int32_t chunk_x, int32_t chunk_z);
};
#endif
