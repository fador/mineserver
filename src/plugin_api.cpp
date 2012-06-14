/*
  Copyright (c) 2011, The Mineserver Project
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

#include "sys/stat.h"

#include "mineserver.h"
#include "logger.h"
#include "chat.h"
#include "permissions.h"

#include "plugin.h"
#include "config.h"
#include "map.h"
#include "mob.h"
#include "random.h"
#include "blocks/default.h"
#include "blocks/falling.h"
#include "blocks/torch.h"
#include "blocks/plant.h"
#include "blocks/snow.h"
#include "blocks/liquid.h"
#include "blocks/fire.h"
#include "blocks/stair.h"
#include "blocks/door.h"
#include "blocks/sign.h"
#include "blocks/tracks.h"
#include "blocks/chest.h"
#include "blocks/note.h"
#define MINESERVER
#include "plugin_api.h"

mineserver_pointer_struct plugin_api_pointers;

// HELPER FUNCTIONS
User* userFromName(std::string user)
{
  for (std::set<User*>::const_iterator it = ServerInstance->users().begin(); it != ServerInstance->users().end(); ++it)
  {
    // Don't send to his user if he is DND and the message is a chat message
    if (((*it)->fd && (*it)->logged && user == (*it)->nick))
    {
      return *it;
    }
  }
  return NULL;
}

// PLUGIN_API FUNCTIONS

bool plugin_hasPluginVersion(const char* name)
{
  return ServerInstance->plugin()->hasPluginVersion(std::string(name));
}

float plugin_getPluginVersion(const char* name)
{
  return ServerInstance->plugin()->getPluginVersion(std::string(name));
}

void plugin_setPluginVersion(const char* name, float version)
{
  ServerInstance->plugin()->setPluginVersion(std::string(name), version);
}

void plugin_remPluginVersion(const char* name)
{
  ServerInstance->plugin()->remPluginVersion(std::string(name));
}

bool plugin_hasPointer(const char* name)
{
  return ServerInstance->plugin()->hasPointer(std::string(name));
}

void* plugin_getPointer(const char* name)
{
  return ServerInstance->plugin()->getPointer(std::string(name));
}

void plugin_setPointer(const char* name, void* pointer)
{
  ServerInstance->plugin()->setPointer(std::string(name), pointer);
}

void plugin_remPointer(const char* name)
{
  ServerInstance->plugin()->remPointer(std::string(name));
}

bool plugin_hasHook(const char* hookID)
{
  return ServerInstance->plugin()->hasHook(hookID);
}

Hook* plugin_getHook(const char* hookID)
{
  return ServerInstance->plugin()->getHook(hookID);
}

void plugin_setHook(const char* hookID, Hook* hook)
{
  ServerInstance->plugin()->setHook(hookID, hook);
}

void plugin_remHook(const char* hookID)
{
  ServerInstance->plugin()->remHook(hookID);
}

bool hook_hasCallback(const char* hookID, voidF function)
{
  return ServerInstance->plugin()->getHook(hookID)->hasCallback(function);
}

void hook_addCallback(const char* hookID, voidF function)
{
  ServerInstance->plugin()->getHook(hookID)->addCallback(function);
}

void hook_addIdentifiedCallback(const char* hookID, void* identifier, voidF function)
{
  ServerInstance->plugin()->getHook(hookID)->addIdentifiedCallback(identifier, function);
}

void hook_remCallback(const char* hookID, voidF function)
{
  ServerInstance->plugin()->getHook(hookID)->remCallback(function);
}

bool hook_doUntilTrue(const char* hookID, ...)
{
  bool result = false;
  va_list argList;
  va_start(argList, hookID);
  result = ServerInstance->plugin()->getHook(hookID)->doUntilTrueVA(argList);
  va_end(argList);
  return result;
}

bool hook_doUntilFalse(const char* hookID, ...)
{
  bool result = false;
  va_list argList;
  va_start(argList, hookID);
  result = ServerInstance->plugin()->getHook(hookID)->doUntilFalseVA(argList);
  va_end(argList);
  return result;
}

void hook_doAll(const char* hookID, ...)
{
  va_list argList;
  va_start(argList, hookID);
  ServerInstance->plugin()->getHook(hookID)->doAllVA(argList);
  va_end(argList);
}

// LOGGER WRAPPER FUNCTIONS
void logger_log(int type, const char* source, const char* message)
{
  ServerInstance->logger()->log((LogType::LogType)type, std::string(source), std::string(message));
}

// CHAT WRAPPER FUNCTIONS
bool chat_sendmsgTo(const char* user, const char* msg)
{
  const std::string userStr(user);

  if (userStr == "[Server]")
  {
    LOG(INFO, "Chat", msg);
    return true;
  }

  for (std::set<User*>::const_iterator it = ServerInstance->users().begin(); it != ServerInstance->users().end(); ++it)
  {
    // Don't send to his user if he is DND and the message is a chat message
    if ((*it)->fd && (*it)->logged && userStr == (*it)->nick)
    {
      (*it)->buffer << (int8_t)PACKET_CHAT_MESSAGE << std::string(msg);
      return true;
    }
  }

  return false;
}

bool chat_sendmsg(const char* msg)
{
  const std::string msgStr(msg);

  for (std::set<User*>::const_iterator it = ServerInstance->users().begin(); it != ServerInstance->users().end(); ++it)
  {
    // Don't send to his user if he is DND and the message is a chat message
    if ((*it)->fd && (*it)->logged && !(*it)->dnd)
    {
      (*it)->buffer << (int8_t)PACKET_CHAT_MESSAGE << msgStr;
    }
  }

  return true;
}

bool chat_sendUserlist(const char* user)
{
  const std::string userStr(user);

  User* userPtr = userFromName(userStr);

  if (userPtr != NULL)
  {
    ServerInstance->chat()->sendUserlist(userPtr);
    return true;
  }

  return false;
}

bool chat_handleMessage(const char* username, const char* message)
{
  if (strcmp(username, "[Server]") == 0)
  {
    User serverUser(-1, SERVER_CONSOLE_UID);
    serverUser.changeNick("[Server]");

    ServerInstance->chat()->handleMsg(&serverUser, message);
  }
  else
  {
    User* user = userFromName(std::string(username));
    if (user != NULL)
    {
      ServerInstance->chat()->handleMsg(user, message);
    }
  }
  return false;
}

// MAP WRAPPER FUNCTIONS
bool map_setTime(int timeValue)
{
  ServerInstance->map(0)->mapTime = timeValue;
  Packet pkt;
  pkt << (int8_t)PACKET_TIME_UPDATE << (int64_t)ServerInstance->map(0)->mapTime;

  if (!User::all().empty())
  {
    (*User::all().begin())->sendAll(pkt);
  }
  return true;
}

int map_getTime()
{
  return (int64_t)ServerInstance->map(0)->mapTime;
}

void map_createPickupSpawn(int x, int y, int z, int type, int count, int health, const char* user)
{
  User* tempUser = NULL;
  if (user != NULL)
  {
    tempUser = userFromName(std::string(user));
    ServerInstance->map(tempUser->pos.map)->createPickupSpawn(x, y, z, type, count, health, tempUser);

  }
  else
  {
    ServerInstance->map(0)->createPickupSpawn(x, y, z, type, count, health, NULL);
  }
}

void map_getSpawn(int* x, int* y, int* z)
{
  *x = ServerInstance->map(0)->spawnPos.x();
  *y = ServerInstance->map(0)->spawnPos.y();
  *z = ServerInstance->map(0)->spawnPos.z();
}

void map_setSpawn(int x, int y, int z)
{
  ServerInstance->map(0)->spawnPos.x() = x;
  ServerInstance->map(0)->spawnPos.y() = y;
  ServerInstance->map(0)->spawnPos.z() = z;
}

bool map_getBlock(int x, int y, int z, unsigned char* type, unsigned char* meta)
{
  return ServerInstance->map(0)->getBlock(x, y, z, type, meta);
}

bool map_setBlock(int x, int y, int z, unsigned char type, unsigned char meta)
{
  ServerInstance->map(0)->sendBlockChange(x, y, z, type, meta);
  return ServerInstance->map(0)->setBlock(x, y, z, type, meta);
}

bool map_getBlockW(int x, int y, int z, int w, unsigned char* type, unsigned char* meta)
{
  return ServerInstance->map(w)->getBlock(x, y, z, type, meta);
}

bool map_setBlockW(int x, int y, int z, int w, unsigned char type, unsigned char meta)
{
  ServerInstance->map(w)->sendBlockChange(x, y, z, type, meta);
  return ServerInstance->map(w)->setBlock(x, y, z, type, meta);
}


void map_saveWholeMap(void)
{
  ServerInstance->saveAll();
}

unsigned char* map_getMapData_block(int x, int z)
{
  sChunk* chunk = ServerInstance->map(0)->getMapData(x, z);
  if (chunk != NULL)
  {
    return chunk->blocks;
  }
  return NULL;
}
unsigned char* map_getMapData_meta(int x, int z)
{
  sChunk* chunk = ServerInstance->map(0)->getMapData(x, z);
  if (chunk != NULL)
  {
    return chunk->data;
  }
  return NULL;
}
unsigned char* map_getMapData_skylight(int x, int z)
{
  sChunk* chunk = ServerInstance->map(0)->getMapData(x, z);
  if (chunk != NULL)
  {
    return chunk->skylight;
  }
  return NULL;
}
unsigned char* map_getMapData_blocklight(int x, int z)
{
  sChunk* chunk = ServerInstance->map(0)->getMapData(x, z);
  if (chunk != NULL)
  {
    return chunk->blocklight;
  }
  return NULL;
}

// USER WRAPPER FUNCTIONS
bool user_toggleDND(const char* user)
{
  const std::string username(user);

  for (std::set<User*>::const_iterator it = ServerInstance->users().begin(); it != ServerInstance->users().end(); ++it)
  {
    if ((*it)->fd && (*it)->logged && username == (*it)->nick)
    {
      (*it)->toggleDND();
      return true;
    }
  }

  return false;
}

bool user_getPosition(const char* user, double* x, double* y, double* z, float* yaw, float* pitch, double* stance)
{
  const std::string userStr(user);

  for (std::set<User*>::const_iterator it = ServerInstance->users().begin(); it != ServerInstance->users().end(); ++it)
  {
    //Is this the user?
    if ((*it)->fd && (*it)->logged && userStr == (*it)->nick)
    {
      //For safety, check for NULL pointers!
      if (x != NULL)  *x = (*it)->pos.x;
      if (y != NULL)  *y = (*it)->pos.y;
      if (z != NULL)  *z = (*it)->pos.z;
      if (yaw != NULL)    *yaw    = (*it)->pos.yaw;
      if (pitch != NULL)  *pitch  = (*it)->pos.pitch;
      if (stance != NULL) *stance = (*it)->pos.stance;

      return true;
    }
  }

  return false;
}

bool user_getPositionW(const char* user, double* x, double* y, double* z, int* w, float* yaw, float* pitch, double* stance)
{
  const std::string userStr(user);

  for (std::set<User*>::const_iterator it = ServerInstance->users().begin(); it != ServerInstance->users().end(); ++it)
  {
    //Is this the user?
    if ((*it)->fd && (*it)->logged && userStr == (*it)->nick)
    {
      //For safety, check for NULL pointers!
      if (x != NULL)      *x = (*it)->pos.x;
      if (y != NULL)      *y = (*it)->pos.y;
      if (z != NULL)      *z = (*it)->pos.z;
      if (yaw != NULL)    *yaw = (*it)->pos.yaw;
      if (pitch != NULL)  *pitch = (*it)->pos.pitch;
      if (stance != NULL) *stance = (*it)->pos.stance;
      if (w != NULL)      *w = (*it)->pos.map;

      return true;
    }
  }
  return false;
}

bool user_teleport(const char* user, double x, double y, double z)
{
  User* tempUser = userFromName(std::string(user));
  if (tempUser != NULL)
  {
    tempUser->teleport(x, y, z);
    return true;
  }
  return false;
}

bool user_teleportMap(const char* user, double x, double y, double z, size_t map)
{
  if (map >= ServerInstance->mapCount())
  {
    return false;
  }
  User* tempUser = userFromName(std::string(user));
  if (tempUser != NULL && map != tempUser->pos.map)
  {
    tempUser->teleport(x, y, z, map);
    return true;
  }
  return false;
}

bool user_sethealth(const char* user, int userHealth)
{
  User* tempUser = userFromName(std::string(user));
  if (tempUser != NULL)
  {
    tempUser->sethealth(userHealth);
    return true;
  }
  return false;
}

int user_gethealth(const char* user)
{
  User* tempUser = userFromName(std::string(user));
  if (tempUser != NULL)
  {
    return tempUser->health;
  }
  return 0;
}

int user_getCount()
{
  return ServerInstance->users().size();
}

const char* user_getUserNumbered(int c)
{
  std::set<User*>::const_iterator it = ServerInstance->users().begin();
  std::advance(it, c);
  return (*it)->nick.c_str();
}

bool user_getItemInHand(const char* user, int* type, int* meta, int* quant)
{
  User* tempUser = userFromName(std::string(user));
  if (tempUser != NULL)
  {
    Item item = tempUser->inv[tempUser->curItem + 36];
    if (type != NULL)
    {
      *type = item.getType();
    }
    if (meta != NULL)
    {
      *meta = item.getHealth();
    }
    if (quant != NULL)
    {
      *quant = item.getCount();
    }
    return true;
  }
  return false;
}

bool user_setItemInHand(const char* user, int type, int meta, int quant)
{
  if (quant > 64)
  {
    quant = 64;
  }
  if (quant < 0)
  {
    quant = 0;
  }
  User* tempUser = userFromName(std::string(user));
  if (tempUser != NULL)
  {
    Item* item = &tempUser->inv[tempUser->curItem + 36];
    item->setHealth(meta);
    item->setCount(quant);
    if (item->getType() != -1)
    {
      item->setType(type);
    }
    return true;
  }
  return false;
}

bool user_addItem(const char* user, int item, int count, int health)
{
  int total = count;
  User* tempuser = userFromName(std::string(user));
  if (tempuser != NULL)
  {
    bool checkingTaskbar = true;
    for (uint8_t i = 36 - 9; i < 36 - 9 || checkingTaskbar; i++)
    {
      if (i == 36)
      {
        checkingTaskbar = false;
        i = 0;
      }
      Item* slot = &tempuser->inv[i + 9];
      if (item == slot->getType() && health == slot->getHealth())
      {
        if (slot->getCount() < 64)
        {
          int a = 64 - slot->getCount();
          if (a < total)
          {
            total -= a;
            slot->setCount(64);
          }
          else
          {
            slot->decCount(-total);
            total = 0;
          }
        }
      }
      else if (slot->getType() == -1)
      {
        slot->setType(item);
        slot->setHealth(health);
        if (total < 65)
        {
          slot->setCount(total);
          total = 0;
        }
        else
        {
          slot->setCount(64);
          total -= 64;
        }
      }
      if (total == 0)
      {
        return true;
      }
    }
  }
  return false;
}

bool user_hasItem(const char* user, int item, int count, int health)
{
  User* tempuser = userFromName(std::string(user));
  if (tempuser == NULL)
  {
    return false;
  }
  bool checkingTaskbar = true;
  int total = 0;

  for (uint8_t i = 36 - 9; i < 36 - 9 || checkingTaskbar; i++)
  {
    //First, the "task bar"
    if (i == 36)
    {
      checkingTaskbar = false;
      i = 0;
    }
    Item* slot = &tempuser->inv[i + 9];
    if (item == slot->getType() && (health == slot->getHealth() || health == -1))
    {
      total += slot->getCount();
      if (total >= count)
      {
        return true;
      }
    }
  }
  return false;
}

bool user_delItem(const char* user, int item, int count, int health)
{
  User* tempuser = userFromName(std::string(user));
  if (tempuser == NULL)
  {
    return false;
  }
  bool checkingTaskbar = true;
  int total = count;

  for (uint8_t i = 36 - 9; i < 36 - 9 || checkingTaskbar; i++)
  {
    //First, the "task bar"
    if (i == 36)
    {
      checkingTaskbar = false;
      i = 0;
    }
    Item* slot = &tempuser->inv[i + 9];
    if (item == slot->getType() && (health == slot->getHealth() || health == -1))
    {
      if (slot->getCount() > total)
      {
        int a = slot->getCount();
        slot->decCount(total);
        total -= a;
        return true;
      }
      else
      {
        total -= slot->getCount();
        slot->setType(-1);
      }
    }
  }
  return false;
}

bool user_kick(const char* user)
{
  User* tempuser = userFromName(std::string(user));
  if (tempuser == NULL)
  {
    return false;
  }
  tempuser->kick("You have been kicked!"); // Need to allow other languages.
  return true;
}

bool user_getItemAt(const char* user, int slotn, int* type, int* meta, int* quant)
{
  User* tempuser = userFromName(std::string(user));
  if (tempuser == NULL)
  {
    return false;
  }
  Item* slot = &tempuser->inv[slotn];
  if (type != NULL)
  {
    *type = slot->getType();
  }
  if (meta != NULL)
  {
    *meta = slot->getHealth();
  }
  if (quant != NULL)
  {
    *quant = slot->getCount();
  }
  return true;
}

bool user_setItemAt(const char* user, int slotn, int type, int meta, int quant)
{
  User* tempuser = userFromName(std::string(user));
  if (tempuser == NULL)
  {
    return false;
  }
  Item* slot = &tempuser->inv[slotn];
  slot->setType(type);
  if (slot->getType() != -1)
  {
    slot->setHealth(meta);
    slot->setCount(quant);
  }
  return true;
}

// CONFIG WRAPPER FUNCTIONS
bool config_has(const char* name)
{
  return ServerInstance->config()->has(std::string(name));
}

int config_iData(const char* name)
{
  return ServerInstance->config()->iData(std::string(name));
}

int64_t config_lData(const char* name)
{
  return ServerInstance->config()->lData(std::string(name));
}

float config_fData(const char* name)
{
  return ServerInstance->config()->fData(std::string(name));
}

double config_dData(const char* name)
{
  return ServerInstance->config()->dData(std::string(name));
}

const char* config_sData(const char* name)
{
  return ServerInstance->config()->sData(std::string(name)).c_str();
}

bool config_bData(const char* name)
{
  return ServerInstance->config()->bData(std::string(name));
}

int mob_createMob(int type)
{
  MobPtr m = ServerInstance->mobs()->createMob();
  m->type = type;
  return ServerInstance->mobs()->getAll().size() - 1;
}

int mob_createSpawnMob(int type)
{
  MobPtr m = ServerInstance->mobs()->createMob();
  m->type = type;
  m->spawnToAll();
  m->teleportToAll();
  return ServerInstance->mobs()->getAll().size() - 1;
}

void mob_spawnMob(int uid)
{
  MobPtr m = ServerInstance->mobs()->getMobByID(uid);
  m->spawnToAll();
}

void mob_despawnMob(int uid)
{
  MobPtr m = ServerInstance->mobs()->getMobByID(uid);
  m->deSpawnToAll();
}

void mob_moveMob(int uid, double x, double y, double z)
{
  MobPtr m = ServerInstance->mobs()->getMobByID(uid);
  m->moveTo(x, y, z, -1);
}

void mob_moveMobW(int uid, double x, double y, double z, int map)
{
  MobPtr m = ServerInstance->mobs()->getMobByID(uid);
  m->moveTo(x, y, z, map);
}

int mob_getMobID(int uid)
{
  MobPtr m = ServerInstance->mobs()->getMobByID(uid);
  return m->UID;
}

int mob_getHealth(int uid)
{
  MobPtr m = ServerInstance->mobs()->getMobByID(uid);
  return m->health;
}

void mob_setHealth(int uid, int mobHealth)
{
  ServerInstance->mobs()->getMobByID(uid)->sethealth(mobHealth);
}

void mob_moveAnimal(const char*, size_t mobID)
{
  MobPtr m = ServerInstance->mobs()->getMobByID((int)mobID);
  m->moveAnimal();
}

void mob_animateMob(const char*, size_t mobID, int animID) 
{
  MobPtr m = ServerInstance->mobs()->getMobByID(mobID);
  m->animateMob(animID);
}

void mob_animateDamage(const char*, size_t mobID, int animID) 
{
 MobPtr m = ServerInstance->mobs()->getMobByID(mobID);
 m->animateDamage(animID);
}

int mob_getType(size_t uid)
{
  MobPtr m = ServerInstance->mobs()->getMobByID(uid);
  return m->type;
}

bool mob_getLook(int uid, double* rot, double* pitch, double* head_yaw)
{
  MobPtr m = ServerInstance->mobs()->getMobByID(uid);
  if (m != NULL)
  {
    if (rot != NULL)
    {
      *rot = (double)((m->yaw * 1.0) * 360.0 / 256.0);
    }
    if (pitch != NULL)
    {
      *pitch = (double)((m->pitch * 1.0) * 360.0 / 256.0);
    }
    if (head_yaw != NULL)
    {
      *head_yaw = (double)((m->head_yaw * 1.0) * 360.0 / 256.0);
    }
    return true;
  }
  return false;
}

bool mob_setLook(int uid, double rot, double pitch, double head_yaw)
{
  MobPtr m = ServerInstance->mobs()->getMobByID(uid);
  if (m != NULL)
  {
    m->look((int16_t)rot, (int16_t)pitch);
    m->headLook((int16_t)head_yaw);
    return true;
  }
  return false;
}


bool mob_getMobPositionW(int uid, double* x, double* y, double* z, int* w)
{
  MobPtr m = ServerInstance->mobs()->getMobByID(uid);
  if (m != NULL)
  {
    if (w != NULL)
    {
      *w = m->map;
    }
    if (x != NULL)
    {
      *x = m->x;
    }
    if (y != NULL)
    {
      *y = m->y;
    }
    if (z != NULL)
    {
      *z = m->z;
    }
    return true;
  }
  return false;
}

bool mob_setByteMetadata(int uid, int8_t idx, int8_t val)
{
  MobPtr m = ServerInstance->mobs()->getMobByID(uid);
  if (!m) return false;
  m->metadata.set(new MetaDataElemByte(idx, val));
  return true;
}

bool mob_updateMetadata(int uid)
{
  MobPtr m = ServerInstance->mobs()->getMobByID(uid);
  if (!m) return false;
  m->updateMetadata();
  return true;
}

int8_t mob_getByteMetadata(int uid, int idx)
{
  MobPtr m = ServerInstance->mobs()->getMobByID(uid);
  if (!m) return false;
  MetaDataElemPtr x = m->metadata.get(idx);
  MetaDataElemByte* data = dynamic_cast<MetaDataElemByte*>(x.get());
  if (!data) return 0;
  return data->val;
}

bool permission_setAdmin(const char* name)
{
  User* tempuser = userFromName(std::string(name));
  if (tempuser == NULL)
  {
    return false;
  }
  SET_ADMIN(tempuser->permissions);
  return true;
}

bool permission_setOp(const char* name)
{
  User* tempuser = userFromName(std::string(name));
  if (tempuser == NULL)
  {
    return false;
  }
  tempuser->permissions = 0; // reset any previous permissions
  SET_OP(tempuser->permissions);
  return true;
}

bool permission_setMember(const char* name)
{
  User* tempuser = userFromName(std::string(name));
  if (tempuser == NULL)
  {
    return false;
  }
  tempuser->permissions = 0; // reset any previous permissions
  SET_MEMBER(tempuser->permissions);
  return true;
}

bool permission_setGuest(const char* name)
{
  User* tempuser = userFromName(std::string(name));
  if (tempuser == NULL)
  {
    return false;
  }
  tempuser->permissions = 0; // reset any previous permissions
  SET_GUEST(tempuser->permissions);
  return true;
}

bool permission_isAdmin(const char* name)
{
  User* tempuser = userFromName(std::string(name));
  if (tempuser == NULL)
  {
    return false;
  }
  return IS_ADMIN(tempuser->permissions);
}

bool permission_isOp(const char* name)
{
  User* tempuser = userFromName(std::string(name));
  if (tempuser == NULL)
  {
    return false;
  }
  return IS_OP(tempuser->permissions);
}

bool permission_isMember(const char* name)
{
  User* tempuser = userFromName(std::string(name));
  if (tempuser == NULL)
  {
    return false;
  }
  return IS_MEMBER(tempuser->permissions);
}

bool permission_isGuest(const char* name)
{
  User* tempuser = userFromName(std::string(name));
  if (tempuser == NULL)
  {
    return false;
  }
  return IS_GUEST(tempuser->permissions);
}

int tools_uniformInt(int a, int b)
{
  if (a < 0)
  {
    return uniformUINT(0, b - a) + a;
  }
  return uniformUINT(a, b);
}

double tools_uniform01()
{
  return uniform01();
}

void init_plugin_api(void)
{
  plugin_api_pointers.logger.log                   = &logger_log;

  plugin_api_pointers.chat.sendmsg                 = &chat_sendmsg;
  plugin_api_pointers.chat.sendmsgTo               = &chat_sendmsgTo;
  plugin_api_pointers.chat.sendUserlist            = &chat_sendUserlist;
  plugin_api_pointers.chat.handleMessage           = &chat_handleMessage;

  plugin_api_pointers.plugin.hasPluginVersion      = &plugin_hasPluginVersion;
  plugin_api_pointers.plugin.getPluginVersion      = &plugin_getPluginVersion;
  plugin_api_pointers.plugin.setPluginVersion      = &plugin_setPluginVersion;
  plugin_api_pointers.plugin.remPluginVersion      = &plugin_remPluginVersion;
  plugin_api_pointers.plugin.hasPointer            = &plugin_hasPointer;
  plugin_api_pointers.plugin.getPointer            = &plugin_getPointer;
  plugin_api_pointers.plugin.setPointer            = &plugin_setPointer;
  plugin_api_pointers.plugin.remPointer            = &plugin_remPointer;
  plugin_api_pointers.plugin.hasHook               = &plugin_hasHook;
  plugin_api_pointers.plugin.getHook               = &plugin_getHook;
  plugin_api_pointers.plugin.setHook               = &plugin_setHook;
  plugin_api_pointers.plugin.remHook               = &plugin_remHook;
  plugin_api_pointers.plugin.hasCallback           = &hook_hasCallback;
  plugin_api_pointers.plugin.addCallback           = &hook_addCallback;
  plugin_api_pointers.plugin.addIdentifiedCallback = &hook_addIdentifiedCallback;
  plugin_api_pointers.plugin.remCallback           = &hook_remCallback;
  plugin_api_pointers.plugin.doUntilTrue           = &hook_doUntilTrue;
  plugin_api_pointers.plugin.doUntilFalse          = &hook_doUntilFalse;
  plugin_api_pointers.plugin.doAll                 = &hook_doAll;

  plugin_api_pointers.map.setTime                  = &map_setTime;
  plugin_api_pointers.map.getTime                  = &map_getTime;
  plugin_api_pointers.map.createPickupSpawn        = &map_createPickupSpawn;
  plugin_api_pointers.map.getSpawn                 = &map_getSpawn;
  plugin_api_pointers.map.setSpawn                 = &map_setSpawn;
  plugin_api_pointers.map.setBlock                 = &map_setBlock;
  plugin_api_pointers.map.getBlock                 = &map_getBlock;
  plugin_api_pointers.map.saveWholeMap             = &map_saveWholeMap;
  plugin_api_pointers.map.getMapData_block         = &map_getMapData_block;
  plugin_api_pointers.map.getMapData_meta          = &map_getMapData_meta;
  plugin_api_pointers.map.getMapData_skylight      = &map_getMapData_skylight;
  plugin_api_pointers.map.getMapData_blocklight    = &map_getMapData_blocklight;
  plugin_api_pointers.map.setBlockW                = &map_setBlockW;
  plugin_api_pointers.map.getBlockW                = &map_getBlockW;

  plugin_api_pointers.user.getPosition             = &user_getPosition;
  plugin_api_pointers.user.teleport                = &user_teleport;
  plugin_api_pointers.user.sethealth               = &user_sethealth;
  plugin_api_pointers.user.teleportMap             = &user_teleportMap;
  plugin_api_pointers.user.getCount                = &user_getCount;
  plugin_api_pointers.user.getUserNumbered         = &user_getUserNumbered;
  plugin_api_pointers.user.getPositionW            = &user_getPositionW;
  plugin_api_pointers.user.getItemInHand           = &user_getItemInHand;
  plugin_api_pointers.user.setItemInHand           = &user_setItemInHand;
  plugin_api_pointers.user.addItem                 = &user_addItem;
  plugin_api_pointers.user.hasItem                 = &user_hasItem;
  plugin_api_pointers.user.delItem                 = &user_delItem;
  plugin_api_pointers.user.toggleDND               = &user_toggleDND;
  plugin_api_pointers.user.gethealth               = &user_gethealth;
  plugin_api_pointers.user.kick                    = &user_kick;
  plugin_api_pointers.user.getItemAt               = &user_getItemAt;
  plugin_api_pointers.user.setItemAt               = &user_setItemAt;

  plugin_api_pointers.config.has                   = &config_has;
  plugin_api_pointers.config.iData                 = &config_iData;
  plugin_api_pointers.config.lData                 = &config_lData;
  plugin_api_pointers.config.fData                 = &config_fData;
  plugin_api_pointers.config.dData                 = &config_dData;
  plugin_api_pointers.config.sData                 = &config_sData;
  plugin_api_pointers.config.bData                 = &config_bData;

  plugin_api_pointers.mob.createMob                = &mob_createMob;
  plugin_api_pointers.mob.createSpawnMob           = &mob_createSpawnMob;
  plugin_api_pointers.mob.spawnMob                 = &mob_spawnMob;
  plugin_api_pointers.mob.despawnMob               = &mob_despawnMob;
  plugin_api_pointers.mob.moveMob                  = &mob_moveMob;
  plugin_api_pointers.mob.moveMobW                 = &mob_moveMobW;
  plugin_api_pointers.mob.getHealth                = &mob_getHealth;
  plugin_api_pointers.mob.getMobID                 = &mob_getMobID;
  plugin_api_pointers.mob.setHealth                = &mob_setHealth;
  plugin_api_pointers.mob.moveAnimal               = &mob_moveAnimal;
  plugin_api_pointers.mob.animateMob               = &mob_animateMob;
  plugin_api_pointers.mob.animateDamage            = &mob_animateDamage;
  plugin_api_pointers.mob.getType                  = &mob_getType;
  plugin_api_pointers.mob.getMobPositionW          = &mob_getMobPositionW;
  plugin_api_pointers.mob.getLook                  = &mob_getLook;
  plugin_api_pointers.mob.setLook                  = &mob_setLook;
  plugin_api_pointers.mob.setByteMetadata          = &mob_setByteMetadata;
  plugin_api_pointers.mob.updateMetadata           = &mob_updateMetadata;
  plugin_api_pointers.mob.getByteMetadata          = &mob_getByteMetadata;

  plugin_api_pointers.permissions.setAdmin         = &permission_setAdmin;
  plugin_api_pointers.permissions.setOp            = &permission_setOp;
  plugin_api_pointers.permissions.setMember        = &permission_setMember;
  plugin_api_pointers.permissions.setGuest         = &permission_setGuest;
  plugin_api_pointers.permissions.isAdmin          = &permission_isAdmin;
  plugin_api_pointers.permissions.isOp             = &permission_isOp;
  plugin_api_pointers.permissions.isMember         = &permission_isMember;
  plugin_api_pointers.permissions.isGuest          = &permission_isGuest;
  
  plugin_api_pointers.tools.uniformInt             = &tools_uniformInt;
  plugin_api_pointers.tools.uniform01              = &tools_uniform01;
}
