/** \file ISocketHandler.h
 **	\date  2004-02-13
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2004-2010  Anders Hedstrom

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
#ifndef _SOCKETS_ISocketHandler_H
#define _SOCKETS_ISocketHandler_H
#include "sockets-config.h"

#include <list>
#include <map>

#include "socket_include.h"
#include "Socket.h"
#include "StdLog.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

class SocketAddress;
class IMutex;


/** Socket container class, event generator. 
	\ingroup basic */
class ISocketHandler
{
	friend class Socket;

public:
	/** Connection pool class for internal use by the ISocketHandler. 
		\ingroup internal */
#ifdef ENABLE_POOL
	class PoolSocket : public Socket
	{
	public:
		PoolSocket(ISocketHandler& h,Socket *src) : Socket(h) {
			CopyConnection( src );
			SetIsClient();
		}

		void OnRead() {
			Handler().LogError(this, "OnRead", 0, "data on hibernating socket", LOG_LEVEL_FATAL);
			SetCloseAndDelete();
		}
		void OnOptions(int,int,int,SOCKET) {}

	};
#endif

public:
	virtual ~ISocketHandler() {}

	/** Return another instance */
	virtual ISocketHandler *Create(StdLog * = NULL) = 0;

	/** Return another instance */
	virtual ISocketHandler *Create(IMutex&, ISocketHandler&, StdLog * = NULL) = 0;

	/** Handler created with parent */
	virtual bool ParentHandlerIsValid() = 0;

	/** Get parent sockethandler */
	virtual ISocketHandler& ParentHandler() = 0;

	/** Get thread handler with least connections */
	virtual ISocketHandler& GetRandomHandler() = 0;

	/** Return parent handler if valid, otherwise return normal handler */
	virtual ISocketHandler& GetEffectiveHandler() = 0;

	/** Enable threading */
	virtual void SetNumberOfThreads(size_t n) = 0;

	/** Threading is enabled */
	virtual bool IsThreaded() = 0;

	/** Enable select release */
	virtual void EnableRelease() = 0;

	/** Make select release */
	virtual void Release() = 0;

	/** Get mutex reference for threadsafe operations. */
	virtual IMutex& GetMutex() const = 0;

	/** Register StdLog object for error callback. 
		\param log Pointer to log class */
	virtual void RegStdLog(StdLog *log) = 0;

	/** Log error to log class for print out / storage. */
	virtual void LogError(Socket *p,const std::string& user_text,int err,const std::string& sys_err,loglevel_t t = LOG_LEVEL_WARNING) = 0;

	// -------------------------------------------------------------------------
	// Socket stuff
	// -------------------------------------------------------------------------
	/** Add socket instance to socket map. Removal is always automatic. */
	virtual void Add(Socket *) = 0;

protected:
	/** Remove socket from socket map, used by Socket class. */
	virtual void Remove(Socket *) = 0;

	/** Actual call to select() */
	virtual int ISocketHandler_Select(struct timeval *) = 0;

public:
	/** Set read/write/exception file descriptor sets (fd_set). */
	virtual void ISocketHandler_Add(Socket *,bool bRead,bool bWrite) = 0;
	virtual void ISocketHandler_Mod(Socket *,bool bRead,bool bWrite) = 0;
	virtual void ISocketHandler_Del(Socket *) = 0;

	/** Wait for events, generate callbacks. */
	virtual int Select(long sec,long usec) = 0;
	/** This method will not return until an event has been detected. */
	virtual int Select() = 0;
	/** Wait for events, generate callbacks. */
	virtual int Select(struct timeval *tsel) = 0;

	/** Check that a socket really is handled by this socket handler. */
	virtual bool Valid(Socket *) = 0;
	/** Preferred method - Check that a socket still is handled by this socket handler. */
	virtual bool Valid(socketuid_t) = 0;

	/** Return number of sockets handled by this handler.  */
	virtual size_t GetCount() = 0;

	/** Return maximum number of sockets allowed. */
	virtual size_t MaxCount() = 0;

	/** Override and return false to deny all incoming connections. 
		\param p ListenSocket class pointer (use GetPort to identify which one) */
	virtual bool OkToAccept(Socket *p) = 0;

