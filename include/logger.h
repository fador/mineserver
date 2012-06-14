/*
   Copyright (c) 2011, The Mineserver Project
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

#ifndef _LOGGER_H
#define _LOGGER_H

//
// Mineserver logger.h
//
#include <string>

#include "logtype.h"
#include "tools.h"

#define LOGLF(msg) ServerInstance->logger()->log(msg, std::string(__FILE__), __LINE__)

#define LOG(type, source, msg) ServerInstance->logger()->log(LogType::LOG_##type, source, msg)


// TODO: winex: log(type, file, line, msg) might be better than this
#define LOG_FILENAME std::string(__FILE__).substr(std::string(__FILE__).rfind(PATH_SEPARATOR) + 1)
#ifdef DEBUG
#define LOG_FORMAT   LOG_FILENAME + ":" + dtos(__LINE__) + "::" + std::string(__FUNCTION__) + "()"
#else
#define LOG_FORMAT   LOG_FILENAME + "::" + std::string(__FUNCTION__) + "()"
#endif

#define LOG2(type, msg) ServerInstance->logger()->log(LogType::LOG_##type, LOG_FORMAT, msg)


class Logger
{
public:
  void log(const std::string& message, const std::string& file, int line);
  void log(LogType::LogType type, const std::string& source, const std::string& message);
  void log(LogType::LogType type, const std::string& source, const char* message, ...);

};

#endif
