#ifndef _CONSTANTS_H
#define _CONSTANTS_H

// Mineserver constants

// Chat colors
#define COLOR_BLACK std::string("§0")
#define COLOR_DARK_BLUE std::string("§1")
#define COLOR_DARK_GREEN std::string("§2")
#define COLOR_DARK_CYAN std::string("§3")
#define COLOR_DARK_RED std::string("§4")
#define COLOR_DARK_MAGENTA std::string("§5")
#define COLOR_DARK_ORANGE std::string("§6")
#define COLOR_GREY std::string("§7")
#define COLOR_DARK_GREY std::string("§8")
#define COLOR_BLUE std::string("§9")
#define COLOR_GREEN std::string("§a")
#define COLOR_CYAN std::string("§b")
#define COLOR_RED std::string("§c")
#define COLOR_MAGENTA std::string("§d")
#define COLOR_YELLOW std::string("§e")
#define COLOR_WHITE std::string("§f")


const int DEFAULT_PORT = 25565;
const std::string VERSION = "0.1.7";
const std::string ADMINFILE = "admin.txt";
const std::string DEFAULTADMINFILE = "# This is default admin file. Write admin nicks here one per line";
const std::string MOTDFILE = "motd.txt";
const std::string CONFIGFILE = "config.cfg";

const char SERVERMSGPREFIX = '%';
const char CHATCMDPREFIX = '/';
const char COMMENTPREFIX = '#';
const std::string DEFAULTKICKMSG = "This is a default kick message";

//Chunk release time on seconds when not user
const int DEFAULT_MAP_RELEASE_TIME = 10;

#endif