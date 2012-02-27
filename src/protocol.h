#ifndef _PROTOCOL_H
#define _PROTOCOL_H
#include "constants.h"
#include "mob.h"
#include "tools.h"
#include "utf8.h"

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
    static Packet armAnimation(int eid, int aid)
    {
      Packet ret;
      ret << (int8_t)PACKET_ARM_ANIMATION << (int32_t)eid << (int8_t)aid;
      return ret;
    }

    static Packet deathAnimation(int eid, int aid)
    {
      Packet ret;
      ret << (int8_t)PACKET_DEATH_ANIMATION << (int32_t)eid << (int8_t)aid;
      return ret;
    }

    static Packet entityMetadata(int eid, MetaData& metadata)
    {
      Packet ret;
      ret << (int8_t)PACKET_ENTITY_METADATA << (int32_t)eid << metadata;
      return ret;
    }

    static Packet mobSpawn(int eid, int8_t type, double x, double y, double z, int yaw, int pitch, MetaData& metadata)
    {
      // Warning! This converts absolute double coordinates to absolute integer coordinates!
      Packet ret;
      ret << (int8_t)PACKET_MOB_SPAWN << (int32_t)eid << (int8_t)type
          << (int32_t)(x * 32) << (int32_t)(y * 32) << (int32_t)(z * 32)
          << (int8_t)yaw << (int8_t)pitch << metadata;
      return ret;
    }

    static Packet mobSpawn(Mob& mob)
    {
      return Protocol::mobSpawn(mob.UID, mob.type, mob.x, mob.y, mob.z, mob.yaw, mob.pitch, mob.metadata);
    }

    static Packet destroyEntity(int eid)
    {
      Packet ret;
      ret << (int8_t)PACKET_DESTROY_ENTITY << (int32_t)eid;
      return ret;
    }

    static Packet entityTeleport(int eid, double x, double y, double z, int yaw, int pitch)
    {
      Packet ret;
      ret << (int8_t)PACKET_ENTITY_TELEPORT << (int32_t)eid
          << (int32_t)(x * 32) << (int32_t)(y * 32) << (int32_t)(z * 32)
          << (int8_t)yaw << (int8_t)pitch;
      return ret;
    }

    static Packet entityLook(int eid, int yaw, int pitch)
    {
      Packet ret;
      ret << (int8_t)PACKET_ENTITY_LOOK << (int32_t)eid << (int8_t)yaw << (int8_t)pitch;
      return ret;
    }
    
    static Packet entityLook(int eid, double yaw, double pitch)
    {
      return entityLook(eid, angleToByte(yaw), angleToByte(pitch));
    }

    static Packet entityRelativeMove(int eid, double dx, double dy, double dz)
    {
      Packet ret;
      ret << (int8_t)PACKET_ENTITY_RELATIVE_MOVE << (int32_t)eid
          << (int8_t)(dx * 32) << (int8_t)(dy * 32) << (int8_t)(dz * 32);
      return ret;
    }
    
    static Packet entityLookRelativeMove(int eid, double dx, double dy, double dz, int yaw, int pitch)
    {
      Packet ret;
      ret << (int8_t)PACKET_ENTITY_LOOK_RELATIVE_MOVE << (int32_t)eid
          << (int8_t)(dx * 32) << (int8_t)(dy * 32) << (int8_t)(dz * 32)
          << (int8_t)yaw << (int8_t)pitch;
      return ret;
    }

    static Packet loginResponse(int eid)
    {
      Packet ret;
      ret << (int8_t)PACKET_LOGIN_RESPONSE << (int32_t)eid << std::string("") << (int64_t)0 
          << std::string("DEFAULT") << (int32_t)0 << (int8_t)0 << (int8_t)2 << (int8_t)128 << (int8_t)64;
      return ret;
    }

    static Packet spawnPosition(int x, int y, int z)
    {
      Packet ret;
      ret << (int8_t)PACKET_SPAWN_POSITION << (int32_t)x << (int32_t)y << (int32_t)z;
      return ret;
    }

    static Packet timeUpdate(int64_t time)
    {
      Packet ret;
      ret << (int8_t)PACKET_TIME_UPDATE << (int64_t)time;
      return ret;
    }

    static Packet kick(std::string msg)
    {
      Packet ret;
      ret << (int8_t)PACKET_KICK << msg;
      return ret;
    }

    static Packet namedEntitySpawn(int eid, std::string nick, double x, double y, double z, int yaw, int pitch, int item)
    {
      Packet ret;
      ret << (int8_t)PACKET_NAMED_ENTITY_SPAWN << (int32_t)eid << nick
          << (int32_t)(x * 32) << (int32_t)(y * 32) << (int32_t)(z * 32)
          << (int8_t)yaw << (int8_t)pitch << (int16_t)item;
      return ret;
    }

    static Packet collectItem(int itemEid, int eid)
    {
      Packet ret;
      ret << (int8_t)PACKET_COLLECT_ITEM << (int32_t)itemEid << (int32_t)eid;
      return ret;
    }

    static Packet preChunk(int x, int z, bool create)
    {
      Packet ret;
      ret << (int8_t)PACKET_PRE_CHUNK << (int32_t)x << (int32_t)z << (int8_t)(create ? 1 : 0);
      return ret;
    }

    static Packet playerPositionAndLook(double x, double y, double stance, double z, float yaw, float pitch, bool onGround)
    {
      Packet ret;
      ret << (int8_t)PACKET_PLAYER_POSITION_AND_LOOK << x << y << stance << z << yaw << pitch << (int8_t)(onGround ? 1 : 0);
      return ret;
    }

    static Packet respawn(int world = 0)
    {
      Packet ret;
      ret << (int8_t)PACKET_RESPAWN << (int8_t)world;
      return ret;
    }

    static Packet updateHealth(int health, int food=20)
    {
      Packet ret;
      ret << (int8_t)PACKET_UPDATE_HEALTH << (int16_t)health << (int16_t)food << (float)5.0;
      return ret;
    }

    static Packet entityEquipment(int eid, int slot, int type, int damage)
    {
      Packet ret;
      ret << (int8_t)PACKET_ENTITY_EQUIPMENT << (int32_t)eid << (int16_t)slot << (int16_t)type << (int16_t)damage;
      return ret;
    }
};
#endif
