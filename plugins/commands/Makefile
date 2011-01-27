# Makefile for plugin
PLUGIN_NAME=command

SRCS = command.cpp

all: complete
SHELL = /bin/sh


COMP = g++
ifeq ($(CLANG), YES)
  COMP = clang
endif

OBJS = $(SRCS:.cpp=.o)

complete: $(OBJS) $(PLUGIN_NAME).so

install:
	cp $(PLUGIN_NAME).so ../../bin/$(PLUGIN_NAME).so

$(OBJS):
	$(COMP) -fPIC -c ./src/$(@:.o=.cpp) -o ./$@

$(PLUGIN_NAME).so:
	$(COMP) -shared $(OBJS) -o $(PLUGIN_NAME).so


clean:
	rm *.o
	rm *.so
