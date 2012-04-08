cmake_minimum_required(VERSION 2.6)
# forbid running cmake from this subdir
if(PROJECT_NAME STREQUAL "Project")
  message(FATAL_ERROR "\nplease run cmake from the project's parent directory\n")
endif()

# Get all the current config files and such from the current directory
FILE(GLOB mineserver_configs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.txt *.cfg)
LIST(REMOVE_ITEM mineserver_configs "CMakeLists.txt")
LIST(REMOVE_DUPLICATES mineserver_configs)

# Install all the Recipes
FILE(GLOB_RECURSE mineserver_recipes RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "recipes/*")
LIST(REMOVE_DUPLICATES mineserver_recipes)

# copy configs for local usage
FOREACH(path ${mineserver_configs} ${mineserver_recipes})
  SET(dest "${PROJECT_SOURCE_DIR}/bin/${path}")
  IF(NOT EXISTS ${dest})
    MESSAGE(STATUS "Configuring: ${dest}")
    CONFIGURE_FILE(${path} ${dest} COPYONLY)
  ENDIF()
ENDFOREACH()