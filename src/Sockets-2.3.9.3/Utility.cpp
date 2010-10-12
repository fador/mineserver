/** \file Utility.cpp
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
#include <string>
#include "Utility.h"
#include "Parse.h"
#include "Ipv4Address.h"
#include "Ipv6Address.h"
#include "Base64.h"
#include <vector>
#ifdef _WIN32
#include <time.h>
#else
#include <netdb.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
// --- stack
#ifdef LINUX
#include <cxxabi.h>
#include <execinfo.h>
#include <dlfcn.h>
#endif
// ---
#include <map>

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


// defines for the random number generator
#define TWIST_IA        397
#define TWIST_IB       (TWIST_LEN - TWIST_IA)
#define UMASK           0x80000000
#define LMASK           0x7FFFFFFF
#define MATRIX_A        0x9908B0DF
#define TWIST(b,i,j)   ((b)[i] & UMASK) | ((b)[j] & LMASK)
#define MAGIC_TWIST(s) (((s) & 1) * MATRIX_A)


// statics
std::string Utility::m_host;
bool Utility::m_local_resolved = false;
ipaddr_t Utility::m_ip = 0;
std::string Utility::m_addr;
#ifdef ENABLE_IPV6
#ifdef IPPROTO_IPV6
struct in6_addr Utility::m_local_ip6;
std::string Utility::m_local_addr6;
#endif
#endif
const char *Utility::Logo =
	"iVBORw0KGgoAAAANSUhEUgAAAGAAAABeCAIAAABTioayAAAACXBIWXMAAABkAAAAZAAPlsXdAAAAUHRF"
	"WHRSYXcgcHJvZmlsZSB0eXBlIEFQUDEyAApnZW5lcmljIHByb2ZpbGUKICAgICAgMTUKNDQ3NTYzNmI3"
	"OTAwMDEwMDA0MDAwMDAwM2MwMDAwCg2F1B0AABb4SURBVHja7Xx7rGVXed/3WGs/zvPeuTPXMzbjcUpq"
	"HAwlpQlYoUlDaJsAiVuFFKSKGlUlKgXqIKJAKxfRR4IUZEpSt1FoQEK4oYQEErlSFAoKIEoo2BQasBmc"
	"2NjyYzxz5957Hnufvfd6fF//2Oecez2hvbXvmZkg3U9b556Zc85ea//291rf+n0bVRWO5P8udLUn8Jdd"
	"jgA6QI4AOkCOADpAjgA6QI4AOkCOADpAjgA6QI4AOkCOADpAjgA6QI4AOkCOADpAjgA6QMxVHHtfoUWx"
	"/dP+C5fvABGv4gwBAK9qPWg5NLbvrjIY302ujgapAmKrQQiqC3xAQQF0H1AIcyXSq4XeZdegfedfWs78"
	"jYKAAgDi3sVri4mqzH+D9DSTu+I2dyVNTEFRl34GUTUikiqotrgoAqqIAhBxqzj7ZocAighXWJUuI0Cq"
	"injJ+SMAIZKolkXlGrOzM61mjfckCoN+X9UjorU26wgb7HTSPAfDRlVVFQAR21NeOT26XAC1p10CpKoI"
	"iKTO+ckkFpO4vVX1Bh3kkKRsDCIBE4sIMRNx8FTPYgwQfJ1k4ZpT3W4nAQBVQaSF/1pcw+VE63JrEKiC"
	"KhChC83Fi6EYEyp2uzxYN4kFY6H1O63TIUQAiCIIEgK5Rke7zWhUE2a9AR+/JunmpIr7of/eA2hhWREA"
	"QUlBkGg8af7s7E4n75+6Lu/2iBkUAgJLhP2+RkSJsNU5ACUCQCAyxTQ88eS0rt1gSGdOD621Ik8zs8uH"
	"0eUCSDSqCDHGUD/2mCvLXpbWp051bEqqGoIAINHeVRHRJTMREQBEACQEUDa0vT07/2QZvbvxphO9vhGh"
	"1m0j0uVzSysGaOF6QASJQvD4yKN1UeiZ053eULzX4FsspB221RdVZWYARaTFaeYXjIgiqqpEag16jw9+"
	"e3daTH/gBevHjw1EoIXle0ODdJH/KQiCxiiPPOLrWX7taeh1o3ek8wQH26jUao1qa1ZPy6VbgNovtIAi"
	"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";


std::string Utility::base64(const std::string& str_in)
{
	std::string str;
	Base64 m_b;
	m_b.encode(str_in, str, false); // , false == do not add cr/lf
	return str;
}


std::string Utility::base64d(const std::string& str_in)
{
	std::string str;
	Base64 m_b;
	m_b.decode(str_in, str);
	return str;
}


std::string Utility::l2string(long l)
{
	std::string str;
	char tmp[100];
	snprintf(tmp,sizeof(tmp),"%ld",l);
	str = tmp;
	return str;
}


std::string Utility::bigint2string(uint64_t l)
{
	std::string str;
	uint64_t tmp = l;
	while (tmp)
	{
		uint64_t a = tmp % 10;
		str = (char)(a + 48) + str;
		tmp /= 10;
	}
	if (!str.size())
	{
		str = "0";
	}
	return str;
}


uint64_t Utility::atoi64(const std::string& str) 
{
	uint64_t l = 0;
	for (size_t i = 0; i < str.size(); ++i)
	{
		l = l * 10 + str[i] - 48;
	}
	return l;
}


unsigned int Utility::hex2unsigned(const std::string& str)
{
	unsigned int r = 0;
	for (size_t i = 0; i < str.size(); ++i)
	{
		r = r * 16 + str[i] - 48 - ((str[i] >= 'A') ? 7 : 0) - ((str[i] >= 'a') ? 32 : 0);
	}
	return r;
}


/*
* Encode string per RFC1738 URL encoding rules
* tnx rstaveley
*/
std::string Utility::rfc1738_encode(const std::string& src)
{
static	char hex[] = "0123456789ABCDEF";
	std::string dst;
	for (size_t i = 0; i < src.size(); ++i)
	{
		if (isalnum(src[i]))
		{
			dst += src[i];
		}
		else
		if (src[i] == ' ')
		{
			dst += '+';
		}
		else
		{
			unsigned char c = static_cast<unsigned char>(src[i]);
			dst += '%';
			dst += hex[c / 16];
			dst += hex[c % 16];
		}
	}
	return dst;
} // rfc1738_encode


