//
// Mineserver logger.h
//

class Logger
{
    private:
        Logger() {};
    public:
        void tLog(std::string msg);
        static Logger &get();

};