/*
   Copyright (c) 2010, The Mineserver Project
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

#include <vector>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <deque>
#include <algorithm>
#include <fstream>
#include <map>

#include <sys/stat.h>
#ifdef WIN32
  #include <winsock2.h>
  #include <direct.h>
#else
  #include <netinet/in.h>
  #include <string.h>
#endif
#include <zlib.h>
#include <ctime>
#include "constants.h"

#include "logger.h"
#include "tools.h"
#include "map.h"
#include "user.h"
#include "nbt.h"
#include "chat.h"
#include "packets.h"
#include "mineserver.h"
#include "config.h"
#include "sockets.h"

//Generate "unique" entity ID
uint32 generateEID()
{
  static uint32 EID = 0;
  return ++EID;
}


User::User(int sock, uint32 EID)
{
  this->action          = 0;
  this->muted           = false;
	this->dnd							= false;
  this->waitForData     = false;
  this->fd              = sock;
  this->UID             = EID;
  this->logged          = false;
  // ENABLED FOR DEBUG

  this->pos.x           = Map::get()->spawnPos.x();
  this->pos.y           = Map::get()->spawnPos.y();
  this->pos.z           = Map::get()->spawnPos.z();
  this->write_err_count = 0;
  this->health          = 20;
  this->attachedTo      = 0;
  this->timeUnderwater  = 0;

	// Ignore this user if it's the server console
	if(this->UID != SERVER_CONSOLE_UID)
	{
  	Mineserver::get().users().push_back(this);
	}
}


bool User::checkBanned(std::string _nick)
{
  nick = _nick;

  // Check banstatus
  for(unsigned int i = 0; i < Conf::get()->banned().size(); i++)
  {
    if(Conf::get()->banned()[i] == nick)
    {
      return true;
    }
  }

  return false;
}

bool User::checkWhitelist(std::string _nick)
{
	nick = _nick;

    // Check if nick is whitelisted, providing it is enabled
  for(unsigned int i = 0; i < Conf::get()->whitelist().size(); i++)
  {
    if(Conf::get()->whitelist()[i] == nick)
    {
      return true;
    }
  }

  return false;
}

bool User::changeNick(std::string _nick)
{
  nick = _nick;

  SET_GUEST(permissions); // default

  // Check adminstatus
  for(unsigned int i = 0; i < Conf::get()->admins().size(); i++)
  {
    if(Conf::get()->admins()[i] == nick)
    {
      SET_ADMIN(permissions);
      break;
    }
  }

  // Check op status
  for(unsigned int i = 0; i < Conf::get()->ops().size(); i++)
  {
    if(Conf::get()->ops()[i] == nick)
    {
      SET_OP(permissions);
      break;
    }
  }

  // Check member status
  for(unsigned int i = 0; i < Conf::get()->members().size(); i++)
  {
    if(Conf::get()->members()[i] == nick)
    {
      SET_MEMBER(permissions);
      break;
    }
  }

	// Update the player list with the new name!
	Mineserver::get().updatePlayerList();
	
  return true;
}

User::~User()
{
  for(std::vector<User*>::iterator it = Mineserver::get().users().begin();
      it != Mineserver::get().users().end();
      it++)
  {
    if((*it) == this)
    {
      Mineserver::get().users().erase(it);
			
			// Update the player list
			Mineserver::get().updatePlayerList();

      break;
    }
  }

  if(this->nick.size())
  {
      for(int mapx = -viewDistance+curChunk.x(); mapx <= viewDistance+curChunk.x(); mapx++)
      {
        for(int mapz = -viewDistance+curChunk.z(); mapz <= viewDistance+curChunk.z(); mapz++)
        {
			sChunk *chunk = Map::get()->chunks.GetChunk(mapx, mapz);
			if(chunk != NULL)
				chunk->users.erase(this);
        }
      }


    Chat::get()->sendMsg(this, this->nick + " disconnected!", Chat::OTHERS);
    this->saveData();

    //Send signal to everyone that the entity is destroyed
    uint8 entityData[5];
    entityData[0] = 0x1d; //Destroy entity;
    putSint32(&entityData[1], this->UID);
    this->sendOthers(&entityData[0], 5);
  }
}


bool User::sendLoginInfo()
{
  std::string player = temp_nick;
  User *user = this;

  user->changeNick(player);

  //Load user data
  user->loadData();

  //Login OK package
  user->buffer << (sint8)PACKET_LOGIN_RESPONSE
    << (sint32)user->UID << std::string("") << std::string("") << (sint64)0 << (sint8)0;

  //Send server time (after dawn)
  user->buffer << (sint8)PACKET_TIME_UPDATE << (sint64)Map::get()->mapTime;

  //Inventory
  for(sint32 invType=-1; invType != -4; invType--)
  {
    Item *inventory = NULL;
    sint16 inventoryCount = 0;

    if(invType == -1)
    {
      inventory = user->inv.main;
      inventoryCount = 36;
    }
    else if(invType == -2)
    {
      inventory = user->inv.equipped;
      inventoryCount = 4;
    }
    else if(invType == -3)
    {
      inventory = user->inv.crafting;
      inventoryCount = 4;
    }
    user->buffer << (sint8)PACKET_PLAYER_INVENTORY << invType << inventoryCount;

    for(int i=0; i<inventoryCount; i++)
    {
      if(inventory[i].count)
      {
        user->buffer << (sint16)inventory[i].type << (sint8)inventory[i].count << (sint16)inventory[i].health;
      }
      else
      {
        user->buffer << (sint16)-1;
      }
    }
  }

  // Send motd
  std::ifstream motdfs(Conf::get()->sValue("motd_file").c_str());

  std::string temp;

  while(getline( motdfs, temp ))
  {
    // If not commentline
    if(temp[0] != COMMENTPREFIX)
    {
        user->buffer << (sint8)PACKET_CHAT_MESSAGE << temp;
    }
  }
  motdfs.close();

  //Teleport player
  user->teleport(user->pos.x, user->pos.y+2, user->pos.z);

  //Put nearby chunks to queue
  for(int x = -user->viewDistance; x <= user->viewDistance; x++)
  {
    for(int z = -user->viewDistance; z <= user->viewDistance; z++)
    {
      user->addQueue((sint32)user->pos.x/16+x, (sint32)user->pos.z/16+z);
    }
  }
  // Push chunks to user
  user->pushMap();

  //Spawn this user to others
  user->spawnUser((sint32)user->pos.x*32, ((sint32)user->pos.y+2)*32, (sint32)user->pos.z*32);
  //Spawn other users for connected user
  user->spawnOthers();

  user->sethealth(user->health);
  user->logged = true;

  Chat::get()->sendMsg(user, player+" connected!", Chat::ALL);

  return true;
}

// Kick player
bool User::kick(std::string kickMsg)
{
  buffer << (sint8)PACKET_KICK << kickMsg;
  Screen::get()->log(nick + " kicked. Reason: " + kickMsg);
  return true;
}
bool User::mute(std::string muteMsg)
{
  if(!muteMsg.empty())
    muteMsg = MC_COLOR_YELLOW + "You have been muted.  Reason: " + muteMsg;
  else
    muteMsg = MC_COLOR_YELLOW + "You have been muted. ";

  Chat::get()->sendMsg(this, muteMsg, Chat::USER);
  this->muted = true;
  Screen::get()->log(nick + " muted. Reason: " + muteMsg);
  return true;
}
bool User::unmute()
{
    Chat::get()->sendMsg(this, MC_COLOR_YELLOW + "You have been unmuted.", Chat::USER);
    this->muted = false;
    Screen::get()->log(nick + " unmuted. ");
    return true;
}
bool User::toggleDND()
{
	if(!this->dnd) {
		Chat::get()->sendMsg(this, MC_COLOR_YELLOW + "You have enabled 'Do Not Disturb' mode.", Chat::USER);
		Chat::get()->sendMsg(this, MC_COLOR_YELLOW + "You will no longer see chat or private messages.", Chat::USER);
		Chat::get()->sendMsg(this, MC_COLOR_YELLOW + "Type /dnd again to disable 'Do Not Disturb' mode.", Chat::USER);
		this->dnd = true;
	}
	else {
		this->dnd = false;
		Chat::get()->sendMsg(this, MC_COLOR_YELLOW + "You have disabled 'Do Not Disturb' mode.", Chat::USER);
		Chat::get()->sendMsg(this, MC_COLOR_YELLOW + "You can now see chat and private messages.", Chat::USER);
		Chat::get()->sendMsg(this, MC_COLOR_YELLOW + "Type /dnd again to enable 'Do Not Disturb' mode.", Chat::USER);
	}
	return this->dnd;
}
bool User::isAbleToCommunicate(std::string communicateCommand)
{
	// Check if this is chat or a regular command and prefix with a slash accordingly
	if(communicateCommand != "chat")
		communicateCommand = "/" + communicateCommand;

	if(this->muted) {
		Chat::get()->sendMsg(this, MC_COLOR_YELLOW + "You cannot " + communicateCommand + " while muted.", Chat::USER);
		return false;
	}
	if(this->dnd) {
		Chat::get()->sendMsg(this, MC_COLOR_YELLOW + "You cannot " + communicateCommand + " while in 'Do Not Disturb' mode.", Chat::USER);
		Chat::get()->sendMsg(this, MC_COLOR_YELLOW + "Type /dnd to disable.", Chat::USER);
		return false;
	}
	return true;
}
bool User::loadData()
{
  std::string infile = Map::get()->mapDirectory+"/players/"+this->nick+".dat";

  struct stat stFileInfo;
  if(stat(infile.c_str(), &stFileInfo) != 0)
    return false;

  NBT_Value * playerRoot = NBT_Value::LoadFromFile(infile.c_str());
  NBT_Value &nbtPlayer = *playerRoot;
  if(playerRoot == NULL)
  {
    LOG("Failed to open player file");
    return false;
  }

  std::vector<NBT_Value*> *_pos = nbtPlayer["Pos"]->GetList();
  pos.x = (double)(*(*_pos)[0]);
  pos.y = (double)(*(*_pos)[1]);
  pos.z = (double)(*(*_pos)[2]);

  health = *nbtPlayer["Health"];

  std::vector<NBT_Value*> *rot = nbtPlayer["Rotation"]->GetList();
  pos.yaw = (float)(*(*rot)[0]);
  pos.yaw = (float)(*(*rot)[1]);

  std::vector<NBT_Value*> *_inv = nbtPlayer["Inventory"]->GetList();
  std::vector<NBT_Value*>::iterator iter = _inv->begin(), end = _inv->end();

  for( ; iter != end ; iter++ )
  {
    sint8 slot, count;
    sint16 damage, item_id;

    slot = *(**iter)["Slot"];
    count = *(**iter)["Count"];
    damage = *(**iter)["Damage"];
    item_id = *(**iter)["id"];

      //Main inventory slot
      if(slot >= 0 && slot <= 35)
      {
        inv.main[(uint8)slot].count  = count;
        inv.main[(uint8)slot].health = damage;
        inv.main[(uint8)slot].type   = item_id;
        
        // Add item at slot 0 to currentItem
        if(slot == 0)
        {
          m_currentItem = item_id;
        }
      }
      //Crafting
      else if(slot >= 80 && slot <= 83)
      {
        inv.crafting[(uint8)slot-80].count  = count;
        inv.crafting[(uint8)slot-80].health = damage;
        inv.crafting[(uint8)slot-80].type   = item_id;   
      }
      //Equipped
      else if(slot >= 100 && slot <= 103)
      {
        inv.equipped[(uint8)slot-100].count  = count;
        inv.equipped[(uint8)slot-100].health = damage;
        inv.equipped[(uint8)slot-100].type   = item_id;
      }
  }

  delete playerRoot;

  return true;
}

bool User::saveData()
{
  std::string outfile = Map::get()->mapDirectory+"/players/"+this->nick+".dat";
  // Try to create parent directories if necessary
  struct stat stFileInfo;
  if(stat(outfile.c_str(), &stFileInfo) != 0)
  {
    std::string outdir = Map::get()->mapDirectory+"/players";

    if(stat(outdir.c_str(), &stFileInfo) != 0)
    {
#ifdef WIN32
      if(_mkdir(outdir.c_str()) == -1)
#else
      if(mkdir(outdir.c_str(), 0755) == -1)
#endif

        return false;
    }
  }

  NBT_Value val(NBT_Value::TAG_COMPOUND);
  val.Insert("OnGround", new NBT_Value((sint8)1));
  val.Insert("Air", new NBT_Value((sint16)300));
  val.Insert("AttackTime", new NBT_Value((sint16)0));
  val.Insert("DeathTime", new NBT_Value((sint16)0));
  val.Insert("Fire", new NBT_Value((sint16)-20));
  val.Insert("Health", new NBT_Value((sint16)health));
  val.Insert("HurtTime", new NBT_Value((sint16)0));
  val.Insert("FallDistance", new NBT_Value(54.f));

  NBT_Value *nbtInv = new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_COMPOUND);

  //Start with main items
  Item *slots   = (Item *)&inv.main;
  char slotid   = 0;
  char itemslot = 0;
  for(int i = 0; i < 36+4+4; i++)
  {
    //Crafting items after main
    if(i == 36)
    {
      slots    = (Item *)&inv.crafting;
      itemslot = 80;
      slotid   = 0;
    }
    //Equipped items last
    else if(i == 36+4)
    {
      slots    = (Item *)&inv.equipped;
      itemslot = 100;
      slotid   = 0;
    }
    if(slots[(uint8)slotid].count)
    {
    NBT_Value *val = new NBT_Value(NBT_Value::TAG_COMPOUND);
    val->Insert("Count", new NBT_Value((sint8)slots[(uint8)slotid].count));
    val->Insert("Slot", new NBT_Value((sint8)itemslot));
    val->Insert("Damage", new NBT_Value((sint16)slots[(uint8)slotid].health));
    val->Insert("id", new NBT_Value((sint16)slots[(uint8)slotid].type));
    nbtInv->GetList()->push_back(val);
    }

    slotid++;
    itemslot++;
  }

  val.Insert("Inventory", nbtInv);

  NBT_Value *nbtPos = new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_DOUBLE);
  nbtPos->GetList()->push_back(new NBT_Value((double)pos.x));
  nbtPos->GetList()->push_back(new NBT_Value((double)pos.y));
  nbtPos->GetList()->push_back(new NBT_Value((double)pos.z));
  val.Insert("Pos", nbtPos);


  NBT_Value *nbtRot = new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_FLOAT);
  nbtRot->GetList()->push_back(new NBT_Value((float)pos.yaw));
  nbtRot->GetList()->push_back(new NBT_Value((float)pos.pitch));
  val.Insert("Rotation", nbtRot);


  NBT_Value *nbtMotion = new NBT_Value(NBT_Value::TAG_LIST, NBT_Value::TAG_DOUBLE);
  nbtMotion->GetList()->push_back(new NBT_Value((double)0.0));
  nbtMotion->GetList()->push_back(new NBT_Value((double)0.0));
  nbtMotion->GetList()->push_back(new NBT_Value((double)0.0));
  val.Insert("Motion", nbtMotion);

  val.SaveToFile(outfile);

  return true;

}

bool User::checkInventory(sint16 itemID, char count)
{
  int leftToFit = count;
  for(uint8 i = 0; i < 36; i++)
  {
    if(inv.main[i].type == 0)
      return true;

    if(inv.main[i].type == itemID)
    {
      if(64-inv.main[i].count >= leftToFit)
        return true;
      else if(64-inv.main[i].count > 0)
        leftToFit -= 64-inv.main[i].count;
    }
  }
  return false;
}

bool User::updatePos(double x, double y, double z, double stance)
{	
  
  //Riding on a minecart?
  if(y==-999)
  {
    //attachedTo
    //ToDo: Get pos from minecart
    return false;
  }

  if(nick.size() && logged)
  {
	sChunk *newChunk = Map::get()->loadMap(blockToChunk((sint32)x), blockToChunk((sint32)z));
    sChunk *oldChunk = Map::get()->loadMap(blockToChunk((sint32)pos.x), blockToChunk((sint32)pos.z));
	if(newChunk == oldChunk)
	{
		Packet telePacket;
		telePacket << (sint8)PACKET_ENTITY_TELEPORT 
		   << (sint32)UID << (sint32)(x * 32) << (sint32)(y * 32) << (sint32)(z * 32) << angleToByte(pos.yaw) << angleToByte(pos.pitch);
		newChunk->sendPacket(telePacket, this);
	}
	else if(abs(newChunk->x - oldChunk->x) <= 1  && abs(newChunk->z - oldChunk->z) <= 1)
	{
		std::list<User*> toremove;
		std::list<User*> toadd;

		sChunk::UserBoundry(oldChunk, toremove, newChunk, toadd);

		if(toremove.size())
		{
			Packet pkt;
			pkt << (sint8)PACKET_DESTROY_ENTITY << (sint32)UID;
			std::list<User*>::iterator iter = toremove.begin(), end = toremove.end();
			for( ; iter != end ; iter++)
			{
				(*iter)->buffer.addToWrite(pkt.getWrite(), pkt.getWriteLen());
			}
		}

		if(toadd.size())
		{
			Packet pkt;
			pkt << (sint8)PACKET_NAMED_ENTITY_SPAWN << (sint32)UID << nick
				<< (sint32)(x * 32) << (sint32)(y * 32) << (sint32)(z * 32) 
				<< angleToByte(pos.yaw) << angleToByte(pos.pitch) << (sint16)curItem;

			std::list<User*>::iterator iter = toadd.begin(), end = toadd.end();
			for( ; iter != end ; iter++)
			{
				if((*iter) != this)
				  (*iter)->buffer.addToWrite(pkt.getWrite(), pkt.getWriteLen());
			}
		}

		// TODO: Determine those who where present for both.
		Packet telePacket;
		telePacket << (sint8)PACKET_ENTITY_TELEPORT 
		   << (sint32)UID << (sint32)(x * 32) << (sint32)(y * 32) << (sint32)(z * 32) << angleToByte(pos.yaw) << angleToByte(pos.pitch);
		newChunk->sendPacket(telePacket, this);

		int chunkDiffX = newChunk->x - oldChunk->x;
		int chunkDiffZ = newChunk->z - oldChunk->z;

		// Send new chunk and clear old chunks
		for(int mapx = newChunk->x-viewDistance; mapx <= newChunk->x+viewDistance; mapx++)
        {
          for(int mapz = newChunk->z-viewDistance; mapz <= newChunk->z+viewDistance; mapz++)
          {
		    if(!withinViewDistance(chunkDiffX, oldChunk->x) || !withinViewDistance(chunkDiffZ, oldChunk->z))
				addQueue(mapx, mapz);

			if(!withinViewDistance((mapx - chunkDiffX), newChunk->x) || !withinViewDistance((mapz - chunkDiffZ), newChunk->z))
				addRemoveQueue(mapx-chunkDiffX, mapz-chunkDiffZ);
          }
        }
	}
	else
	{
		std::set<User*> toRemove;
		std::set<User*> toAdd;

 		int chunkDiffX = newChunk->x - oldChunk->x;
		int chunkDiffZ = newChunk->z - oldChunk->z;
		for(int mapx = newChunk->x-viewDistance; mapx <= newChunk->x+viewDistance; mapx++)
        {
          for(int mapz = newChunk->z-viewDistance; mapz <= newChunk->z+viewDistance; mapz++)
          {
		    if(!withinViewDistance(chunkDiffX, oldChunk->x) || !withinViewDistance(chunkDiffZ, oldChunk->z))
			{
				addQueue(mapx, mapz);
				sChunk *chunk = Map::get()->chunks.GetChunk(mapx, mapz);

				if(chunk != NULL)
				{
					toAdd.insert(chunk->users.begin(), chunk->users.end());
				}
			}

			if(!withinViewDistance((mapx - chunkDiffX), newChunk->x) || !withinViewDistance((mapz - chunkDiffZ), newChunk->z))
			{
				addRemoveQueue(mapx-chunkDiffX, mapz-chunkDiffZ);

				sChunk *chunk = Map::get()->chunks.GetChunk((mapx - chunkDiffX), (mapz - chunkDiffZ));

				if(chunk != NULL)
				{
					toRemove.insert(chunk->users.begin(), chunk->users.end());
				}
			}
          }
        }

		std::set<User*> toTeleport;
		std::set<User*>::iterator iter = toRemove.begin(), end = toRemove.end();
		for( ; iter != end ; iter++ )
		{
			std::set<User*>::iterator result = toAdd.find(*iter);
			if(result != toAdd.end())
			{
				toTeleport.insert(*iter);
				toAdd.erase(result);

#ifdef _MSC_VER
				iter = toRemove.erase(iter);
#else
				// TODO: Optimise
				toRemove.erase(iter);
				iter = toRemove.begin();
#endif
				end = toRemove.end();
				if(iter == end)
					break;
			}
		}

		Packet destroyPkt;
		destroyPkt << (sint8)PACKET_DESTROY_ENTITY << (sint32)UID;

		Packet spawnPkt;
		spawnPkt << (sint8)PACKET_NAMED_ENTITY_SPAWN << (sint32)UID << nick
			<< (sint32)(x * 32) << (sint32)(y * 32) << (sint32)(z * 32) << angleToByte(pos.yaw) << angleToByte(pos.pitch) << (sint16)curItem;

		Packet telePacket;
		telePacket << (sint8)PACKET_ENTITY_TELEPORT 
		   << (sint32)UID << (sint32)(x * 32) << (sint32)(y * 32) << (sint32)(z * 32) << angleToByte(pos.yaw) << angleToByte(pos.pitch);

		toTeleport.erase(this);
		toAdd.erase(this);
		toRemove.erase(this);

		iter = toRemove.begin(); end = toRemove.end();
		for( ; iter != end ; iter++ )
			(*iter)->buffer.addToWrite(destroyPkt.getWrite(), destroyPkt.getWriteLen());

		iter = toAdd.begin(); end = toAdd.end();
		for( ; iter != end ; iter++ )
			(*iter)->buffer.addToWrite(spawnPkt.getWrite(), spawnPkt.getWriteLen());

		iter = toTeleport.begin(); end = toTeleport.end();
		for( ; iter != end ; iter++ )
			(*iter)->buffer.addToWrite(telePacket.getWrite(), telePacket.getWriteLen());
	}
/*
    //Do we send relative or absolute move values
    if(0)  //abs(x-this->pos.x)<127
           //&& abs(y-this->pos.y)<127
           //&& abs(z-this->pos.z)<127)
    {
      uint8 movedata[8];
      movedata[0] = 0x1f; //Relative move
      putSint32(&movedata[1], (sint32)this->UID);
      movedata[5] = (char)(x-this->pos.x);
      movedata[6] = (char)(y-this->pos.y);
      movedata[7] = (char)(z-this->pos.z);
      this->sendOthers(&movedata[0], 8);
    }
    else
    {
      this->pos.x      = x;
      this->pos.y      = y;
      this->pos.z      = z;
      this->pos.stance = stance;

      // Fix yaw & pitch to be a 1 byte fraction of 360 (no negatives)
      int fixedYaw = int(this->pos.yaw) % 360;
      if(fixedYaw < 0)
        fixedYaw = 360 + fixedYaw;
      fixedYaw = int(float(fixedYaw) * float(255.0f/360.0f));

      int fixedPitch = int(this->pos.pitch) % 360;
      if(fixedPitch < 0)
        fixedPitch = 360 + fixedPitch;
      fixedPitch = int(float(fixedPitch) * float(255.0f/360.0f));

      Packet pkt;
      pkt << PACKET_ENTITY_TELEPORT << (sint32)this->UID;
      pkt << (sint32)(this->pos.x*32) << (sint32)(this->pos.y*32) << (sint32)(this->pos.z*32);
      pkt << (sint8)fixedYaw << (sint8) fixedPitch;
      this->sendOthers((uint8 *)pkt.getWrite(), pkt.getWriteLen());
    }

    //Check if there are items in this chunk!
    sint32 chunk_x = blockToChunk((sint32)x);
    sint32 chunk_z = blockToChunk((sint32)z);
    uint32 chunkHash;
    Map::get().posToId(chunk_x, chunk_z, &chunkHash); */
  	if(newChunk->items.size())
    {
      //Loop through items and check if they are close enought to be picked up
      std::vector<spawnedItem*>::iterator iter = newChunk->items.begin(), end = newChunk->items.end();
      for( ; iter != end ; iter++)
      {
        //No more than 2 blocks away
		if( abs((sint32)x-((*iter)->pos.x()/32)) < 2 && 
            abs((sint32)y-((*iter)->pos.y()/32)) < 2 && 
            abs((sint32)z-((*iter)->pos.z()/32)) < 2)
		{
          //Dont pickup own spawns right away
          if((*iter)->spawnedBy != this->UID ||
             (*iter)->spawnedAt+2 < time(0))
          {
            //Check player inventory for space!
            if(checkInventory((*iter)->item,
                              (*iter)->count))
            {
              //Send player collect item packet
			  buffer << (sint8)PACKET_COLLECT_ITEM << (sint32)(*iter)->EID << (sint32)UID;

              //Send everyone destroy_entity-packet
			  Packet pkt;
			  pkt << (sint8)PACKET_DESTROY_ENTITY << (sint32)(*iter)->EID;
			  newChunk->sendPacket(pkt);
/*
	Map::get()->posToId(chunk_x, chunk_z, &chunkHash);
    if(Map::get()->mapItems.count(chunkHash))
    {
      //Loop through items and check if they are close enought to be picked up
      for(sint32 i = Map::get()->mapItems[chunkHash].size()-1; i >= 0; i--)
      {
        //No more than 2 blocks away
        if(abs((sint32)x-Map::get()->mapItems[chunkHash][i]->pos.x()/32) < 2 &&
           abs((sint32)z-Map::get()->mapItems[chunkHash][i]->pos.z()/32) < 2 &&
           abs((sint32)y-Map::get()->mapItems[chunkHash][i]->pos.y()/32) < 2)
        {
          //Dont pickup own spawns right away
          if(Map::get()->mapItems[chunkHash][i]->spawnedBy != this->UID ||
             Map::get()->mapItems[chunkHash][i]->spawnedAt+2 < time(0))
          {
            //Check player inventory for space!
            if(checkInventory(Map::get()->mapItems[chunkHash][i]->item,
                              Map::get()->mapItems[chunkHash][i]->count))
            {
              //Send player collect item packet
              uint8 *packet = new uint8[9];
              packet[0] = PACKET_COLLECT_ITEM;
              putSint32(&packet[1], Map::get()->mapItems[chunkHash][i]->EID);
              putSint32(&packet[5], this->UID);
        buffer.addToWrite(packet, 9);

              //Send everyone destroy_entity-packet
              packet[0] = PACKET_DESTROY_ENTITY;
              putSint32(&packet[1], Map::get()->mapItems[chunkHash][i]->EID);
              //ToDo: Only send users in range
              this->sendAll(packet, 5);

              packet[0] = PACKET_ADD_TO_INVENTORY;
              putSint16(&packet[1], Map::get()->mapItems[chunkHash][i]->item);
              packet[3] = Map::get()->mapItems[chunkHash][i]->count;
              putSint16(&packet[4], Map::get()->mapItems[chunkHash][i]->health);
*/
			  buffer << (sint8)PACKET_ADD_TO_INVENTORY << (sint16)(*iter)->item << (sint8)(*iter)->count << (sint16)(*iter)->health;


			  Map::get()->items.erase((*iter)->EID);
			  delete *iter;
			  iter = newChunk->items.erase(iter);
			  end = newChunk->items.end();

			  if(iter == end)
				  break;

            }
          }
        }
      }
    }
}
/*
    //Chunk position changed, check for map updates
    if((int)(x/16) != curChunk.x() || (int)(z/16) != curChunk.z())
    {
      //This is not accurate chunk!!
      curChunk.x() = (int)(x/16);
      curChunk.z() = (int)(z/16);

      for(int mapx = -viewDistance+curChunk.x(); mapx <= viewDistance+curChunk.x(); mapx++)
      {
        for(int mapz = -viewDistance+curChunk.z(); mapz <= viewDistance+curChunk.z(); mapz++)
        {
          addQueue(mapx, mapz);
        }
      }

      for(unsigned int i = 0; i < mapKnown.size(); i++)
      {
        //If client has map data more than viesDistance+1 chunks away, remove it
        if(mapKnown[i].x() < curChunk.x()-viewDistance-1 ||
           mapKnown[i].x() > curChunk.x()+viewDistance+1 ||
           mapKnown[i].z() < curChunk.z()-viewDistance-1 ||
           mapKnown[i].z() > curChunk.z()+viewDistance+1)
          addRemoveQueue(mapKnown[i].x(), mapKnown[i].z());
      }
    }
  }
*/
  this->pos.x      = x;
  this->pos.y      = y;
  this->pos.z      = z;
  this->pos.stance = stance;
  curChunk.x() = (int)(x/16);
  curChunk.z() = (int)(z/16);
  return true;
}

