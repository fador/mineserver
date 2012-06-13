/*
  Copyright (c) 2011-2012, The Mineserver Project
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of the The Mineserver Project nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef SYSTEM_EXCEPTIONS_H
#define SYSTEM_EXCEPTIONS_H

#include <string>
#include <exception>

class CoreException : public std::exception
{
 public:
	// Holds the error message to be displayed
	const std::string err;
	// Source of the exception
	const std::string source;
	// Default constructor, just uses the error mesage 'Core threw an exception'.
	CoreException() : err("Core threw an exception"), source("The core") { }
	// This constructor can be used to specify an error message before throwing.
	CoreException(const std::string &message) : err(message), source("The core") { }
	// This constructor can be used to specify an error message before throwing,
	// and to specify the source of the exception.
	CoreException(const std::string &message, const std::string &src) : err(message), source(src) { }
	// This destructor solves world hunger, cancels the world debt, and causes the world to end.
	// Actually no, it does nothing. Never mind.
	virtual ~CoreException() throw() { };
	// Returns the reason for the exception.
	// The module should probably put something informative here as the user will see this upon failure.
	virtual const char* GetReason() const
	{
	  return err.c_str();
	}

	virtual const char* GetSource()
	{
	  return source.c_str();
	}
};
#endif