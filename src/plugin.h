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

class User;

typedef srutil::delegate6<void, User*, sint8, sint32, sint8, sint32, sint8> Function;

class Callback
{
public:
   void add(std::string name, Function func)
   {
      remove(name);
      callbacks.insert(std::pair<std::string, Function>(name, func));
   }
   
   bool remove(std::string name)
   {
      for (Events::iterator iter = callbacks.begin(); iter != callbacks.end(); ++iter)
      {
         if ((*iter).first == name)
         {
              callbacks.erase(iter);
              return true;
         }
      }
      return false;
   }

   Function get(std::string name)
   {
      for (Events::iterator iter = callbacks.begin(); iter != callbacks.end(); iter++)
      {
         if ((*iter).first == name)
            return iter->second;
      }

      Function empty;
      return empty;
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
private:
   Plugin()
   {
   }
   typedef std::map<int, Callback> Callbacks;
   Callbacks blockevents;
   static Plugin *mPlugin;
public:
   void init();
   void free();
   void setBlockCallback(const int type, Callback call);
   Callback getBlockCallback(const int type);
   bool removeBlockCallback(const int type);
   static Plugin* get()
   {
      if(!mPlugin) {
         mPlugin = new Plugin();
      }
      return mPlugin;
   }
};

