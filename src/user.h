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

#ifndef _USER_H
#define _USER_H

#include <vector>

#ifdef WIN32
// This is needed for event to work on Windows.
#include <winsock2.h>
#else
#include <sys/types.h>
#endif
#include <event.h>

#include "vec.h"
#include "inventory.h"
#include "packets.h"

struct position
{
  double x;
  double y;
  double z;
  int map;
  double stance;
  float yaw;
  float pitch;
};

class User
{
public:

  User(int sock, uint32_t EID);
  ~User();

  int fd;

  //When we last received data from this user
  time_t lastData;

  //View distance in chunks -viewDistance <-> viewDistance
  static const int viewDistance = 10;
  uint8_t action;
  bool waitForData;
  uint32_t write_err_count;
  bool logged;
  bool muted;
  bool dnd;
  int16_t health;
  uint16_t timeUnderwater;
  double fallDistance;
  unsigned int UID;
  std::string nick;
  std::string temp_nick;
  position pos;
  vec curChunk;
  Item inv[45];
  int16_t curItem;
  time_t healthtimeout;
  Item inventoryHolding;
  //Do we have an open _shared_ inventory?
  bool isOpenInv;
  //More info on the inventory
  OpenInventory openInv;

  bool serverAdmin;
  int permissions; // bitmask for permissions. See permissions.h

  int32_t attachedTo;

  //Input buffer
  Packet buffer;
  Packet loginBuffer; // Used to send all login info at once

  static std::vector<User*>& all();
  static bool isUser(int sock);
  static User* byNick(std::string nick);

  bool changeNick(std::string _nick);
  void checkEnvironmentDamage();
  bool updatePos(double x, double y, double z, double stance);
  bool updatePosM(double x, double y, double z, int map, double stance);
  /** Check if the user is standing on this block */
  bool checkOnBlock(int32_t x, int8_t y, int32_t z);
  bool updateLook(float yaw, float pitch);
  int8_t relativeToBlock(const int32_t x, const int8_t y, const int32_t z);

  bool sendOthers(uint8_t* data, uint32_t len);
  static bool sendAll(uint8_t* data, uint32_t len);
  static bool sendAdmins(uint8_t* data, uint32_t len);
  static bool sendOps(uint8_t* data, uint32_t len);
  static bool sendGuests(uint8_t* data, uint32_t len);

  //Login
  bool sendLoginInfo();

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
  bool pushMap(bool login = false);

  //Push remove queued map data to client
  bool popMap();

  bool teleport(double x, double y, double z, int map = -1);
  bool spawnUser(int x, int y, int z);
  bool spawnOthers();
  bool sethealth(int userHealth);
  bool respawn();
  bool dropInventory();
  bool isUnderwater();

  void clearLoadingMap();


  // Getter/Setter for item currently in hold
  int16_t currentItemSlot();
  void setCurrentItemSlot(int16_t item_slot);


  bool withinViewDistance(int a, int b)
  {
    return a > b ? (a - b) < viewDistance : (b - a) < viewDistance;
  }

  struct event* GetEvent();

private:
  event m_event;

  // Item currently in hold
  int16_t m_currentItemSlot;
};

#endif
