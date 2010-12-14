cmake_minimum_required(VERSION 2.6)
project(Mineserver)
cmake_policy(SET CMP0003 NEW)

# project version
set( ${PROJECT_NAME}_MAJOR_VERSION 0 )
set( ${PROJECT_NAME}_MINOR_VERSION 1 )
set( ${PROJECT_NAME}_PATCH_LEVEL 0 )

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# Set compiler specific build flags
IF (UNIX)
   add_definitions(-msse -Wunused -g3 -O3 -Wall)
ENDIF()

IF (MSVC_IDE)
	set(PLATFORM_C_FLAGS "/D \"WIN32\" /D \"_WINDOWS\" /fp:fast /EHsc")
	set(CMAKE_CXX_FLAGS "${PLATFORM_C_FLAGS}")
	set(CMAKE_CXX_FLAGS_RELEASE "${PLATFORM_C_FLAGS}")
	set(CMAKE_CXX_FLAGS_DEBUG "${PLATFORM_C_FLAGS} /MDd")
ENDIF (MSVC_IDE)

file(GLOB_RECURSE folder_source ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
source_group("Mineserver" FILES ${folder_source})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)

set(dependency_error False)

# find libraries

find_package(ZLIB REQUIRED)
if (NOT ZLIB_FOUND)
   set(errors "${errors}\t\t- ZLib\n")
   set(dependency_error True)
endif()


#find_package(Lua51)
#if (NOT LUA51_FOUND)
#   set(errors "${errors}\t\t- LUA 5.1\n")
#   set(dependency_error True)
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
  add_definitions(-DDEBIAN)
endif()

#Provides ncurses, too
find_package(Curses)

if (WINDOWS)
   # even if 64bit this is set
   set(exe "WIN32")
endif ()

if (NOT dependency_error)
   include_directories(${ZLIB_INCLUDE_DIR})
#   include_directories(${LUA_INCLUDE_DIR})
   include_directories(${EVENT_INCLUDE_DIR})
   include_directories(${NOISE_INCLUDE_DIR})
   include_directories(${CURSES_INCLUDE_DIR})

   add_executable(mineserver ${exe} ${folder_source})

   target_link_libraries(mineserver ${ZLIB_LIBRARY})
#   target_link_libraries(mineserver ${LUA_LIBRARY})
   target_link_libraries(mineserver ${EVENT_LIBRARY})
   target_link_libraries(mineserver ${NOISE_LIBRARY})
   target_link_libraries(mineserver ${CURSES_LIBRARY})
else()
   message(FATAL_ERROR "\n\tNot all dependencies could be found:\n${errors}\n After installing them please rerun cmake.\n")
endif()

set(CPACK_PACKAGE_NAME "Mineserver")
set(CPACK_PACKAGE_VENDOR "The Mineserver Project")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Mineserver Developers <developers@mineserver.be>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Mineserver - C++ Minecraft server software")
set(CPACK_PACKAGE_VERSION "0.1.12")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "12")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "/usr/local/bin")

include(CPack)
