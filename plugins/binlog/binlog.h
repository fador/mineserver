/*
   Copyright (c) 2010, The Mineserver Project
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

//
// Mineserver trxlogger.h
//

#include <string>
#include <fstream>
#include <time.h>

#include "plugin_api.h"

#ifndef _BINLOG_H
#define _BINLOG_H
#define PLUGIN_VERSION 0.1
#endif

struct event_t {
  time_t timestamp;
  int x;
  int y;
  int z;
  unsigned char otype, ntype;
  unsigned char ometa, nmeta;
  int nsize;
  char nick[17];
};

class Binlog 
{

public:
  void log(event_t event);
  static Binlog &get(std::string filename);
  bool getLogs(time_t t, std::string &nick, std::vector<event_t> *logs);
  bool getLogs(time_t t, std::vector<event_t> *logs);
  bool getLogs(std::vector<event_t> *logs);
  bool getEvent(event_t *event);

private:
  std::fstream log_stream;
  Binlog(std::string filename);
  ~Binlog();
};
