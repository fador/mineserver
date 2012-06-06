#include <entity.h>
#include <user.h>
#include <protocol.h>

Entity::Entity()
  :
  UID(Mineserver::generateEID()),
  type(0),
  x(0),
  y(0),
  z(0),
  spawned(false)
{  
}

Entity::Entity(int8_t type, int32_t x, int32_t y, int32_t z)
  :
  UID(Mineserver::generateEID()),
  type(type),
  x(x),
  y(y),
  z(z),
  spawned(false)
{  
}

void Entity::deSpawnToAll()
{
  User::sendAll(Protocol::destroyEntity(UID));
  spawned = false;
}