# Standard things
.SUFFIXES:
.SUFFIXES:  .c .o .so

# Defaults
PLUGIN       = YES

# Sources
SRC          = mineserver.cpp map.cpp chat.cpp constants.cpp logger.cpp nbt.cpp
SRC         += furnace.cpp furnaceManager.cpp packets.cpp physics.cpp sockets.cpp
SRC         += tools.cpp user.cpp tree.cpp inventory.cpp mob.cpp

SRC         += screenBase.cpp cliScreen.cpp

SRC         += config.cpp config/node.cpp config/scanner.cpp config/lexer.cpp config/parser.cpp

SRC         += blocks/basic.cpp blocks/door.cpp blocks/falling.cpp blocks/fire.cpp
SRC         += blocks/liquid.cpp blocks/plant.cpp blocks/sign.cpp blocks/snow.cpp 
SRC         += blocks/stair.cpp blocks/torch.cpp blocks/tracks.cpp blocks/default.cpp
SRC         += blocks/chest.cpp blocks/ladder.cpp blocks/cake.cpp blocks/note.cpp
SRC         += blocks/workbench.cpp blocks/blockfurnace.cpp


SRC         += worldgen/mapgen.cpp worldgen/cavegen.cpp worldgen/nethergen.cpp

SRC         += plugin.cpp plugin_api.cpp worldgen/heavengen.cpp


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

ifeq ($(DEBUG), YES)
  CXXFLAGS  += -DDEBUG -g -O0
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

ifeq ($(findstring Ubuntu,$(shell cat /etc/issue)),Ubuntu)
  DEBIAN     = 1
endif

ifeq ($(findstring Mint,$(shell cat /etc/issue)),Mint)
  DEBIAN = 1
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
