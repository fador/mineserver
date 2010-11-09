# Mineserver
*by Fador & Psoden*

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Custom Minecraft Alpha server software written in C++.

### NOTICE
Currently in development. (Alpha stage)
Server still lacks some critical features and should be only used for testing.

Tested to build on Windows (Visual Studio 2010 project included) and on Linux (Makefile included).

We are trying to minimize memory and cpu usage compared to original Java server.

### Features
 * Various chat commands, more to come..
 * NBT parsing/saving
 * Lightmap generation
 * Config file
 * Normal mining/item spawning/item pickups
  
### ToDo (Arranged by priority)
 * Map generation
 * Getting chests,furnaces,signs etc to work
 * Physics
 * Multithreading
 * Growing trees etc.
 * and more
 
### Chat commands

**For all players**

*  /players : Lists online players
*  /about : Server name & version
*  /rules : Shows server rules

**Admin only**

*  %text : Servermessage 
*  &text : Admin-only message
*  /kick nick (kickmsg) : Kicks user with optional kick message
*  /save : Manually save map to disc
*  /ctp x y z : Teleport to coordinates (eg. /ctp 100 100 100)
*  /tp nick : Teleport yourself to nick's position
*  /tp nick1 nick2 : Teleport nick1 to nick2
*  /reload : Reload admins and configuration
*  /give nick id/alias (count) : Gives nick count pieces of id/alias. count = 1 is used if it is not provided. Support for over 64 items. Aliases configurable with item_alias.cfg
*  /rules nick : Shows server rules (from rules.txt) to nick
 
### Compiling
Depends on (and tested with):

 * [zlib 1.2.5](http://www.zlib.org)
 * [libevent 1.4.14b](http://monkey.org/~provos/libevent/)
 * [lua 5.1.4](http://www.lua.org) (Not yet in use!)

 Installing on Debian and Ubuntu:

    sudo apt-get install libevent1 libevent-dev zlib1g zlib1g-dev


Compiling using linux (make & gcc):

 * Download and extract source or use `git clone git://github.com/fador/mineserver.git`
 * Go to mineserver/src/ directory
 * Run make
 * Run server with ./mineserver
  
Compiling using windows (VS2010):

 * Download and compile [zlib](http://www.zlib.org) or use [pre-built binaries](http://www.winimage.com/zLibDll/index.html)
 * Add zlib libraries to project (zlibstat.lib or zlibwapi.lib which requires also zlibwapi.dll in the same dir with the executable)
 * Download and compile [libevent](http://monkey.org/~provos/libevent/)
 * Add libevent library to project (libevent.lib)
 * Add libraries to project and library headers to include path
 * Build
 * Run mineserver.exe
 
 An example using commandline compiler available at http://www.microsoft.com/express/Windows/ Please change the ZLIB_INC_DIR, LIBEVENT_INC_DIR, ZLIB_LIB_DIR and LIBEVENT_LIB_DIR to those you keep the includes/libs.
 
    call "%VS100COMNTOOLS%vsvars32.bat"
    cl /I"ZLIB_INC_DIR;LIBEVENT_INC_DIR" /W3 /WX- /O2 /D WIN32 /D NDEBUG /D _CRT_SECURE_NO_WARNINGS /EHsc *.cpp zlibwapi.lib libevent.lib Ws2_32.lib /link /OUT:mineserver.exe /LIBPATH:"ZLIB_LIB_DIR;LIBEVENT_LIB_DIR"

  
 