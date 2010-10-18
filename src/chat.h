#ifndef _CHAT_H
#define _CHAT_H

#define ALL 1
#define USER 2

class Chat 
{
    private:
        bool sendMsg( User *user, std::string msg, int action );
    public:
        bool handleMsg( User *user, std::string msg );
};

#endif