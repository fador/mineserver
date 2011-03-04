cmake_minimum_required(VERSION 2.6)
project(mineserver)
cmake_policy(SET CMP0003 NEW)

# project version
# TODO: get that from src/constants.h
set( ${PROJECT_NAME}_MAJOR_VERSION 0 )
set( ${PROJECT_NAME}_MINOR_VERSION 1 )
set( ${PROJECT_NAME}_PATCH_LEVEL 15 )

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

#
# Configure paths
#
set(CONFIG_DIR_BIN   bin/)
set(CONFIG_DIR_ETC   etc/)
set(CONFIG_DIR_SHARE share/)
# local build by default
set(CONFIG_LOCAL True)

if(CONFIG_LOCAL)
  message(STATUS "CONFIG_LOCAL is set -- assuming local build")
  set(CONFIG_PREFIX    "../" CACHE STRING "CONFIG_PREFIX" FORCE)
  set(CONFIG_DIR_BIN   bin/)
  set(CONFIG_DIR_ETC   bin/)
  set(CONFIG_DIR_SHARE files/)
else()
  message(STATUS "CONFIG_LOCAL is not set -- assuming installation build")
  set(CONFIG_PREFIX    ${CMAKE_INSTALL_PREFIX})
endif()
set(CMAKE_INSTALL_PREFIX ${CONFIG_PREFIX})

# preprocess configuration file
configure_file(${PROJECT_SOURCE_DIR}/src/configure.h.in
  ${PROJECT_SOURCE_DIR}/src/configure.h
)


# Output paths
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)


# Set compiler specific build flags
IF (UNIX)
  add_definitions(-msse -Wunused -g3 -O3 -Wall)
ENDIF()

IF (CMAKE_SYSTEM_NAME MATCHES Linux)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ldl")
ENDIF()


IF (MSVC_IDE)
  set(PLATFORM_C_FLAGS "/D \"WIN32\" /D \"_WINDOWS\" /fp:fast /EHsc")
  set(CMAKE_CXX_FLAGS "${PLATFORM_C_FLAGS}")
  set(CMAKE_CXX_FLAGS_RELEASE "${PLATFORM_C_FLAGS}")
  set(CMAKE_CXX_FLAGS_DEBUG "${PLATFORM_C_FLAGS} /MDd")
ENDIF (MSVC_IDE)


#
# mineserver sources
#
set(mineserver_source
  src/lighting.cpp
  src/logger.cpp
  src/furnace.cpp
  src/plugin_api.cpp
  src/constants.cpp
  src/map.cpp
  src/mineserver.cpp
  src/furnaceManager.cpp
  src/screenBase.cpp
  src/inventory.cpp
  src/packets.cpp
  src/plugin.cpp
  src/nbt.cpp
  src/user.cpp
  src/config.cpp
  src/config/lexer.cpp
  src/config/scanner.cpp
  src/config/parser.cpp
  src/config/node.cpp
  src/sockets.cpp
  src/chat.cpp
  src/tree.cpp
  src/worldgen/cavegen.cpp
  src/worldgen/heavengen.cpp
  src/worldgen/mapgen.cpp
  src/worldgen/nethergen.cpp
  src/blocks/cake.cpp
  src/blocks/stair.cpp
  src/blocks/chest.cpp
  src/blocks/snow.cpp
  src/blocks/fire.cpp
  src/blocks/default.cpp
  src/blocks/plant.cpp
  src/blocks/sign.cpp
  src/blocks/door.cpp
  src/blocks/ladder.cpp
  src/blocks/torch.cpp
  src/blocks/basic.cpp
  src/blocks/tracks.cpp
  src/blocks/falling.cpp
  src/blocks/liquid.cpp
  src/blocks/redstone.cpp
  src/tools.cpp
  src/physics.cpp
  src/cliScreen.cpp
  src/blocks/blockfurnace.cpp
  src/blocks/dyed.cpp
  src/blocks/note.cpp
  src/blocks/workbench.cpp
  src/items/food.cpp
  src/items/itembasic.cpp
  src/items/projectile.cpp
  src/mob.cpp
  src/worldgen/biomegen.cpp
)
source_group(${PROJECT_NAME} FILES ${mineserver_source})


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
  plugins/commands/commands.cpp
)
set(filelog_source
  plugins/filelog/filelog.cpp
)
set(nBreak_source
  plugins/nBreak/nBreak.cpp
)
set(nether_source
  plugins/nether/nether.cpp
)
set(binlog_source
  plugins/binlog/binlog.cpp
)
set(crapMobs_source
  plugins/crapMobs/crapMobs.cpp
)
set(cursesui_source
  plugins/cursesui/screen.cpp
)
set(flatpermissions_source
  plugins/flatpermissions/flatpermissions.cpp
)
set(passiveMobs_source
  plugins/passiveMobs/passiveMobs.cpp
)


#
# project files
#
set(mineserver_configs
  files/banned.txt
  files/commands.cfg
  files/config.cfg
  files/item_alias.cfg
  files/motd.txt
  files/permissions.txt
  files/roles.txt
  files/rules.txt
  files/whitelist.txt
)

set(mineserver_dirs
  files/recipes
)


add_definitions(-DFADOR_PLUGIN=1)

include_directories(src/)


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

if (WINDOWS)
  # even if 64bit this is set
  set(exe "WIN32")
endif ()

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

add_executable(mineserver ${exe} ${mineserver_source})

target_link_libraries(mineserver ${ZLIB_LIBRARY})
#target_link_libraries(mineserver ${LUA_LIBRARY})
target_link_libraries(mineserver ${EVENT_LIBRARY})
target_link_libraries(mineserver ${NOISE_LIBRARY})
target_link_libraries(mineserver ${CURSES_LIBRARY})

# plugins
foreach(p ${mineserver_plugins})
  message(STATUS "Plugin target added: ${p}")
  add_library(${p} SHARED ${${p}_source})
  set_target_properties(${p} PROPERTIES PREFIX "")
endforeach()

# copy configs for local usage
foreach(path ${mineserver_configs})
  # destination doesn't have files/ prefix
  string(REGEX REPLACE "^files/" "${EXECUTABLE_OUTPUT_PATH}/" dest "${path}")
  if(NOT EXISTS ${dest})
    file(INSTALL ${path} DESTINATION ${EXECUTABLE_OUTPUT_PATH})
  endif()
endforeach()


#
# install
#
install(TARGETS mineserver ${mineserver_plugins}
  RUNTIME DESTINATION bin/
  LIBRARY DESTINATION share/${PROJECT_NAME}/plugins/
)

install(FILES ${mineserver_configs}
  DESTINATION etc/${PROJECT_NAME}/
)
install(DIRECTORY ${mineserver_dirs}
  DESTINATION share/${PROJECT_NAME}/
)


#
# cpack
#
set(CPACK_PACKAGE_VENDOR "The Mineserver Project")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Mineserver Developers <developers@mineserver.be>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Mineserver - C++ Minecraft server software")
set(CPACK_PACKAGE_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_LEVEL})
set(CPACK_PACKAGE_VERSION_MAJOR ${${PROJECT_NAME}_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${${PROJECT_NAME}_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${${PROJECT_NAME}_PATCH_LEVEL})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "/usr/local/bin")

include(CPack)
