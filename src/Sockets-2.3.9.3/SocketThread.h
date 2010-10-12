#ifndef _SOCKETTHREAD_H
#define _SOCKETTHREAD_H

#include "sockets-config.h"
#ifdef ENABLE_DETACH

#include "Thread.h"
#include "SocketHandler.h"

class Socket;

/** Detached socket run thread. 
	\ingroup internal */
class SocketThread : public Thread
{
public:
	SocketThread(Socket *p);
	~SocketThread();

	void Run();

private:
	SocketThread(const SocketThread& s) : m_socket(s.m_socket) {}
	SocketThread& operator=(const SocketThread& ) { return *this; }

	SocketHandler m_h;
	Socket *m_socket;
};

#endif // ENABLE_DETACH

#endif // _SOCKETTHREAD_H
