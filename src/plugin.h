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

#pragma once

#include <string>
#include <map>
#include <vector>
#include <stdio.h>

#include "delegate/delegate.hpp"
#include "constants.h"
#include "tools.h"
#include "user.h"
#include "hook.h"

typedef srutil::delegate6<void, User*, sint8, sint32, sint8, sint32, sint8> Function;

class Callback
{
public:
  void add(const std::string name, Function func)
  {
    remove(name);
    callbacks.insert(std::pair<std::string, Function>(name, func));
  }
   
  bool remove(const std::string name)
  {
    Events::iterator iter = callbacks.find(name);

    if (iter == callbacks.end())
    {
      return false;
    }
      
    callbacks.erase(iter);
    return true;
  }

  Function* get(const std::string name)
  {
    Events::iterator iter = callbacks.find(name);

    if (iter == callbacks.end())
    {
      return NULL;
    }

    return &iter->second;
  }
   
  bool run(const std::string name, const Function::invoker_type function)
  {
    Events::iterator iter = callbacks.find(name);

    if (iter == callbacks.end())
    {
      return false;
    }

    function(iter->second);
    return true;
  }

  void reset()
  {
    callbacks.clear();
  }

private:
  typedef std::map<std::string, Function > Events;
  Events callbacks;
};

class Plugin
{
public:
  static Plugin* get()
  {
    if (!m_plugin)
    {
      m_plugin = new Plugin;
    }
    return m_plugin;
  }

  void init();
  void free();

  void setBlockCallback(const int type, Callback call);
  Callback* getBlockCallback(const int type);
  bool runBlockCallback(const int type, const std::string name, const Function::invoker_type function);
  bool removeBlockCallback(const int type);

  typedef functor3<bool,std::string,User*,std::string> typeChatRecv;
  Hook<typeChatRecv>* hookChatRecv;
  typedef typeChatRecv::ftype funcChatRecv;
  typedef typeChatRecv::atype argsChatRecv;

private:
  Plugin()
  {
    hookChatRecv = new Hook<typeChatRecv>;
  }

  typedef std::map<sint16, Callback> Callbacks;
  Callbacks blockevents;
  static Plugin* m_plugin;
};

