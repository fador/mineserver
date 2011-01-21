all:
	g++ -c -fPIC binlog.cpp
	g++ -shared binlog.o -o binlog.so
	g++ -o binlogdump binlog_dump.cpp binlog.o
clean:
	rm binlog.o binlog.so
