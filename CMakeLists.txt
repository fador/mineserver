CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(mineserver)
CMAKE_POLICY(SET CMP0003 NEW)

# project version
# TODO: get that from src/constants.h
# louisdx: Why not go the other way and make constants.h take it from here?
SET(${PROJECT_NAME}_MAJOR_VERSION 0)
SET(${PROJECT_NAME}_MINOR_VERSION 1)
SET(${PROJECT_NAME}_PATCH_LEVEL 15)

# set default build to Debug
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug
    CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
    FORCE
  )
ENDIF()

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

#
# Configure paths
#
SET(CONFIG_DIR_BIN   bin/)
SET(CONFIG_DIR_ETC   etc/${PROJECT_NAME}/)
SET(CONFIG_DIR_LIB   lib/${PROJECT_NAME}/)
SET(CONFIG_DIR_SHARE ./)
IF(WIN32)
  MESSAGE(STATUS "CONFIG_LOCAL is set -- assuming local build")
  SET(CONFIG_DIR_BIN   bin/)
  SET(CONFIG_DIR_ETC   bin/)
  SET(CONFIG_DIR_LIB   bin/)
  SET(CONFIG_DIR_SHARE ./)
  SET(CONFIG_PREFIX    ${CMAKE_INSTALL_PREFIX})
ENDIF()

# The DISTSOURCE parameter tells the executable where to find the factory defaults.
# It is used by Mineserver::configDirectoryPrepare() to install the factory defaults.
# In the Release build, this should eventually be overridden by a central location
# like $PREFIX/share/mineserver/files, I suppose.
SET(CONFIG_DIR_DISTSOURCE files)

# preprocess configuration file
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/configure.h.in ${PROJECT_SOURCE_DIR}/src/configure.h)

# Output paths
SET(EXECUTABLE_OUTPUT_PATH bin)
SET(LIBRARY_OUTPUT_PATH bin)


# Set compiler specific build flags
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "-W -Wall -Wno-unused -pedantic -DHAVE_TR1_SUBDIR")
  SET(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g -O0")
  SET(CMAKE_CXX_FLAGS_PROFILE "-DDEBUG -g -pg")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O4 -s -DNDEBUG")
ENDIF()


IF(MSVC_IDE)
  SET(CMAKE_CXX_FLAGS "/DWIN32 /D_CONSOLE /DZLIB_WINAPI /fp:fast /EHsc")
  SET(CMAKE_CXX_FLAGS_DEBUG "/DDEBUG /D_DEBUG /MTd /Zi")
  SET(CMAKE_CXX_FLAGS_RELEASE "/MT /D_SECURE_SCL=0")
ENDIF(MSVC_IDE)


#
# mineserver sources
#
SET(mineserver_source
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
  src/random.cpp
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
  src/blocks/tnt.cpp
  src/blocks/jackolantern.cpp
  src/blocks/wool.cpp
  src/blocks/wood.cpp
  src/blocks/ice.cpp
  src/items/food.cpp
  src/items/itembasic.cpp
  src/items/projectile.cpp
  src/mob.cpp
  src/metadata.cpp
)
SOURCE_GROUP(${PROJECT_NAME} FILES ${mineserver_source})


#
# project files
#
SET(mineserver_configs
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

FILE(GLOB_RECURSE mineserver_recipes RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} files/recipes/*)

INCLUDE_DIRECTORIES(src)


#
# dependencies
#
SET(mineserver_depends
  ZLIB
  #Lua51
  Event
  Noise
)


#
# dependency check
#
SET(TARGET mineserver)
MESSAGE(STATUS "Dependency check: ${TARGET}")
SET(dependency_error False)
FOREACH(lib ${${TARGET}_depends})
  FIND_PACKAGE(${lib})
  STRING(TOUPPER ${lib} LIB)
  IF(NOT ${LIB}_FOUND)
    SET(errors "${errors}\t\t- ${lib}\n")
    SET(dependency_error True)
  ENDIF()
ENDFOREACH()


#
# platform-specific tweaks
#
IF(WIN32)
  # this requires WinMain() entry, disabled until console/GUI option is added
  #set(exe "WIN32")

  # this shouldn't be here, but can cmake find these .lib's?
  # do we really need winmm?
  FOREACH(lib ws2_32 winmm)
    STRING(TOUPPER ${lib} LIB)
    LIST(APPEND mineserver_depends "${lib}")
    SET(${LIB}_LIBRARY "${lib}")
  ENDFOREACH()
ELSE()
  IF(NOISE_DIR_IS_LIBNOISE)
    ADD_DEFINITIONS(-DLIBNOISE)
  ENDIF()
ENDIF()


#
# subdirectories
#
ADD_SUBDIRECTORY(plugins)


#
# build targets
#
SET(TARGET mineserver)
MESSAGE(STATUS "Target: ${TARGET}")
ADD_EXECUTABLE(${TARGET} ${exe} ${${TARGET}_source})
TARGET_LINK_LIBRARIES(${TARGET} ${CMAKE_DL_LIBS})

FOREACH(lib ${${TARGET}_depends})
  STRING(TOUPPER ${lib} LIB)
  INCLUDE_DIRECTORIES(${${LIB}_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(${TARGET} ${${LIB}_LIBRARY})
ENDFOREACH()

# copy configs for local usage
FOREACH(path ${mineserver_configs} ${mineserver_recipes})
  SET(dest "${EXECUTABLE_OUTPUT_PATH}/${path}")
  IF(NOT EXISTS ${dest})
    MESSAGE(STATUS "Configuring: ${dest}")
    CONFIGURE_FILE(${path} ${dest} COPYONLY)
  ENDIF()
ENDFOREACH()

# In the Debug build, we provide a local config file.
#IF(CMAKE_BUILD_TYPE MATCHES Debug)
    CONFIGURE_FILE(files/config.cfg "${EXECUTABLE_OUTPUT_PATH}/config.cfg" COPYONLY)
    CONFIGURE_FILE(files/item_alias.cfg "${EXECUTABLE_OUTPUT_PATH}/item_alias.cfg" COPYONLY)
#ENDIF()




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
