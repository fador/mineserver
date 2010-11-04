DEP=$(CXX) -MM
CXXFLAGS= -O2 -lpthread -levent -lz
INC=./

SRC_MS=$(wildcard *.cpp)
SRC_ZLIB=$(wildcard zlib/*.c)
SRC=$(SRC_MS) $(SRC_ZLIB)

OBJS_MS=$(subst .cpp,.o,$(SRC_MS))
OBJS_ZLIB=$(subst .c,.o,$(SRC_ZLIB))
OBJS=$(OBJS_MS) $(OBJS_ZLIB)

BIN=mineserver

LINK.o=$(LINK.cpp)

$(BIN): $(OBJS)

%.deps: %
	$(DEP) $(CXXFLAGS) $(CFLAGS) $< > $@

clean:
	$(RM) $(BIN) $(OBJS) *~ core *.dep *.style
