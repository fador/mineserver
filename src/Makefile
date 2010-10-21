##############################################################################

DEP=$(CXX) -MM
CXXFLAGS=-I ./Sockets-2.3.9.3/ -I ./zlib/ -O2 -lpthread
SRC1=$(wildcard *.cpp)
SRC2=$(wildcard Sockets-2.3.9.3/*.cpp)
SRC3=$(wildcard zlib/*.c)
SRC=$(SRC1) $(SRC2) $(SRC3)
INC=./Sockets-2.3.9.5/
OBJS2=$(subst .cpp,.o,$(SRC))
OBJS=$(subst .c,.o,$(OBJS2))
BIN=mineserver
LINK.o=$(LINK.cpp) # GNUmake spesiaali

$(BIN): $(OBJS)

%.dep: %
	$(DEP) $(CXXFLAGS) $(CFLAGS) $< > $@

clean:
	$(RM) $(BIN) $(OBJS) *~ core *.dep *.style
