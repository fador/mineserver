#include "projectile.h"
#include "../packets.h"
#include "../mineserver.h"
#include "../map.h"
#include <cmath>


uint32_t generateEID();

bool ItemProjectile::affectedItem(int item)
{
  switch (item)
  {
  case ITEM_SNOWBALL:
  case ITEM_EGG:
  case ITEM_BOW:
    return true;
  }
  return false;
}

void ItemProjectile::onRightClick(User* user, Item* item)
{
  int8_t projID = 0;
  switch (item->getType())
  {
  case ITEM_SNOWBALL:
    projID = 61;
    break;
  case ITEM_EGG:
    projID = 62;
    break;
  case ITEM_BOW:
    projID = 60;
    break;
  }

  // TODO check arrows and remove one!
  // instead of BOW itself here
  item->decCount();

  Mineserver::get()->map(user->pos.map)->sendProjectileSpawn(user, projID);
}
