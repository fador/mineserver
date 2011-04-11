cmake_minimum_required(VERSION 2.6)
project(mineserver)
cmake_policy(SET CMP0003 NEW)

# project version
# TODO: get that from src/constants.h
set( ${PROJECT_NAME}_MAJOR_VERSION 0 )
set( ${PROJECT_NAME}_MINOR_VERSION 1 )
set( ${PROJECT_NAME}_PATCH_LEVEL 15 )

# set default build to Debug
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug
    CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
    FORCE
  )
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

#
# Configure paths
#
set(CONFIG_DIR_BIN   bin/)
set(CONFIG_DIR_ETC   etc/${PROJECT_NAME}/)
set(CONFIG_DIR_LIB   lib/${PROJECT_NAME}/)
set(CONFIG_DIR_SHARE ./)
if(WIN32)
  message(STATUS "CONFIG_LOCAL is set -- assuming local build")
  set(CONFIG_DIR_BIN   bin/)
  set(CONFIG_DIR_ETC   bin/)
  set(CONFIG_DIR_LIB   bin/)
  set(CONFIG_DIR_SHARE ./)
  set(CONFIG_PREFIX    ${CMAKE_INSTALL_PREFIX})
endif()

# preprocess configuration file
configure_file(${PROJECT_SOURCE_DIR}/src/configure.h.in
  ${PROJECT_SOURCE_DIR}/src/configure.h
)


# Output paths
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)


# Set compiler specific build flags
IF (UNIX)
  set(CMAKE_CXX_FLAGS "-march=native -W -Wall -Wno-unused -pedantic")
  set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g")
  set(CMAKE_CXX_FLAGS_PROFILE "-DDEBUG -g -pg")
  set(CMAKE_CXX_FLAGS_RELEASE "-O2 -s -DNDEBUG")
ENDIF()


IF (MSVC_IDE)
  set(CMAKE_CXX_FLAGS "/DWIN32 /D_CONSOLE /DZLIB_WINAPI /fp:fast /EHsc")
  set(CMAKE_CXX_FLAGS_DEBUG "/DDEBUG /D_DEBUG /MTd /Zi")
  set(CMAKE_CXX_FLAGS_RELEASE "/MT")
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
  src/mcregion.cpp
  src/worldgen/cavegen.cpp
  src/worldgen/heavengen.cpp
  src/worldgen/mapgen.cpp
  src/worldgen/nethergen.cpp
  src/worldgen/biomegen.cpp
  src/worldgen/eximgen.cpp
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
  src/blocks/leaves.cpp
  src/tools.cpp
  src/physics.cpp
  src/cliScreen.cpp
  src/blocks/blockfurnace.cpp
  src/blocks/note.cpp
  src/blocks/workbench.cpp
  src/blocks/pumpkin.cpp
  src/blocks/bed.cpp
  src/blocks/step.cpp
  src/blocks/jackolantern.cpp
  src/blocks/wool.cpp
  src/blocks/wood.cpp
  src/items/food.cpp
  src/items/itembasic.cpp
  src/items/projectile.cpp
  src/mob.cpp
)
source_group(${PROJECT_NAME} FILES ${mineserver_source})


#
# project files
#
set(mineserver_configs
  files/banned.txt
  files/commands.cfg
  files/config.cfg
  files/item_alias.cfg
  files/ENABLED_RECIPES.cfg
  files/motd.txt
  files/permissions.txt
  files/roles.txt
  files/rules.txt
  files/whitelist.txt
)

set(mineserver_dirs
  files/recipes
)


include_directories(src/)


#
# dependencies
#
set(mineserver_depends
  ZLIB
  #Lua51
  Event
  Noise
)


#
# dependency check
#
set(TARGET mineserver)
message(STATUS "Dependency check: ${TARGET}")
set(dependency_error False)
foreach(lib ${${TARGET}_depends})
  find_package(${lib})
  string(TOUPPER ${lib} LIB)
  if (NOT ${LIB}_FOUND)
    set(errors "${errors}\t\t- ${lib}\n")
    set(dependency_error True)
  endif()
endforeach()

if (dependency_error)
  message(FATAL_ERROR "\n\tNot all dependencies could be found:\n${errors}\nAfter installing them please rerun cmake.\n")
endif()


#
# platform-specific tweaks
#
if(WIN32)
  # this requires WinMain() entry, disabled until console/GUI option is added
  #set(exe "WIN32")

  # this shouldn't be here, but can cmake find these .lib's?
  # do we really need winmm?
  foreach(lib ws2_32 winmm)
    string(TOUPPER ${lib} LIB)
    list(APPEND mineserver_depends "${lib}")
    set(${LIB}_LIBRARY "${lib}")
  endforeach()
else()
  if(NOISE_DIR_IS_LIBNOISE)
    add_definitions(-DLIBNOISE)
  endif()
endif()


#
# subdirectories
#
add_subdirectory(plugins)


#
# build targets
#
set(TARGET mineserver)
message(STATUS "Target: ${TARGET}")
add_executable(${TARGET} ${exe} ${${TARGET}_source})
target_link_libraries(${TARGET} ${CMAKE_DL_LIBS})

foreach(lib ${${TARGET}_depends})
  string(TOUPPER ${lib} LIB)
  include_directories(${${LIB}_INCLUDE_DIR})
  target_link_libraries(${TARGET} ${${LIB}_LIBRARY})
endforeach()

# copy configs for local usage
foreach(path ${mineserver_configs})
  # make full path w/o "files/"
  string(REGEX REPLACE "^files/" "${EXECUTABLE_OUTPUT_PATH}/" dest "${path}")
  if(NOT EXISTS ${dest})
    message(STATUS "Configuring: ${dest}")
    configure_file(${path} ${dest} COPYONLY)
  endif()
endforeach()


#
# install
#
install(TARGETS mineserver
  RUNTIME DESTINATION ${CONFIG_DIR_BIN}/
)

install(FILES ${mineserver_configs}
  DESTINATION ${CONFIG_DIR_ETC}/
)
install(DIRECTORY ${mineserver_dirs}
  DESTINATION ${CONFIG_DIR_SHARE}/
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
