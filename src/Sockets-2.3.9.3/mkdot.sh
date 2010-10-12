#!/bin/sh
PROJECT_DIR=/usr/local/apache/www.alhem.net/htdocs/Sockets
PROJECT_NAME=sockets
/usr/local/bin/dp \
	-D_RUN_DP \
	-DUSE_SCTP -D_THREADSAFE_SOCKETS \
	-DHAVE_OPENSSL -DENABLE_POOL -DENABLE_RECONNECT -DENABLE_SOCKS4 \
	-DENABLE_IPV6 -DENABLE_DETACH -DENABLE_RESOLVER -DENABLE_TRIGGERS \
	-DENABLE_XML \
	-fontsize 12 \
	-lr -dot -cluster | grep -v std > x.dot 
dot -Tpng -o $PROJECT_DIR/sockets.png x.dot
dot -Tcmapx -o $PROJECT_DIR/sockets.cmap x.dot
/usr/local/bin/mkjs $PROJECT_DIR/$PROJECT_NAME.cmap > $PROJECT_DIR/$PROJECT_NAME.js