/*
* Decode string per RFC1738 URL encoding rules
* tnx rstaveley
*/
std::string Utility::rfc1738_decode(const std::string& src)
{
	std::string dst;
	for (size_t i = 0; i < src.size(); ++i)
	{
		if (src[i] == '%' && isxdigit(src[i + 1]) && isxdigit(src[i + 2]))
		{
			char c1 = src[++i];
			char c2 = src[++i];
			c1 = c1 - 48 - ((c1 >= 'A') ? 7 : 0) - ((c1 >= 'a') ? 32 : 0);
			c2 = c2 - 48 - ((c2 >= 'A') ? 7 : 0) - ((c2 >= 'a') ? 32 : 0);
			dst += (char)(c1 * 16 + c2);
		}
		else
		if (src[i] == '+')
		{
			dst += ' ';
		}
		else
		{
			dst += src[i];
		}
	}
	return dst;
} // rfc1738_decode


bool Utility::isipv4(const std::string& str)
{
	int dots = 0;
	// %! ignore :port?
	for (size_t i = 0; i < str.size(); ++i)
	{
		if (str[i] == '.')
			dots++;
		else
		if (!isdigit(str[i]))
			return false;
	}
	if (dots != 3)
		return false;
	return true;
}


bool Utility::isipv6(const std::string& str)
{
	size_t qc = 0;
	size_t qd = 0;
	for (size_t i = 0; i < str.size(); ++i)
	{
		qc += (str[i] == ':') ? 1 : 0;
		qd += (str[i] == '.') ? 1 : 0;
	}
	if (qc > 7)
	{
		return false;
	}
	if (qd && qd != 3)
	{
		return false;
	}
	Parse pa(str,":.");
	std::string tmp = pa.getword();
	while (tmp.size())
	{
		if (tmp.size() > 4)
		{
			return false;
		}
		for (size_t i = 0; i < tmp.size(); ++i)
		{
			if (tmp[i] < '0' || (tmp[i] > '9' && tmp[i] < 'A') ||
				(tmp[i] > 'F' && tmp[i] < 'a') || tmp[i] > 'f')
			{
				return false;
			}
		}
		//
		tmp = pa.getword();
	}
	return true;
}


bool Utility::u2ip(const std::string& str, ipaddr_t& l)
{
	struct sockaddr_in sa;
	bool r = Utility::u2ip(str, sa);
	memcpy(&l, &sa.sin_addr, sizeof(l));
	return r;
}


#ifdef ENABLE_IPV6
#ifdef IPPROTO_IPV6
bool Utility::u2ip(const std::string& str, struct in6_addr& l)
{
	struct sockaddr_in6 sa;
	bool r = Utility::u2ip(str, sa);
	l = sa.sin6_addr;
	return r;
}
#endif
#endif


