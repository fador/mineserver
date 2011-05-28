cmake_minimum_required(VERSION 2.6)
# forbid running cmake from this subdir
if(PROJECT_NAME STREQUAL "Project")
  message(FATAL_ERROR "\nplease run cmake from the project's parent directory\n")
endif()

#
# plugin sources
# RULES: each unique subdirectory is plugin name
#
set(plugins_source
  commands/commands.cpp
  filelog/filelog.cpp
  nBreak/nBreak.cpp
  nether/nether.cpp
  binlog/binlog.cpp
  crapMobs/crapMobs.cpp
  cursesui/screen.cpp
  flatpermissions/flatpermissions.cpp
  passiveMobs/passiveMobs.cpp
)


#
# custom defines
#
add_definitions(-DFADOR_PLUGIN=1)


#
# dependencies
#
set(cursesui_depends
  Curses
)


###############################################################################


#
# make lists from plugins_source
#
foreach(src ${plugins_source})
  string(REGEX REPLACE "/.*$" "" p "${src}")
  list(APPEND plugins ${p})
  list(APPEND ${p}_source ${src})
endforeach()
list(REMOVE_DUPLICATES plugins)


#
# dependency check
#
set(TARGETS "")
foreach(p ${plugins})
  message(STATUS "Dependency check: ${p}")
  foreach(lib ${${p}_depends})
    find_package(${lib})
    string(TOUPPER ${lib} LIB)
    if (NOT ${LIB}_FOUND)
      set(${p}_error True)
      break()
    endif()
  endforeach()

  if(NOT ${p}_error)
    list(APPEND TARGETS ${p})
  else()
    message("\t-- DISABLED: ${p}")
  endif()
endforeach()


#
# build targets
#
foreach(p ${TARGETS})
  message(STATUS "Target: ${p}")
  add_library(${p} SHARED ${${p}_source})
  set_target_properties(${p} PROPERTIES PREFIX "")

  foreach(lib ${${p}_depends})
    string(TOUPPER ${lib} LIB)
    include_directories(${${LIB}_INCLUDE_DIR})
    target_link_libraries(${p} ${${LIB}_LIBRARY})
  endforeach()
endforeach()

set(LIBRARY_OUTPUT_PATH ${CONFIG_DIR_BIN}/plugins)


#
# install
#
install(TARGETS ${TARGETS}
  LIBRARY DESTINATION ${CONFIG_DIR_BIN}/plugins
)