bool User::checkOnBlock(sint32 x, sint8 y, sint32 z)
{
   double diffX = x - this->pos.x;
   double diffZ = z - this->pos.z;

   if ((y == (int)this->pos.y)
         && (diffZ > -1.3 && diffZ < 0.3)
         && (diffX > -1.3 && diffX < 0.3))
      return true;
   return false;
}

bool User::updateLook(float yaw, float pitch)
{
  Packet pkt;
  pkt << (sint8)PACKET_ENTITY_LOOK << (sint32)UID << angleToByte(yaw) << angleToByte(pitch);

  sChunk *chunk = Map::get()->chunks.GetChunk(blockToChunk((sint32)pos.x),blockToChunk((sint32)pos.z));
  if(chunk != NULL)
	chunk->sendPacket(pkt, this);

  this->pos.yaw   = yaw;
  this->pos.pitch = pitch;
  return true;
}

bool User::sendOthers(uint8 *data, uint32 len)
{
  for(unsigned int i = 0; i < Mineserver::get().users().size(); i++)
  {
    if(Mineserver::get().users()[i]->fd != this->fd && Mineserver::get().users()[i]->logged)
    {
      // Don't send to his user if he is DND and the message is a chat message
      if(!(Mineserver::get().users()[i]->dnd && data[0] == PACKET_CHAT_MESSAGE))
      {
    	  Mineserver::get().users()[i]->buffer.addToWrite(data, len);
  	  }
  	}
  }
  return true;
}

