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

#ifndef _CONSTANTS_H
#define _CONSTANTS_H

#include <map>
#include <string>
#include <stdint.h>
#include <iostream>

#include "tr1.h"
#include TR1INCLUDE(memory)

// configuration from build system
#include "configure.h"
#include "constants_num.h"
#include "extern.h"

//
// Drops from blocks
//
struct Drop;
typedef std::tr1::shared_ptr<Drop> DropPtr;

struct Drop
{
  uint16_t item_id;
  uint32_t probability;
  uint8_t count;
  int16_t meta;
  DropPtr alt_drop;

  explicit Drop(uint16_t _item_id = 0, uint32_t _probability = 0, uint8_t _count = 0, int16_t _meta = -1, DropPtr _alt_drop = DropPtr())
    :
    item_id(_item_id),
    probability(_probability),
    count(_count),
    meta(_meta),
    alt_drop(_alt_drop)
  {
  }

  void getDrop(int16_t& item, uint8_t& count, uint8_t& meta);
};

extern void initConstants();

extern std::map<uint8_t, DropPtr> BLOCKDROPS;

// Chat prefixes
enum
{
  SERVERMSGPREFIX = '%',
  CHATCMDPREFIX   = '/',
  ADMINCHATPREFIX = '&'
};

extern const unsigned int SERVER_CONSOLE_UID;

extern const std::string VERSION;

extern const int PROTOCOL_VERSION;

extern const char COMMENTPREFIX;

// Configuration
extern const std::string CONFIG_FILE;

// PID file
extern const std::string PID_FILE;

//allocate 1 MB for chunk files
extern const int ALLOCATE_NBTFILE;

extern const int kMaxChatMessageLength;

#endif
