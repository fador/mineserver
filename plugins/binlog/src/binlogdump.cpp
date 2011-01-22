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
// Mineserver binlogdump.cpp
//

#include <vector>
#include <stdio.h>

#include "binlog.h"

int main (int argc, const char* argv[] ) 
{
  if(argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return 1;
  }
  std::vector<event_t> logs;
  Binlog::get(argv[1]).getLogs(&logs);
  std::vector<event_t>::iterator event;
  for(event = logs.begin(); event != logs.end(); event++) 
  {
    printf("{timestamp:%d, nick:%s, x:%i, y:%i, z:%i, old_type:%#x, old_meta:%#x, new_type:%#x, new_meta:%#x}\n", 
      event->timestamp, event->nick, event->x, event->y, event->z, 
      (int) event->otype, (int) event->ometa, (int) event->ntype, (int) event->nmeta );
  }
  return 0;
}
