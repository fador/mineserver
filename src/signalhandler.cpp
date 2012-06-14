/*
  Copyright (c) 2012, The Mineserver Project
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

#include "signalhandler.h"
#include "mineserver.h"
#include "constants.h"
#include "logger.h"
#include <cstdio>
#include <sstream>
#include <iostream>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <signal.h>

#ifndef SIGBREAK
# define SIGBREAK 0 // Define this for unix systems
#endif
char segv_location[255];

void SignalHandler(int);

void InitSignals()
{
  signal(SIGTERM, SignalHandler); // Justasic: Terminate signal sent by kill
  #ifndef WIN32
  signal(SIGKILL, SignalHandler); // Justasic: Kill signal sent by kill or kernel i think
  signal(SIGHUP, SignalHandler);  // Justasic: Signal Hangup, typically used as a Config Rehash
  #endif
  signal(SIGINT, SignalHandler);  // Justasic: Signal Interrupt, usually a CTRL+C at console
  signal(SIGBREAK, SignalHandler); // Justasic: ?? is this like sigterm?
  #ifdef HAVE_BACKTRACE
  signal(SIGSEGV, SignalHandler); // Justasic: Segmentation Fault signal, used for generating segfault reports
  #endif
}

void HandleSegfault()
{
  // Justasic: We only use this statement if we have the capability of outputting a backtrace >.<
  // Justasic: import some crap from another project i did this to.
  #ifdef HAVE_BACKTRACE
  void *array[10];
  char **strings;
  char tbuf[256];
  size_t size;
  time_t now = time(NULL);
  size = backtrace(array, 10);
  
  #ifdef HAVE_SYS_UTSNAME_H
  struct utsname uts;
  if(uname(&uts) < 0)
    std::cerr << "uname() error: " << strerror(errno) << std::endl;
  #endif

  strftime(tbuf, sizeof(tbuf), "[%b %d %H:%M:%S %Y]", localtime(&now));
  std::stringstream slog;
  slog << "====================== Segmentation Fault ======================" << std::endl;
  slog << "Please note that the Mineserver developers may ask you to re-run this under gdb!" << std::endl;
  slog << "Time of crash: " << tbuf << std::endl;
  slog << "Mineserver version: " << VERSION_COMPLETE << std::endl;
  #ifdef HAVE_SYS_UTSNAME_H
  slog << "System info: " << uts.sysname << " " << uts.nodename << " " <<  uts.release << " " << uts.machine << std::endl;
  slog << "System version: " << uts.version << std::endl;
  #endif
  slog << "C++ Version: " << __VERSION__ << std::endl; // GCC only? hmmnn
  //slog << "Socket Buffer: " << LastBuf << std::endl; // Use this for the last thing sent/received by the socket.
  slog << "Location: " << segv_location << std::endl;
  strings = backtrace_symbols(array, size);

  for(unsigned i=1; i < size; i++)
    slog << "BackTrace(" << (i - 1) << "): " << strings[i] << std::endl;

  free(strings);
  slog << "======================== END OF REPORT ==========================" << std::endl;

  // FIXME: Maybe log to a file somewhere instead of crashing?
  std::cout << slog.str(); //Write to terminal.
  std::cout.flush(); //Clear output
  #endif // HAVE_BACKTRACE
  
  ServerInstance->stop();
  exit(SIGSEGV); // Exit so we're not still running
}

// TODO: On signal event: close active connections, save world, etc.
void SignalHandler(int sig)
{
  switch(sig)
  {

#ifdef HAVE_BACKTRACE
    case SIGSEGV:
      // Justasic: You can stop SIGSEGV's but I HIGHLY recommend against it unless you have good reason to.
      // Justasic: as that can cause stack corruption and all kinds of bad things. if you do, use setjmp >.>
      HandleSegfault();
      break;
#endif
    #ifndef WIN32
    case SIGHUP:
      signal(sig, SIG_IGN);
      /* TODO: Rehash a config? hmmnn */
      LOG2(INFO, "Received SIGHUP, rehashing..");
      break;
    case SIGPIPE:
      // Justasic: Ignore sigpipe since it just gets in the way.
      signal(sig, SIG_IGN);
      break;
    case SIGKILL:
    #endif
    case SIGINT:    
    case SIGBREAK:
    case SIGTERM:
      signal(sig, SIG_IGN);
      #ifndef WIN32
      signal(SIGHUP, SIG_IGN);
      #endif
      LOG2(INFO, "Received SIGTERM, Exiting..");
      ServerInstance->stop();
      break;
    default:
      static_cast<void>(0); // TODO: fix this to do something useful?
  }
}