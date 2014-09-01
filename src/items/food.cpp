#include "food.h"
#include "packets.h"
#include "protocol.h"
#include "map.h"
#include "metadata.h"

bool ItemFood::affectedItem(int item) const
{
  switch (item)
  {
  case ITEM_GOLDEN_APPLE:
  case ITEM_MUSHROOM_SOUP:
  case ITEM_PORK:
  case ITEM_GRILLED_PORK:
  case ITEM_BREAD:
  case ITEM_COOKED_FISH:
  case ITEM_RAW_FISH:
  case ITEM_APPLE:
    return true;
  }
  return false;
}

void ItemFood::onRightClick(User* user, Item* item)
{
  int healammount = 0;
  switch (item->getType())
  {
  case ITEM_GOLDEN_APPLE:
    healammount = 20;
    break;
  case ITEM_MUSHROOM_SOUP:
    healammount = 10;
    break;
  case ITEM_GRILLED_PORK:
    healammount = 8;
    break;
  case ITEM_PORK:
    healammount = 3;
    break;
  case ITEM_BREAD:
    healammount = 5;
    break;
  case ITEM_COOKED_FISH:
    healammount = 5;
    break;
  case ITEM_RAW_FISH:
    healammount = 2;
    break;
  case ITEM_APPLE:
    healammount = 4;
    break;
  }

  int newhealth = user->health + healammount;
  if (newhealth > 20)
  {
    newhealth = 20;
  }

  user->sethealth(newhealth);
   
  //Accept eating
  user->buffer << Protocol::entityStatus(user->UID, 9);

  //Eating animation
  MetaData meta;
  MetaDataElemByte *element = new MetaDataElemByte(0,0x10);
  meta.set(element);
  Packet pkt = Protocol::animation(user->UID,5);
  //pkt << Protocol::entityMetadata(user->UID, meta);
  //ToDo: add timer stop animation
  
  sChunk* chunk = ServerInstance->map(user->pos.map)->getChunk(blockToChunk((int32_t)user->pos.x), blockToChunk((int32_t)user->pos.z));
  if (chunk != NULL)
  {
    chunk->sendPacket(pkt);
  }

  if(item->getCount() > 1)
  {
    item->setCount(item->getCount()-1);
  }
  else
  {
    item->setType(-1);
    item->setCount(0);
  }
}

