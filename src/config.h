#ifndef _CONFIG_H
#define _CONFIG_H

class Conf
{
    private:
        Conf() {};
        std::map<std::string, std::string> confSet;
    public:
        bool load(std::string configFile);
        bool save(std::string configFile);
        std::string value(std::string name);
        static Conf &get();

};

#endif