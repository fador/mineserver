cmake_minimum_required(VERSION 2.6)
# forbid running cmake from this subdir
if(PROJECT_NAME STREQUAL "Project")
  message(FATAL_ERROR "\nplease run cmake from the project's parent directory\n")
endif()

#
# Get Mineserver sources
#
FILE(GLOB_RECURSE ${PROJECT_NAME}_source RELATIVE ${PROJECT_SOURCE_DIR}/src/ "*.cpp")

#
# Remove any duplicates and sort the list
#
LIST(REMOVE_DUPLICATES ${PROJECT_NAME}_source)
LIST(SORT ${PROJECT_NAME}_source)

# Exclude sources in src/plugins directory, that's compiled elsewhere
FOREACH(source ${${PROJECT_NAME}_source})
  if(${source} MATCHES "plugins/*")
    LIST(REMOVE_ITEM ${PROJECT_NAME}_source ${source})
  ENDIF(${source} MATCHES "plugins/*")
ENDFOREACH(source ${${PROJECT_NAME}_source})

# Set the list as s source group
SOURCE_GROUP(${PROJECT_NAME} FILES ${${PROJECT_NAME}_source})

# Override the default link destination
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../${CONFIG_DIR_BIN}")

# Build Mineserver
SET(TARGET ${PROJECT_NAME})
MESSAGE(STATUS "Target: ${TARGET}")
ADD_EXECUTABLE(${TARGET} ${exe} ${${TARGET}_source})
TARGET_LINK_LIBRARIES(${TARGET} ${CMAKE_DL_LIBS} ${mineserver_depends})

# FOREACH(lib ${${TARGET}_depends})
#   STRING(TOUPPER ${lib} LIB)
#   INCLUDE_DIRECTORIES(${${LIB}_INCLUDE_DIR})
#   TARGET_LINK_LIBRARIES(${TARGET} ${${LIB}_LIBRARY})
# ENDFOREACH()

# Where to install the target file.
install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION ${CONFIG_DIR_BIN}/
)