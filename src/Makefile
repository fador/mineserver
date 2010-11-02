##############################################################################

DEP=$(CXX) -MM
CXXFLAGS= -I ./zlib/ -O2 -lpthread -levent
SRC1=$(wildcard *.cpp)
SRC3=$(wildcard zlib/*.c)
SRC=$(SRC1) $(SRC2) $(SRC3)
INC=./
OBJS2=$(subst .cpp,.o,$(SRC))
OBJS=$(subst .c,.o,$(OBJS2))
BIN=mineserver
LINK.o=$(LINK.cpp) # GNUmake spesiaali

$(BIN): $(OBJS)

%.dep: %
	$(DEP) $(CXXFLAGS) $(CFLAGS) $< > $@

clean:
	$(RM) $(BIN) $(OBJS) *~ core *.dep *.style