sint8 User::relativeToBlock(const sint32 x, const sint8 y, const sint32 z)
{
   sint8 direction;
   double diffX, diffZ;
   diffX = x - this->pos.x;
   diffZ = z - this->pos.z;

   if (diffX > diffZ)
   {
     // We compare on the x axis
     if (diffX > 0) {
       direction = BLOCK_BOTTOM;
     } else {
       direction = BLOCK_EAST;
     }
   } else {
     // We compare on the z axis
     if (diffZ > 0) {
       direction = BLOCK_SOUTH;
     } else {
       direction = BLOCK_NORTH;
     }
   }
   return direction;
}

bool User::sendAll(uint8 *data, uint32 len)
{
  for(unsigned int i = 0; i < Mineserver::get().users().size(); i++)
  {
    if(Mineserver::get().users()[i]->fd && Mineserver::get().users()[i]->logged)
    {
      // Don't send to his user if he is DND and the message is a chat message
      if(!(Mineserver::get().users()[i]->dnd && data[0] == PACKET_CHAT_MESSAGE))
      {
    	  Mineserver::get().users()[i]->buffer.addToWrite(data, len);
  	  }
  	}
  }
  return true;
}

bool User::sendAdmins(uint8 *data, uint32 len)
{
  for(unsigned int i = 0; i < Mineserver::get().users().size(); i++)
  {
    if(Mineserver::get().users()[i]->fd && Mineserver::get().users()[i]->logged && IS_ADMIN(Mineserver::get().users()[i]->permissions))
    	Mineserver::get().users()[i]->buffer.addToWrite(data, len);
  }
  return true;
}

