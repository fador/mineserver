//
// Mineserver logger.h
//
#include <string>

#define LOG(msg) Logger::get().log(msg, std::string(__FILE__), __LINE__)

class Logger
{
    private:
        Logger() {};
    public:
        void log(std::string msg, std::string file, int line);
        static Logger &get();

};