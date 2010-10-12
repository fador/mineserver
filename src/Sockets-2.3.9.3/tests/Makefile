SOCKETS =	..
PLATFORM =	linux-x86-32
LIBS =		
#-L$(SOCKETS) -lSockets 

INCLUDE =	-I$(SOCKETS)
include		$(SOCKETS)/Makefile.version
include		$(SOCKETS)/Makefile.Defines.$(PLATFORM)

CPPFLAGS =	$(CFLAGS)

#LIBS +=		-lsctp

LDFLAGS +=	-rdynamic

PROGS =		events resolve sockets_test base64 semtest \
		echoserver stressclient http httpd retry resume \
		http_post x copy crlf https sloppy_http chunked \
		threadstress listener httpd_test scanr

all:		$(PROGS)

events:		events.o ../libSockets.a 
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

resolve:	resolve.o ../libSockets.a 
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

sockets_test:	sockets_test.o ../libSockets.a 
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

base64:		base64.o ../libSockets.a 
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

semtest:	semtest.o ../libSockets.a 
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

echoserver:	echoserver.o ../libSockets.a 
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

stressclient:	stressclient.o ../libSockets.a 
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

http:		http.o ../libSockets.a 
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

httpd:		httpd.o ../libSockets.a 
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

retry:		retry.o ../libSockets.a 
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

resume:		resume.o ../libSockets.a 
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

http_post:	http_post.o ../libSockets.a 
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

x:		x.o ../libSockets.a 
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

copy:		copy.o ../libSockets.a 
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

crlf:		crlf.o ../libSockets.a 
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

https:		https.o ../libSockets.a
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

sloppy_http:	sloppy_http.o ../libSockets.a
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

chunked:	chunked.o ../libSockets.a
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

threadstress:	threadstress.o ../libSockets.a
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

listener:	listener.o ../libSockets.a
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

scanr:		scanr.o ../libSockets.a
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

httpd_test:	httpd_test.o ../libSockets.a
		$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

clean:
		rm -f *.o *~ slask *.d $(PROGS)

-include	*.d