void Utility::l2ip(const ipaddr_t ip, std::string& str)
{
	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	memcpy(&sa.sin_addr, &ip, sizeof(sa.sin_addr));
	Utility::reverse( (struct sockaddr *)&sa, sizeof(sa), str, NI_NUMERICHOST);
}


void Utility::l2ip(const in_addr& ip, std::string& str)
{
	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr = ip;
	Utility::reverse( (struct sockaddr *)&sa, sizeof(sa), str, NI_NUMERICHOST);
}


#ifdef ENABLE_IPV6
#ifdef IPPROTO_IPV6
void Utility::l2ip(const struct in6_addr& ip, std::string& str,bool mixed)
{
	char slask[100]; // l2ip temporary
	*slask = 0;
	unsigned int prev = 0;
	bool skipped = false;
	bool ok_to_skip = true;
	if (mixed)
	{
		unsigned short x;
		unsigned short addr16[8];
		memcpy(addr16, &ip, sizeof(addr16));
		for (size_t i = 0; i < 6; ++i)
		{
			x = ntohs(addr16[i]);
			if (*slask && (x || !ok_to_skip || prev))
			{
#if defined( _WIN32) && !defined(__CYGWIN__)
				strcat_s(slask,sizeof(slask),":");
#else
				strcat(slask,":");
#endif
			}
			if (x || !ok_to_skip)
			{
				snprintf(slask + strlen(slask),sizeof(slask) - strlen(slask),"%x", x);
				if (x && skipped)
					ok_to_skip = false;
			}
			else
			{
				skipped = true;
			}
			prev = x;
		}
		x = ntohs(addr16[6]);
		snprintf(slask + strlen(slask),sizeof(slask) - strlen(slask),":%u.%u",x / 256,x & 255);
		x = ntohs(addr16[7]);
		snprintf(slask + strlen(slask),sizeof(slask) - strlen(slask),".%u.%u",x / 256,x & 255);
	}
	else
	{
		struct sockaddr_in6 sa;
		memset(&sa, 0, sizeof(sa));
		sa.sin6_family = AF_INET6;
		sa.sin6_addr = ip;
		Utility::reverse( (struct sockaddr *)&sa, sizeof(sa), str, NI_NUMERICHOST);
		return;
	}
	str = slask;
}


int Utility::in6_addr_compare(in6_addr a,in6_addr b)
{
	for (size_t i = 0; i < 16; ++i)
	{
		if (a.s6_addr[i] < b.s6_addr[i])
			return -1;
		if (a.s6_addr[i] > b.s6_addr[i])
			return 1;
	}
	return 0;
}
#endif
#endif


void Utility::ResolveLocal()
{
	char h[256];

	// get local hostname and translate into ip-address
	*h = 0;
	gethostname(h,255);
	{
		if (Utility::u2ip(h, m_ip))
		{
			Utility::l2ip(m_ip, m_addr);
		}
	}
#ifdef ENABLE_IPV6
#ifdef IPPROTO_IPV6
	memset(&m_local_ip6, 0, sizeof(m_local_ip6));
	{
		if (Utility::u2ip(h, m_local_ip6))
		{
			Utility::l2ip(m_local_ip6, m_local_addr6);
		}
	}
#endif
#endif
	m_host = h;
	m_local_resolved = true;
}


const std::string& Utility::GetLocalHostname()
{
	if (!m_local_resolved)
	{
		ResolveLocal();
	}
	return m_host;
}


ipaddr_t Utility::GetLocalIP()
{
	if (!m_local_resolved)
	{
		ResolveLocal();
	}
	return m_ip;
}


const std::string& Utility::GetLocalAddress()
{
	if (!m_local_resolved)
	{
		ResolveLocal();
	}
	return m_addr;
}


#ifdef ENABLE_IPV6
#ifdef IPPROTO_IPV6
const struct in6_addr& Utility::GetLocalIP6()
{
	if (!m_local_resolved)
	{
		ResolveLocal();
	}
	return m_local_ip6;
}


const std::string& Utility::GetLocalAddress6()
{
	if (!m_local_resolved)
	{
		ResolveLocal();
	}
	return m_local_addr6;
}
#endif
#endif


const std::string Utility::GetEnv(const std::string& name)
{
#if defined( _WIN32) && !defined(__CYGWIN__)
	size_t sz = 0;
	char tmp[2048];
	if (getenv_s(&sz, tmp, sizeof(tmp), name.c_str()))
	{
		*tmp = 0;
	}
	return tmp;
#else
	char *s = getenv(name.c_str());
	if (!s)
		return "";
	return s;
#endif
}


