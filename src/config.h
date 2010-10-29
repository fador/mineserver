#ifndef _CONFIG_H
#define _CONFIG_H

class Conf
{
    private:
        Conf() {};
        std::vector<std::map<std::string name, std::string value> > confSet;
    public:
        bool load(std::string configFile);
        bool save(std::string configFile);
        std::string value(std::string name);
        static Conf &get();

};

#endif