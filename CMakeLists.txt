CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(mineserver)
CMAKE_POLICY(SET CMP0003 NEW)

#
# project version
#

# Justasic: Get the git revision location for the branch we're on
if(EXISTS "${${PROJECT_NAME}_SOURCE_DIR}/.git/HEAD")
  file(STRINGS ${${PROJECT_NAME}_SOURCE_DIR}/.git/HEAD GIT_HEAD_LOC REGEX ".*")
  #file(GIT_HEAD_LOC ${${PROJECT_NAME}_SOURCE_DIR}/.git/HEAD REGEX ".*")
  string(LENGTH ${GIT_HEAD_LOC} HEAD_LEN)
  math(EXPR LEN "${HEAD_LEN} - 5")
  string(SUBSTRING ${GIT_HEAD_LOC} 5 ${LEN} GIT_HEAD)
endif(EXISTS "${${PROJECT_NAME}_SOURCE_DIR}/.git/HEAD")

# Justasic: Get the git revision we're on for the version string
if(EXISTS "${${PROJECT_NAME}_SOURCE_DIR}/.git/${GIT_HEAD}")
  # Justasic: read_from_file(${${PROJECT_NAME}_SOURCE_DIR}/.git/${GIT_HEAD} ".*" VERSION_STR)
  file(STRINGS ${${PROJECT_NAME}_SOURCE_DIR}/.git/${GIT_HEAD} VERSION_STR REGEX ".*")
  # Justasic: Get the length of the string
  string(LENGTH ${VERSION_STR} VERSION_LEN)
  # Justasic: Subtract 7 from the string's length
  math(EXPR VERSION_NUM_LEN "${VERSION_LEN} - ${VERSION_LEN} + 7")
  # Justasic: Extract the value from the string
  string(SUBSTRING ${VERSION_STR} 0 ${VERSION_NUM_LEN} VERSION_GIT)
endif(EXISTS "${${PROJECT_NAME}_SOURCE_DIR}/.git/${GIT_HEAD}")

# Set the actual version strings, these are used inside the program later
SET(${PROJECT_NAME}_MAJOR_VERSION 0)
SET(${PROJECT_NAME}_MINOR_VERSION 1)
SET(${PROJECT_NAME}_PATCH_LEVEL 17)

# Justasic: the rest is automatic
SET(${PROJECT_NAME}_VERSION_SIMPLE "${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_LEVEL}")
# Justasic: Get the git revision as well, really only useful for locating what revision we're on
SET(${PROJECT_NAME}_VERSION_COMPLETE "${${PROJECT_NAME}_VERSION_SIMPLE}-${VERSION_GIT}")

# set default build to Debug
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug
    CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
    FORCE
  )
ENDIF(NOT CMAKE_BUILD_TYPE)

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
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/configure.h.in ${PROJECT_BINARY_DIR}/include/configure.h)

#
# Include directories
#
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/include)


# CMake crap to include
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckTypeSize)
include(CheckLibraryExists)
include(CheckCXXCompilerFlag)

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
SET(TARGET ${PROJECT_NAME})
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
# Create some directories we need
#
FILE(MAKE_DIRECTORY "${CONFIG_DIR_BIN}/plugins")

#
# subdirectories to build
#
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(plugins)
ADD_SUBDIRECTORY(files)

# In the Debug build, we provide a local config file.
#IF(CMAKE_BUILD_TYPE MATCHES Debug)
    #CONFIGURE_FILE(files/config.cfg "${EXECUTABLE_OUTPUT_PATH}/config.cfg" COPYONLY)
    #CONFIGURE_FILE(files/item_alias.cfg "${EXECUTABLE_OUTPUT_PATH}/item_alias.cfg" COPYONLY)
#ENDIF()

#
# install
#
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
