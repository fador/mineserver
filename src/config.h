#ifndef _CONFIG_H
#define _CONFIG_H

class Config
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