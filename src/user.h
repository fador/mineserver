#ifndef _USER_H
#define _USER_H

#include <deque>
#include <event.h>
#include "tools.h"
#include "constants.h"

  struct position
  {
    double x;
    double y;
    double z;
    double stance;
    float yaw;
    float pitch;
  };

  struct coord
  {
    int x;
    int y;
    int z;
  };

  struct Item
  {
    sint16 type;
    sint8 count;
    sint16 health;
    Item()
    {
      type=0;
      count=0;
      health=0;
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
    public:

      User(int sock,uint32 EID);
      ~User();

      int fd;
      struct bufferevent *buf_ev;
      //View distance in chunks -viewDistance <-> viewDistance
      static const int viewDistance=10;
      uint8 action;
      bool waitForData;
      bool logged;
      bool admin;
      unsigned int UID;
      std::string nick;
      position pos;
      coord curChunk;
      Inventory inv;

      //Input buffer
      std::deque<unsigned char> buffer;


      bool changeNick(std::string nick, std::deque<std::string> admins);
      bool updatePos(double x, double y, double z, double stance);
      bool updateLook(float yaw, float pitch);

      bool sendOthers(uint8* data,uint32 len);
      bool sendAll(uint8* data,uint32 len);
      bool sendAdmins(uint8* data,uint32 len);

      //Check inventory for space
      bool checkInventory(sint16 itemID, char count);

      //Load/save player data from/to a file at <mapdir>/players/<nick>.dat
      bool saveData();
      bool loadData();
        
      // Kick player
      bool kick(std::string kickMsg);

      //Map related

      //Map queue
      std::vector<coord> mapQueue;

      //Chunks needed to be removed from client
      std::vector<coord> mapRemoveQueue;

      //Known map pieces
      std::vector<coord> mapKnown;

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
  };

  User *addUser(int sock,uint32 EID);
  bool remUser(int sock);
  bool isUser(int sock);
  uint32 generateEID();

  extern std::vector<User *> Users;
    
  User *getUserByNick(std::string nick);

  bool SortVect(const coord &first, const coord &second);

#endif