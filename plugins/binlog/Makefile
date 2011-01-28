# Makefile for plugin
PLUGIN_NAME=binlog

SRCS = ./src/binlog.cpp

binlogdump: ./src/binlogdump.o ./src/binlog.o
	$(CPP) binlogdump.o binlog.o -o binlogdump

binlogdump.o: ./src/binlogdump.cpp
	$(CPP) -c ./src/binlogdump.cpp


include ../Makefile.inc
