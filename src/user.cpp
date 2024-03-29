/*
   Copyright (c) 2012, The Mineserver Project
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

#include <cmath>
#include <sys/stat.h>
#include <algorithm>
#include <stdio.h>

using namespace std;

#include "constants.h"

#include "user.h"
#include "mineserver.h"
#include "map.h"
#include "chat.h"
#include "plugin.h"
#include "permissions.h"
#include "mob.h"
#include "logger.h"
#include "protocol.h"
#define LOADBLOCK(x,y,z) ServerInstance->map(pos.map)->getBlock(int(std::floor(double(x))), int(std::floor(double(y))), int(std::floor(double(z))), &type, &meta)


// Generate "unique" entity ID

User::User(int sock, uint32_t EID)
{
  this->action          = 0;
  this->muted           = false;
  this->dnd             = false;
  this->waitForData     = false;
  this->fd              = sock;
  this->UID             = EID;
  this->logged          = false;
  this->serverAdmin     = false;
  this->pos.map         = 0;
  this->pos.x           = ServerInstance->map(pos.map)->spawnPos.x();
  this->pos.y           = ServerInstance->map(pos.map)->spawnPos.y();
  this->pos.z           = ServerInstance->map(pos.map)->spawnPos.z();
  this->write_err_count = 0;
  this->health          = 20;
  this->attachedTo      = 0;
  this->timeUnderwater  = 0;
  this->isOpenInv       = false;
  this->lastData        = time(NULL);
  this->permissions     = 0;
  this->fallDistance    = -10;
  this->healthtimeout   = time(NULL) - 1;
  this->crypted         = false;
  this->gameState       = 0;
  this->compression     = 0;
  this->packetsPerSecond = 0;
  this->gamemode        = GameMode::Survival;
  this->deleting        = false;

  this->m_currentItemSlot = 0;
  this->inventoryHolding  = Item(this, -1);
  this->curItem          = 0;

  //Generate UUID based on the EID
  const char hex[] = "0123456789abcdef";
  this->uuid = "7fe8a8c1-f48c-3509-ac5c-97c3";
  for (int i = 0; i < 8; i++) {
    this->uuid+=hex[(EID>>(i-7)*4)&0xf];
  }

  setUUID(this->uuid, true);

  // Ignore this user if it's the server console
  if (this->UID != SERVER_CONSOLE_UID)
  {
    ServerInstance->users().insert(this);
  }

  for (int count = 0; count < 45; count ++)
  {
    inv[count] = Item(this, count);
  }

  invulnerable = false;
  creative = false;
}

bool User::changeNick(std::string _nick)
{
  runAllCallback("PlayerNickPost", nick.c_str(), _nick.c_str());

  nick = _nick;

  return true;
}

bool User::setUUID(std::string uuid, bool dashes)
{
  this->uuid = uuid;
  std::string temp_uuid = this->uuid;
  if (dashes)
  {
    temp_uuid.erase(temp_uuid.begin()+23);
    temp_uuid.erase(temp_uuid.begin()+18);
    temp_uuid.erase(temp_uuid.begin()+13);
    temp_uuid.erase(temp_uuid.begin()+8);
  } else {
    this->uuid.insert(8, "-");
    this->uuid.insert(13, "-");
    this->uuid.insert(18, "-");
    this->uuid.insert(23, "-");    
  }
  for (int i = 0; i < 16; i++) {
    uuid_raw[i] = (hexToByte(temp_uuid[i*2])<<4)+hexToByte(temp_uuid[i*2+1]);
  }
  return true;
}

User::~User()
{
  EVP_CIPHER_CTX_free(en);
  EVP_CIPHER_CTX_free(de);
  if (this->UID != SERVER_CONSOLE_UID)
  {
    event_del(getReadEvent());
    event_free(getReadEvent());
  }
  if (this->UID != SERVER_CONSOLE_UID)
  {
    event_del(getWriteEvent());
    event_free(getWriteEvent());
  }

  if (fd != -1)
  {
#ifdef WIN32
    closesocket(fd);
#else
    close(fd);
#endif
  }

  this->buffer.reset();
  this->bufferCrypted.reset();

  // Create a temp array since the mapKnown is altered by the delKnown() function
  std::vector<vec> tempMapKnown = mapKnown;

  // Remove all known chunks
  for (uint32_t i = 0; i < tempMapKnown.size(); i++)
  {
    delKnown(tempMapKnown[i].x(), tempMapKnown[i].z());
  }

  std::set<User*>::iterator user_set_it = ServerInstance->users().find(this);
  if (user_set_it != ServerInstance->users().end())
  {
    ServerInstance->users().erase(user_set_it);
  }

  if (logged)
  {
    ServerInstance->chat()->sendMsg(this, this->nick + " disconnected!", Chat::OTHERS);
    //LOG2(WARNING, this->nick + " removed!");
    this->saveData();

    // Send signal to everyone that the entity is destroyed
    Packet pkt = Protocol::destroyEntity(this->UID);
    this->sendOthers(pkt);

    pkt = Protocol::PlayerListItemRemoveSingle(this->uuid_raw);
    this->sendOthers(pkt);

    //If still holding something, dump the items to ground
    if (inventoryHolding.getType() != -1)
    {
      ServerInstance->map(pos.map)->createPickupSpawn((int)pos.x, (int)pos.y, (int)pos.z,
                                                      inventoryHolding.getType(), inventoryHolding.getCount(),
                                                      inventoryHolding.getHealth(), this);
      inventoryHolding.setType(-1);
    }

    //Close open inventory
    if (isOpenInv)
    {
      ServerInstance->inventory()->windowClose(this, openInv.type, openInv.x, openInv.y, openInv.z);
    }


  }

  if (fd != -1 && logged)
  {
    runAllCallback("PlayerQuitPost",nick.c_str());
  }
}

bool User::sendLoginInfo()
{
  // Load user data
  loadData();

  writePacket(Protocol::setCompression(256));
  this->compression = 256;

  // This packet moves gameState to "play"
  writePacket(Protocol::loginSuccess(this->uuid, this->nick));
  this->gameState++;

  // Login OK package
  writePacket(Protocol::joinGame(UID));
  setGameMode(gamemode);
  
  

  // Send spawn position
  writePacket(Protocol::spawnPosition(int(pos.x), int(pos.y + 2), int(pos.z)));
  writePacket(Protocol::timeUpdate(ServerInstance->map(pos.map)->mapTime));

  writePacket(Protocol::playerAbilities(5, 0.1, 0.2));

  writePacket(Protocol::playerPositionAndLook(pos.x, pos.y, pos.z, pos.yaw, pos.pitch, 0));
  
  // Put nearby chunks to queue
  for (int x = -viewDistance; x <= viewDistance; x++)
  {
    for (int z = -viewDistance; z <= viewDistance; z++)
    {
      addQueue((int32_t)pos.x / 16 + x, (int32_t)pos.z / 16 + z);
    }
  }
  // Push chunks to user
  pushMap(true);


  const std::vector<MobPtr>& mobs = ServerInstance->mobs()->getAll();

  for (std::vector<MobPtr>::const_iterator i = mobs.begin(); i != mobs.end(); ++i)
  {
    if (pos.map == (*i)->map && (*i)->spawned)
    {
      writePacket(Protocol::mobSpawn(**i));
    }
  }
  

  logged = true;
  
  for (int i = 1; i < 45; i++)
  {
    inv[i].ready = true;
    inv[i].sendUpdate();
  }

  spawnOthers();

  sethealth(health);

  spawnUser((int32_t)pos.x * 32, (int32_t)((pos.y + 2) * 32), (int32_t)pos.z * 32);

  // Teleport player (again)
  teleport(pos.x, pos.y + 2, pos.z);



  //ServerInstance->chat()->sendMsg(this, nick + " connected!", Chat::ALL);

  return true;
}

// Kick player
bool User::kick(std::string kickMsg)
{
  writePacket(Protocol::disconnect("{\"text\": \""+json_esc(kickMsg)+"\"}"));
  runAllCallback("PlayerKickPost",nick.c_str(), kickMsg.c_str());

  LOG2(WARNING, nick + " kicked. Reason: " + kickMsg);

  return true;
}

bool User::mute(std::string muteMsg)
{
  if (!muteMsg.empty())
  {
    muteMsg = MC_COLOR_YELLOW + "You have been muted.  Reason: " + muteMsg;
  }
  else
  {
    muteMsg = MC_COLOR_YELLOW + "You have been muted. ";
  }

  ServerInstance->chat()->sendMsg(this, muteMsg, Chat::USER);
  this->muted = true;
  LOG2(WARNING, nick + " muted. Reason: " + muteMsg);
  return true;
}

bool User::unmute()
{
  ServerInstance->chat()->sendMsg(this, MC_COLOR_YELLOW + "You have been unmuted.", Chat::USER);
  this->muted = false;
  LOG2(WARNING, nick + " unmuted.");
  return true;
}

bool User::toggleDND()
{
  if (!this->dnd)
  {
    ServerInstance->chat()->sendMsg(this, MC_COLOR_YELLOW + "You have enabled 'Do Not Disturb' mode.", Chat::USER);
    ServerInstance->chat()->sendMsg(this, MC_COLOR_YELLOW + "You will no longer see chat or private messages.", Chat::USER);
    ServerInstance->chat()->sendMsg(this, MC_COLOR_YELLOW + "Type /dnd again to disable 'Do Not Disturb' mode.", Chat::USER);
    this->dnd = true;
  }
  else
  {
    this->dnd = false;
    ServerInstance->chat()->sendMsg(this, MC_COLOR_YELLOW + "You have disabled 'Do Not Disturb' mode.", Chat::USER);
    ServerInstance->chat()->sendMsg(this, MC_COLOR_YELLOW + "You can now see chat and private messages.", Chat::USER);
    ServerInstance->chat()->sendMsg(this, MC_COLOR_YELLOW + "Type /dnd again to enable 'Do Not Disturb' mode.", Chat::USER);
  }
  return this->dnd;
}

bool User::isAbleToCommunicate(std::string communicateCommand)
{
  // Check if this is chat or a regular command and prefix with a slash accordingly
  if (communicateCommand != "chat")
  {
    communicateCommand = "/" + communicateCommand;
  }

  if (this->muted)
  {
    ServerInstance->chat()->sendMsg(this, MC_COLOR_YELLOW + "You cannot " + communicateCommand + " while muted.", Chat::USER);
    return false;
  }
  if (this->dnd)
  {
    ServerInstance->chat()->sendMsg(this, MC_COLOR_YELLOW + "You cannot " + communicateCommand + " while in 'Do Not Disturb' mode.", Chat::USER);
    ServerInstance->chat()->sendMsg(this, MC_COLOR_YELLOW + "Type /dnd to disable.", Chat::USER);
    return false;
  }
  return true;
}

bool User::loadData()
{
  std::string infile = ServerInstance->map(0)->mapDirectory + "/players/" + this->nick + ".dat";
  // Player data will ALWAYS use the first world in your map

  struct stat stFileInfo;
  if (stat(infile.c_str(), &stFileInfo) != 0)
  {
    return false;
  }

  NBT_Value*  playerRoot = NBT_Value::LoadFromFile(infile.c_str());
  NBT_Value& nbtPlayer = *playerRoot;
  if (playerRoot == NULL)
  {
    LOGLF("Failed to open player file");
    return false;
  }

  std::vector<NBT_Value*>* _pos = nbtPlayer["Pos"]->GetList();
  pos.x = (double)(*(*_pos)[0]);
  pos.y = (double)(*(*_pos)[1]);
  pos.z = (double)(*(*_pos)[2]);

  health = *nbtPlayer["Health"];
  gamemode = (int)*nbtPlayer["playerGameType"] == 1 ? User::Creative : User::Survival;

  std::vector<NBT_Value*>* rot = nbtPlayer["Rotation"]->GetList();
  pos.yaw = (float)(*(*rot)[0]);
  pos.yaw = (float)(*(*rot)[1]);

  std::vector<NBT_Value*>* _inv = nbtPlayer["Inventory"]->GetList();
  std::vector<NBT_Value*>::iterator iter = _inv->begin(), end = _inv->end();

  for (; iter != end ; iter++)
  {
    int8_t slot, count;
    int16_t damage, item_id;

    slot    = *(**iter)["Slot"];
    count   = *(**iter)["Count"];
    damage  = *(**iter)["Damage"];
    item_id = *(**iter)["id"];
    if (item_id == 0 || count < 1)
    {
      item_id = -1;
      count   =  0;
    }

    // Main inventory slot, converting 0-35 slots to 9-44
    if (slot >= 0 && slot <= 35)
    {
      inv[(uint8_t)slot + 9].setCount(count);
      inv[(uint8_t)slot + 9].setHealth(damage);
      inv[(uint8_t)slot + 9].setType(item_id);
    }
    // Crafting, converting 80-83 slots to 1-4
    else if (slot >= 80 && slot <= 83)
    {
      inv[(uint8_t)slot - 79].setCount(count);
      inv[(uint8_t)slot - 79].setHealth(damage);
      inv[(uint8_t)slot - 79].setType(item_id);
    }
    // Equipped, converting 100-103 slots to 8-5 (reverse order!)
    else if (slot >= 100 && slot <= 103)
    {
      inv[(uint8_t)8 + (100 - slot)].setCount(count);
      inv[(uint8_t)8 + (100 - slot)].setHealth(damage);
      inv[(uint8_t)8 + (100 - slot)].setType(item_id);
    }
  }
  delete playerRoot;

  return true;
}

bool User::saveData()
{
  std::string outfile = ServerInstance->map(0)->mapDirectory + "/players/" + this->nick + ".dat";
  // Try to create parent directories if necessary
  struct stat stFileInfo;
  if (stat(outfile.c_str(), &stFileInfo) != 0)
  {
    std::string outdir = ServerInstance->map(0)->mapDirectory + "/players";

    if (stat(outdir.c_str(), &stFileInfo) != 0)
    {
      if (!makeDirectory(outdir))
      {
        return false;
      }
    }
  }

  NBT_Value val(NBT_Value::TAG_COMPOUND);
  val.Insert("OnGround", new NBT_Value((int8_t)1));
  val.Insert("Air", new NBT_Value((int16_t)300));
  val.Insert("AttackTime", new NBT_Value((int16_t)0));
  val.Insert("DeathTime", new NBT_Value((int16_t)0));
  val.Insert("Fire", new NBT_Value((int16_t) - 20));
  val.Insert("Health", new NBT_Value((int16_t)health));
  val.Insert("HurtTime", new NBT_Value((int16_t)0));
  val.Insert("FallDistance", new NBT_Value(54.f));
  val.Insert("playerGameType", new NBT_Value(creative ? 1 : 0));

  NBT_Value* nbtInv = new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND);

  char itemslot = 0;
  // Start with main items
  for (int slotid = 9; slotid < 45; slotid++, itemslot++)
  {
    if (inv[(uint8_t)slotid].getCount() && inv[(uint8_t)slotid].getType() != 0 && inv[(uint8_t)slotid].getType() != -1)
    {
      NBT_Value* val = new NBT_Value(NBT_Value::TAG_COMPOUND);
      val->Insert("Count", new NBT_Value((int8_t)inv[(uint8_t)slotid].getCount()));
      val->Insert("Slot", new NBT_Value((int8_t)itemslot));
      val->Insert("Damage", new NBT_Value((int16_t)inv[(uint8_t)slotid].getHealth()));
      val->Insert("id", new NBT_Value((int16_t)inv[(uint8_t)slotid].getType()));
      nbtInv->GetList()->push_back(val);
    }
  }
  // Crafting slots
  itemslot = 80;
  for (int slotid = 1; slotid < 6; slotid++, itemslot++)
  {
    if (inv[(uint8_t)slotid].getCount() && inv[(uint8_t)slotid].getType() != 0 && inv[(uint8_t)slotid].getType() != -1)
    {
      NBT_Value* val = new NBT_Value(NBT_Value::TAG_COMPOUND);
      val->Insert("Count", new NBT_Value((int8_t)inv[(uint8_t)slotid].getCount()));
      val->Insert("Slot", new NBT_Value((int8_t)itemslot));
      val->Insert("Damage", new NBT_Value((int16_t)inv[(uint8_t)slotid].getHealth()));
      val->Insert("id", new NBT_Value((int16_t)inv[(uint8_t)slotid].getType()));
      nbtInv->GetList()->push_back(val);
    }
  }

  // Equipped items last
  itemslot = 103;
  for (int slotid = 5; slotid < 9; slotid++, itemslot--)
  {
    if (inv[(uint8_t)slotid].getCount() && inv[(uint8_t)slotid].getType() != 0 && inv[(uint8_t)slotid].getType() != -1)
    {
      NBT_Value* val = new NBT_Value(NBT_Value::TAG_COMPOUND);
      val->Insert("Count", new NBT_Value((int8_t)inv[(uint8_t)slotid].getCount()));
      val->Insert("Slot", new NBT_Value((int8_t)itemslot));
      val->Insert("Damage", new NBT_Value((int16_t)inv[(uint8_t)slotid].getHealth()));
      val->Insert("id", new NBT_Value((int16_t)inv[(uint8_t)slotid].getType()));
      nbtInv->GetList()->push_back(val);
    }
  }


  val.Insert("Inventory", nbtInv);

  NBT_Value* nbtPos = new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_DOUBLE);
  nbtPos->GetList()->push_back(new NBT_Value((double)pos.x));
  nbtPos->GetList()->push_back(new NBT_Value((double)pos.y));
  nbtPos->GetList()->push_back(new NBT_Value((double)pos.z));
  val.Insert("Pos", nbtPos);


  NBT_Value* nbtRot = new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_FLOAT);
  nbtRot->GetList()->push_back(new NBT_Value((float)pos.yaw));
  nbtRot->GetList()->push_back(new NBT_Value((float)pos.pitch));
  val.Insert("Rotation", nbtRot);


  NBT_Value* nbtMotion = new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_DOUBLE);
  nbtMotion->GetList()->push_back(new NBT_Value((double)0.0));
  nbtMotion->GetList()->push_back(new NBT_Value((double)0.0));
  nbtMotion->GetList()->push_back(new NBT_Value((double)0.0));
  val.Insert("Motion", nbtMotion);

  val.SaveToFile(outfile);

  return true;

}


bool User::updatePosM(double x, double y, double z, size_t map, double stance)
{
  if (map != pos.map && logged)
  {

    // Loop every loaded chunk to make sure no user pointers are left!

    for (ChunkMap::const_iterator it = ServerInstance->map(pos.map)->chunks.begin(); it != ServerInstance->map(pos.map)->chunks.end(); ++it)
    {
      it->second->users.erase(this);

      if (it->second->users.empty())
      {
        ServerInstance->map(pos.map)->releaseMap(it->first.first, it->first.second);
      }
    }

    // TODO despawn players who are no longer in view
    // TODO despawn self to players on last world
    pos.map = map;
    pos.x = x;
    pos.y = y;
    pos.z = z;
    LOG2(INFO, "World changing");
    // TODO spawn self to nearby players
    // TODO spawn players who are NOW in view
    return false;
  }
  updatePos(x, y, z, stance);
  return true;
}

bool User::updatePos(double x, double y, double z, double stance)
{

  // Riding other entity?
  if (y == -999)
  {
    // attachedTo
    // ToDo: Get pos from minecart/player/boat
    return false;
  }

  if (logged)
  {
    sChunk* newChunk = ServerInstance->map(pos.map)->loadMap(blockToChunk((int32_t)x), blockToChunk((int32_t)z));
    sChunk* oldChunk = ServerInstance->map(pos.map)->loadMap(blockToChunk((int32_t)pos.x), blockToChunk((int32_t)pos.z));

    if (!newChunk || !oldChunk)
    {
      LOG2(WARNING, "failed to update user position");
      return false;
    }

    if (newChunk == oldChunk)
    {
      Packet telePacket = Protocol::entityTeleport(UID, x, y, z, pos.yaw, pos.pitch);
      newChunk->sendPacket(telePacket, this);
    }
    else
    {
      std::list<User*> toremove;
      std::list<User*> toadd;

      sChunk::userBoundary(oldChunk, toremove, newChunk, toadd);

      /// update this player's pos for others

      Packet dtPkt = Protocol::destroyEntity(UID);
      std::list<User*>::iterator iter = toremove.begin(), end = toremove.end();
      for (; iter != end ; iter++)
      {
        (*iter)->writePacket(dtPkt);

        this->writePacket(Protocol::destroyEntity((*iter)->UID));
      }

      Packet spawnPkt = Protocol::spawnPlayer(UID, uuid_raw, nick, (float)health, x, y, z, pos.yaw, pos.pitch, curItem);
      iter = toadd.begin(), end = toadd.end();
      for (; iter != end ; iter++)
      {
        (*iter)->writePacket(spawnPkt);

        this->writePacket(
              Protocol::spawnPlayer((*iter)->UID, (*iter)->uuid_raw, (*iter)->nick, (float)(*iter)->health, (*iter)->pos, (*iter)->curItem));
      }

      Packet tpPkt = Protocol::entityTeleport(UID, x, y, z, pos.yaw, pos.pitch);

      newChunk->sendPacket(tpPkt,this);

      std::vector<vec> removeQueue;

      // Check if known chunks are out of view distance now
      for (vec& chunk : mapKnown)
      {
        if (!withinViewDistance(chunk.x(), newChunk->x) || !withinViewDistance(chunk.z(), newChunk->z))
        { 
          removeQueue.push_back(vec(chunk.x(), 0, chunk.z()));          
        }
      }

      
      // ..or chunks in send queue for the player
      for (vec& chunk : mapQueue)
      {
        if (!withinViewDistance(chunk.x(), newChunk->x) || !withinViewDistance(chunk.z(), newChunk->z))
        {
          removeQueue.push_back(vec(chunk.x(), 0, chunk.z()));
        }
      }

      // Push to queue
      for (vec& chunk : removeQueue)
      {
        addRemoveQueue(chunk.x(), chunk.z());
      }

      for (int mapx = newChunk->x - viewDistance; mapx <= newChunk->x + viewDistance; mapx++)
      {
        for (int mapz = newChunk->z - viewDistance; mapz <= newChunk->z + viewDistance; mapz++)
        {
          if (!withinViewDistance(mapx, oldChunk->x) || !withinViewDistance(mapz, oldChunk->z))
          {
            addQueue(mapx, mapz);
          }
        }
      }
    }

    if (newChunk->items.size())
    {
      // Loop through items and check if they are close enought to be picked up
      for (std::vector<spawnedItem*>::iterator iter = newChunk->items.begin(), end = newChunk->items.end(); iter != end; ++iter)
      {
        // No more than 2 blocks away
        if (abs((int32_t)x - ((*iter)->pos.x() / 32)) < 2 &&
            abs((int32_t)y - ((*iter)->pos.y() / 32)) < 2 &&
            abs((int32_t)z - ((*iter)->pos.z() / 32)) < 2)
        {
          // Dont pickup own spawns right away
          if ((*iter)->spawnedBy != this->UID || (*iter)->spawnedAt + 2 < time(NULL))
          {
            // Check player inventory for space!
            if (ServerInstance->inventory()->isSpace(this, (*iter)->item, (*iter)->count))
            {
              // Send player collect item packet
              writePacket(Protocol::collectItem((*iter)->EID, UID));

              // Send everyone destroy_entity-packet
              Packet pkt = Protocol::destroyEntity((*iter)->EID);
              newChunk->sendPacket(pkt);

              // Add items to inventory
              ServerInstance->inventory()->addItems(this, (*iter)->item, (*iter)->count, (*iter)->health);

              ServerInstance->map(pos.map)->items.erase((*iter)->EID);
              delete *iter;
              iter = newChunk->items.erase(iter);
              end = newChunk->items.end();

              if (iter == end)
              {
                break;
              }
            }
          }
        }
      }
    }
  }

  if (ServerInstance->m_damage_enabled)
  {
    uint8_t type, meta;
    if ((std::floor(pos.y - 0.5) < 256) && LOADBLOCK(pos.x, pos.y - 0.5, pos.z))
    {
      switch (type)
      {
      case BLOCK_AIR:
      case BLOCK_SAPLING:
      case BLOCK_YELLOW_FLOWER:
      case BLOCK_RED_ROSE:
      case BLOCK_BROWN_MUSHROOM:
      case BLOCK_RED_MUSHROOM:
      case BLOCK_TORCH:
      case BLOCK_FIRE:
      case BLOCK_REDSTONE_WIRE:
      case BLOCK_CROPS:
      case BLOCK_MINECART_TRACKS:
      case BLOCK_LEVER:
      case BLOCK_REDSTONE_TORCH_OFF:
      case BLOCK_REDSTONE_TORCH_ON:
      case BLOCK_STONE_BUTTON:
      case BLOCK_SNOW:
      case BLOCK_REED:
        fallDistance += this->pos.y - y;
        //if(fallDistance<0){ fallDistance=0; }
        break;
      case BLOCK_WATER:
      case BLOCK_STATIONARY_WATER:
        fallDistance = 0;
        break;
      default:
        if (fallDistance > 3)
        {
          sethealth(std::max(0, health - int(fallDistance - 4)));
        }
        fallDistance = 0;
        break;
      }
    }
  }
  this->pos.x      = x;
  this->pos.y      = y;
  this->pos.z      = z;
  this->pos.stance = stance;
  curChunk.x() = (int)(x / 16);
  curChunk.z() = (int)(z / 16);
  checkEnvironmentDamage();
  return true;
}

bool User::checkOnBlock(int32_t x, int16_t y, int32_t z)
{
  const double diffX = x - this->pos.x;
  const double diffZ = z - this->pos.z;

  if ((y == (int)this->pos.y)
      && (diffZ > -1.3 && diffZ < 0.3)
      && (diffX > -1.3 && diffX < 0.3))
  {
    return true;
  }
  return false;
}

bool User::updateLook(float yaw, float pitch)
{

  sChunk* chunk = ServerInstance->map(pos.map)->getChunk(blockToChunk((int32_t)pos.x), blockToChunk((int32_t)pos.z));
  if (chunk != NULL)
  {
    chunk->sendPacket(Protocol::entityLook(UID, yaw, pitch), this);
    chunk->sendPacket(Protocol::entityHeadLook(UID, angleToByte(yaw)), this);
  }

  this->pos.yaw   = yaw;
  this->pos.pitch = pitch;
  return true;
}

bool User::sendOthers(const Packet& packet)
{
  for (std::set<User*>::const_iterator it = ServerInstance->users().begin(); it != ServerInstance->users().end(); ++it)
  {
    if ((*it)->fd != this->fd && (*it)->logged && (packet.firstwrite() != PACKET_OUT_CHAT_MESSAGE || !((*it)->dnd)))
    {
      (*it)->writePacket(packet);
    }
  }

  return true;
}

bool User::sendOthers(uint8_t* data, size_t len)
{
  for (std::set<User*>::const_iterator it = ServerInstance->users().begin(); it != ServerInstance->users().end(); ++it)
  {
    if ((*it)->fd != this->fd && (*it)->logged && !((*it)->dnd && data[0] == PACKET_OUT_CHAT_MESSAGE))
    {
      (*it)->buffer.addToWrite(data, len);
    }
  }

  return true;
}

int8_t User::relativeToBlock(const int32_t x, const int16_t y, const int32_t z)
{
  int8_t direction;
  double diffX, diffZ;
  diffX = x - this->pos.x;
  diffZ = z - this->pos.z;

  if (diffX > diffZ)
  {
    // We compare on the x axis
    if (diffX > 0)
    {
      direction = BLOCK_BOTTOM;
    }
    else
    {
      direction = BLOCK_EAST;
    }
  }
  else
  {
    // We compare on the z axis
    if (diffZ > 0)
    {
      direction = BLOCK_SOUTH;
    }
    else
    {
      direction = BLOCK_NORTH;
    }
  }
  return direction;
}

bool User::sendAll(const Packet& packet)
{
  for (std::set<User*>::const_iterator it = ServerInstance->users().begin(); it != ServerInstance->users().end(); ++it)
  {
    if ((*it)->fd && (*it)->logged)
    {
      (*it)->writePacket(packet);
    }
  }

  return true;
}

bool User::sendAll(uint8_t* data, size_t len)
{
  for (std::set<User*>::const_iterator it = ServerInstance->users().begin(); it != ServerInstance->users().end(); ++it)
  {
    if ((*it)->fd && (*it)->logged)
    {
      (*it)->buffer.addToWrite(data, len);
    }
  }

  return true;
}

bool User::sendAdmins(const Packet& packet)
{
  for (std::set<User*>::const_iterator it = ServerInstance->users().begin(); it != ServerInstance->users().end(); ++it)
  {
    if ((*it)->fd && (*it)->logged && IS_ADMIN((*it)->permissions))
    {
      (*it)->writePacket(packet);
    }
  }

  return true;
}

bool User::sendAdmins(uint8_t* data, size_t len)
{
  for (std::set<User*>::const_iterator it = ServerInstance->users().begin(); it != ServerInstance->users().end(); ++it)
  {
    if ((*it)->fd && (*it)->logged && IS_ADMIN((*it)->permissions))
    {
      (*it)->buffer.addToWrite(data, len);
    }
  }

  return true;
}

bool User::sendOps(const Packet& packet)
{
  for (std::set<User*>::const_iterator it = ServerInstance->users().begin(); it != ServerInstance->users().end(); ++it)
  {
    if ((*it)->fd && (*it)->logged && IS_ADMIN((*it)->permissions))
    {
      (*it)->writePacket(packet);
    }
  }

  return true;
}

bool User::sendOps(uint8_t* data, size_t len)
{
  for (std::set<User*>::const_iterator it = ServerInstance->users().begin(); it != ServerInstance->users().end(); ++it)
  {
    if ((*it)->fd && (*it)->logged && IS_ADMIN((*it)->permissions))
    {
      (*it)->buffer.addToWrite(data, len);
    }
  }

  return true;
}

bool User::sendGuests(const Packet& packet)
{
  for (std::set<User*>::const_iterator it = ServerInstance->users().begin(); it != ServerInstance->users().end(); ++it)
  {
    if ((*it)->fd && (*it)->logged && IS_ADMIN((*it)->permissions))
    {
      (*it)->writePacket(packet);
    }
  }

  return true;
}

bool User::sendGuests(uint8_t* data, size_t len)
{
  for (std::set<User*>::const_iterator it = ServerInstance->users().begin(); it != ServerInstance->users().end(); ++it)
  {
    if ((*it)->fd && (*it)->logged && IS_ADMIN((*it)->permissions))
    {
      (*it)->buffer.addToWrite(data, len);
    }
  }

  return true;
}

bool User::addQueue(int x, int z)
{
  vec newMap(x, 0, z);

  // Make sure this chunk is not being removed, if it is, delete it from remove queue
  for (unsigned int i = 0; i < mapRemoveQueue.size(); i++)
  {
    if (mapRemoveQueue[i].x() == newMap.x() && mapRemoveQueue[i].z() == newMap.z())
    {
      mapRemoveQueue.erase(mapRemoveQueue.begin() + i);
      break;
    }
  }

  for (unsigned int i = 0; i < mapQueue.size(); i++)
  {
    // Check for duplicates
    if (mapQueue[i].x() == newMap.x() && mapQueue[i].z() == newMap.z())
    {
      return false;
    }
  }

  for (unsigned int i = 0; i < mapKnown.size(); i++)
  {
    // Check for duplicates
    if (mapKnown[i].x() == newMap.x() && mapKnown[i].z() == newMap.z())
    {
      return false;
    }
  }

  this->mapQueue.push_back(newMap);

  return true;
}

bool User::addRemoveQueue(int x, int z)
{
  vec newMap(x, 0, z);

  for (unsigned int i = 0; i < mapQueue.size(); i++)
  {
    // Check if the chunk is going to be loaded, and erase it from the queue
    if (mapQueue[i].x() == newMap.x() && mapQueue[i].z() == newMap.z())
    {
      mapQueue.erase(mapQueue.begin() + i);
      break;
    }
  }

  this->mapRemoveQueue.push_back(newMap);

  return true;
}

bool User::addKnown(int x, int z)
{
  vec newMap(x, 0, z);
  sChunk* chunk = ServerInstance->map(pos.map)->getChunk(x, z);
  if (chunk == NULL)
  {
    return false;
  }

  chunk->users.insert(this);
  this->mapKnown.push_back(newMap);

  return true;
}

bool User::delKnown(int x, int z)
{
  sChunk* chunk = ServerInstance->map(pos.map)->getChunk(x, z);
  if (chunk != NULL)
  {
    chunk->users.erase(this);
  }

  for (unsigned int i = 0; i < mapKnown.size(); i++)
  {
    if (mapKnown[i].x() == x && mapKnown[i].z() == z)
    {
      mapKnown.erase(mapKnown.begin() + i);
      return true;
    }
  }

  return false;
}

bool User::popMap()
{
  // If map in queue, push it to client
  while (this->mapRemoveQueue.size())
  {
    // Delete from known list
    delKnown(mapRemoveQueue[0].x(), mapRemoveQueue[0].z());

    // Send unload packet to client
    writePacket(Protocol::chunkDataUnload(mapRemoveQueue[0].x(), mapRemoveQueue[0].z()));

    // Remove from queue
    mapRemoveQueue.erase(mapRemoveQueue.begin());

    // return true;
  }

  return false;
}

namespace
{

class DistanceComparator
{
private:
  vec target;
public:
  DistanceComparator(vec tgt) : target(tgt)
  {
    target.y() = 0;
  }
  bool operator()(vec a, vec b) const
  {
    a.y() = 0;
    b.y() = 0;
    return vec::squareDistance(a, target) <
        vec::squareDistance(b, target);
  }
};

}

bool User::pushMap(bool login)
{
  //Dont send all at once
  int maxcount = 5;
  // If map in queue, push it to client
  while (this->mapQueue.size() > 0 && maxcount > 0)
  {
    maxcount--;
    // Sort by distance from center
    vec target(static_cast<int>(pos.x / 16),
               static_cast<int>(pos.y / 16),
               static_cast<int>(pos.z / 16));
    sort(mapQueue.begin(), mapQueue.end(), DistanceComparator(target));

    ServerInstance->map(pos.map)->sendToUser(this, mapQueue[0].x(), mapQueue[0].z(), login);

    // Add this to known list
    addKnown(mapQueue[0].x(), mapQueue[0].z());

    // Remove from queue
    mapQueue.erase(mapQueue.begin());
  }

  return true;
}

bool User::teleport(double x, double y, double z, size_t map)
{
  if (map == size_t(-1))
  {
    map = pos.map;
  }
  if (y > 255.0)
  {
    y = 255.0;
    LOGLF("Player Attempted to teleport with y > 255.0");
  }
  if (map == pos.map)
  {
    writePacket(Protocol::playerPositionAndLook(x, y, z, 0, 0, 0));
  }

  //Also update pos for other players
  updatePosM(x, y, z, map, pos.stance);
  pushMap();
  pushMap();
  pushMap();
  updatePosM(x, y, z, map, pos.stance);
  return true;
}

bool User::spawnUser(int x, int y, int z)
{
  Packet pkt = Protocol::spawnPlayer(UID, uuid_raw, nick, (float)health, x, y, z, 0, 0, 0);
  Packet playerListAddPkt = Protocol::PlayerListItemAddSingle(uuid_raw, nick, gamemode, 10, properties);
  sChunk* chunk = ServerInstance->map(pos.map)->getChunk(blockToChunk(x >> 5), blockToChunk(z >> 5));
  if (chunk != NULL)
  {
    chunk->sendPacket(playerListAddPkt);
    chunk->sendPacket(pkt, this);    
  }
  return true;
}

bool User::spawnOthers()
{
  for (std::set<User*>::const_iterator it = ServerInstance->users().begin(); it != ServerInstance->users().end(); ++it)
  {
    if ((*it)->logged && (*it)->UID != this->UID)
    {
      this->writePacket(Protocol::PlayerListItemAddSingle((*it)->uuid_raw, (*it)->nick, (*it)->gamemode, 10, (*it)->properties));
      this->writePacket(Protocol::spawnPlayer((*it)->UID, (*it)->uuid_raw, (*it)->nick, (float)(*it)->health, (*it)->pos.x, (*it)->pos.y, (*it)->pos.z, 0, 0, 0));      
      for (int b = 0; b < 5; b++)
      {
        const int n = b == 0 ? (*it)->curItem + 36 : 9 - b;
        this->writePacket(Protocol::entityEquipment((*it)->UID, b, (*it)->inv[n]));
      }
      this->writePacket(Protocol::entityHeadLook((*it)->UID,angleToByte((*it)->pos.yaw)));
    }
  }
  return true;
}

void User::checkEnvironmentDamage()
{
  const double yVal = std::floor(pos.y - 0.5);

  if (yVal > 255.0)
  {
    return;
  }

  /// louisdx: This makes no sense at the moment; type is not initialized!
  //uint8_t type, meta;
  uint8_t type = 0, meta = 0;

  int16_t d = 0;
  /*
  if (type == BLOCK_CACTUS && LOADBLOCK(pos.x, yVal, pos.z))
  {
    d = 1;
  }
  */

  const double xbit = pos.x - std::floor(pos.x);
  const double zbit = pos.z - std::floor(pos.z);

  if (xbit > 0.6)
  {
    LOADBLOCK(pos.x + 1, pos.y + 0.5, pos.z);
    if (type == BLOCK_CACTUS)
    {
      d = 1;
    }
  }
  else if (xbit < 0.4)
  {
    LOADBLOCK(pos.x - 1, pos.y + 0.5, pos.z);
    if (type == BLOCK_CACTUS)
    {
      d = 1;
    }
  }
  if (zbit > 0.6)
  {
    LOADBLOCK(pos.x, pos.y + 0.5, pos.z + 1);
    if (type == BLOCK_CACTUS)
    {
      d = 1;
    }
  }
  else if (zbit < 0.4)
  {
    LOADBLOCK(pos.x, pos.y + 0.5, pos.z - 1);
    if (type == BLOCK_CACTUS)
    {
      d = 1;
    }
  }

  if (LOADBLOCK(pos.x, pos.y + 0.5, pos.z))
  {
    if (type == BLOCK_LAVA || type == BLOCK_STATIONARY_LAVA)
    {
      d = 10;
    }
    else if (type == BLOCK_FIRE)
    {
      d = 5;
    }
  }

  if(std::floor(pos.y + 1.5) > 255)
  {
    sethealth(std::max(0, health - d));
    return;
  }

  if (LOADBLOCK(pos.x, pos.y + 1.5, pos.z))
  {
    switch (type)
    {
    case BLOCK_AIR:
    case BLOCK_SAPLING:
    case BLOCK_WATER:            // For a certain value of "breathable" ;)
    case BLOCK_STATIONARY_WATER: // Water is treated seperatly
    case BLOCK_YELLOW_FLOWER:
    case BLOCK_RED_ROSE:
    case BLOCK_BROWN_MUSHROOM:
    case BLOCK_RED_MUSHROOM:
    case BLOCK_TORCH:
    case BLOCK_REDSTONE_WIRE:
    case BLOCK_CROPS:
    case BLOCK_LEVER:
    case BLOCK_REDSTONE_TORCH_ON:
    case BLOCK_REDSTONE_TORCH_OFF:
    case BLOCK_SNOW:
    case BLOCK_STONE_BUTTON:
    case BLOCK_REED:
    case BLOCK_PORTAL:
    case BLOCK_LADDER:
    case BLOCK_WOODEN_DOOR:
    case BLOCK_IRON_DOOR:
    case BLOCK_WALL_SIGN:
    case BLOCK_SIGN_POST:
      break;
    default:
      if (d == 0)
      {
        d = 1;
      }
      break;
    }
  }

  sethealth(std::max(0, health - d));
}

