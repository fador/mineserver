/**
 **	\file SctpSocket.cpp
 **	\date  2006-09-04
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2007-2010  Anders Hedstrom

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
#include "SctpSocket.h"
#ifdef USE_SCTP
#include "Utility.h"
#include "ISocketHandler.h"
#include <errno.h>
#include "Ipv4Address.h"
#include "Ipv6Address.h"
#ifdef ENABLE_EXCEPTIONS
#include "Exception.h"
#endif

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE
{
#endif


SctpSocket::SctpSocket(ISocketHandler& h,int type) : StreamSocket(h)
,m_type(type)
,m_buf(new char[SCTP_BUFSIZE_READ])
{
	if (type != SOCK_STREAM && type != SOCK_SEQPACKET)
	{
	}
}


SctpSocket::~SctpSocket()
{
	delete[] m_buf;
}


int SctpSocket::Bind(const std::string& a,port_t p)
{
#ifdef ENABLE_IPV6
#ifdef IPPROTO_IPV6
	if (IsIpv6())
	{
		Ipv6Address ad(a, p);
		return Bind(ad);
	}
#endif
#endif
	Ipv4Address ad(a, p);
	return Bind(ad);
}


int SctpSocket::Bind(SocketAddress& ad)
{
	if (!ad.IsValid())
	{
		Handler().LogError(this, "SctpSocket", -1, "invalid address", LOG_LEVEL_ERROR);
		return -1;
	}
	if (GetSocket() == INVALID_SOCKET)
	{
		Attach(CreateSocket(ad.GetFamily(), m_type, "sctp"));
	}
	if (GetSocket() != INVALID_SOCKET)
	{
		int n = bind(GetSocket(), ad, ad);
		if (n == -1)
		{
			Handler().LogError(this, "SctpSocket", -1, "bind() failed", LOG_LEVEL_ERROR);
#ifdef ENABLE_EXCEPTIONS
			throw Exception("bind() failed for SctpSocket, port: " + Utility::l2string(ad.GetPort()));
#endif
		}
		return n;
	}
	return -1;
}


int SctpSocket::AddAddress(const std::string& a,port_t p)
{
#ifdef ENABLE_IPV6
#ifdef IPPROTO_IPV6
	if (IsIpv6())
	{
		Ipv6Address ad(a, p);
		return AddAddress(ad);
	}
#endif
#endif
	Ipv4Address ad(a, p);
	return AddAddress(ad);
}


int SctpSocket::AddAddress(SocketAddress& ad)
{
	if (!ad.IsValid())
	{
		Handler().LogError(this, "SctpSocket", -1, "invalid address", LOG_LEVEL_ERROR);
		return -1;
	}
	if (GetSocket() == INVALID_SOCKET)
	{
		Handler().LogError(this, "SctpSocket", -1, "AddAddress called with invalid file descriptor", LOG_LEVEL_ERROR);
		return -1;
	}
	int n = sctp_bindx(GetSocket(), ad, ad, SCTP_BINDX_ADD_ADDR);
	if (n == -1)
	{
		Handler().LogError(this, "SctpSocket", -1, "sctp_bindx() failed", LOG_LEVEL_ERROR);
	}
	return n;
}


int SctpSocket::RemoveAddress(const std::string& a,port_t p)
{
#ifdef ENABLE_IPV6
#ifdef IPPROTO_IPV6
	if (IsIpv6())
	{
		Ipv6Address ad(a, p);
		return RemoveAddress(ad);
	}
#endif
#endif
	Ipv4Address ad(a, p);
	return RemoveAddress(ad);
}


int SctpSocket::RemoveAddress(SocketAddress& ad)
{
	if (!ad.IsValid())
	{
		Handler().LogError(this, "SctpSocket", -1, "invalid address", LOG_LEVEL_ERROR);
		return -1;
	}
	if (GetSocket() == INVALID_SOCKET)
	{
		Handler().LogError(this, "SctpSocket", -1, "RemoveAddress called with invalid file descriptor", LOG_LEVEL_ERROR);
		return -1;
	}
	int n = sctp_bindx(GetSocket(), ad, ad, SCTP_BINDX_REM_ADDR);
	if (n == -1)
	{
		Handler().LogError(this, "SctpSocket", -1, "sctp_bindx() failed", LOG_LEVEL_ERROR);
	}
	return n;
}


int SctpSocket::Open(const std::string& a,port_t p)
{
#ifdef ENABLE_IPV6
#ifdef IPPROTO_IPV6
	if (IsIpv6())
	{
		Ipv6Address ad(a, p);
		return Open(ad);
	}
#endif
#endif
	Ipv4Address ad(a, p);
	return Open(ad);
}


int SctpSocket::Open(SocketAddress& ad)
{
	if (!ad.IsValid())
	{
		Handler().LogError(this, "SctpSocket", -1, "invalid address", LOG_LEVEL_ERROR);
		return -1;
	}
	if (GetSocket() == INVALID_SOCKET)
	{
		Attach(CreateSocket(ad.GetFamily(), m_type, "sctp"));
	}
	if (GetSocket() != INVALID_SOCKET)
	{
		if (!SetNonblocking(true))
		{
			return -1;
		}
		int n = connect(GetSocket(), ad, ad);
		if (n == -1)
		{
			// check error code that means a connect is in progress
#ifdef _WIN32
			if (Errno == WSAEWOULDBLOCK)
#else
			if (Errno == EINPROGRESS)
#endif
			{
				Handler().LogError(this, "connect: connection pending", Errno, StrError(Errno), LOG_LEVEL_INFO);
				SetConnecting( true ); // this flag will control fd_set's
			}
			else
			{
				Handler().LogError(this, "SctpSocket", -1, "connect() failed", LOG_LEVEL_ERROR);
			}
		}
		return n;
	}
	return -1;
}


#ifndef SOLARIS
int SctpSocket::AddConnection(const std::string& a,port_t p)
{
#ifdef ENABLE_IPV6
#ifdef IPPROTO_IPV6
	if (IsIpv6())
	{
		Ipv6Address ad(a, p);
		return AddConnection(ad);
	}
#endif
#endif
	Ipv4Address ad(a, p);
	return AddConnection(ad);
}


int SctpSocket::AddConnection(SocketAddress& ad)
{
	if (!ad.IsValid())
	{
		Handler().LogError(this, "SctpSocket", -1, "invalid address", LOG_LEVEL_ERROR);
		return -1;
	}
	if (GetSocket() == INVALID_SOCKET)
	{
		Handler().LogError(this, "SctpSocket", -1, "AddConnection called with invalid file descriptor", LOG_LEVEL_ERROR);
		return -1;
	}
	int n = sctp_connectx(GetSocket(), ad, ad);
	if (n == -1)
	{
		Handler().LogError(this, "SctpSocket", -1, "sctp_connectx() failed", LOG_LEVEL_ERROR);
	}
	else
	{
		SetConnecting();
	}
	return n;
}
#endif


int SctpSocket::getpaddrs(sctp_assoc_t id,std::list<std::string>& vec)
{
	struct sockaddr *p = NULL;
	int n = sctp_getpaddrs(GetSocket(), id, &p);
	if (!n || n == -1)
	{
		Handler().LogError(this, "SctpSocket", -1, "sctp_getpaddrs failed", LOG_LEVEL_WARNING);
		return n;
	}
	for (int i = 0; i < n; i++)
	{
		vec.push_back(Utility::Sa2String(&p[i]));
	}
	sctp_freepaddrs(p);
	return n;
}


int SctpSocket::getladdrs(sctp_assoc_t id,std::list<std::string>& vec)
{
	struct sockaddr *p = NULL;
	int n = sctp_getladdrs(GetSocket(), id, &p);
	if (!n || n == -1)
	{
		Handler().LogError(this, "SctpSocket", -1, "sctp_getladdrs failed", LOG_LEVEL_WARNING);
		return n;
	}
	for (int i = 0; i < n; i++)
	{
		vec.push_back(Utility::Sa2String(&p[i]));
	}
	sctp_freeladdrs(p);
	return n;
}


int SctpSocket::PeelOff(sctp_assoc_t id)
{
	int n = sctp_peeloff(GetSocket(), id);
	if (n == -1)
	{
		Handler().LogError(this, "SctpSocket", -1, "PeelOff failed", LOG_LEVEL_WARNING);
		return -1;
	}
	Socket *p = Create();
	p -> Attach(n);
	p -> SetDeleteByHandler();
	Handler().Add(p);
	return n;
}


void SctpSocket::OnRead()
{
/*
	int sctp_recvmsg(int sd, void * msg, size_t * len,
		struct sockaddr * from, socklen_t * fromlen,
		struct sctp_sndrcvinfo * sinfo, int * msg_flags);

	DESCRIPTION
	sctp_recvmsg  is  a  wrapper library function that can be used to receive a message from a socket while using the advanced
	features of SCTP.  sd is the socket descriptor on which the message pointed to by msg of length len is received.

	If from is not NULL, the source address of the message is filled in. The argument fromlen  is  a  value-result  parameter.
	initialized  to  the  size  of the buffer associated with from , and modified on return to indicate the actual size of the
	address stored.

	sinfo is a pointer to a sctp_sndrcvinfo structure to be filled upon receipt of the message.  msg_flags is a pointer  to  a
	integer that is filled with any message flags like MSG_NOTIFICATION or MSG_EOR.

*/
	struct sockaddr sa;
	socklen_t sa_len = 0;
	struct sctp_sndrcvinfo sinfo;
	int flags = 0;
	int n = sctp_recvmsg(GetSocket(), m_buf, SCTP_BUFSIZE_READ, &sa, &sa_len, &sinfo, &flags);
	if (n == -1)
	{
		Handler().LogError(this, "SctpSocket", Errno, StrError(Errno), LOG_LEVEL_FATAL);
		SetCloseAndDelete();
	}
	else
	{
		OnReceiveMessage(m_buf, n, &sa, sa_len, &sinfo, flags);
	}
}