void Utility::SetEnv(const std::string& var,const std::string& value)
{
#if (defined(SOLARIS8) || defined(SOLARIS))
	{
		static std::map<std::string, char *> vmap;
		if (vmap.find(var) != vmap.end())
		{
			delete[] vmap[var];
		}
		size_t sz = var.size() + 1 + value.size() + 1;
		vmap[var] = new char[sz];
		snprintf(vmap[var], sz, "%s=%s", var.c_str(), value.c_str());
		putenv( vmap[var] );
	}
#elif defined _WIN32
	{
		std::string slask = var + "=" + value;
		_putenv( (char *)slask.c_str());
	}
#else
	setenv(var.c_str(), value.c_str(), 1);
#endif
}


std::string Utility::Sa2String(struct sockaddr *sa)
{
#ifdef ENABLE_IPV6
#ifdef IPPROTO_IPV6
	if (sa -> sa_family == AF_INET6)
	{
		struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)sa;
		std::string tmp;
		Utility::l2ip(sa6 -> sin6_addr, tmp);
		return tmp + ":" + Utility::l2string(ntohs(sa6 -> sin6_port));
	}
#endif
#endif
	if (sa -> sa_family == AF_INET)
	{
		struct sockaddr_in *sa4 = (struct sockaddr_in *)sa;
		ipaddr_t a;
		memcpy(&a, &sa4 -> sin_addr, 4);
		std::string tmp;
		Utility::l2ip(a, tmp);
		return tmp + ":" + Utility::l2string(ntohs(sa4 -> sin_port));
	}
	return "";
}


void Utility::GetTime(struct timeval *p)
{
#ifdef _WIN32
	FILETIME ft; // Contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
	GetSystemTimeAsFileTime(&ft);
	uint64_t tt;
	memcpy(&tt, &ft, sizeof(tt));
	tt /= 10; // make it usecs
	p->tv_sec = (long)tt / 1000000;
	p->tv_usec = (long)tt % 1000000;
#else
	gettimeofday(p, NULL);
#endif
}


std::auto_ptr<SocketAddress> Utility::CreateAddress(struct sockaddr *sa,socklen_t sa_len)
{
	switch (sa -> sa_family)
	{
	case AF_INET:
		if (sa_len == sizeof(struct sockaddr_in))
		{
			struct sockaddr_in *p = (struct sockaddr_in *)sa;
			return std::auto_ptr<SocketAddress>(new Ipv4Address(*p));
		}
		break;
#ifdef ENABLE_IPV6
#ifdef IPPROTO_IPV6
	case AF_INET6:
		if (sa_len == sizeof(struct sockaddr_in6))
		{
			struct sockaddr_in6 *p = (struct sockaddr_in6 *)sa;
			return std::auto_ptr<SocketAddress>(new Ipv6Address(*p));
		}
		break;
#endif
#endif
	}
	return std::auto_ptr<SocketAddress>(NULL);
}


bool Utility::u2ip(const std::string& host, struct sockaddr_in& sa, int ai_flags)
{
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
#ifdef NO_GETADDRINFO
	if ((ai_flags & AI_NUMERICHOST) != 0 || isipv4(host))
	{
		Parse pa((char *)host.c_str(), ".");
		union {
			struct {
				unsigned char b1;
				unsigned char b2;
				unsigned char b3;
				unsigned char b4;
			} a;
			ipaddr_t l;
		} u;
		u.a.b1 = static_cast<unsigned char>(pa.getvalue());
		u.a.b2 = static_cast<unsigned char>(pa.getvalue());
		u.a.b3 = static_cast<unsigned char>(pa.getvalue());
		u.a.b4 = static_cast<unsigned char>(pa.getvalue());
		memcpy(&sa.sin_addr, &u.l, sizeof(sa.sin_addr));
		return true;
	}
#ifndef LINUX
	struct hostent *he = gethostbyname( host.c_str() );
	if (!he)
	{
		return false;
	}
	memcpy(&sa.sin_addr, he -> h_addr, sizeof(sa.sin_addr));
#else
	struct hostent he;
	struct hostent *result = NULL;
	int myerrno = 0;
	char buf[2000];
	int n = gethostbyname_r(host.c_str(), &he, buf, sizeof(buf), &result, &myerrno);
	if (n || !result)
	{
		return false;
	}
	if (he.h_addr_list && he.h_addr_list[0])
		memcpy(&sa.sin_addr, he.h_addr, 4);
	else
		return false;
#endif
	return true;
#else
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	// AI_NUMERICHOST
	// AI_CANONNAME
	// AI_PASSIVE - server
	// AI_ADDRCONFIG
	// AI_V4MAPPED
	// AI_ALL
	// AI_NUMERICSERV
	hints.ai_flags = ai_flags;
	hints.ai_family = AF_INET;
	hints.ai_socktype = 0;
	hints.ai_protocol = 0;
	struct addrinfo *res;
	if (Utility::isipv4(host))
		hints.ai_flags |= AI_NUMERICHOST;
	int n = getaddrinfo(host.c_str(), NULL, &hints, &res);
	if (!n)
	{
		std::vector<struct addrinfo *> vec;
		struct addrinfo *ai = res;
		while (ai)
		{
			if (ai -> ai_addrlen == sizeof(sa))
				vec.push_back( ai );
			ai = ai -> ai_next;
		}
		if (!vec.size())
			return false;
		ai = vec[Utility::Rnd() % vec.size()];
		{
			memcpy(&sa, ai -> ai_addr, ai -> ai_addrlen);
		}
		freeaddrinfo(res);
		return true;
	}
	std::string error = "Error: ";
#ifndef __CYGWIN__
	error += gai_strerror(n);
#endif
	return false;
#endif // NO_GETADDRINFO
}


