DEP=$(CXX) -MM
CXXFLAGS= -O2 -lpthread -levent -lz -Wall -Werror
INC=./

SRC_MS=$(wildcard *.cpp)
SRC=$(SRC_MS)

OBJS_MS=$(subst .cpp,.o,$(SRC_MS))
OBJS=$(OBJS_MS)

BIN=mineserver

LINK.o=$(LINK.cpp)

$(BIN): $(OBJS)

%.deps: %
	$(DEP) $(CXXFLAGS) $(CFLAGS) $< > $@

clean:
	$(RM) $(BIN) $(OBJS) *~ core *.dep *.style