void SctpSocket::OnReceiveMessage(const char *buf,size_t sz,struct sockaddr *sa,socklen_t sa_len,struct sctp_sndrcvinfo *sinfo,int msg_flags)
{
}


void SctpSocket::OnWrite()
{
	if (Connecting())
	{
		int err = SoError();

		// don't reset connecting flag on error here, we want the OnConnectFailed timeout later on
		/// \todo add to read fd_set here
		if (!err) // ok
		{
			Set(!IsDisableRead(), false);
			SetConnecting(false);
			SetCallOnConnect();
			return;
		}
		Handler().LogError(this, "sctp: connect failed", err, StrError(err), LOG_LEVEL_FATAL);
		Set(false, false); // no more monitoring because connection failed

		// failed
#ifdef ENABLE_SOCKS4
		if (Socks4())
		{
			OnSocks4ConnectFailed();
			return;
		}
#endif
		if (GetConnectionRetry() == -1 ||
			(GetConnectionRetry() && GetConnectionRetries() < GetConnectionRetry()) )
		{
			// even though the connection failed at once, only retry after
			// the connection timeout.
			// should we even try to connect again, when CheckConnect returns
			// false it's because of a connection error - not a timeout...
			return;
		}
		SetConnecting(false);
		SetCloseAndDelete( true );
		/// \todo state reason why connect failed
		OnConnectFailed();
		return;
	}
}