bool User::sethealth(int userHealth)
{
  if (!logged)
    return false;

  health = min(max(userHealth, 0), 20);

  if (health == userHealth)
  {
    return false;
  }
  if (userHealth < health)
  {
    // One hit per 2 seconds
    if (time(NULL) - healthtimeout < 1)
    {
      return false;
    }
    sendAll(Protocol::animation(UID, ANIMATE_DAMAGE));
    sendAll(Protocol::entityStatus(UID, ENTITY_STATUS_HURT));
  }
  healthtimeout = time(NULL);

  health = userHealth;
  writePacket(Protocol::updateHealth(userHealth));
  return true;
}

bool User::respawn()
{
  this->health = 20;
  this->timeUnderwater = 0;
  writePacket(Protocol::respawn()); //FIXME: send the correct world id
  sethealth(20);
  Packet destroyPkt;
  destroyPkt << Protocol::destroyEntity(UID);
  sChunk* chunk = ServerInstance->map(pos.map)->getMapData(blockToChunk((int32_t)pos.x), blockToChunk((int32_t)pos.z));
  if (chunk != NULL)
  {
    chunk->sendPacket(destroyPkt, this);
  }

  runCallbackUntilFalse("PlayerRespawn",nick.c_str());
  if (callbackReturnValue)
  {
    // In this case, the plugin teleports automatically
  }
  else
  {
    teleport(ServerInstance->map(pos.map)->spawnPos.x(), ServerInstance->map(pos.map)->spawnPos.y() + 2, ServerInstance->map(pos.map)->spawnPos.z(), 0);
  }

  Packet spawnPkt = Protocol::spawnPlayer(UID, uuid_raw, nick, (float)health, pos.x, pos.y, pos.z, pos.yaw, pos.pitch, curItem);  

  chunk = ServerInstance->map(pos.map)->getMapData(blockToChunk((int32_t)pos.x), blockToChunk((int32_t)pos.z));
  if (chunk != NULL)
  {
    chunk->sendPacket(spawnPkt, this);
  }

  return true;
}

