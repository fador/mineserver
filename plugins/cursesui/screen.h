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

#ifndef _CURSESSCREEN_H
#define _CURSESSCREEN_H

#ifdef WIN32
  #pragma warning( disable: 4005)
  #include <winsock2.h>
  #include <curses.h>
#else
  #include <ncurses.h>
#endif

#include <string>
#include <vector>

#include "../../src/logtype.h"
enum
{
  TEXT_COLOR_RED = 1,
  TEXT_COLOR_GREEN,
  TEXT_COLOR_YELLOW,
  TEXT_COLOR_BLUE,
  TEXT_COLOR_MAGENTA,
  TEXT_COLOR_CYAN,
  TEXT_COLOR_WHITE,
  TEXT_COLOR_INVERSE
};

#define COMMAND_HISTORY_SIZE 40

class CursesScreen
{
public:
  void init(std::string version);
  WINDOW* createWindow(int width, int height, int startx, int starty);
  void destroyWindow(WINDOW *local_win);
  void log(LogType::LogType type, const std::string& source, const std::string& message);
  void updatePlayerList(bool joining, const char *username);
  void end();
  WINDOW *commandLog;
  bool hasCommand();
  std::string getCommand();

private:
  WINDOW *title;
  WINDOW *generalLog;
  WINDOW *chatLog;
  WINDOW *playerList;

  std::vector<std::string> usernames;

  unsigned int commandX;
  int currentCommandHistoryIndex;
  int nextCommandHistoryIndex;

  std::string currentCommand;
  std::string commandHistory[COMMAND_HISTORY_SIZE];

  static const int commandHistorySize = COMMAND_HISTORY_SIZE;
};

#undef COMMAND_HISTORY_SIZE

#endif /* _CURSESSCREEN_H */
