SHELL = /bin/sh
SYSTEM = $(shell uname)
PDCURSES = ${pdcurses}

DFLAGS = 

ifeq ($(SYSTEM),Darwin)
DFLAGS += -D__APPLE__
endif

ifeq ($(SYSTEM),FreeBSD)
DFLAGS += -D__FREEBSD__
endif

ifeq ($(SYSTEM),SunOS)
DFLAGS += -D__SOLARIS__
endif

ifeq ($(findstring Ubuntu,$(shell cat /etc/issue)),Ubuntu)
DEBIAN = 1
endif

ifeq ($(findstring Debian,$(shell cat /etc/issue)),Debian)
DEBIAN = 1
endif

ifeq ($(DEBIAN), 1)
CXXFLAGS += -DDEBIAN
endif

ifeq ($(findstring MINGW32,$(SYSTEM)),MINGW32)
LFLAGS = -lws2_32 -lwinmm -export-all-symbols
else
LFLAGS = -rdynamic
endif

ifeq ($(PDCURSES),true)
LFLAGS += -lpdcurses
else
LFLAGS += -lncurses
endif

ifeq ($(PEDANTIC), 1)
LDFLAGS += -Wall -pedantic
CXXFLAGS += -Wall -pedantic
endif

RM = rm -f
CXX = g++
CC = $(CXX)
LDFLAGS += -L/usr/local/lib -L/usr/lib -levent -lz -lnoise $(LFLAGS)
CXXFLAGS += $(DFLAGS) -I. -I/usr/Local/include -I/usr/include

OBJS  = mineserver.o map.o chat.o constants.o logger.o nbt.o furnace.o furnaceManager.o packets.o physics.o sockets.o tools.o user.o screen.o plugin.o tree.o inventory.o plugin_api.o
OBJS += config.o config/node.o config/scanner.o config/lexer.o config/parser.o
OBJS += blocks/basic.o blocks/door.o blocks/falling.o blocks/fire.o blocks/liquid.o blocks/plant.o blocks/sign.o blocks/snow.o blocks/stair.o blocks/torch.o blocks/tracks.o blocks/default.o blocks/chest.o
OBJS += worldgen/mapgen.o worldgen/cavegen.o worldgen/mersenne.o
PROG  = ./mineserver
PROGS = $(PROG)

$(PROGS): $(OBJS)

$(PROG):
	$(CXX) $(OBJS) -o ../bin/mineserver $(LDFLAGS)

clean: 
	$(RM) $(OBJS) $(PROGS)

dist:
	tar cvzf ../mineserver-latest-`uname`-bin.tar.gz -C ../bin/ . ../LICENSE
	zip -j ../mineserver-latest-`uname`-bin.zip ../bin/* ../LICENSE

all: $(PROGS)

chat.o: chat.cpp logger.h constants.h tools.h map.h user.h chat.h config.h physics.h plugin.h
config/node.o: config/node.cpp config/node.h
config/scanner.o: config/scanner.cpp config/scanner.h
config/lexer.o: config/lexer.cpp config/lexer.h
config/parser.o: config/parser.cpp config/parser.h
config.o: config.cpp config.h config/node.h config/scanner.h config/lexer.h config/parser.h
constants.o: constants.cpp constants.h
logger.o: logger.cpp logger.h mineserver.h
map.o: map.cpp logger.h tools.h map.h user.h nbt.h config.h worldgen/mapgen.h
inventory.o: inventory.cpp inventory.h tools.h map.h user.h mineserver.h
nbt.o: nbt.cpp tools.h nbt.h map.h
furnace.o: furnace.cpp furnace.h mineserver.h
furnaceManager.o: furnaceManager.cpp furnaceManager.h
packets.o: packets.cpp constants.h mineserver.h logger.h sockets.h tools.h map.h user.h chat.h config.h nbt.h packets.h physics.h
physics.o: physics.cpp logger.h constants.h config.h user.h map.h vec.h physics.h
screen.o: screen.cpp screen.h
sockets.o: sockets.cpp logger.h constants.h tools.h mineserver.h user.h map.h chat.h nbt.h packets.h
tools.o: tools.cpp tools.h
tree.o: tree.cpp tree.h
user.o: user.cpp constants.h logger.h tools.h mineserver.h map.h user.h nbt.h chat.h packets.h
mineserver.o: mineserver.cpp mineserver.h constants.h logger.h sockets.h tools.h map.h user.h chat.h worldgen/mapgen.h config.h nbt.h packets.h physics.h plugin_api.h
plugin.o: plugin.cpp plugin.h hook.h mineserver.h blocks/basic.h blocks/door.h blocks/falling.h blocks/fire.h blocks/liquid.h blocks/plant.h blocks/sign.h blocks/snow.h blocks/stair.h blocks/torch.h blocks/tracks.h blocks/chest.h
blocks/basic.o: blocks/basic.cpp blocks/basic.h
blocks/default.o: blocks/default.cpp blocks/default.h
blocks/door.o: blocks/door.cpp blocks/basic.h blocks/door.h
blocks/falling.o: blocks/falling.cpp blocks/basic.h blocks/falling.h
blocks/fire.o: blocks/fire.cpp blocks/basic.h blocks/fire.h
blocks/liquid.o: blocks/liquid.cpp physics.h blocks/basic.h blocks/liquid.h
blocks/plant.o: blocks/plant.cpp blocks/basic.h blocks/plant.h
blocks/sign.o: blocks/sign.cpp blocks/basic.h blocks/sign.h
blocks/snow.o: blocks/snow.cpp blocks/basic.h blocks/snow.h
blocks/stair.o: blocks/stair.cpp blocks/basic.h blocks/stair.h
blocks/torch.o: blocks/torch.cpp blocks/basic.h blocks/torch.h
blocks/tracks.o: blocks/tracks.cpp blocks/tracks.h
blocks/chest.o: blocks/chest.cpp blocks/chest.h
worldgen/cavegen.o: worldgen/cavegen.cpp worldgen/cavegen.h worldgen/mersenne.h map.h constants.h logger.h tools.h config.h
worldgen/mapgen.o: worldgen/mapgen.cpp worldgen/mapgen.h logger.h constants.h config.h worldgen/mersenne.h worldgen/cavegen.h
worldgen/mersenne.o: worldgen/mersenne.cpp worldgen/mersenne.h