bool User::dropInventory()
{
  for (int i = 1; i < 45; i++)
  {
    if (inv[i].getType() != -1)
    {
      ServerInstance->map(pos.map)->createPickupSpawn((int)pos.x, (int)pos.y, (int)pos.z, inv[i].getType(), inv[i].getCount(), inv[i].getHealth(), this);
      inv[i].setType(-1);
    }
  }
  return true;
}

bool User::isUnderwater()
{
  uint8_t topblock, topmeta;
  const int y = int((pos.y - int(pos.y) <= 0.25) ? pos.y + 1 : pos.y + 2);

  if (y > 127)
  {
    return false;
  }

  ServerInstance->map(pos.map)->getBlock((int)pos.x, y, (int)pos.z, &topblock, &topmeta);

  if (topblock == BLOCK_WATER || topblock == BLOCK_STATIONARY_WATER)
  {
    if ((timeUnderwater / 5) > 15 && timeUnderwater % 5 == 0)  // 13 is Trial and Erorr
    {
      sethealth(health - 2);
    }
    timeUnderwater += 1;
    return true;
  }

  timeUnderwater = 0;
  return false;
}

struct event* User::getReadEvent()
{
  return m_readEvent;
}

bool User::setReadEvent(struct event* new_event)
{
  m_readEvent = new_event;
  return true;
}

