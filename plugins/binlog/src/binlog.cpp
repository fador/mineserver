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
// Mineserver binlog.cpp
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>

#include "../../../src/plugin_api.h"
#include "binlog.h"

std::string dtos( double n )
{
  std::ostringstream result;
  result << n;
  return result.str();
}

mineserver_pointer_struct* mineserver;
std::string pluginName = "binlog";

Binlog::Binlog (std::string filename) 
{
  log_stream.open(filename.c_str(), std::fstream::in | std::fstream::out | std::fstream::binary );
  if (!log_stream.is_open()) {
    mineserver->screen.log("Problem opening binary log!");
  } 
}

Binlog &Binlog::get()
{
  static Binlog instance(Conf::get()->sValue("binary_log"));
  return instance;
}
 
// Log action to binary log 
void Binlog::log (event_t event)
{
  if(!log_stream.bad()) {
    event.timestamp = time (NULL);
    event.nsize = strlen(event.nick);

    log_stream.seekp(0, std::ios::end);
    log_stream.write((char *) &event.timestamp, sizeof(time_t));
    log_stream.write((char *) &event.x, sizeof(int));
    log_stream.write((char *) &event.y, sizeof(int));
    log_stream.write((char *) &event.z, sizeof(int));
    log_stream.write((char *) &event.otype, sizeof(unsigned char));
    log_stream.write((char *) &event.ntype, sizeof(unsigned char));
    log_stream.write((char *) &event.ometa, sizeof(unsigned char));
    log_stream.write((char *) &event.nmeta, sizeof(unsigned char));
    log_stream.write((char *) &event.nsize, sizeof(int));
    log_stream.write((char *) &event.nick, event.nsize+1);
  } else {
    mineserver->screen.log("Binary log stream is bad!");
  } 
}

// Get logs based on nick and timestamp
bool Binlog::getLogs (time_t t, std::string &nick, std::vector<event_t> *logs) 
{
  event_t event; 

  log_stream.flush();
  log_stream.seekg(0, std::ios::beg);
  while(this->getEvent(&event)) {
    if(event.timestamp > t && (strcmp(event.nick, nick.c_str()) == 0)) {
      logs->push_back(event);
    }
  }
  return true;
}

// Get logs based on timestamp
bool Binlog::getLogs (time_t t, std::vector<event_t> *logs) 
{
  event_t event;

  log_stream.flush();
  log_stream.seekg(0, std::ios::beg);
  while(this->getEvent(&event)) {
    if(event.timestamp > t) {
      logs->push_back(event);
    }
  }
  return true;
}

// Get all logs
bool Binlog::getLogs (std::vector<event_t> *logs) 
{
  event_t event;

  log_stream.flush();
  log_stream.seekg(0, std::ios::beg);
  while(this->getEvent(&event)) {
    logs->push_back(event);
  }
  return true;
}

// Get event from log
bool Binlog::getEvent (event_t *event) 
{
  if(!log_stream.eof()) {
    log_stream.read((char *) &event->timestamp, sizeof(time_t));
    log_stream.read((char *) &event->x, sizeof(int));
    log_stream.read((char *) &event->y, sizeof(int));
    log_stream.read((char *) &event->z, sizeof(int));
    log_stream.read((char *) &event->otype, sizeof(unsigned char));
    log_stream.read((char *) &event->ntype, sizeof(unsigned char));
    log_stream.read((char *) &event->ometa, sizeof(unsigned char));
    log_stream.read((char *) &event->nmeta, sizeof(unsigned char));
    log_stream.read((char *) &event->nsize, sizeof(int));
    log_stream.read((char *) &event->nick, event->nsize+1);
    return true;
  }
  log_stream.clear(); 
  return false;
}

Binlog::~Binlog() 
{
  log_stream.close();
}

// Rollback Transaction Logs
void rollBack (User *user, std::string command, std::deque<std::string> args)
{
  std::vector<event_t> logs;
  time_t timestamp;

  if(args.size() > 0) {
    std::stringstream ss (std::stringstream::in | std::stringstream::out);
    ss << args[0];
    ss >> timestamp;
  }

  if(args.size() == 2 ) {
    std::string victim = args[1];
    Binlog::get().getLogs(timestamp, victim, &logs);
  } else if (args.size() == 1) {
    Binlog::get().getLogs(timestamp, &logs);
  } else {
    Binlog::get().getLogs(&logs);
  }

  std::vector<event_t>::reverse_iterator event;
  if(logs.size() > 0) {
    mineserver->chat.sendMsg(user, "Rolling back map...", Chat::USER);
    for(event = logs.rbegin(); event != logs.rend(); event++) {
      mineserver->map.setBlock(event->x, event->y, event->z, event->otype, event->ometa);
      mineserver->map.sendBlockChange(event->x, event->y, event->z, (char)event->otype, event->ometa);
    }
    mineserver->chat.sendMsg(user, "Map roll back completed!", Chat::USER);
  } else {
    mineserver->chat.sendMsg(user, "No binary logs found!");
  }
}

// Playback Transaction Logs
void playBack (User *user, std::string command, std::deque<std::string> args)
{
  return;
}

// Block Break Callback
bool callbackBlockBreakPre (User* user,sint32 x,sint8 y,sint32 z) 
{
  event_t event;
  event.nick = user;
  event.x = x;
  event.y = y;
  event.z = z;
  event.ntype = 0;
  event.nmeta = 0;

  minserver->map.getBlock(x,y,z,&event.otype, &event.ometa);
  BINLOG(event);

  return true;
}
// Block Place Callback
bool callbackBlockPlacePre (User* user,sint32 x,sint8 y,sint32 z, unsigned char type, unsigned char meta) 
{
  event_t event;
  event.nick = user;
  event.x = x;
  event.y = y;
  event.z = z;
  event.otype = 0;
  event.ometa = 0;
  event.ntype = type;
  event.nmeta = meta;

  BINLOG(event);

  return true;
}
// Command Registration
bool callbackPlayerChatPre (const char* user, size_t timestamp, const char* msg) 
{
  return true;
}

PLUGIN_API_EXPORT void CALLCONVERSION binlog_init(mineserver_pointer_struct* mineserver_temp)
{
  mineserver = mineserver_temp;
  if (mineserver->plugin.getPluginVersion("binlog") > 0)
  {
    mineserver->screen.log("binlog is already loaded v." +dtos(mineserver->plugin.getPluginVersion("binlog")));
    return;
  }
  mineserver->screen.log("Loaded \"binlog\"!");
  mineserver->plugin.setPluginVersion("binlog", PLUGIN_VERSION);
  mineserver->callback.add_hook("BlockPlacePre", (void *) callbackBlockPlacePre);
  mineserver->callback.add_hook("BlockBreakPre", (void *) callbackBlockBreakPre);
  mineserver->callback.add_hook("PlayerChatPre", (void *) callbackPlayerChatPre);
}

PLUGIN_API_EXPORT void CALLCONVERSION binlog_shutdown(void)
{
  if (mineserver->plugin.getPluginVersion("binlog") <= 0)
  {
    mineserver->screen.log("binlog is not loaded!");
    return;
  }
}
