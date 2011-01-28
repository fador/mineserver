CC           = g++
LIBRARIES    = 
INC          = -I. -I/usr/include -I/usr/local/include
CFLAGS     = -DFADOR_PLUGIN
LDFLAGS      = -L/usr/local/lib -L/usr/lib
DEPDIR       = .deps
BUILDDIR     = .build
ARCHFLAGS    =

ifeq ($(DEBUG), YES)
  CFLAGS  += -DDEBUG -g -O0
endif

ifeq ($(PEDANTIC), YES)
  CFLAGS  += -Wall -Werror -pedantic
endif

ifeq ($(findstring MINGW32,$(SYSTEM)),MINGW32)
LDFLAGS += -lws2_32 -lwinmm
else
LDFLAGS += -ldl
endif


# define for <libnoise/noise.h> vs <noise/noise.h>
LIBNOISE=1

ifeq ($(LIBNOISE), 1)
  CFLAGS  += -DLIBNOISE
endif