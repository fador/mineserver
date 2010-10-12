/** \file SocketHandlerEp.h
 **	\date  2010-02-13
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2010  Anders Hedstrom

This library is made available under the terms of the GNU GPL, with
the additional exemption that compiling, linking, and/or using OpenSSL 
is allowed.

If you would like to use this library in a closed-source application,
a separate license agreement is available. For information about 
the closed-source license agreement for the C++ sockets library,
please visit http://www.alhem.net/Sockets/license.html and/or
email license@alhem.net.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef _SOCKETHANDLEREP_H
#define _SOCKETHANDLEREP_H

#include "SocketHandler.h"
#ifdef LINUX
#include <sys/epoll.h>

#define MAX_EVENTS_EP_WAIT 100
#endif // LINUX

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

class SocketHandlerEp : public SocketHandler
{
public:
	/** SocketHandler constructor.
		\param log Optional log class pointer */
	SocketHandlerEp(StdLog *log = NULL);

	/** SocketHandler threadsafe constructor.
		\param mutex Externally declared mutex variable
		\param log Optional log class pointer */
	SocketHandlerEp(IMutex& mutex,StdLog *log = NULL);

	SocketHandlerEp(IMutex&, ISocketHandler& parent, StdLog * = NULL);

	~SocketHandlerEp();

	ISocketHandler *Create(StdLog * = NULL);
	ISocketHandler *Create(IMutex&, ISocketHandler&, StdLog * = NULL);

#ifdef LINUX

	/** Set read/write/exception file descriptor sets (fd_set). */
	void ISocketHandler_Add(Socket *,bool bRead,bool bWrite);
	void ISocketHandler_Mod(Socket *,bool bRead,bool bWrite);
	void ISocketHandler_Del(Socket *);

	size_t MaxCount() {
		return 10000; // %!
	}

protected:
	/** Actual call to select() */
	int ISocketHandler_Select(struct timeval *);
#endif // LINUX

private:
	int m_epoll; ///< epoll file descriptor
#ifdef LINUX
	struct epoll_event m_events[MAX_EVENTS_EP_WAIT];

#endif // LINUX

};


#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // _SOCKETHANDLEREP_H
