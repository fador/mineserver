/**
 **	\file Exception.h
 **	\date  2007-09-28
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
#ifndef _Sockets_Exception_H
#define _Sockets_Exception_H

#include "sockets-config.h"
#include <string>

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif



class Exception
{
public:
	Exception(const std::string& description);
	virtual ~Exception() {}

	virtual const std::string ToString() const;
	virtual const std::string Stack() const;

	Exception(const Exception& ) {} // copy constructor

	Exception& operator=(const Exception& ) { return *this; } // assignment operator

private:
	std::string m_description;
	std::string m_stack;

};



#ifdef SOCKETS_NAMESPACE
} // namespace SOCKETS_NAMESPACE {
#endif

#endif // _Sockets_Exception_H

