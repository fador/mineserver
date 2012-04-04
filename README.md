# Mineserver
Custom Minecraft server software written in C++.
*by Fador & Nredor and others*

You can find the core team from #mineserver @ irc.freenode.net
Also mail to developers@mineserver.be will reach us.

    Copyright (c) 2010-2012, The Mineserver Project

### NOTICE
Server still lacks features compared to original minecraft server.

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

### Current bugs
 * Lightning bug that causes clients massive CPU usage

### Configuration Notes (Deprecated)

All runtime configuration of mineserver begins with the *config file*. This file is searched for in the following order:

 1. A single non-option argument (not starting with '+' or '-') is tried as a filename.
 2. (Debug build only:) A file named `config.cfg` is searched in the same directory as the executable.
 3. (The canonical location:) A file named `config.cfg` is searched for in `$HOME/.mineserver` (Unix-style)
    or `%APPDATA%\Mineserver` (Windows).

An example `config.cfg` file is located in the `files` subdirectory.

The config file sets three path names and one file name which are relevant to the server; with their example values they are:

    system.path.data    = "~/.mineserver/files";
    system.path.plugins = "~/.mineserver/plugins";
    system.path.home    = "~/.mineserver";
    system.pid_file     = "mineserver.pid";

These paths may be either absolute (as in the first three examples) or relative (as in the last case). A path is
absolute if it starts with a path separator or a tilde; otherwise it is relative. (Drive letters for Windows will
have to be incorporated eventually.) Relative paths are *relative to the location of the config file!*

The server requires a set of configuration files which are located in the `files` subdirectory
of the distribution. The program contains rudimentary logic to copy some of those files, but this
is incomplete at present and you must ensure manually that those config files are copied into
the directory pointed to by your config file, or you must set the path parameters to the directory
in your source distribution. The same applies to the plugin binaries, which you must place in the
appropriate location for the server to find them.


### Bug tracker

Use the GitHub Issues list.

### To Do / Roadmap

Please check the [Mineserver wiki]((http://www.mineserver.be/wiki) for details about the project status and development plans.
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
 * Run `make all`  (`make install` does not currently work)
 * Run server with `cd bin && ./mineserver`

**Compiling using FreeBSD / PCBSD (cmake & gmake & g++):**

 * Download and extract source or use `git clone git://github.com/fador/mineserver.git`
 * Go to mineserver directory
 * Run `cmake .`
 * Run `gmake all install`
 * Run server with `cd bin && ./mineserver`

**Compiling using Windows (VS2010/VS2008): (Might be outdated)**

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


