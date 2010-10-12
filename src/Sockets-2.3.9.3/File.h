/** \file File.h
 **	\date  2005-04-25
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
#ifndef _SOCKETS_File_H
#define _SOCKETS_File_H

#include "sockets-config.h"
#include "IFile.h"
#include <stdio.h>

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


/** IFile implementation of a disk file. 
	\ingroup file */
class File : public IFile
{
public:
	File();
	/** convenience: calls fopen() */
	File(const std::string& path, const std::string& mode);
	~File();

	bool fopen(const std::string& path, const std::string& mode);
	void fclose() const;

	size_t fread(char *, size_t, size_t) const;
	size_t fwrite(const char *, size_t, size_t);

	char *fgets(char *, int) const;
	void fprintf(const char *format, ...);

	off_t size() const;
	bool eof() const;

	void reset_read() const;
	void reset_write();

	const std::string& Path() const;

private:
	File(const File& ) {} // copy constructor
	File& operator=(const File& ) { return *this; } // assignment operator

	std::string m_path;
	std::string m_mode;
	mutable FILE *m_fil;
	mutable long m_rptr;
	long m_wptr;
};




#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // _SOCKETS_File_H

