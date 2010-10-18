#ifndef _USER_H
#define _USER_H

  typedef struct
  {
    double x;
    double y;
    double z;
    double stance;
    float yaw;
    float pitch;
  } position;

    class User
    {
      public:

        User(SOCKET sock,uint32 EID);
        ~User();

        uint8 action;
        bool waitForData;
        bool logged;
        bool admin;
        SOCKET sock;
        unsigned int UID;
        std::string nick;
        position pos;  
        std::deque<unsigned char> buffer;

        bool changeNick(std::string nick, std::deque<std::string> admins);
        bool updatePos(double x, double y, double z, double stance);
        bool updateLook(float yaw, float pitch);

        bool sendOthers(uint8* data,uint32 len);
        bool sendAll(uint8* data,uint32 len);
    };

    bool addUser(SOCKET sock,uint32 EID);
    bool remUser(SOCKET sock);
    bool isUser(SOCKET sock);
    uint32 generateEID();

    extern std::vector<User> Users;



#endif