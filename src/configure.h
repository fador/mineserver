//
// this is default configure.h for non-cmake builds
// it will be overwritten by running cmake
//

#ifndef _CONFIGURE_H
#define _CONFIGURE_H


// prefix and directory names
#define CONFIG_PREFIX    "./"
#define CONFIG_DIR_BIN   "bin/"
#define CONFIG_DIR_ETC   "bin/"
#define CONFIG_DIR_LIB   "bin/"
#define CONFIG_DIR_SHARE "files/"

// paths
// NOTE: these paths are absolute only when prefix is absolute path!
#define CONFIG_PATH_BIN   CONFIG_PREFIX""CONFIG_DIR_BIN
#define CONFIG_PATH_ETC   CONFIG_PREFIX""CONFIG_DIR_ETC
#define CONFIG_PATH_LIB   CONFIG_PREFIX""CONFIG_DIR_LIB
#define CONFIG_PATH_SHARE CONFIG_PREFIX""CONFIG_DIR_SHARE


#endif // _CONFIGURE_H
