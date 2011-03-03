#
# plugin sources
#
set(mineserver_plugins
  commands
  filelog
  nBreak
  nether
  binlog
  crapMobs
  cursesui
  flatpermissions
  passiveMobs
)

set(commands_source
  commands/commands.cpp
)
set(filelog_source
  filelog/filelog.cpp
)
set(nBreak_source
  nBreak/nBreak.cpp
)
set(nether_source
  nether/nether.cpp
)
set(binlog_source
  binlog/binlog.cpp
)
set(crapMobs_source
  crapMobs/crapMobs.cpp
)
set(cursesui_source
  cursesui/screen.cpp
)
set(flatpermissions_source
  flatpermissions/flatpermissions.cpp
)
set(passiveMobs_source
  passiveMobs/passiveMobs.cpp
)

include_directories(src/)

add_definitions(-DFADOR_PLUGIN=1)


#
# find libraries
#
set(dependency_error False)

find_package(ZLIB REQUIRED)
if (NOT ZLIB_FOUND)
  set(errors "${errors}\t\t- ZLib\n")
  set(dependency_error True)
endif()

#find_package(Lua51)
#if (NOT LUA51_FOUND)
#  set(errors "${errors}\t\t- LUA 5.1\n")
#  set(dependency_error True)
#endif()

find_package(Event REQUIRED)
if (NOT EVENT_FOUND)
  set(errors "${errors}\t\t- Event library\n")
  set(dependency_error True)
endif()

find_package(Noise REQUIRED)
if (NOT NOISE_FOUND)
  set(errors "${errors}\t\t- libNoise library\n")
  set(dependency_error True)
endif()

#Debian?
FIND_PATH(NOISE_ON_DEBIANISH libnoise/noise.h ${NOISE_INCLUDE_DIR})
if(NOISE_ON_DEBIANISH)
  add_definitions(-DLIBNOISE)
endif()

#Provides ncurses, too
find_package(Curses)

if (dependency_error)
  message(FATAL_ERROR "\n\tNot all dependencies could be found:\n${errors}\n After installing them please rerun cmake.\n")
endif()


#
# build targets
#
include_directories(${ZLIB_INCLUDE_DIR})
#include_directories(${LUA_INCLUDE_DIR})
include_directories(${EVENT_INCLUDE_DIR})
include_directories(${NOISE_INCLUDE_DIR})
include_directories(${CURSES_INCLUDE_DIR})

# plugins
foreach(p ${mineserver_plugins})
  message(STATUS "Plugin added: ${p}")
  add_library(${p} SHARED ${${p}_source})
  set_target_properties(${p} PROPERTIES PREFIX "")
endforeach()

#
# install
#
install(TARGETS ${mineserver_plugins}
  LIBRARY DESTINATION share/${PROJECT_NAME}/plugins/
)
