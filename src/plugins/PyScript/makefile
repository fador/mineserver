# Makefile for PyScript
all: complete
SHELL = /bin/sh

CFLAGS = $(shell python-config --cflags) -fPIC -O0 -g
LDFLAGS = $(shell python-config --ldflags) -shared -Xlinker -export-dynamic

ifeq ($(DEBIAN), 1)
  CFLAGS  += -DDEBIAN
endif

COMP = g++
ifeq ($(CLANG), YES)
  COMP = clang
endif

OBJS = PyScript.o python_plugin_api.o

complete: PyScript.so _MineServer.so MineServer.py
	cp PyScript.so ../../../bin
	cp _MineServer.so MineServer.py ../../../bin
	cp -r python ../../../bin/
	touch complete

PyScript.so : python/TriggTest.py _MineServer.so $(OBJS)
	$(COMP) $(LDFLAGS) -g -rdynamic -Wl,-soname,PyScript.so -o PyScript.so -lc $(OBJS)

PyScript.o : PyScript.cpp MineServer_wrap.cxx PyScript.h
	$(COMP) $(CFLAGS) -c PyScript.cpp

python_plugin_api.o: python_plugin_api.cpp python_plugin_api.h ../../plugin_api.cpp ../../plugin_api.h
	$(COMP) $(CFLAGS) -c python_plugin_api.cpp

MineServer_wrap.cxx: MineServer.i ../../plugin_api.cpp ../../plugin_api.h python_plugin_api.cpp python_plugin_api.h
	swig -c++ -python MineServer.i

MineServer_wrap.o: MineServer_wrap.cxx MineServer.py
	$(COMP) $(CFLAGS) -c MineServer_wrap.cxx

_MineServer.so: $(OBJS)
	$(COMP) $(LDFLAGS) $(OBJS) -Wl,-soname,_MineServer.so -o _MineServer.so 

clean:
	rm *.o
	rm *.so
	rm *.py*
	rm MineServer_w*
	rm complete
