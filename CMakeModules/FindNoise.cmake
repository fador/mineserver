# NOISE_LIBRARY
# NOISE_FOUND, if false, libnoise wasn't found
# NOISE_INCLUDE_DIR, where to find the headers
# NOISE_DIR_IS_LIBNOISE, if true, include as libnoise/noise.h (Debian)

foreach(header noise/noise.h libnoise/noise.h)
    FIND_PATH(NOISE_INCLUDE_DIR ${header}
        $ENV{NOISE_DIR}/include
        $ENV{NOISE_DIR}
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
    if(NOISE_INCLUDE_DIR)
      break()
    endif()
endforeach()

if(EXISTS ${NOISE_INCLUDE_DIR}/libnoise/noise.h)
  set(NOISE_DIR_IS_LIBNOISE True)
endif()

IF(CMAKE_SYSTEM_NAME MATCHES FreeBSD)
 set(CMAKE_FIND_LIBRARY_SUFFIXES_ORIG ${CMAKE_FIND_LIBRARY_SUFFIXES})
 set(CMAKE_FIND_LIBRARY_SUFFIXES ".so.0")
ENDIF()

FIND_LIBRARY(NOISE_LIBRARY 
    NAMES noise libnoise
    PATHS
    $ENV{NOISE_DIR}/lib
    $ENV{NOISE_DIR}
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

IF(CMAKE_SYSTEM_NAME MATCHES FreeBSD)
 set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_ORIG})
ENDIF()

# handle the QUIETLY and REQUIRED arguments and set NOISE_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE("FindPackageHandleStandardArgs")
FIND_PACKAGE_HANDLE_STANDARD_ARGS(NOISE REQUIRED_VARS NOISE_LIBRARY NOISE_INCLUDE_DIR)

