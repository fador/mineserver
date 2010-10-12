#ifndef _USER_H
#define _USER_H

    typedef struct
    {
        unsigned int UID;
        std::string nick;
        bool active;
    } MyUser;

    extern std::vector<MyUser> Users;
    
    bool addUser(int UID, std::string nick);
    bool remUser(int UID);
    bool isUser(int UID);


#endif