#ifdef ENABLE_IPV6
#ifdef IPPROTO_IPV6
bool Utility::u2ip(const std::string& host, struct sockaddr_in6& sa, int ai_flags)
{
	memset(&sa, 0, sizeof(sa));
	sa.sin6_family = AF_INET6;
#ifdef NO_GETADDRINFO
	if ((ai_flags & AI_NUMERICHOST) != 0 || isipv6(host))
	{
		std::list<std::string> vec;
		size_t x = 0;
		for (size_t i = 0; i <= host.size(); ++i)
		{
			if (i == host.size() || host[i] == ':')
			{
				std::string s = host.substr(x, i - x);
				//
				if (strstr(s.c_str(),".")) // x.x.x.x
				{
					Parse pa(s,".");
					char slask[100]; // u2ip temporary hex2string conversion
					unsigned long b0 = static_cast<unsigned long>(pa.getvalue());
					unsigned long b1 = static_cast<unsigned long>(pa.getvalue());
					unsigned long b2 = static_cast<unsigned long>(pa.getvalue());
					unsigned long b3 = static_cast<unsigned long>(pa.getvalue());
					snprintf(slask,sizeof(slask),"%lx",b0 * 256 + b1);
					vec.push_back(slask);
					snprintf(slask,sizeof(slask),"%lx",b2 * 256 + b3);
					vec.push_back(slask);
				}
				else
				{
					vec.push_back(s);
				}
				//
				x = i + 1;
			}
		}
		size_t sz = vec.size(); // number of byte pairs
		size_t i = 0; // index in in6_addr.in6_u.u6_addr16[] ( 0 .. 7 )
		unsigned short addr16[8];
		for (std::list<std::string>::iterator it = vec.begin(); it != vec.end(); ++it)
		{
			std::string bytepair = *it;
			if (bytepair.size())
			{
				addr16[i++] = htons(Utility::hex2unsigned(bytepair));
			}
			else
			{
				addr16[i++] = 0;
				while (sz++ < 8)
				{
					addr16[i++] = 0;
				}
			}
		}
		memcpy(&sa.sin6_addr, addr16, sizeof(addr16));
		return true;
	}
#ifdef SOLARIS
	int errnum = 0;
	struct hostent *he = getipnodebyname( host.c_str(), AF_INET6, 0, &errnum );
#else
	struct hostent *he = gethostbyname2( host.c_str(), AF_INET6 );
#endif
	if (!he)
	{
		return false;
	}
	memcpy(&sa.sin6_addr,he -> h_addr_list[0],he -> h_length);
#ifdef SOLARIS
	free(he);
#endif
	return true;
#else
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = ai_flags;
	hints.ai_family = AF_INET6;
	hints.ai_socktype = 0;
	hints.ai_protocol = 0;
	struct addrinfo *res;
	if (Utility::isipv6(host))
		hints.ai_flags |= AI_NUMERICHOST;
	int n = getaddrinfo(host.c_str(), NULL, &hints, &res);
	if (!n)
	{
		std::vector<struct addrinfo *> vec;
		struct addrinfo *ai = res;
		while (ai)
		{
			if (ai -> ai_addrlen == sizeof(sa))
				vec.push_back( ai );
			ai = ai -> ai_next;
		}
		if (!vec.size())
			return false;
		ai = vec[Utility::Rnd() % vec.size()];
		{
			memcpy(&sa, ai -> ai_addr, ai -> ai_addrlen);
		}
		freeaddrinfo(res);
		return true;
	}
	std::string error = "Error: ";
#ifndef __CYGWIN__
	error += gai_strerror(n);
#endif
	return false;
#endif // NO_GETADDRINFO
}
#endif // IPPROTO_IPV6
#endif // ENABLE_IPV6