bool User::sendOps(uint8 *data, uint32 len)
{
  for(unsigned int i = 0; i < Mineserver::get().users().size(); i++)
  {
    if(Mineserver::get().users()[i]->fd && Mineserver::get().users()[i]->logged && IS_ADMIN(Mineserver::get().users()[i]->permissions))
    	Mineserver::get().users()[i]->buffer.addToWrite(data, len);
  }
  return true;
}

bool User::sendGuests(uint8 *data, uint32 len)
{
  for(unsigned int i = 0; i < Mineserver::get().users().size(); i++)
  {
    if(Mineserver::get().users()[i]->fd && Mineserver::get().users()[i]->logged && IS_ADMIN(Mineserver::get().users()[i]->permissions))
    	Mineserver::get().users()[i]->buffer.addToWrite(data, len);
  }
  return true;
}

bool User::addQueue(int x, int z)
{
  vec newMap(x, 0, z);

  for(unsigned int i = 0; i < mapQueue.size(); i++)
  {
    // Check for duplicates
    if(mapQueue[i].x() == newMap.x() && mapQueue[i].z() == newMap.z())
      return false;
  }

  for(unsigned int i = 0; i < mapKnown.size(); i++)
  {
    //Check for duplicates
    if(mapKnown[i].x() == newMap.x() && mapKnown[i].z() == newMap.z())
      return false;
  }

  this->mapQueue.push_back(newMap);

  return true;
}

