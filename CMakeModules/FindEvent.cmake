# This module defines
# EVENT_LIBRARY
# EVENT_FOUND, if false, libevent wasn't found 
# EVENT_INCLUDE_DIR, where to find the headers

FIND_PATH(EVENT_INCLUDE_DIR event.h
    $ENV{EVENT_DIR}/include
    $ENV{EVENT_DIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    /usr/freeware/include
)

FIND_LIBRARY(EVENT_LIBRARY 
    NAMES event libevent
    PATHS
    $ENV{EVENT_DIR}/lib
    $ENV{EVENT_DIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    /usr/freeware/lib64
)


# handle the QUIETLY and REQUIRED arguments and set EVENT_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE("FindPackageHandleStandardArgs")
FIND_PACKAGE_HANDLE_STANDARD_ARGS(EVENT REQUIRED_VARS EVENT_LIBRARY EVENT_INCLUDE_DIR)
