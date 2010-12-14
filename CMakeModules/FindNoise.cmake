# NOISE_LIBRARY
# NOISE_FOUND, if false, libnoise wasn't found
# NOISE_INCLUDE_DIR, where to find the headers

FIND_PATH(NOISE_INCLUDE_DIR noise/noise.h
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

#Debian
IF(NOT NOISE_INCLUDE_DIR)
    FIND_PATH(NOISE_INCLUDE_DIR libnoise/noise.h
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
ENDIF(NOT NOISE_INCLUDE_DIR)

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

# handle the QUIETLY and REQUIRED arguments and set NOISE_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE("FindPackageHandleStandardArgs")
FIND_PACKAGE_HANDLE_STANDARD_ARGS(NOISE REQUIRED_VARS NOISE_LIBRARY NOISE_INCLUDE_DIR)

