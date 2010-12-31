# Standard things
.SUFFIXES:
.SUFFIXES:	.c .o .so

# Defaults
PLUGIN       = NO

# Variables
ARCHFLAGS    = 
LIBRARIES    = 
INC          = -I. -I/usr/include -I/usr/local/include
CFLAGS       = 
LDFLAGS      = -L/usr/local/lib -L/usr/lib -levent -lz -lnoise
COMMIT       = $(shell git log | head -n1 | cut -b8-14)
BUILD        = "$(COMMIT)"
DF           = $(*F)
DEPDIR       = .deps
BUILDDIR     = .build
OBJS         = $(patsubst %.cpp,%.o,$(SRC))

ifeq ($(PLUGIN), YES)
	OBJS    += $(patsubst %.cpp,%.o,$(PLUGINS))
endif

ifeq ($(PDCURSES), YES)
	LDFLAGS += -lpdcurses
else
	LDFLAGS += -lncurses
endif

ifeq ($(DEBUG), YES)
	CFLAGS  += -DDEBUG
endif

ifeq ($(PEDANTIC), YES)
	CFLAGS  += -Wall -Werror -pedantic
endif

ifeq ($(findstring Ubuntu,$(shell cat /etc/issue)),Ubuntu)
	DEBIAN   = 1
endif

ifeq ($(findstring Debian,$(shell cat /etc/issue)),Debian)
	DEBIAN   = 1
endif

ifeq ($(DEBIAN), 1)
	CFLAGS  += -DDEBIAN
endif

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

ifeq ($(PLUGIN),YES)
	SRC     +=  plugin.cpp
endif

# Tools
CC           = clang
OBJCOPY      = objcopy
COMPILE      = $(CC) $(INC) $(CFLAGS) $(ARCHFLAGS) -DSHA1=$(BUILD) -c $< -o $@
MAKEDEPEND   = $(CC) -M $(INC) $(CFLAGS) $(ARCHFLAGS) -o "$(DEPDIR)/$*.d" $<

# Rules
.PHONY: clean all

all:	mineserver

%.o: %.cpp
	@echo "Compiling $<"
	@mkdir -p $(DEPDIR)/$(dir $@)
	@$(MAKEDEPEND)
	@sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $(DEPDIR)/$(dir $@)/$(*F).d >> $(DEPDIR)/$(dir $@)/$(*F).d
	@$(COMPILE)

-include $(addprefix $(DEPDIR)/,$(OBJS:.o=.d))

mineserver: $(OBJS)
	@echo "Building $@"
	@$(CC) $(ARCHFLAGS) $(LDFLAGS) $(OBJS) $(LIBRARIES) -o ../bin/$@

#.PHONY: clean
clean:
	@rm -rf *.o $(DEPDIR) ../bin/mineserver
