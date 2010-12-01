# Mineserver
*by Fador & Nredor and others*

You can find the core team from #mineserver @ irc.esper.net   
Also mail to developers@mineserver.be will reach us.

    Copyright (c) 2010, The Mineserver Project

Custom Minecraft Alpha server software written in C++.

### == NOTICE ==
Currently in (early) development. (Alpha stage)   
Server still lacks some critical features and should be only used for testing.

**Tested to build on**

 * Windows (VS2010 and VS2008 projects included)
 * Linux (Makefile included)
 * Mac OS X
 * FreeBSD / PCBSD
 
We are trying to minimize memory and cpu usage compared to original Java server.

### Features
 * Various chat commands, more to come..
 * NBT parsing/saving
 * Lightmap generation
 * Config file
 * Normal mining/item spawning/item pickups
 * Basic physics for gravel and sand
 * Experimental physics for water and lava
 * Flatland map generation
 * Working chests & signs
 * Terrain generation
  
### ToDo (Arranged by priority)
 * Getting furnaces to work (In progress)
 * Plugin support (and Lua plugin for scripting)
 * Multithreading
 * Server console
 * Growing trees and cactuses
 * One-click update
 * and more!
 
### Chat commands

**For all players**

*  /players, /who : Lists online players
*  /about : Server name & version
*  /rules : Shows server rules
*  /home : Teleports player to map spawn location
*  /kit (name) : Gives kit. Items for kit defined in config.cfg with kit_(name) using itemId's
*  /motd : Displays Message Of The Day
*  /dnd : Do Not Disturb - toggles whether the player receives chat & private messages
*  /emote, /em, /e, /me : Emote
*  /whisper, /w, /tell, /t : Send a private message to another player

**Admin only**

*  %text : Servermessage 
*  &text : Admin-only message
*  /kick player [kickmsg] : Kicks player with optional kick message
*  /ban player [reason] : Bans (and kicks if online) player from server
*  /unban player : Lift a ban
*  /save : Manually save map to disc
*  /ctp x y z : Teleport to coordinates (eg. /ctp 100 100 100)
*  /tp player : Teleport yourself to player's position
*  /tp player1 player2 : Teleport player1 to player2
*  /reload : Reload admins and configuration
*  /give player id/alias (count) : Gives player count pieces of id/alias. count = 1 is used if it is not provided. Support for over 64 items. Aliases configurable with item_alias.cfg
*  /rules player : Shows server rules (from rules.txt) to player
*  /gps [player] : Without player shows own coordinates & heading. With player shows player's coordinates & heading
*  /settime time : Sets server time. time = 0-24000 (0 and 24000 is day and about 15000 is night)
*  /mute player [reason] : Mutes player with optional reason
*  /unmute player : Unmutes player
 
### Compiling
Depends on (and tested with):

 [zlib 1.2.5](http://www.zlib.org)   
 [libevent 1.4.14b](http://monkey.org/~provos/libevent/)   
 [libnoise 1.0](http://libnoise.sourceforge.net/)   

 * Installing on Debian and Ubuntu: (For Ubuntu libevent1 -> libevent-1.4-2)

    `sudo apt-get install libevent1 libevent-dev zlib1g zlib1g-dev libnoise-dev`

 * Installing on CentOS and RHEL

    `# Install EPEL (Extra Packages for Enterprise Linux)`   
    `sudo su -c 'rpm -Uvh http://download.fedora.redhat.com/pub/epel/5/i386/epel-release-5-4.noarch.rpm'`   
    `sudo yum install git libevent libevent-devel zlib zlib-devel libnoise-dev`   

 * Installing on FreeBSD / PCBSD

    `sudo pkg_add -r libevent`

**Compiling using linux (make & gcc):**

 * Download and extract source or use `git clone git://github.com/fador/mineserver.git`
 * Go to mineserver/src/ directory
 * Run `make`
 * Run server with `./mineserver`

**Compiling using FreeBSD / PCBSD (gmake & g++):**

 * Download and extract source or use `git clone git://github.com/fador/mineserver.git`
 * Go to mineserver/src/ directory
 * Run `gmake`
 * Run server with `./mineserver`
  
**Compiling using windows (VS2010/VS2008):**

 * Download and compile [zlib](http://www.zlib.org) or use [pre-built binaries](http://www.winimage.com/zLibDll/index.html)
 * Add zlib libraries to project (zlibstat.lib or zlibwapi.lib which requires also zlibwapi.dll in the same dir with the executable)
 * Download and compile [libevent](http://monkey.org/~provos/libevent/)
 * Download [libnoise 1.0](http://libnoise.sourceforge.net/) and add header/library files to project
 * Add libevent library to project (libevent.lib)
 * Build
 * Run mineserver.exe
 
 An example using commandline compiler available at http://www.microsoft.com/express/Windows/ Please change the ZLIB_INC_DIR, LIBEVENT_INC_DIR, ZLIB_LIB_DIR and LIBEVENT_LIB_DIR to those you keep the includes/libs.
 
    call "%VS100COMNTOOLS%vsvars32.bat"
    cl /I"ZLIB_INC_DIR;LIBEVENT_INC_DIR" /W3 /WX- /O2 /D ZLIB_WINAPI /D WIN32 /D NDEBUG /D _CRT_SECURE_NO_WARNINGS /EHsc *.cpp zlibwapi.lib libevent.lib Ws2_32.lib /link /OUT:mineserver.exe /LIBPATH:"ZLIB_LIB_DIR;LIBEVENT_LIB_DIR"


 
