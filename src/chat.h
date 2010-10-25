#ifndef _CHAT_H
#define _CHAT_H

#define ALL 1
#define USER 2
#define OTHERS 3

class Chat 
{
    public:
        Chat();
        std::deque<std::string> admins;
        bool handleMsg( User *user, std::string msg );
        bool sendMsg( User *user, std::string msg, int action );
        bool sendUserlist( User *user );
    private:
        std::deque<std::string> parseCmd(std::string cmd);
};

#endif