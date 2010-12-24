# Makefile for PyScript


PyScript.so : PyScript.o
	g++ `python-config --ldflags` -g -export-dynamic -rdynamic -shared -Wl,-soname,PyScript.so -o PyScript.so PyScript.o -lc
	cp PyScript.so ../../../bin/

PyScript.o : main.cpp
	g++ -fPIC -DDEBIAN `python-config --cflags` -o PyScript.o -c main.cpp

clean:
	rm *.o PyScript.so