	/** Use with care, always lock with h.GetMutex() if multithreaded */
	virtual const std::map<SOCKET, Socket *>& AllSockets() = 0;

	/** Override to accept longer lines than TCP_LINE_SIZE */
	virtual size_t MaxTcpLineSize() = 0;

	virtual void SetCallOnConnect(bool = true) = 0;
	virtual void SetDetach(bool = true) = 0;
	virtual void SetTimeout(bool = true) = 0;
	virtual void SetRetry(bool = true) = 0;
	virtual void SetClose(bool = true) = 0;


	// -------------------------------------------------------------------------
	// Connection pool
	// -------------------------------------------------------------------------
#ifdef ENABLE_POOL
	/** Find available open connection (used by connection pool). */
	virtual ISocketHandler::PoolSocket *FindConnection(int type,const std::string& protocol,SocketAddress&) = 0;
	/** Enable connection pool (by default disabled). */
	virtual void EnablePool(bool = true) = 0;
	/** Check pool status. 
		\return true if connection pool is enabled */
	virtual bool PoolEnabled() = 0;
#endif // ENABLE_POOL

	// -------------------------------------------------------------------------
	// Socks4
	// -------------------------------------------------------------------------
#ifdef ENABLE_SOCKS4
	/** Set socks4 server ip that all new tcp sockets should use. */
	virtual void SetSocks4Host(ipaddr_t) = 0;
	/** Set socks4 server hostname that all new tcp sockets should use. */
	virtual void SetSocks4Host(const std::string& ) = 0;
	/** Set socks4 server port number that all new tcp sockets should use. */
	virtual void SetSocks4Port(port_t) = 0;
	/** Set optional socks4 userid. */
	virtual void SetSocks4Userid(const std::string& ) = 0;
	/** If connection to socks4 server fails, immediately try direct connection to final host. */
	virtual void SetSocks4TryDirect(bool = true) = 0;
	/** Get socks4 server ip. 
		\return socks4 server ip */
	virtual ipaddr_t GetSocks4Host() = 0;
	/** Get socks4 port number.
		\return socks4 port number */
	virtual port_t GetSocks4Port() = 0;
	/** Get socks4 userid (optional).
		\return socks4 userid */
	virtual const std::string& GetSocks4Userid() = 0;
	/** Check status of socks4 try direct flag.
		\return true if direct connection should be tried if connection to socks4 server fails */
	virtual bool Socks4TryDirect() = 0;
#endif // ENABLE_SOCKS4

	// -------------------------------------------------------------------------
	// DNS resolve server
	// -------------------------------------------------------------------------
#ifdef ENABLE_RESOLVER
	/** Enable asynchronous DNS. 
		\param port Listen port of asynchronous dns server */
	virtual void EnableResolver(port_t = 16667) = 0;
	/** Check resolver status.
		\return true if resolver is enabled */
	virtual bool ResolverEnabled() = 0;
	/** Queue a dns request.
		\param host Hostname to be resolved
		\param port Port number will be echoed in Socket::OnResolved callback */
	virtual int Resolve(Socket *,const std::string& host,port_t port) = 0;
#ifdef ENABLE_IPV6
	virtual int Resolve6(Socket *,const std::string& host,port_t port) = 0;
#endif
	/** Do a reverse dns lookup. */
	virtual int Resolve(Socket *,ipaddr_t a) = 0;
#ifdef ENABLE_IPV6
	virtual int Resolve(Socket *,in6_addr& a) = 0;
#endif
	/** Get listen port of asynchronous dns server. */
	virtual port_t GetResolverPort() = 0;
	/** Resolver thread ready for queries. */
	virtual bool ResolverReady() = 0;
	/** Returns true if socket waiting for a resolve event. */
	virtual bool Resolving(Socket *) = 0;
#endif // ENABLE_RESOLVER

#ifdef ENABLE_DETACH
	/** Indicates that the handler runs under SocketThread. */
	virtual void SetSlave(bool x = true) = 0;
	/** Indicates that the handler runs under SocketThread. */
	virtual bool IsSlave() = 0;
#endif // ENABLE_DETACH

};


#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // _SOCKETS_ISocketHandler_H

