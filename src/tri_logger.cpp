//
// Copyright (c) 2005 - 2007
// Seweryn Habdank-Wojewodzki
//
// Distributed under the Boost Software License, Version 1.0.
// ( copy at http://www.boost.org/LICENSE_1_0.txt )

#include "tri_logger.hpp"

#if !defined(CLEANTLOG)

#if defined (FTLOG)
#include <fstream>

#else
#include <iostream>
#include "nullstream.hpp"
#endif

namespace trivial_logger
{

    tri_logger_t::tri_logger_t()
    {}

    tri_logger_t::~tri_logger_t()
    {}

    bool tri_logger_t::is_activated()
    {
        return tri_logger_t::is_activated_;
    }

    void tri_logger_t::activate ( bool const activate )
    {
        is_activated_ = activate;
    }

    ::std::ostream *& tri_logger_t::ostream_ptr()
    {
        return outstream_;
    }

    //void tri_logger_t::set_out_stream ( 
    //    ::std::auto_ptr<::std::ostream > ostream_ptr )
    //{
        //outstream_helper_ptr
    //}

    //void tri_logger_t::set_out_stream ( ::std::ostream * & ostream )
    //{
    //    outstream_helper_ptr.release();
    //    outstream = ostream;
    //}

    //void tri_logger_t::set_out_stream ( ::std::ostream * const ostream )
    //{
    //    outstream_helper_ptr.release();
    //    outstream = ostream;
    //}

    //void tri_logger_t::set_out_stream ( ::std::ostream & ostream )
    //{
        //outstream_helper_ptr_ = ::std::auto_ptr<::std::ostream>( new null_stream );
    //    outstream_ = &ostream;
    //}

    // activate logger by default
    bool tri_logger_t::is_activated_ = true;

    #if defined(TLOG)
    // set auto pointer to the null stream
    // reason: ::std::cout can not be created in runtime, so
    // the auto pointer has nothing to do with its resources
    ::std::auto_ptr < ::std::ostream > 
        tri_logger_t::outstream_helper_ptr_
        = ::std::auto_ptr < ::std::ostream > ( new null_stream );
    ::std::ostream * tri_logger_t::outstream_ = &::std::cout;

    #elif defined (ETLOG)
    // look to the TLOG comments
    ::std::auto_ptr < ::std::ostream >
        tri_logger_t::outstream_helper_ptr_
        = ::std::auto_ptr < ::std::ostream > ( new null_stream );
    ::std::ostream * tri_logger_t::outstream_ = &::std::cerr;

    #elif defined (FTLOG)

    #include <cctype>

    #define XSTR(s) STR(s)
    #define STR(s) #s

    #define MAX(x,y) ( (x) < (y) ? (y) : (x) )

        namespace detail
        {
            /** 
            * Function paste rhs C string to the lhs C string.
            lhs should be long enough for that operation.
            Additionally coping is stared from the point which
            points lhs.
            */
            template < typename Char_type >
            size_t const str_cat ( Char_type *& lhs, Char_type const * rhs )
            {
                size_t len = 0;
                while ( *rhs != '\0' )
                {
                    *lhs = *rhs;
                    ++rhs;
                    ++lhs;
                    ++len;
                }
                return len;
            }
            
            /// Function calculates length of C string
            /// It can be used with wide characters
            template < typename Char_type >
            size_t const str_len ( Char_type const * str )
            {
                size_t len = 0;
                while ( *str != '\0' )
                {
                    ++str;
                    ++len;
                }
                return len;
            }
            
            /** 
            * Function copy rhs C string in to the lhs.
            * It do not check size of target C string
            * It starts to copy from the beginning of the C string,
            * but it begins put characters at the point where lhs points,
            * so there can be a problem when lhs points on the end of lhs
            * C string.
            */
            template < typename Char_type >
            size_t const str_cpy ( Char_type *& lhs, Char_type const * rhs )
            {
                size_t len = 0;
                while ( *rhs != '\0' )
                {
                    *lhs = *rhs;
                    ++rhs;
                    ++lhs;
                    ++len;
                }
                *lhs = '\0';
                return len+1;
            }

            /**
            * Function converts existing file name to the file name
            * which has no non-printable signs and 
            * at the end is added extension.
            * The space sign in file name is converted to the underscore.
            * Lengths of C strings has to be proper.
            */
            template < typename Char_type >
            size_t const 
            createfilename ( Char_type * result, 
                            Char_type const * filename,
                            Char_type const * ext,
                            Char_type const * helper )
            {
                size_t len = 0; 
                if ( str_len ( filename ) > 1 )
                {
                    while ( *filename != '\0' )
                    {
                        // check if characters have grapnical
                        // reprasentation
                        if ( 0 != ::std::isgraph ( *filename ) )
                        {
                            *result = *filename;
                            ++result;
                            ++len;
                        }
                        else
                        {
                            // convert space to underscore
                            if ( *filename == ' ' )
                            {
                                *result = '_';
                                ++result;
                                ++len;
                            }
                        }
                        ++filename;
                    }
                    // add extension
                    str_cat ( result, ext ); 
                    *result = '\0';
                }
                else
                {
                    result = &result[0];
                    len = str_cpy ( result, helper );
                }
                return len;
            }
        
            template < typename T >
            T const max ( T const x, T const y )
            {
                return ( x < y ? y : x );
            }

        } // namespace detail

        typedef char char_type;

        // convert definition of the TLOGFILE
        // to the C string
        char_type const tlogfilename[] = XSTR(FTLOG);
        
        // extension C string
        char_type const ext[] = ".log";

        char_type const helper_name[] = "_logger.log";
        
        // container for final file name
        char_type filename[(MAX(sizeof(tlogfilename),sizeof(helper_name))+sizeof(ext))/sizeof(char_type)];
        // create file name
        size_t const len = detail::createfilename ( filename, tlogfilename, ext, helper_name );

    #undef MAX
    #undef STR
    #undef XSTR

    // new file is opened and its destruction is managed by auto_ptr
    ::std::auto_ptr < ::std::ostream >
        tri_logger_t::outstream_helper_ptr_
        = ::std::auto_ptr < ::std::ostream > 
            ( new ::std::ofstream ( filename ));
    // set pointer output stream
    ::std::ostream * tri_logger_t::outstream_ = outstream_helper_ptr_.get();

    // here is a place for user defined output stream and flag

    // ...

    #else
    ::std::auto_ptr < ::std::ostream >
        tri_logger_t::outstream_helper_ptr_ 
        = ::std::auto_ptr < ::std::ostream > ( new null_stream );
    ::std::ostream * tri_logger_t::outstream_ = outstream_helper_ptr_.get();

    #endif

    ::std::auto_ptr<tri_logger_t> detail::tri_logger_out_ptr ( new tri_logger_t() );

    tri_logger_t& tri_logger()
    {
        //static tri_logger_t * ans = new tri_logger_t();
        return *detail::tri_logger_out_ptr;
        //return *ans;
    }

} // namespace trivial_logger

#endif // !CLEANTLOG