void SctpSocket::OnConnectTimeout()
{
	Handler().LogError(this, "connect", -1, "connect timeout", LOG_LEVEL_FATAL);
#ifdef ENABLE_SOCKS4
	if (Socks4())
	{
		OnSocks4ConnectFailed();
		// retry direct connection
	}
	else
#endif
	if (GetConnectionRetry() == -1 ||
		(GetConnectionRetry() && GetConnectionRetries() < GetConnectionRetry()) )
	{
		IncreaseConnectionRetries();
		// ask socket via OnConnectRetry callback if we should continue trying
		if (OnConnectRetry())
		{
			SetRetryClientConnect();
		}
		else
		{
			SetCloseAndDelete( true );
			/// \todo state reason why connect failed
			OnConnectFailed();
		}
	}
	else
	{
		SetCloseAndDelete(true);
		/// \todo state reason why connect failed
		OnConnectFailed();
	}
	//
	SetConnecting(false);
}


#ifdef _WIN32
void SctpSocket::OnException()
{
	if (Connecting())
	{
#ifdef ENABLE_SOCKS4
		if (Socks4())
			OnSocks4ConnectFailed();
		else
#endif
		if (GetConnectionRetry() == -1 ||
			(GetConnectionRetry() &&
			 GetConnectionRetries() < GetConnectionRetry() ))
		{
			// even though the connection failed at once, only retry after
			// the connection timeout
			// should we even try to connect again, when CheckConnect returns
			// false it's because of a connection error - not a timeout...
		}
		else
		{
			SetConnecting(false); // tnx snibbe
			SetCloseAndDelete();
			OnConnectFailed();
		}
		return;
	}
	// %! exception doesn't always mean something bad happened, this code should be reworked
	// errno valid here?
	int err = SoError();
	Handler().LogError(this, "exception on select", err, StrError(err), LOG_LEVEL_FATAL);
	SetCloseAndDelete();
}
#endif // _WIN32


int SctpSocket::Protocol()
{
	return IPPROTO_SCTP;
}


#ifdef SOCKETS_NAMESPACE
} // namespace SOCKETS_NAMESPACE
#endif


#endif // USE_SCTP

