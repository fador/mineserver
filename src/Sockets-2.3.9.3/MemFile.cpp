/** \file MemFile.cpp
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
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif
#include <stdio.h>
#include <stdarg.h>
#include <cstring>
#include <iostream>

#include "MemFile.h"
#include "Exception.h"
#include "Lock.h"
#include "Utility.h"


#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


MemFile::MemFile()
:m_src(m_src)
,m_src_valid(false)
,m_base(new block_t)
,m_current_read(m_base)
,m_current_write(m_base)
,m_current_write_nr(0)
,m_read_ptr(0)
,m_write_ptr(0)
,m_b_read_caused_eof(false)
,m_ref_count(0)
,m_ref_decreased(false)
{
}


MemFile::MemFile(MemFile& s)
:m_src(s)
,m_src_valid(true)
,m_base(s.m_base)
,m_current_read(m_base)
,m_current_write(s.m_current_write)
,m_current_write_nr(s.m_current_write_nr)
,m_read_ptr(0)
,m_write_ptr(s.m_write_ptr)
,m_b_read_caused_eof(false)
,m_ref_count(0)
,m_ref_decreased(false)
,m_path(s.m_path)
{
	m_src.Increase();
}


MemFile::MemFile(File& f)
:m_src(m_src)
,m_src_valid(false)
,m_base(new block_t)
,m_current_read(NULL)
,m_current_write(NULL)
,m_current_write_nr(0)
,m_read_ptr(0)
,m_write_ptr(0)
,m_b_read_caused_eof(false)
,m_ref_count(0)
,m_ref_decreased(false)
,m_path(f.Path())
{
	m_current_read = m_base;
	m_current_write = m_base;
	char slask[32768];
	size_t n;
	while ((n = f.fread(slask, 1, 32768)) > 0)
	{
		fwrite(slask, 1, n);
	}
}


MemFile::~MemFile()
{
	if (m_ref_count)
		std::cerr << "MemFile destructor with a ref count of " + Utility::l2string(m_ref_count) << std::endl;
	while (m_base && !m_src_valid)
	{
		block_t *p = m_base;
		m_base = p -> next;
		delete p;
	}
	if (m_src_valid && !m_ref_decreased)
	{
		m_src.Decrease();
		m_ref_decreased = true;
	}
}


bool MemFile::fopen(const std::string& path, const std::string& mode)
{
	return true;
}


void MemFile::fclose() const
{
	if (m_src_valid && !m_ref_decreased)
	{
		m_src.Decrease();
		m_ref_decreased = true;
	}
}


size_t MemFile::fread(char *ptr, size_t size, size_t nmemb) const
{
	size_t p = m_read_ptr % BLOCKSIZE;
	size_t sz = size * nmemb;
	size_t available = m_write_ptr - m_read_ptr;
	if (sz > available) // read beyond eof
	{
		sz = available;
		m_b_read_caused_eof = true;
	}
	if (!sz)
	{
		return 0;
	}
	if (p + sz < BLOCKSIZE)
	{
		memcpy(ptr, m_current_read -> data + p, sz);
		m_read_ptr += sz;
	}
	else
	{
		size_t sz1 = BLOCKSIZE - p;
		size_t sz2 = sz - sz1;
		memcpy(ptr, m_current_read -> data + p, sz1);
		m_read_ptr += sz1;
		while (sz2 > BLOCKSIZE)
		{
			if (m_current_read -> next)
			{
				m_current_read = m_current_read -> next;
				memcpy(ptr + sz1, m_current_read -> data, BLOCKSIZE);
				m_read_ptr += BLOCKSIZE;
				sz1 += BLOCKSIZE;
				sz2 -= BLOCKSIZE;
			}
			else
			{
				return sz1;
			}
		}
		if (m_current_read -> next)
		{
			m_current_read = m_current_read -> next;
			memcpy(ptr + sz1, m_current_read -> data, sz2);
			m_read_ptr += sz2;
		}
		else
		{
			return sz1;
		}
	}
	return sz;
}


size_t MemFile::fwrite(const char *ptr, size_t size, size_t nmemb)
{
	size_t p = m_write_ptr % BLOCKSIZE;
	int nr = (int)m_write_ptr / BLOCKSIZE;
	size_t sz = size * nmemb;
	if (m_current_write_nr < nr)
	{
		block_t *next = new block_t;
		m_current_write -> next = next;
		m_current_write = next;
		m_current_write_nr++;
	}
	if (p + sz <= BLOCKSIZE)
	{
		memcpy(m_current_write -> data + p, ptr, sz);
		m_write_ptr += sz;
	}
	else
	{
		size_t sz1 = BLOCKSIZE - p; // size left
		size_t sz2 = sz - sz1;
		memcpy(m_current_write -> data + p, ptr, sz1);
		m_write_ptr += sz1;
		while (sz2 > BLOCKSIZE)
		{
			if (m_current_write -> next)
			{
				m_current_write = m_current_write -> next;
				m_current_write_nr++;
			}
			else
			{
				block_t *next = new block_t;
				m_current_write -> next = next;
				m_current_write = next;
				m_current_write_nr++;
			}
			memcpy(m_current_write -> data, ptr + sz1, BLOCKSIZE);
			m_write_ptr += BLOCKSIZE;
			sz1 += BLOCKSIZE;
			sz2 -= BLOCKSIZE;
		}
		if (m_current_write -> next)
		{
			m_current_write = m_current_write -> next;
			m_current_write_nr++;
		}
		else
		{
			block_t *next = new block_t;
			m_current_write -> next = next;
			m_current_write = next;
			m_current_write_nr++;
		}
		memcpy(m_current_write -> data, ptr + sz1, sz2);
		m_write_ptr += sz2;
	}
	return sz;
}



char *MemFile::fgets(char *s, int size) const
{
	int n = 0;
	while (n < size - 1 && !eof())
	{
		char c;
		size_t sz = fread(&c, 1, 1);
		if (sz)
		{
			if (c == 10)
			{
				s[n] = 0;
				return s;
			}
			s[n++] = c;
		}
	}
	s[n] = 0;
	return s;
}


void MemFile::fprintf(const char *format, ...)
{
	va_list ap;
	char tmp[BLOCKSIZE];
	va_start(ap, format);
	vsnprintf(tmp, sizeof(tmp), format, ap);
	va_end(ap);
	fwrite(tmp, 1, strlen(tmp));
}


off_t MemFile::size() const
{
	return (off_t)m_write_ptr;
}


bool MemFile::eof() const
{
	return m_b_read_caused_eof; //(m_read_ptr < m_write_ptr) ? false : true;
}


void MemFile::reset_read() const
{
	m_read_ptr = 0;
	m_current_read = m_base;
}


void MemFile::reset_write()
{
	m_write_ptr = 0;
	m_current_write = m_base;
	m_current_write_nr = 0;
}


int MemFile::RefCount() const
{
	return m_ref_count;
}


void MemFile::Increase()
{
	++m_ref_count;
}


void MemFile::Decrease()
{
	--m_ref_count;
}


const std::string& MemFile::Path() const
{
	return m_path;
}


#ifdef SOCKETS_NAMESPACE
}
#endif


