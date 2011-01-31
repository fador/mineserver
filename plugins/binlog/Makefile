# Makefile for plugin
PLUGIN_NAME=binlog

SRCS = ./src/binlog.cpp

binlogdump: ./src/binlogdump.o ./src/binlog.o
	$(CC) binlogdump.o binlog.o -o binlogdump

binlogdump.o: ./src/binlogdump.cpp
	$(CC) -c ./src/binlogdump.cpp


include ../plugins.mk

