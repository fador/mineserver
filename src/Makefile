SHELL = /bin/sh
SYSTEM = $(shell uname)

DFLAGS = 

ifeq ($(SYSTEM),FreeBSD)
DFLAGS += -D__FREEBSD__
endif

RM = rm -f
C++ = g++
LFLAGS = -I. -I/usr/local/include -L/usr/local/lib -lpthread -levent -lz
CFLAGS = $(DFLAGS) -I. -I/usr/local/include -L/usr/local/lib

OBJS = map.o chat.o commands.o config.o constants.o logger.o mapgen.o nbt.o packets.o physics.o sockets.o tools.o user.o mineserver.o noise.o
PROG = ./mineserver
PROGS = $(PROG)

all: $(OBJS) $(PROGS)

$(PROG): $(OBJS)
	$(C++) -o $(PROG) $(OBJS) $(LFLAGS)

clean: 
	$(RM) $(OBJS) $(PROGS)

$(OBJS): %.o: %.cpp
	$(C++) -o $@ $(CFLAGS) -c $<

$(PROG): $(OBJS)

all: $(PROGS)

chat.o: logger.h constants.h tools.h map.h user.h chat.h config.h physics.h
commands.o: logger.h constants.h tools.h map.h user.h chat.h config.h physics.h
config.o: logger.h constants.h config.h
constants.o: constants.h
logger.o: logger.h
map.o: logger.h tools.h map.h user.h nbt.h config.h
mapgen.o: logger.h constants.h config.h mapgen.h
nbt.o: tools.h nbt.h map.h
packets.o: constants.h logger.h sockets.h tools.h map.h user.h chat.h config.h nbt.h packets.h physics.h
physics.o: logger.h constants.h config.h user.h map.h vec.h physics.h
sockets.o: logger.h constants.h tools.h user.h map.h chat.h nbt.h packets.h
tools.o: tools.h
user.o: constants.h logger.h tools.h map.h user.h nbt.h chat.h packets.h
mineserver.o: constants.h logger.h sockets.h tools.h map.h user.h chat.h mapgen.h config.h nbt.h packets.h physics.h
noise.o: noise.h constants.h
