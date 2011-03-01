# Makefile for plugin
PLUGIN_NAME=binlog

SRCS = binlog.cpp

binlogdump: binlogdump.o binlog.o
	$(CC) binlogdump.o binlog.o -o binlogdump

binlogdump.o: binlogdump.cpp
	$(CC) -c binlogdump.cpp


include ../plugins.mk

