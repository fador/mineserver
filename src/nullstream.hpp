//
// Copyright (c) 2006 - 2007
// Seweryn Habdank-Wojewodzki
//
// Distributed under the Boost Software License, Version 1.0.
// ( copy at http://www.boost.org/LICENSE_1_0.txt )


//
// Copyright Maciej Sobczak, 2002
//
// Permission to copy, use, modify, sell and distribute this software
// is granted provided this copyright notice appears in all copies.
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//

#ifndef NULLSTREAM_HPP_INCLUDED
#define NULLSTREAM_HPP_INCLUDED 1

#include <streambuf>
#include <ostream>

// null stream buffer class
template <class charT, class traits = ::std::char_traits<charT> >
class null_buffer : public ::std::basic_streambuf<charT, traits>
{
public:
	typedef typename ::std::basic_streambuf<charT, traits>::int_type int_type;

	null_buffer() {}

private:
	virtual int_type overflow(int_type c)
	{
		// just ignore the character
		return traits::not_eof(c);
	}
};

// generic null output stream class
template <class charT, class traits = ::std::char_traits<charT> >
class basic_null_stream
	: private null_buffer<charT, traits>,
	public ::std::basic_ostream<charT, traits>
{
public:
	basic_null_stream()
		: ::std::basic_ostream<charT, traits>(this)
	{
	}
};

// helper declarations for narrow and wide streams
typedef basic_null_stream<char> null_stream;
typedef basic_null_stream<wchar_t> null_wstream;

#endif // NULLSTREAM_HPP_INCLUDED
