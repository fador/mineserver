OS           = $(shell uname -s)
CXX          = g++
LIBRARIES    = 
INC          = -I. -I/usr/include -I/usr/local/include
CXXFLAGS     = -O
LDFLAGS      = -L/usr/local/lib -L/usr/lib
DEPDIR       = .deps
BUILDDIR     = .build
ARCHFLAGS    =
BUILDFLAGS   =  -DFADOR_PLUGIN

ifeq ($(DEBUG), YES)
  BUILDFLAGS  += -DDEBUG -g -O0
endif

ifeq ($(PEDANTIC), YES)
  BUILDFLAGS  += -Wall -Werror -pedantic
endif

ifeq ($(findstring MINGW32,$(SYSTEM)),MINGW32)
  LDFLAGS += -lws2_32 -lwinmm
endif

ifeq ($(OS),Linux)
  LDFLAGS += -ldl
  LIBNOISE=1
endif

ifeq ($(OS),FreeBSD)
  LIBNOISE=0
  LIBRARIES+=/usr/local/lib/libnoise.so.0
endif

# define for <libnoise/noise.h> vs <noise/noise.h>
# LIBNOISE=1

ifeq ($(LIBNOISE), 1)
  BUILDFLAGS  += -DLIBNOISE
endif
