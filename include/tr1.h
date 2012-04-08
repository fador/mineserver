#ifndef _TR1_INCLUDE_H
#define _TR1_INCLUDE_H

/** Usage: #include TR1INCLUDE(unordered_map)
 **
 ** Configuration: Define HAVE_TR1_SUBDIR if you need #include <tr1/unordered_map>; otherwise we take #include <unordered_map>.
 **
 **/

#define QUOTE(arg) <arg>

#ifdef HAVE_TR1_SUBDIR
#  define TR1IFY(arg) tr1/arg
#else
#  define TR1IFY(arg) arg
#endif

#define TR1INCLUDE(arg) QUOTE(TR1IFY(arg))

#endif
