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
#include <string.h>
#include <time.h>
#include <vector>

#include "../../src/plugin_api.h"
#include "binlog.h"
                                     

Binlog::Binlog (std::string filename) 
{
  log_stream.open(filename.c_str(), std::fstream::in | std::fstream::out | std::fstream::binary );
  if (!log_stream.is_open()) {
    return;
  } 
}

Binlog &Binlog::get(std::string filename)
{
  static Binlog instance(filename);
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

mineserver_pointer_struct* mineserver;
std::string pluginName = "binlog";
std::string filename;
bool enabled;

// Rollback Transaction Logs
void rollBack (const char* user, int argc, const char** args)
{
  std::vector<event_t> logs;                                                         
  time_t timestamp;

  if(argc > 0) {
    std::stringstream ss (std::stringstream::in | std::stringstream::out);
    ss << args[0];
    ss >> timestamp;
  }

  if(argc == 2 ) {
    std::string victim = args[1];
    Binlog::get(filename).getLogs(timestamp, victim, &logs);
  } else if (argc == 1) {
    Binlog::get(filename).getLogs(timestamp, &logs);
  } else {
    Binlog::get(filename).getLogs(&logs);
  }

  std::vector<event_t>::reverse_iterator event;
  if(logs.size() > 0) {
    mineserver->chat.sendmsgTo(user, "Rolling back map...");
    for(event = logs.rbegin(); event != logs.rend(); event++) {
      mineserver->map.setBlock(event->x, event->y, event->z, event->otype, event->ometa);
    }
    mineserver->chat.sendmsgTo(user, "Map roll back completed!");
  } else {
    mineserver->chat.sendmsgTo(user, "No binary logs found!");
  }
}

// Playback Transaction Logs
void playBack (const char* user, int argc, const char** args)
{
  return;
  std::vector<event_t> logs;                                                         
  time_t timestamp;

  if(argc > 0) {
    std::stringstream ss (std::stringstream::in | std::stringstream::out);
    ss << args[0];
    ss >> timestamp;
  }

  if(argc == 2 ) {
    std::string victim = args[1];
    Binlog::get(filename).getLogs(timestamp, victim, &logs);
  } else if (argc == 1) {
    Binlog::get(filename).getLogs(timestamp, &logs);
  } else {
    Binlog::get(filename).getLogs(&logs);
  }

  std::vector<event_t>::reverse_iterator event;
  if(logs.size() > 0) {
    mineserver->chat.sendmsgTo(user, "Playing back binary log...");
    for(event = logs.rbegin(); event != logs.rend(); event++) {
      mineserver->map.setBlock(event->x, event->y, event->z, event->otype, event->ometa);
    }
    mineserver->chat.sendmsgTo(user, "Binary log playback completed!");
  } else {
    mineserver->chat.sendmsgTo(user, "No binary logs found!");
  }
}

// Block Break Callback
bool callbackBlockBreakPre (const char* user,int x,int y,int z) 
{
  event_t event;
  strcpy(event.nick, user);
  event.x = x;
  event.y = y;
  event.z = z;
  event.ntype = 0;
  event.nmeta = 0;

  mineserver->map.getBlock(x,y,z,&event.otype, &event.ometa);
  Binlog::get(filename).log(event);

  return true;
}
// Direction
enum Direction
{
   BLOCK_BOTTOM, BLOCK_NORTH, BLOCK_SOUTH, BLOCK_EAST, BLOCK_WEST, BLOCK_TOP
};

bool translateDirection(int32_t *x, int8_t *y, int32_t *z, int8_t direction)
{
    switch(direction)
    {
      case BLOCK_BOTTOM: (*y)--;  break;
      case BLOCK_TOP:    (*y)++;  break;
      case BLOCK_NORTH:  (*x)++;  break;
      case BLOCK_SOUTH:  (*x)--;  break;
      case BLOCK_EAST:   (*z)++;  break;
      case BLOCK_WEST:   (*z)--;  break;
      default:                    break;
    }
  return true;
}

// Block Place Callback
bool callbackBlockPlacePre (const char* user,int32_t x,int8_t y,int32_t z, unsigned char type, unsigned char meta) 
{
  event_t event;
  translateDirection(&x,&y,&z,meta);

  strcpy(event.nick, user);
  event.x = x;
  event.y = y;
  event.z = z;
  event.ntype = type;
  event.nmeta = meta;
  
  mineserver->map.getBlock(x,y,z,&event.otype, &event.ometa);
  Binlog::get(filename).log(event);

  return true;
}

// Command Registration
bool callbackPlayerChatCommand (const char* user, const char* command, int argc, const char** args) 
{
  if(strcmp(command, "rollback") == 0) {
    rollBack(user, argc, args);
    return true;
  } else if (strcmp(command, "playback") == 0) {
    playBack(user, argc, args);
    return true;
  }
  return false;
}

std::string dtos( double n )
{
  std::ostringstream result;
  result << n;
  return result.str();
}

#define LOG_INFO 6
PLUGIN_API_EXPORT void CALLCONVERSION binlog_init(mineserver_pointer_struct* mineserver_temp)
{
  mineserver = mineserver_temp;
  enabled = mineserver->config.bData("enable_binary_logging");
  filename = mineserver->config.sData("binary_log");

  if (mineserver->plugin.getPluginVersion("binlog") > 0)
  {
    std::string msg = "binlog is already loaded v."+dtos(mineserver->plugin.getPluginVersion(pluginName.c_str()));
    mineserver->logger.log(LOG_INFO, "plugin.binlog", msg.c_str());
    return;
  }
  std::string msg = "Loaded "+pluginName+"!";
  mineserver->logger.log(LOG_INFO, "plugin.binlog", msg.c_str());
  mineserver->plugin.setPluginVersion("binlog", PLUGIN_VERSION);
  if(enabled) 
  {
    mineserver->plugin.addCallback("BlockPlacePre", (void *) callbackBlockPlacePre);
    mineserver->plugin.addCallback("BlockBreakPre", (void *) callbackBlockBreakPre);
  }
  mineserver->plugin.addCallback("PlayerChatCommand", (void *) callbackPlayerChatCommand);
}

PLUGIN_API_EXPORT void CALLCONVERSION binlog_shutdown(void)
{
  if (mineserver->plugin.getPluginVersion("binlog") <= 0)
  {
    mineserver->logger.log(LOG_INFO, "plugin.binlog", "binlog is not loaded!");
    return;
  }
}