bool Utility::reverse(struct sockaddr *sa, socklen_t sa_len, std::string& hostname, int flags)
{
	std::string service;
	return Utility::reverse(sa, sa_len, hostname, service, flags);
}


bool Utility::reverse(struct sockaddr *sa, socklen_t sa_len, std::string& hostname, std::string& service, int flags)
{
	hostname = "";
	service = "";
#ifdef NO_GETADDRINFO
	switch (sa -> sa_family)
	{
	case AF_INET:
		if (flags & NI_NUMERICHOST)
		{
			union {
				struct {
					unsigned char b1;
					unsigned char b2;
					unsigned char b3;
					unsigned char b4;
				} a;
				ipaddr_t l;
			} u;
			struct sockaddr_in *sa_in = (struct sockaddr_in *)sa;
			memcpy(&u.l, &sa_in -> sin_addr, sizeof(u.l));
			char tmp[100];
			snprintf(tmp, sizeof(tmp), "%u.%u.%u.%u", u.a.b1, u.a.b2, u.a.b3, u.a.b4);
			hostname = tmp;
			return true;
		}
		else
		{
			struct sockaddr_in *sa_in = (struct sockaddr_in *)sa;
			struct hostent *h = gethostbyaddr( (const char *)&sa_in -> sin_addr, sizeof(sa_in -> sin_addr), AF_INET);
			if (h)
			{
				hostname = h -> h_name;
				return true;
			}
		}
		break;
#ifdef ENABLE_IPV6
	case AF_INET6:
		if (flags & NI_NUMERICHOST)
		{
			char slask[100]; // l2ip temporary
			*slask = 0;
			unsigned int prev = 0;
			bool skipped = false;
			bool ok_to_skip = true;
			{
				unsigned short addr16[8];
				struct sockaddr_in6 *sa_in6 = (struct sockaddr_in6 *)sa;
				memcpy(addr16, &sa_in6 -> sin6_addr, sizeof(addr16));
				for (size_t i = 0; i < 8; ++i)
				{
					unsigned short x = ntohs(addr16[i]);
					if (*slask && (x || !ok_to_skip || prev))
					{
#if defined( _WIN32) && !defined(__CYGWIN__)
						strcat_s(slask, sizeof(slask),":");
#else
						strcat(slask,":");
#endif
					}
					if (x || !ok_to_skip)
					{
						snprintf(slask + strlen(slask), sizeof(slask) - strlen(slask),"%x", x);
						if (x && skipped)
							ok_to_skip = false;
					}
					else
					{
						skipped = true;
					}
					prev = x;
				}
			}
			if (!*slask)
			{
#if defined( _WIN32) && !defined(__CYGWIN__)
				strcpy_s(slask, sizeof(slask), "::");
#else
				strcpy(slask, "::");
#endif
			}
			hostname = slask;
			return true;
		}
		else
		{
			// %! TODO: ipv6 reverse lookup
			struct sockaddr_in6 *sa_in = (struct sockaddr_in6 *)sa;
			struct hostent *h = gethostbyaddr( (const char *)&sa_in -> sin6_addr, sizeof(sa_in -> sin6_addr), AF_INET6);
			if (h)
			{
				hostname = h -> h_name;
				return true;
			}
		}
		break;
#endif
	}
	return false;
#else
	char host[NI_MAXHOST];
	// NI_NOFQDN
	// NI_NUMERICHOST
	// NI_NAMEREQD
	// NI_NUMERICSERV
	// NI_DGRAM
	int n = getnameinfo(sa, sa_len, host, sizeof(host), NULL, 0, flags);
	if (n)
	{
		// EAI_AGAIN
		// EAI_BADFLAGS
		// EAI_FAIL
		// EAI_FAMILY
		// EAI_MEMORY
		// EAI_NONAME
		// EAI_OVERFLOW
		// EAI_SYSTEM
		return false;
	}
	hostname = host;
	return true;
#endif // NO_GETADDRINFO
}


bool Utility::u2service(const std::string& name, int& service, int ai_flags)
{
#ifdef NO_GETADDRINFO
	// %!
	return false;
#else
	struct addrinfo hints;
	service = 0;
	memset(&hints, 0, sizeof(hints));
	// AI_NUMERICHOST
	// AI_CANONNAME
	// AI_PASSIVE - server
	// AI_ADDRCONFIG
	// AI_V4MAPPED
	// AI_ALL
	// AI_NUMERICSERV
	hints.ai_flags = ai_flags;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = 0;
	hints.ai_protocol = 0;
	struct addrinfo *res;
	int n = getaddrinfo(NULL, name.c_str(), &hints, &res);
	if (!n)
	{
		service = res -> ai_protocol;
		freeaddrinfo(res);
		return true;
	}
	return false;
#endif // NO_GETADDRINFO
}


