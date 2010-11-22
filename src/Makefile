SHELL = /bin/sh
SYSTEM = $(shell uname)

DFLAGS = -O2

ifeq ($(SYSTEM),Darwin)
DFLAGS += -D__APPLE__
endif

ifeq ($(SYSTEM),FreeBSD)
DFLAGS += -D__FREEBSD__
endif

ifeq ($(SYSTEM),SunOS)
DFLAGS += -D__SOLARIS__
endif

RM = rm -f
CXX = g++
CC = $(CXX)
LDFLAGS = -L/usr/local/lib -lpthread -levent -lz
CXXFLAGS = $(DFLAGS) -I. -I/usr/local/include -L/usr/local/lib

OBJS = map.o chat.o commands.o config.o constants.o logger.o mapgen.o nbt.o packets.o physics.o sockets.o tools.o user.o noise.o mineserver.o
PROG = ./mineserver
PROGS = $(PROG)

$(PROGS): $(OBJS)

clean: 
	$(RM) $(OBJS) $(PROGS)

all: $(PROGS)

chat.o: chat.cpp logger.h constants.h tools.h map.h user.h chat.h config.h physics.h
commands.o: commands.cpp logger.h constants.h tools.h map.h user.h chat.h config.h physics.h
config.o: config.cpp logger.h constants.h config.h
constants.o: constants.cpp constants.h
logger.o: logger.cpp logger.h
map.o: map.cpp logger.h tools.h map.h user.h nbt.h config.h
mapgen.o: mapgen.cpp logger.h constants.h config.h mapgen.h
nbt.o: nbt.cpp tools.h nbt.h map.h
packets.o: packets.cpp constants.h logger.h sockets.h tools.h map.h user.h chat.h config.h nbt.h packets.h physics.h
physics.o: physics.cpp logger.h constants.h config.h user.h map.h vec.h physics.h
sockets.o: sockets.cpp logger.h constants.h tools.h user.h map.h chat.h nbt.h packets.h
tools.o: tools.cpp tools.h
user.o: user.cpp constants.h logger.h tools.h map.h user.h nbt.h chat.h packets.h
mineserver.o: mineserver.cpp constants.h logger.h sockets.h tools.h map.h user.h chat.h mapgen.h config.h nbt.h packets.h physics.h
noise.o: noise.cpp noise.h
