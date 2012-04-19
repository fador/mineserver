#include <explosion.h>
#include <protocol.h>
#include <map.h>
#include <mineserver.h>
#include <logger.h>
#include "blocks/tnt.h"

namespace Explosion {
  void rb(int32_t x,int8_t y,int8_t z,int map, User* user); //rb = remove block
  uint8_t pickint;
}

void Explosion::explode(User* user, int32_t x, int8_t y, int8_t z, int map)
{
  User::sendAll(Protocol::explosionEvent(x, y, z, 3.0));
  //TODO: Damage nearby players.


  // Layer Y-3

  rb(x-1,y-3,z+1,map,user);
  rb(x,y-3,z+1,map,user);
  rb(x+1,y-3,z+1,map,user);
  rb(x-1,y-3,z,map,user);
  rb(x,y-3,z,map,user);
  rb(x+1,y-3,z,map,user);
  rb(x-1,y-3,z-1,map,user);
  rb(x,y-3,z-1,map,user);
  rb(x+1,y-3,z-1,map,user);

  // Layer Y-2

  for (int number=-2; number<=2; number++) {
    rb(x+number,y-2,z+1,map,user);
    rb(x+number,y-2,z,map,user);
    rb(x+number,y-2,z-1,map,user);
  }

  rb(x-1,y-2,z+2,map,user);
  rb(x,y-2,z+2,map,user);
  rb(x+1,y-2,z+2,map,user);

  rb(x-1,y-2,z-2,map,user);
  rb(x,y-2,z-2,map,user);
  rb(x+1,y-2,z-2,map,user);

  // Layer Y-1

  rb(x-3,y-1,z+2,map,user);
  rb(x-3,y-1,z+1,map,user);
  rb(x-3,y-1,z,map,user);
  rb(x-3,y-1,z-1,map,user);
  rb(x-3,y-1,z-2,map,user);

  for (int number=-2; number<=2; number++) {
    rb(x-number,y-1,z+3,map,user);
    rb(x-number,y-1,z+2,map,user);
    rb(x-number,y-1,z+1,map,user);
    rb(x-number,y-1,z,map,user);
    rb(x-number,y-1,z-1,map,user);
    rb(x-number,y-1,z-2,map,user);
    rb(x-number,y-1,z-3,map,user);
  }

  rb(x+3,y-1,z+2,map,user);
  rb(x+3,y-1,z+1,map,user);
  rb(x+3,y-1,z,map,user);
  rb(x+3,y-1,z-1,map,user);
  rb(x+3,y-1,z-2,map,user);


  // Layer Y {+,-} 0, same as TNT block

  //rb(x-4,y,z,map,user);

  rb(x-3,y,z+2,map,user);
  rb(x-3,y,z+1,map,user);
  rb(x-3,y,z,map,user);
  rb(x-3,y,z-1,map,user);
  rb(x-3,y,z-2,map,user);

  for (int number=-2; number<=2; number++) {
    rb(x-number,y,z+3,map,user);
    rb(x-number,y,z+2,map,user);
    rb(x-number,y,z+1,map,user);
    rb(x-number,y,z,map,user);
    rb(x-number,y,z-1,map,user);
    rb(x-number,y,z-2,map,user);
    rb(x-number,y,z-3,map,user);
  }

  rb(x+3,y,z+2,map,user);
  rb(x+3,y,z+1,map,user);
  rb(x+3,y,z,map,user);
  rb(x+3,y,z-1,map,user);
  rb(x+3,y,z-2,map,user);

//rb(x+4,y,z,map,user);

//rb(x,y,z+4,map,user);
//rb(x,y,z-4,map,user);

  // Layer Y+1

  rb(x-3,y+1,z+2,map,user);
  rb(x-3,y+1,z+1,map,user);
  rb(x-3,y+1,z,map,user);
  rb(x-3,y+1,z-1,map,user);
  rb(x-3,y+1,z-2,map,user);

  for (int number=-2; number<=2; number++) {
    rb(x-number,y+1,z+3,map,user);
    rb(x-number,y+1,z+2,map,user);
    rb(x-number,y+1,z+1,map,user);
    rb(x-number,y+1,z,map,user);
    rb(x-number,y+1,z-1,map,user);
    rb(x-number,y+1,z-2,map,user);
    rb(x-number,y+1,z-3,map,user);
  }

  rb(x+3,y+1,z+2,map,user);
  rb(x+3,y+1,z+1,map,user);
  rb(x+3,y+1,z,map,user);
  rb(x+3,y+1,z-1,map,user);
  rb(x+3,y+1,z-2,map,user);

  // Layer Y+2

  for (int number=-2; number<=2; number++) {
    rb(x+number,y+2,z+1,map,user);
    rb(x+number,y+2,z,map,user);
    rb(x+number,y+2,z-1,map,user);
  }

  rb(x-1,y+2,z+2,map,user);
  rb(x,y+2,z+2,map,user);
  rb(x+1,y+2,z+2,map,user);

  rb(x-1,y+2,z-2,map,user);
  rb(x,y+2,z-2,map,user);
  rb(x+1,y+2,z-2,map,user);

  // Layer Y+3

  rb(x-1,y+3,z+1,map,user);
  rb(x,y+3,z+1,map,user);
  rb(x+1,y+3,z+1,map,user);
  rb(x-1,y+3,z,map,user);
  rb(x,y+3,z,map,user);
  rb(x+1,y+3,z,map,user);
  rb(x-1,y+3,z-1,map,user);
  rb(x,y+3,z-1,map,user);
  rb(x+1,y+3,z-1,map,user);

  // Layer Y+4

//rb(x,y+4,z,map,user);
  
  LOG2(INFO,"TNT Block exploded!");
}

void Explosion::rb(int32_t x, int8_t y, int8_t z, int map, User* user)
{
  uint8_t block, meta, count;
  int16_t item;

  Mineserver::get()->map(map)->getBlock(x,y,z,&block,&meta);

  BLOCKDROPS[block]->getDrop(item, count, meta);

  // Undestroyable blocks
  if (block == BLOCK_AIR || block == BLOCK_BEDROCK || block == BLOCK_OBSIDIAN || block == BLOCK_WATER || block == BLOCK_STATIONARY_WATER || block == BLOCK_LAVA || block == BLOCK_STATIONARY_LAVA)
  {
    return;
  }
  else
  {
    Mineserver::get()->map(map)->setBlock(x,y,z,0,0);
    Mineserver::get()->map(map)->sendBlockChange(x,y,z,0,0);
  }
  
  if(block == BLOCK_TNT)
  {
    //explode(user, x, y, z, map);
    BlockTNT tnt;
    tnt.onStartedDigging(user, BLOCK_STATUS_BLOCK_BROKEN, x, y, z, map, BLOCK_TOP);
    return;
  } 
  
  // Pickup Spawn Area
  // The integer "pickint" is used to spawn 1/5 of the blocks, otherwise there would be too much pickups!
  if(pickint == 5)
  {
    if(count) {
      Mineserver::get()->map(map)->createPickupSpawn(x, y, z, item, count, meta, user);
      pickint=0;
    }
  } else {
    pickint++;
  }

}
