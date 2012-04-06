# Mineserver
*by Fador & Nredor and others*

Custom Minecraft server software written in C++.

You can find the core team from #mineserver @ irc.freenode.net Also mail to developers@mineserver.be will reach us.

    Copyright (c) 2010-2012, The Mineserver Project

**NOTICE:** Server still lacks features compared to original minecraft server.


**Supported platforms**

 * Linux
 * Windows (VS2010 project included)
 
Server has also been tested to build on Mac OS X and FreeBSD/PCBSD but we do not officially support those.

We are trying to minimize memory and CPU usage compared to original Java server.

*Important:* The language is C++98 with STL/TR1 library support, so you will need a reasonably modern
C++ compiler. MS Visual C++ 10 and GCC 4.4 should be sufficient. The build system requires [cmake](http://www.cmake.org/).

### Features

 * Plugins (chatcommands, filelog, nether, passivemobs, etc..)
 * NBT parsing/saving
 * Lightmap generation
 * Config file
 * Normal mining/item spawning/item pickups
 * Basic physics for gravel and sand
 * Physics for water (currently revising this)
 * Flatland and terrain map generation (Also biomegen!)
 * Working chests, furnaces & signs

### Configuration Notes

An example `config.cfg` file is located in the `files` subdirectory.

The config file sets three path names and one file name which are relevant to the server; with their example values they are:

    system.path.data    = "files";
    system.path.plugins = "plugins";
    system.path.home    = ".";
    system.pid_file     = "mineserver.pid";

These paths may be either absolute or relative. The server requires a set of configuration files which are located in the `files` subdirectory
of the distribution. 

The plugin binaries must be manually placed in the appropriate location for the server to find them.


### Bug Tracker / To Do / Roadmap

We use GitHub issue tracker and milestones.
Please check the wiki pages for details about the project status and development plans.
Check the [fork network](https://github.com/fador/mineserver/network) for the latest development status.

### Compiling

Mineserver requires the following libraries:

 * [zlib 1.2.5](http://www.zlib.org)
 * [libevent 1.4.14b](http://monkey.org/~provos/libevent/)
 * [libnoise 1.0](http://libnoise.sourceforge.net/)

 * Installing on Debian and Ubuntu: (For Ubuntu libevent1 -> libevent-1.4-2)

    `sudo apt-get install libevent1 libevent-dev zlib1g zlib1g-dev libnoise-dev`

 * Installing on CentOS and RHEL

    `# Install EPEL (Extra Packages for Enterprise Linux)`
    `sudo su -c 'rpm -Uvh http://download.fedora.redhat.com/pub/epel/5/i386/epel-release-5-4.noarch.rpm'`
    `sudo yum install git libevent libevent-devel zlib zlib-devel libnoise-dev`

 * Installing on FreeBSD / PCBSD

    `sudo pkg_add -r libevent`

 * Installing on Mac OS X
    * Install libevent using your favourite tool, e.g., homebrew, fink or macports.
    * Installing libnoise can be painful. Howerver, there is a homebrew formular
      on [krono's homebrew fork](http://github.com/krono/homebrew).

    So for homebrew do

    `brew install libevent libnoise`


**Compiling using Linux (cmake & make & g++):**

 * Download and extract source or use `git clone git://github.com/fador/mineserver.git`
 * Go to mineserver directory
 * Run `cmake .`
 * Run `make all`
 * Run server with `cd bin && ./mineserver`

**Compiling using FreeBSD / PCBSD (cmake & gmake & g++):**

 * Download and extract source or use `git clone git://github.com/fador/mineserver.git`
 * Go to mineserver directory
 * Run `cmake .`
 * Run `gmake all`
 * Run server with `cd bin && ./mineserver`

**Compiling using Windows (VS2010): (Might be outdated)**

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