unsigned long Utility::ThreadID()
{
#ifdef _WIN32
	return GetCurrentThreadId();
#else
	return (unsigned long)pthread_self();
#endif
}


std::string Utility::ToLower(const std::string& str)
{
	std::string r;
	for (size_t i = 0; i < str.size(); ++i)
	{
		if (str[i] >= 'A' && str[i] <= 'Z')
			r += str[i] | 32;
		else
			r += str[i];
	}
	return r;
}


std::string Utility::ToUpper(const std::string& str)
{
	std::string r;
	for (size_t i = 0; i < str.size(); ++i)
	{
		if (str[i] >= 'a' && str[i] <= 'z')
			r += (char)(str[i] - 32);
		else
			r += str[i];
	}
	return r;
}


std::string Utility::ToString(double d)
{
	char tmp[100];
	snprintf(tmp, sizeof(tmp), "%f", d);
	return tmp;
}


unsigned long Utility::Rnd()
{
static	Utility::Rng generator( (unsigned long)time(NULL) );
	return generator.Get();
}


Utility::Rng::Rng(unsigned long seed) : m_value( 0 )
{
	m_tmp[0] = seed & 0xffffffffUL;
	for (int i = 1; i < TWIST_LEN; ++i)
	{
		m_tmp[i] = (1812433253UL * (m_tmp[i - 1] ^ (m_tmp[i - 1] >> 30)) + i);
	}
}
					

unsigned long Utility::Rng::Get()
{
	unsigned long val = m_tmp[m_value];
	++m_value;
	if (m_value == TWIST_LEN)
	{
		for (int i = 0; i < TWIST_IB; ++i)
		{
			unsigned long s = TWIST(m_tmp, i, i + 1);
			m_tmp[i] = m_tmp[i + TWIST_IA] ^ (s >> 1) ^ MAGIC_TWIST(s);
		}
		{
			for (int i = 0; i < TWIST_LEN - 1; ++i)
			{
				unsigned long s = TWIST(m_tmp, i, i + 1);
				m_tmp[i] = m_tmp[i - TWIST_IB] ^ (s >> 1) ^ MAGIC_TWIST(s);
			}
		}
		unsigned long s = TWIST(m_tmp, TWIST_LEN - 1, 0);
		m_tmp[TWIST_LEN - 1] = m_tmp[TWIST_IA - 1] ^ (s >> 1) ^ MAGIC_TWIST(s);

		m_value = 0;
	}
	return val;
}


bool Utility::ncmap_compare::operator()(const std::string& x, const std::string& y) const
{
	return strcasecmp(x.c_str(), y.c_str()) < 0;
}


Utility::Uri::Uri(const std::string& url) : m_url(url), m_port(0), m_path(url)
{
	size_t pos = url.find("://");
	if (pos != std::string::npos)
	{
		m_protocol = Utility::ToLower(url.substr(0, pos));
		m_port = (m_protocol == "http") ? 80 :
			(m_protocol == "https") ? 443 : 0;
		m_host = url.substr(pos + 3);
		pos = m_host.find("/");
		if (pos != std::string::npos)
		{
			m_path = m_host.substr(pos);
			m_host = m_host.substr(0, pos);
		}
		pos = m_host.find(":");
		if (pos != std::string::npos)
		{
			m_port = atoi(m_host.substr(pos + 1).c_str());
			m_host = m_host.substr(0, pos);
		}
	}
	pos = m_path.find("?");
	if (pos != std::string::npos)
	{
		m_uri = m_path.substr(0, pos);
		m_query_string = m_path.substr(pos + 1);
	}
	else
	{
		m_uri = m_path;
	}
	pos = std::string::npos;
	for (size_t i = 0; i < m_uri.size(); ++i)
		if (m_uri[i] == '/')
			pos = i;
	if (pos != std::string::npos)
		m_file = m_uri.substr(pos + 1);
	pos = std::string::npos;
	for (size_t i = 0; i < m_uri.size(); ++i)
		if (m_uri[i] == '.')
			pos = i;
	if (pos != std::string::npos)
		m_ext = m_uri.substr(pos + 1);
}


