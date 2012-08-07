TEMPLATE = app
CONFIG += console
CONFIG -= qt

## TO Use this project file in windows:
## set env var 'MINESERVER_WIN32_LIBS' to the directory containing mineservermingw libs

win32{
	LIBS += -L$$(MINESERVER_WIN32_LIBS)/lib

	LIBS += -lnoise -leay32 -lssleay32 -lssl32 -lpdcurses -lz -levent -lWs2_32 -lwinmm

	INCLUDEPATH += $$(MINESERVER_WIN32_LIBS)/include
}

unix{
	LIBS += -lnoise -leay -lssleay -lssl -lcurses -lz -levent -ldl
}

INCLUDEPATH += "../include"

DEFINES = __GXX_EXPERIMENTAL_CXX0X__ MINESERVER_BUILD

QMAKE_CXX += -std=c++0x

SOURCES += \
    user.cpp \
    tree.cpp \
    tools.cpp \
    sockets.cpp \
    signalhandler.cpp \
    screenBase.cpp \
    redstoneSimulation.cpp \
    random.cpp \
    plugin_api.cpp \
    plugin.cpp \
    physics.cpp \
    packets.cpp \
    nbt.cpp \
    mob.cpp \
    mineserver.cpp \
    metadata.cpp \
    mcregion.cpp \
		map.cpp \
    logger.cpp \
    lighting.cpp \
    inventory.cpp \
    furnaceManager.cpp \
    furnace.cpp \
    constants.cpp \
    cliScreen.cpp \
    chat.cpp \
    config/scanner.cpp \
    config/parser.cpp \
    config/node.cpp \
    config/lexer.cpp \
    blocks/workbench.cpp \
    blocks/wool.cpp \
    blocks/wood.cpp \
    blocks/tracks.cpp \
    blocks/torch.cpp \
    blocks/tnt.cpp \
    blocks/step.cpp \
    blocks/stair.cpp \
    blocks/snow.cpp \
    blocks/sign.cpp \
    blocks/redstoneutil.cpp \
    blocks/redstone.cpp \
    blocks/pumpkin.cpp \
    blocks/plant.cpp \
    blocks/note.cpp \
    blocks/liquid.cpp \
    blocks/leaves.cpp \
    blocks/ladder.cpp \
    blocks/jackolantern.cpp \
    blocks/ice.cpp \
    blocks/fire.cpp \
    blocks/falling.cpp \
    blocks/door.cpp \
    blocks/default.cpp \
    blocks/chest.cpp \
    blocks/cake.cpp \
    blocks/blockfurnace.cpp \
    blocks/bed.cpp \
    blocks/basic.cpp \
    items/projectile.cpp \
    items/itembasic.cpp \
    items/food.cpp \
    worldgen/nethergen.cpp \
    worldgen/mapgen.cpp \
    worldgen/heavengen.cpp \
    worldgen/eximgen.cpp \
    worldgen/cavegen.cpp \
    worldgen/biomegen.cpp

HEADERS += \
    config/scanner.h \
    config/parser.h \
    config/node.h \
    config/lexer.h \
    blocks/workbench.h \
    blocks/wool.h \
    blocks/wood.h \
    blocks/tracks.h \
    blocks/torch.h \
    blocks/tnt.h \
    blocks/step.h \
    blocks/stair.h \
    blocks/snow.h \
    blocks/sign.h \
    blocks/redstoneutil.h \
    blocks/redstone.h \
    blocks/pumpkin.h \
    blocks/plant.h \
    blocks/note.h \
    blocks/liquid.h \
    blocks/leaves.h \
    blocks/ladder.h \
    blocks/jackolantern.h \
    blocks/ice.h \
    blocks/fire.h \
    blocks/falling.h \
    blocks/door.h \
    blocks/default.h \
    blocks/chest.h \
    blocks/cake.h \
    blocks/blockfurnace.h \
    blocks/bed.h \
    blocks/basic.h \
    items/projectile.h \
    items/itembasic.h \
    items/food.h \
    worldgen/nethergen.h \
    worldgen/mapgen.h \
    worldgen/heavengen.h \
    worldgen/eximgen.h \
    worldgen/cavegen.h \
    worldgen/biomegen.h \
    ../include/vec.h \
    ../include/utf8.h \
    ../include/user.h \
    ../include/tree.h \
    ../include/tr1.h \
    ../include/tools.h \
    ../include/sockets.h \
    ../include/SystemExceptions.h \
    ../include/signalhandler.h \
    ../include/screenBase.h \
    ../include/redstoneSimulation.h \
    ../include/random.h \
    ../include/protocol.h \
    ../include/plugin_api.h \
    ../include/plugin.h \
    ../include/physics.h \
    ../include/permissions.h \
    ../include/packets.h \
    ../include/nbt.h \
    ../include/mob.h \
    ../include/mineserver.h \
    ../include/metadata.h \
    ../include/mcregion.h \
    ../include/map.h \
    ../include/logtype.h \
    ../include/logger.h \
    ../include/lighting.h \
    ../include/inventory.h \
    ../include/hook.h \
    ../include/furnaceManager.h \
    ../include/furnace.h \
    ../include/extern.h \
    ../include/constants_num.h \
    ../include/constants.h \
    ../include/config.h \
    ../include/cliScreen.h \
    ../include/chunkmap.h \
    ../include/chat.h \
    configure.h

