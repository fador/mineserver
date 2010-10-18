#ifndef _CHAT_H
#define _CHAT_H

#define ALL 1
#define USER 2

class Chat 
{
    public:
        Chat();
        std::deque<std::string> admins;
        bool handleMsg( User *user, std::string msg );
        bool sendMsg( User *user, std::string msg, int action );
};

#endif