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

#ifndef _USER_H
#define _USER_H

#include <deque>
#include <event.h>
#include "vec.h"
#include "tools.h"
#include "constants.h"
#include "packets.h"

struct position
{
  double x;
  double y;
  double z;
  double stance;
  float yaw;
  float pitch;
};

struct Item
{
  sint16 type;
  sint8 count;
  sint16 health;
  Item()
  {
    type   = 0;
    count  = 0;
    health = 0;
  }
};

struct Inventory
{
  Item main[36];
  Item equipped[4];
  Item crafting[4];

  Inventory()
  {
  }
};

class User
{
private:
  event m_event;
public:

  User(int sock, uint32 EID);
  ~User();

  int fd;

  //View distance in chunks -viewDistance <-> viewDistance
  static const int viewDistance = 10;
  uint8 action;
  bool waitForData;
  uint32 write_err_count;
  bool logged;
  bool admin;
  bool banned;
  bool whitelist;
  bool muted;
	bool dnd;
  sint16 health;
  unsigned int UID;
  std::string nick;
  position pos;
  vec curChunk;
  Inventory inv;

  sint32 attachedTo;

  //Input buffer
  Packet buffer;

  bool checkBanned(std::string _nick);
  bool checkWhitelist(std::string _nick);
  bool changeNick(std::string _nick);
  bool updatePos(double x, double y, double z, double stance);
  bool updateLook(float yaw, float pitch);

  bool sendOthers(uint8 *data, uint32 len);
  static bool sendAll(uint8 *data, uint32 len);
  static bool sendAdmins(uint8 *data, uint32 len);

  //Check inventory for space
  bool checkInventory(sint16 itemID, char count);

  //Load/save player data from/to a file at <mapdir>/players/<nick>.dat
  bool saveData();
  bool loadData();

  // Kick player
  bool kick(std::string kickMsg);

  // Chat blocking
  bool mute(std::string muteMsg);
  bool unmute();
	bool toggleDND();
	bool isAbleToCommunicate(std::string communicateCommand);

  //Map related

  //Map queue
  std::vector<vec> mapQueue;

  //Chunks needed to be removed from client
  std::vector<vec> mapRemoveQueue;

  //Known map pieces
  std::vector<vec> mapKnown;

  //Add map coords to queue
  bool addQueue(int x, int z);

  //Add map coords to remove queue
  bool addRemoveQueue(int x, int z);

  //Add known map piece
  bool addKnown(int x, int z);

  //Delete known map piece
  bool delKnown(int x, int z);

  //Push queued map data to client
  bool pushMap();

  //Push remove queued map data to client
  bool popMap();

  bool teleport(double x, double y, double z);
  bool spawnUser(int x, int y, int z);
  bool spawnOthers();
  bool sethealth(int userHealth);
  bool respawn();
  bool dropInventory();

  struct event *GetEvent();
};

User *addUser(int sock, uint32 EID);
bool remUser(int sock);
bool isUser(int sock);
uint32 generateEID();

extern std::vector<User *> Users;

User *getUserByNick(std::string nick);

#endif
