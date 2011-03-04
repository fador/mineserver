#
# plugin sources
#
set(plugins
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


add_definitions(-DFADOR_PLUGIN=1)


#
# dependencies
#
set(cursesui_depends
  Curses
)


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


#
# install
#
install(TARGETS ${TARGETS}
  LIBRARY DESTINATION share/${PROJECT_NAME}/plugins/
)
