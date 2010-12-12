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
// Mineserver trxlogger.cpp
//

#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <vector>

#include "trxlogger.h"
#include "logger.h"
#include "config.h"

TrxLogger::TrxLogger (std::string filename) {
  log_stream.open(filename.c_str(), std::fstream::in | std::fstream::out | std::fstream::binary );
  if (!log_stream.is_open()) {
    LOG("Problem opening binary log!");
  } 
}

TrxLogger &TrxLogger::get()
{
  static TrxLogger instance(Conf::get().sValue("binlog"));
  return instance;
}
 
// Log action to binary log 
void TrxLogger::log(event_t event)
{
  if(!log_stream.bad()) {
    event.timestamp = time (NULL);
    event.nsize = strlen(event.nick);

    log_stream.seekp(0, std::ios::end);
    log_stream.write((char *) &event.timestamp, sizeof(time_t));
    log_stream.write((char *) &event.x, sizeof(int));
    log_stream.write((char *) &event.y, sizeof(int));
    log_stream.write((char *) &event.z, sizeof(int));
    log_stream.write((char *) &event.otype, sizeof(uint8));
    log_stream.write((char *) &event.ntype, sizeof(uint8));
    log_stream.write((char *) &event.ometa, sizeof(uint8));
    log_stream.write((char *) &event.nmeta, sizeof(uint8));
    log_stream.write((char *) &event.nsize, sizeof(int));
    log_stream.write((char *) &event.nick, event.nsize+1);

  } else {
    LOG("Binary log is bad!");
  }
}
// Get logs based on nick and timestamp
bool TrxLogger::getLogs(time_t t, std::string &nick, std::vector<event_t> *logs) {
 
  std::vector<event_t> tmp;
  std::vector<event_t>::iterator event; 

  this->getLogs(&tmp);

  for(event = tmp.begin(); event != tmp.end(); event++) {
    if(event->timestamp > t && event->nick == nick.c_str()) {
      event_t e;

      e.nsize = event->nsize;
      strcpy(e.nick, event->nick);
      e.x     = event->x;
      e.y     = event->y;
      e.z     = event->z;
      e.otype = event->otype;
      e.ntype = event->ntype;
      e.ometa = event->ometa;
      e.nmeta = event->nmeta;

      logs->push_back(e);
    }
  }
  return true;
}

// Get logs based on timestamp
bool TrxLogger::getLogs(time_t t, std::vector<event_t> *logs) {

  std::vector<event_t> tmp;
  std::vector<event_t>::iterator event;

  this->getLogs(&tmp);

  for(event = tmp.begin(); event != tmp.end(); event++) {
    if(event->timestamp > t) {
      event_t e;

      e.nsize = event->nsize;
      strcpy(e.nick, event->nick);
      e.x     = event->x;
      e.y     = event->y;
      e.z     = event->z;
      e.otype = event->otype;
      e.ntype = event->ntype;
      e.ometa = event->ometa;
      e.nmeta = event->nmeta;

      logs->push_back(e);
    }
  }
  return true;
}

// Get all logs
bool TrxLogger::getLogs(std::vector<event_t> *logs) {
  event_t event;

  log_stream.flush();
  log_stream.seekg(0, std::ios::beg);

  while(log_stream.good()) {
                        
    log_stream.get((char *) &event.timestamp, sizeof(time_t));
    log_stream.get((char *) &event.x, sizeof(int));
    log_stream.get((char *) &event.y, sizeof(int));
    log_stream.get((char *) &event.z, sizeof(int));
    log_stream.get((char *) &event.otype, sizeof(uint8));
    log_stream.get((char *) &event.ntype, sizeof(uint8));
    log_stream.get((char *) &event.ometa, sizeof(uint8));
    log_stream.get((char *) &event.nmeta, sizeof(uint8));
    log_stream.get((char *) &event.nsize, sizeof(int));
    log_stream.get((char *) &event.nick, event.nsize+1);

    logs->push_back(event);
  }
  return true;
}

TrxLogger::~TrxLogger() {
  log_stream.close();
}
