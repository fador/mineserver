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
LFLAGS = -lws2_32 -lwinmm -export-all-symbols
else
LFLAGS = -rdynamic -ldl
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

all: mineserver

%.o: %.cpp
	@echo "Compiling $<"
	@mkdir -p $(DEPDIR)/$(dir $@)
	@$(MAKEDEPEND)
	@sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $(DEPDIR)/$(dir $@)/$(*F).d >> $(DEPDIR)/$(dir $@)/$(*F).d
	@$(COMPILE)

-include $(addprefix $(DEPDIR)/,$(OBJS:.o=.d))

mineserver: $(OBJS)
	@echo "Building $@"
	@$(CXX) $(ARCHFLAGS) $(LDFLAGS) $(OBJS) $(LIBRARIES) -o ../bin/$@

dist:
	tar cvzf ../mineserver-latest-`uname`-bin.tar.gz -C ../bin/ . ../LICENSE
	zip -j ../mineserver-latest-`uname`-bin.zip ../bin/* ../LICENSE

clean:
	@rm -f *.o
	@rm -f */*.o
	@rm -f */*/*.o
	@rm -rf $(DEPDIR)