struct event* User::getWriteEvent()
{
  return m_writeEvent;
}

bool User::setWriteEvent(struct event* new_event)
{
  m_writeEvent = new_event;
  return true;
}

std::set<User*>& User::all()
{
  return ServerInstance->users();
}

bool User::isUser(int sock)
{
  for (std::set<User*>::const_iterator it = ServerInstance->users().begin(); it != ServerInstance->users().end(); ++it)
  {
    if ((*it)->fd == sock) return true;
  }

  return false;
}

// Not case-sensitive search
User* User::byNick(std::string nick)
{
  // Get coordinates
  for (std::set<User*>::const_iterator it = ServerInstance->users().begin(); it != ServerInstance->users().end(); ++it)
  {
    if (strToLower((*it)->nick) == strToLower(nick))
    {
      return *it;
    }
  }
  return NULL;
}

// Getter/Setter for item currently in hold
int16_t User::currentItemSlot()
{
  return m_currentItemSlot;
}

void User::setCurrentItemSlot(int16_t item_slot)
{
  m_currentItemSlot = item_slot;
}

std::string User::generateDigest()
{
  SHA_CTX context;
  unsigned char md[20];
  std::string out;
  int i;
  const char hex[] = "0123456789abcdef";

  SHA1_Init(&context);
  SHA1_Update(&context, (unsigned char*)ServerInstance->serverID.data(), ServerInstance->serverID.size());
  SHA1_Update(&context, (unsigned char*)secret.data(), secret.size());
  SHA1_Update(&context, (unsigned char*)ServerInstance->publicKey.data(), ServerInstance->publicKey.size());
  SHA1_Final(md, &context);

  if(md[0] & 0x80)
  {
    unsigned char carry = 1;
    out = '-';
    for (i = 19; i >= 0; i--)
    {
      unsigned short twocomp = (unsigned char)~md[i];
      twocomp+=carry;
      if(twocomp & 0xff00)
      {
        twocomp = twocomp&0xff;
      }
      else
      {
        carry = 0;
      }
      md[i] = (uint8_t)twocomp;
    }
  }

  for (i = 0; i < 20; i++)
  {
    if(i || md[i]>>4)
    {
      out += hex[(md[i]>>4)];
    }
    if(i || md[i]>>4 || md[i]&0xf)
    {
      out += hex[(md[i]&0xf)];
    }
  }

  return out;
}

bool User::setGameMode(User::GameMode gameMode)
{
  writePacket(Protocol::gameMode(3,gameMode));

  invulnerable = gameMode == User::Creative;
  creative = gameMode == User::Creative;
  return true;
}