Utility::Path::Path(const std::string& _str)
{
	std::string str = _str;
	for (size_t i = 0; i < str.size(); ++i)
	{
#ifdef _WIN32
		if (str[i] == '/')
			str[i] = '\\';
#else
		if (str[i] == '\\')
			str[i] = '/';
#endif
	}
#ifndef _WIN32
	struct stat st;
	stat(str.c_str(), &st);
	if (S_ISDIR(st.st_mode))
	{
		m_path = str;
		return;
	}
#endif
	size_t x = 0;
	for (size_t i = 0; i < str.size(); ++i)
		if (str[i] == '/' || str[i] == '\\')
			x = i + 1;
	m_path = str.substr(0, x);
	m_file = str.substr(x);
	for (size_t i = 0; i < m_file.size(); ++i)
		if (m_file[i] == '.')
			m_ext = m_file.substr(i + 1);
}


const std::string Utility::Stack()
{
#if defined LINUX
#define BFSIZE 255
	void *buffer[BFSIZE];
	int n = backtrace(buffer, BFSIZE);
	char **res = backtrace_symbols(buffer, n);
	std::string tmp;
	for (int i = 0; i < n; ++i)
	{
		std::string x = res[i];
		std::string plus;
		std::string addr;
		size_t pos = x.find("(");
		if (pos != std::string::npos)
		{
			x = x.substr(pos + 1); // skip executable name

			pos = x.find(")");
			if (pos != std::string::npos)
			{
				addr = x.substr(pos + 1);
				x = x.substr(0, pos);
			}

			pos = x.find("+");
			if (pos != std::string::npos)
			{
				plus = x.substr(pos);
				x = x.substr(0, pos);
			}
		}
		char zz[1000];
		{
			size_t sz = 1000;
			int status = 0;
			abi::__cxa_demangle( x.c_str(), zz, &sz, &status);

			if (!status)
			{
				tmp += zz;
				tmp += plus;
				tmp += addr;
			}
			else
			{
				tmp += res[i];
			}
			tmp += "\n";
		}
		// dladdr() test
		if (0)
		{
			Dl_info info;
			int n = dladdr(buffer[i], &info);
			if (!n)
				printf("%d: dladdr() failed\n", i);
			else
			{
				size_t sz = 1000;
				int status = 0;
				abi::__cxa_demangle( info.dli_sname, zz, &sz, &status);

				printf("%d: %s: %s\n", i, info.dli_fname, info.dli_sname);
				if (!status)
					printf("		%s\n", zz);
			}
		} // end of dladdr() test
	} // for (i)
	free(res);
	return tmp;
#else
	return "Not available";
#endif
}


const std::string Utility::FromUtf8(const std::string& str)
{
	if (!str.size())
		return "";
	std::string r;
	for (size_t i = 0; i < str.size(); ++i)
	{
		if (i < str.size() - 1 && (str[i] & 0xe0) == 0xc0 && (str[i + 1] & 0xc0) == 0x80)
		{
			int c1 = str[i] & 0x1f;
			int c2 = str[++i] & 0x3f;
			int c = (c1 << 6) + c2;
			r += (char)c;
		}
		else
		{
			r += str[i];
		}
	}
	return r;
}

// 110yyyxx 10xxxxxx	

const std::string Utility::ToUtf8(const std::string& str)
{
	if (str.empty())
		return "";
	std::string r;
	for (size_t i = 0; i < str.size(); ++i)
	{
		if (((unsigned)str[i] & 0x80) == 0x80)
		{
			r += (str[i] >> 6) | 0xc0;
			r += (str[i] & 0x3f) | 0x80;
		}
		else
		{
			r += str[i];
		}
	}
	return r;
}


const Utility::Path Utility::CurrentDirectory()
{
#ifdef _WIN32
	TCHAR slask[MAX_PATH + 1];
	DWORD ret =
#ifdef UNICODE
	::GetCurrentDirectoryW(MAX_PATH, slask);
#else
	::GetCurrentDirectoryA(MAX_PATH, slask);
#endif
	if (!ret)
	{
		*slask = 0;
		DWORD err = GetLastError();
	}
	return Path((char *)slask);
#else
	char slask[32000];
	if (!getcwd(slask, 32000))
	{
		return Path(".");
	}
	return Path(slask);
#endif
}


bool Utility::ChangeDirectory(const Utility::Path& to_dir)
{
#ifdef _WIN32
	//return SetCurrentDirectory(to_dir.GetPath().c_str()) ? true : false;
#else
	if (chdir( to_dir.GetPath().c_str() ) == -1)
	{
		return false;
	}
	return true;
#endif
}


void Utility::Sleep(int ms)
{
#ifdef _WIN32
	::Sleep(ms);
#else
	struct timeval tv;
	tv.tv_sec = ms / 1000;
	tv.tv_usec = (ms % 1000) * 1000;
	select(0, NULL, NULL, NULL, &tv);
#endif
}


#ifdef SOCKETS_NAMESPACE
}
#endif

