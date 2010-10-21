#ifndef _USER_H
#define _USER_H

#include <deque>
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

    class User
    {
      public:

        User(SOCKET sock,uint32 EID);
        ~User();

        //View distance in chunks -viewDistance <-> viewDistance
        static const int viewDistance=2;
        uint8 action;
        bool waitForData;
        bool logged;
        bool admin;
        SOCKET sock;
        unsigned int UID;
        std::string nick;
        position pos;
        coord curChunk;

        std::deque<unsigned char> buffer;


        bool changeNick(std::string nick, std::deque<std::string> admins);
        bool updatePos(double x, double y, double z, double stance);
        bool updateLook(float yaw, float pitch);

        bool sendOthers(uint8* data,uint32 len);
        bool sendAll(uint8* data,uint32 len);
        
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

    bool addUser(SOCKET sock,uint32 EID);
    bool remUser(SOCKET sock);
    bool isUser(SOCKET sock);
    uint32 generateEID();

    extern std::vector<User> Users;
    
    User *getUserByNick(std::string nick);

    bool SortVect(const coord &first, const coord &second);

#endif