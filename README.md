# Mineserver
*by Fador & Nredor and others*

You can find the core team from #mineserver @ irc.esper.net
Also mail to developers@mineserver.be will reach us.

    Copyright (c) 2010, The Mineserver Project

Custom Minecraft Alpha server software written in C++.

### == NOTICE ==
Currently in (early) development. (Alpha stage)
Server still lacks some features and should be only used for testing.

**Tested to build on**

 * Windows (VS2010 and VS2008 projects included)
 * Linux (Makefile included)
 * Mac OS X
 * FreeBSD / PCBSD

We are trying to minimize memory and CPU usage compared to original Java server.

### Features (Some features are currently not operational because of major code rewrite)
 * Various chat commands, more to come..
 * NBT parsing/saving
 * Lightmap generation
 * Config file
 * Normal mining/item spawning/item pickups
 * Basic physics for gravel and sand
 * Experimental physics for water
 * Flatland map generation
 * Working chests, furnaces & signs
 * Terrain map generation with caves and minerals

### ToDo is located at [http://redmine.fknsrs.biz/projects/mineserver](http://redmine.fknsrs.biz/projects/mineserver)

### Chat commands can be found at [wiki](http://www.mineserver.be/wiki) or by typing `/help` within the client's chat

### Compiling (Better instructions in [wiki](http://www.mineserver.be/wiki))
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

 * Installing on Mac OS X
    * Install libevent using your favourite tool, e.g., homebrew, fink or macports.
    * Installing libnoise can be painful. Howerver, there is a homebrew formular
      on [krono's homebrew fork](http://github.com/krono/homebrew).

    So for homebrew do
    
    `brew install libevent libnoise`


**Compiling using linux (make & gcc):**

 * Download and extract source or use `git clone git://github.com/fador/mineserver.git`
 * Go to mineserver/src/ directory
 * Run `make` or `make DEBIAN=1` for Debian/Ubuntu
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



