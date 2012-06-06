#include <vehicle.h>
#include <user.h>
#include <protocol.h>

void Vehicle::spawnToAll()
{
  User::sendAll(Protocol::spawnObject(UID, type, x, y, z));
  spawned = true;
}

Vehicle::Vehicle()
  : 
  Entity()
{
}

Vehicle::Vehicle(int8_t type, int32_t x,int32_t y,int32_t z)
  : 
  Entity(type, x, y, z)
{
}