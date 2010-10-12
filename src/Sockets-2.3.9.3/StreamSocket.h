/**
 **	\file StreamSocket.h
 **	\date  2008-12-20
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2008-2010  Anders Hedstrom

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
#ifndef _StreamSocket_H
#define _StreamSocket_H

#include "Socket.h"


#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


/** SOCK_STREAM Socket base class.
	\ingroup basic */
class StreamSocket : public Socket
{
public:
	StreamSocket(ISocketHandler& );
	~StreamSocket();

	/** Socket should Check Connect on next write event from select(). */
	void SetConnecting(bool = true);

	/** Check connecting flag.
		\return true if the socket is still trying to connect */
	bool Connecting();

	/** Returns true when socket file descriptor is valid,
		socket connection is established, and socket is not about to
		be closed. */
	bool Ready();

	/** Set timeout to use for connection attempt.
		\param x Timeout in seconds */
	void SetConnectTimeout(int x);

	/** Return number of seconds to wait for a connection.
		\return Connection timeout (seconds) */
	int GetConnectTimeout();

	/** Set flush before close to make a tcp socket completely empty its
		output buffer before closing the connection. */
	void SetFlushBeforeClose(bool = true);

	/** Check flush before status.
		\return true if the socket should send all data before closing */
	bool GetFlushBeforeClose();

	/** Define number of connection retries (tcp only).
	    n = 0 - no retry
	    n > 0 - number of retries
	    n = -1 - unlimited retries */
	void SetConnectionRetry(int n);

	/** Get number of maximum connection retries (tcp only). */
	int GetConnectionRetry();

	/** Increase number of actual connection retries (tcp only). */
	void IncreaseConnectionRetries();

	/** Get number of actual connection retries (tcp only). */
	int GetConnectionRetries();

	/** Reset actual connection retries (tcp only). */
	void ResetConnectionRetries();

	/** Called after OnRead if socket is in line protocol mode.
		\sa SetLineProtocol */
	/** Enable the OnLine callback. Do not create your own OnRead
	 * callback when using this. */
	virtual void SetLineProtocol(bool = true);

	/** Check line protocol mode.
		\return true if socket is in line protocol mode */
	bool LineProtocol();

	/** Set shutdown status. */
	void SetShutdown(int);

	/** Get shutdown status. */
	int GetShutdown();

	/** Returns IPPROTO_TCP or IPPROTO_SCTP */
	virtual int Protocol() = 0;

protected:
	StreamSocket(const StreamSocket& ) {} // copy constructor

private:
	StreamSocket& operator=(const StreamSocket& ) { return *this; } // assignment operator

	bool m_bConnecting; ///< Flag indicating connection in progress
	int m_connect_timeout; ///< Connection timeout (seconds)
	bool m_flush_before_close; ///< Send all data before closing (default true)
	int m_connection_retry; ///< Maximum connection retries (tcp)
	int m_retries; ///< Actual number of connection retries (tcp)
	bool m_line_protocol; ///< Line protocol mode flag
	int m_shutdown; ///< Shutdown status
};


#ifdef SOCKETS_NAMESPACE
} // namespace SOCKETS_NAMESPACE {
#endif


#endif // _StreamSocket_H

