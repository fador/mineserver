cmake_minimum_required(VERSION 2.6)
project(Mineserver)
cmake_policy(SET CMP0003 NEW)

# project version
SET( ${PROJECT_NAME}_MAJOR_VERSION 0 )
SET( ${PROJECT_NAME}_MINOR_VERSION 1 )
SET( ${PROJECT_NAME}_PATCH_LEVEL 0 )

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# Set compiler specific build flags
IF (UNIX)
   add_definitions(-msse -Wunused -g3 -O3 -Wall)
ENDIF()

IF (MSVC_IDE)
	SET(PLATFORM_C_FLAGS "/D \"WIN32\" /D \"_WINDOWS\" /fp:fast /EHsc")
	SET(CMAKE_CXX_FLAGS "${PLATFORM_C_FLAGS}")
	SET(CMAKE_CXX_FLAGS_RELEASE "${PLATFORM_C_FLAGS}")
	SET(CMAKE_CXX_FLAGS_DEBUG "${PLATFORM_C_FLAGS} /MDd")
ENDIF (MSVC_IDE)

FILE(GLOB_RECURSE folder_source ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
SOURCE_GROUP("Mineserver" FILES ${folder_source})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src/)

if (WINDOWS)
   # even if 64bit this is set
   set(exe "WIN32")
endif ()

ADD_EXECUTABLE(mineserver ${exe} ${folder_source})
