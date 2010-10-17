#ifndef _CHAT_H
#define _CHAT_H

class Chat 
{
    public:
        bool handleMsg( User *user, std::string msg );
};

#endif