bool User::addRemoveQueue(int x, int z)
{
  vec newMap(x, 0, z);

  this->mapRemoveQueue.push_back(newMap);

  return true;
}

bool User::addKnown(int x, int z)
{
  vec newMap(x, 0, z);
  sChunk * chunk = Map::get()->chunks.GetChunk(x,z);
  if(chunk == NULL)
	  return false;

  chunk->users.insert(this);
  this->mapKnown.push_back(newMap);

  return true;
}

bool User::delKnown(int x, int z)
{
  sChunk * chunk = Map::get()->chunks.GetChunk(x,z);
  if(chunk != NULL)
    chunk->users.erase(this);

  for(unsigned int i = 0; i < mapKnown.size(); i++)
  {
    if(mapKnown[i].x() == x && mapKnown[i].z() == z)
    {
      mapKnown.erase(mapKnown.begin()+i);
      return true;
    }
  }

  return false;
}

bool User::popMap()
{
  //If map in queue, push it to client
  while(this->mapRemoveQueue.size())
  {
    //Pre chunk
    buffer << (sint8)PACKET_PRE_CHUNK << (sint32)mapRemoveQueue[0].x() << (sint32)mapRemoveQueue[0].z() << (sint8)0;

    //Delete from known list
    delKnown(mapRemoveQueue[0].x(), mapRemoveQueue[0].z());

    //Remove from queue
    mapRemoveQueue.erase(mapRemoveQueue.begin());

    //return true;
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

bool User::pushMap()
{
  //Dont send all at once
  int maxcount = 10;
  // If map in queue, push it to client
  while(this->mapQueue.size() > 0 && maxcount > 0)
  {
    maxcount--;
    // Sort by distance from center
    vec target(static_cast<int>(pos.x / 16),
               static_cast<int>(pos.y / 16),
               static_cast<int>(pos.z / 16));
    sort(mapQueue.begin(), mapQueue.end(), DistanceComparator(target));

    Map::get()->sendToUser(this, mapQueue[0].x(), mapQueue[0].z());

    // Add this to known list
    addKnown(mapQueue[0].x(), mapQueue[0].z());

    // Remove from queue
    mapQueue.erase(mapQueue.begin());
  }

  return true;
}

bool User::teleport(double x, double y, double z)
{
  if(y > 128.0)
  {
	  y = 128.0;
	  LOG("Player Attempted to teleport with y > 128.0");
  }
  buffer << (sint8)PACKET_PLAYER_POSITION_AND_LOOK << x << y << (double)0.0 << z 
    << (float)0.f << (float)0.f << (sint8)1;

  //Also update pos for other players
  updatePos(x, y, z, 0);

  pushMap();
  return true;
}

bool User::spawnUser(int x, int y, int z)
{
  Packet pkt;
  pkt << (sint8)PACKET_NAMED_ENTITY_SPAWN << (sint32)UID << nick
    << (sint32)x << (sint32)y << (sint32)z << (sint8)0 << (sint8)0
    << (sint16)0;
  sChunk *chunk = Map::get()->chunks.GetChunk(blockToChunk(x >> 5), blockToChunk(z >> 5));
  if(chunk != NULL)
	  chunk->sendPacket(pkt, this);
  return true;
}

bool User::spawnOthers()
{

  for(unsigned int i = 0; i < Mineserver::get().users().size(); i++)
  {
    if(Mineserver::get().users()[i]->UID != this->UID && Mineserver::get().users()[i]->nick != this->nick)
    {
    buffer << (sint8)PACKET_NAMED_ENTITY_SPAWN << (sint32)Mineserver::get().users()[i]->UID << Mineserver::get().users()[i]->nick
      << (sint32)(Mineserver::get().users()[i]->pos.x * 32) << (sint32)(Mineserver::get().users()[i]->pos.y * 32) << (sint32)(Mineserver::get().users()[i]->pos.z * 32)
      << (sint8)0 << (sint8)0 << (sint16)0;
    }
  }
  return true;
}

bool User::sethealth(int userHealth)
{
  health = userHealth;
  buffer << (sint8)PACKET_UPDATE_HEALTH << (sint8)userHealth;
  //ToDo: Send destroy entity and spawn entity again
  return true;
}

bool User::respawn()
{
  this->health = 20;
  this->timeUnderwater = 0;
  buffer << (sint8)PACKET_RESPAWN;
  return true;
}

bool User::dropInventory()
{
  for( int i = 0; i < 36; i++ )
  {
    if( inv.main[i].type != 0 )
    {
      Map::get()->createPickupSpawn((int)pos.x, (int)pos.y, (int)pos.z, inv.main[i].type, inv.main[i].count);
      inv.main[i] = Item();
    }

    if( i >= 0 && i < 4 )
    {
      if( inv.equipped[i].type != 0 )
      {
        Map::get()->createPickupSpawn((int)pos.x, (int)pos.y, (int)pos.z, inv.equipped[i].type, inv.equipped[i].count);
        inv.equipped[i] = Item();
      }
      if( inv.crafting[i].type != 0 )
      {
        Map::get()->createPickupSpawn((int)pos.x, (int)pos.y, (int)pos.z, inv.crafting[i].type, inv.crafting[i].count);
        inv.crafting[i] = Item();
      }
    }
  }
  return true;
}

bool User::isUnderwater()
{
   uint8 topblock, topmeta;
   int y = ( pos.y - int(pos.y) <= 0.25 ) ? (int)pos.y + 1: (int)pos.y + 2;

   Map::get()->getBlock((int)pos.x, y, (int)pos.z, &topblock, &topmeta);

   if( topblock == BLOCK_WATER || topblock == BLOCK_STATIONARY_WATER )
   {
      if( (timeUnderwater / 5) > 15 && timeUnderwater % 5 == 0 )// 13 is Trial and Erorr
        sethealth( health - 2 );
      timeUnderwater += 1;
      return true;
   }

   timeUnderwater = 0;
   return false;
}

struct event *User::GetEvent()
{
  return &m_event;
}

std::vector<User *> & User::all()
{
  return Mineserver::get().users();
}

bool User::isUser(int sock)
{
  uint8 i;
  for(i = 0; i < Mineserver::get().users().size(); i++)
  {
    if(Mineserver::get().users()[i]->fd == sock)
      return true;
  }
  return false;
}

//Not case-sensitive search
User* User::byNick(std::string nick)
{
  // Get coordinates
  for(unsigned int i = 0; i < Mineserver::get().users().size(); i++)
  {
    if(strToLower(Mineserver::get().users()[i]->nick) == strToLower(nick))
      return Mineserver::get().users()[i];
  }
  return NULL;
}

// Getter/Setter for item currently in hold
sint16 User::currentItem()
{
  return m_currentItem;
}

void User::setCurrentItem(sint16 item_id)
{
  m_currentItem = item_id;
}
