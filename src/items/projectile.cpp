#include "projectile.h"
#include "packets.h"
#include "mineserver.h"
#include "map.h"
#include <cmath>


uint32_t generateEID();

bool ItemProjectile::affectedItem(int item) const
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

  if(projID != 0)
  {
    if(projID == 60) // Bow and Arrow
    {
      bool foundArrow = false;
      // Hotbar
      for( int i = 36; i < 45; i++ )
      {
        if( user->inv[i].getType() == ITEM_ARROW )
        {
          foundArrow = true;
          user->inv[i].decCount();
          i = 45;
        }
      }
      //Inventory
      if(foundArrow == false)
      {
        for(int i = 9; i < 36; i++)
        {
          if( user->inv[i].getType() == ITEM_ARROW )
          {
            foundArrow = true;
            user->inv[i].decCount();
            i = 36;
          }
        }
      }
      if(!foundArrow)
        return;
      ServerInstance->map(user->pos.map)->sendProjectileSpawn(user, projID);
    }
    else
    {
      item->decCount();
      ServerInstance->map(user->pos.map)->sendProjectileSpawn(user, projID);
    }
  }
}
