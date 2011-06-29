#ifndef _PROTOCOL_H
#define _PROTOCOL_H
#include "constants.h"

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

    static Packet entityLook(int eid, int yaw, int pitch) // BROKEN
    {
      Packet ret;
      ret << (int8_t)PACKET_ENTITY_LOOK << (int8_t)yaw << (int8_t)pitch;
      return ret;
    }
    
    static Packet entityRelativeMove(int eid, double dx, double dy, double dz) // BROKEN
    {
      Packet ret;
      ret << (int8_t)PACKET_ENTITY_RELATIVE_MOVE
          << (int8_t)(dx * 32) << (int8_t)(dy * 32) << (int8_t)(dz * 32);
      return ret;
    }

};
#endif
