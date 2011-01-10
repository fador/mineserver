# Standard things
.SUFFIXES:
.SUFFIXES:  .c .o .so

# Defaults
PLUGIN       = YES

# Sources
SRC          = mineserver.cpp map.cpp chat.cpp constants.cpp logger.cpp nbt.cpp
SRC         += furnace.cpp furnaceManager.cpp packets.cpp physics.cpp sockets.cpp
SRC         += tools.cpp user.cpp screen.cpp tree.cpp inventory.cpp

SRC         += config.cpp config/node.cpp config/scanner.cpp config/lexer.cpp config/parser.cpp

SRC         += blocks/basic.cpp blocks/door.cpp blocks/falling.cpp blocks/fire.cpp
SRC         += blocks/liquid.cpp blocks/plant.cpp blocks/sign.cpp blocks/snow.cpp 
SRC         += blocks/stair.cpp blocks/torch.cpp blocks/tracks.cpp blocks/default.cpp
SRC         += blocks/chest.cpp

SRC         += worldgen/mapgen.cpp worldgen/cavegen.cpp worldgen/mersenne.cpp

SRC         += plugin.cpp plugin_api.cpp


# Variables
ARCHFLAGS    = 
LIBRARIES    = 
INC          = -I. -I/usr/include -I/usr/local/include
CXXFLAGS     = -DFADOR_PLUGIN
LDFLAGS      = -L/usr/local/lib -L/usr/lib -levent -lz -lnoise
COMMIT       = `git log | head -n1 | cut -b8-14`
DF           = $(*F)
DEPDIR       = .deps
BUILDDIR     = .build
OBJS         = $(patsubst %.cpp,%.o,$(SRC))

ifeq ($(PLUGIN), YES)
  OBJS      += $(patsubst %.cpp,%.o,$(PLUGINS))
endif

ifeq ($(PDCURSES), YES)
  LDFLAGS   += -lpdcurses
else
  LDFLAGS   += -lncurses
endif

ifeq ($(DEBUG), YES)
  CXXFLAGS  += -DDEBUG
endif

ifeq ($(PEDANTIC), YES)
  CXXFLAGS  += -Wall -Werror -pedantic
endif

ifeq ($(findstring MINGW32,$(SYSTEM)),MINGW32)
LDFLAGS += -lws2_32 -lwinmm
else
LDFLAGS += -ldl
endif

ifeq ($(findstring Debian,$(shell cat /etc/issue)),Debian)
  DEBIAN     = 1
endif

ifeq ($(DEBIAN), 1)
  CXXFLAGS  += -DDEBIAN
endif

# Tools
CXX          = g++
OBJCOPY      = objcopy
COMPILE      = $(CXX) $(INC) $(CXXFLAGS) $(ARCHFLAGS) -c $< -o $@
MAKEDEPEND   = $(CXX) -M $(INC) $(CXXFLAGS) $(ARCHFLAGS) -o "$(DEPDIR)/$*.d" $<

# Rules
.PHONY: clean all

<<<<<<< HEAD
CXXFLAGS += $(shell python-config --cflags)
LDFLAGS += $(shell python-config --ldflags)

OBJS  = mineserver.o map.o chat.o constants.o logger.o nbt.o furnace.o furnaceManager.o packets.o physics.o sockets.o tools.o user.o screen.o plugin.o tree.o inventory.o
OBJS += config.o config/node.o config/scanner.o config/lexer.o config/parser.o
OBJS += blocks/basic.o blocks/door.o blocks/falling.o blocks/fire.o blocks/liquid.o blocks/plant.o blocks/sign.o blocks/snow.o blocks/stair.o blocks/torch.o blocks/tracks.o blocks/default.o blocks/chest.o
OBJS += worldgen/mapgen.o worldgen/cavegen.o worldgen/mersenne.o MineServer_wrap.o
PROG  = ./mineserver
PROGS = $(PROG)
=======
all: mineserver
>>>>>>> upstream/master

%.o: %.cpp
	@echo "Compiling $<"
	@mkdir -p $(DEPDIR)/$(dir $@)
	@$(MAKEDEPEND)
	@sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $(DEPDIR)/$(dir $@)/$(*F).d >> $(DEPDIR)/$(dir $@)/$(*F).d
	@$(COMPILE)

<<<<<<< HEAD
$(PROG): _MineServer.so
	$(CXX) $(OBJS) -o ../bin/mineserver $(LDFLAGS)
=======
-include $(addprefix $(DEPDIR)/,$(OBJS:.o=.d))
>>>>>>> upstream/master

mineserver: $(OBJS)
	@echo "Building $@"
	@$(CXX) $(ARCHFLAGS) $(LDFLAGS) $(OBJS) $(LIBRARIES) -o ../bin/$@

dist:
	tar cvzf ../mineserver-latest-`uname`-bin.tar.gz -C ../bin/ . ../LICENSE
	zip -j ../mineserver-latest-`uname`-bin.zip ../bin/* ../LICENSE

<<<<<<< HEAD
MineServer_wrap.cxx: MineServer.i
	swig -c++ -python MineServer.i

MineServer_wrap.o: MineServer_wrap.cxx MineServer.py
	$(CXX) $(CXXFLAGS) -fPIC -c MineServer_wrap.cxx

_MineServer.so: MineServer_wrap.o $(OBJS)
	$(CXX) $(LDFLAGS) -shared -Wl,-soname,_MineServer.so -o _MineServer.so $(OBJS)
	cp _MineServer.so ../bin
	cp MineServer.py ../bin


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
mineserver.o: mineserver.cpp mineserver.h constants.h logger.h sockets.h tools.h map.h user.h chat.h worldgen/mapgen.h config.h nbt.h packets.h physics.h
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
=======
clean:
	@rm -f *.o
	@rm -f */*.o
	@rm -f */*/*.o
	@rm -rf $(DEPDIR)
>>>>>>> upstream/master
