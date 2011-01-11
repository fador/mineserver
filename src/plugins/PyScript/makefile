# Makefile for PyScript
all: complete


COMP = g++
ifeq ($(CLANG), YES)
  COMP = clang
endif

OBJS = MineServer_wrap.o PyScript.o python_plugin_api.o

complete: PyScript.so _MineServer.so MineServer.py
	cp PyScript.so ../../../bin
	cp _MineServer.so MineServer.py ../../../bin
	cp -r python ../../../bin/
	touch complete

PyScript.so : python/TriggTest.py _MineServer.so $(OBJS)
	$(COMP) `python-config --ldflags` -g -export-dynamic -rdynamic -shared -Wl,-soname,PyScript.so -o PyScript.so PyScript.o python_plugin_api.o -lc

PyScript.o : PyScript.cpp MineServer_wrap.cxx
	$(COMP) -O2 -fPIC -DDEBIAN `python-config --cflags` -o PyScript.o -c PyScript.cpp

python_plugin_api.o: python_plugin_api.cpp python_plugin_api.h ../../plugin_api.cpp ../../plugin_api.h
	$(COMP) `python-config --cflags` -fPIC -O2 -c python_plugin_api.cpp

MineServer_wrap.cxx: MineServer.i ../../plugin_api.cpp ../../plugin_api.h python_plugin_api.cpp python_plugin_api.h
	swig -c++ -python MineServer.i

MineServer_wrap.o: MineServer_wrap.cxx MineServer.py
	$(COMP) -DDEBIAN `python-config --cflags` -O2 -fPIC -c MineServer_wrap.cxx

_MineServer.so: $(OBJS)
	$(COMP) `python-config --ldflags` -shared python_plugin_api.o MineServer_wrap.o -Wl,-soname,_MineServer.so -o _MineServer.so 

clean:
	rm *.o *.so MineServer_wrap* *.py
