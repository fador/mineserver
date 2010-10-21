//
// Mineserver logger.h
//
#include <string>

class Logger
{
    private:
        Logger() {};
    public:
        void log(std::string msg);
        static Logger &get();

};