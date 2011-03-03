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
SRC         += blocks/workbench.cpp blocks/blockfurnace.cpp blocks/dyed.cpp blocks/redstone.cpp

SRC         += worldgen/mapgen.cpp worldgen/cavegen.cpp worldgen/nethergen.cpp
SRC         += worldgen/heavengen.cpp worldgen/biomegen.cpp

SRC         += items/itembasic.cpp items/food.cpp items/projectile.cpp

SRC         += plugin.cpp plugin_api.cpp


OBJS         = $(patsubst %.cpp,%.o,$(SRC))

include ../config.mk

LDFLAGS     += -levent -lz -lnoise

COMPILE      = $(CXX) $(INC) $(CXXFLAGS) $(BUILDFLAGS) $(ARCHFLAGS) -c $< -o $@
MAKEDEPEND   = $(CXX) -M $(INC) $(CXXFLAGS) $(BUILDFLAGS) $(ARCHFLAGS) -o "$(DEPDIR)/$*.d" $<

all: mineserver

%.o: %.cpp
	mkdir -p $(DEPDIR)/$(dir $@)
	$(MAKEDEPEND)
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $(DEPDIR)/$(dir $@)/$(*F).d >> $(DEPDIR)/$(dir $@)/$(*F).d
	$(COMPILE)

-include $(addprefix $(DEPDIR)/,$(OBJS:.o=.d))

mineserver: $(OBJS)
	$(CXX) $(CXXFLAGS) $(BUILDFLAGS) $(ARCHFLAGS) $(LDFLAGS) $(OBJS) $(LIBRARIES) -o $@

install: mineserver 
	mkdir -p ../bin/
	cp mineserver ../bin

clean:
	find $(CURDIR) -name "*.o" -exec rm '{}' \;
	-rm -r $(DEPDIR)
