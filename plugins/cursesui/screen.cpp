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

#include "screen.h"

#include "../../src/logtype.h"

#define MINESERVER_C_API
#include "../../src/plugin_api.h"

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <stack>

// Constants
#define PLUGIN_NAME "cursesui"
const char* pluginName = PLUGIN_NAME;
const char* logSource = "plugin.screen";
const float pluginVersion = 1.0f;

enum 
{
  LOG_TITLE,
  LOG_COMMAND,
  LOG_GENERAL,
  LOG_PLAYERS
};

// Variables
mineserver_pointer_struct* mineserver;
CursesScreen *screen;

std::string currentTimestamp(bool seconds)
{
  time_t currentTime = time(NULL);
  struct tm *Tm  = localtime(&currentTime);
  std::string timeStamp (asctime(Tm));
  timeStamp = timeStamp.substr(11, seconds ? 8 : 5);

  return timeStamp;
}

void CursesScreen::initWindows()
{
  // Work out our dimensions
  int titleHeight = 5;
  int logBar = titleHeight - 1;
  int commandHeight = 1;
  int commandBar = LINES - commandHeight - 1;
  int helpBar = commandBar - 1;
  int chatBar = (commandBar - logBar)/2 + logBar + 1;
  int logHeight = chatBar - logBar - 1;
  int chatHeight = helpBar - chatBar - 1;
  int playerWidth = 20;
  int playerHeight = commandBar - logBar - 1;
  
  // Create our windows
  // CursesScreen::createWindow(int width, int height, int startx, int starty)
  titleWin = createWindow(COLS, titleHeight, 0, 0);
  logWin = createWindow(COLS - (playerWidth + 1),
                        logHeight, 
                        0, 
                        logBar + 1);
  chatWin = createWindow(COLS - (playerWidth + 1), 
                         chatHeight, 
                         0, 
                         chatBar + 1);
  playerWin = createWindow(playerWidth, 
                           playerHeight, 
                           COLS - playerWidth, 
                           logBar + 1);
  commandWin = createWindow(COLS, 
                            commandHeight, 
                            0, 
                            commandBar + 1);  

  
  // Make sure nothing waits for input
  wtimeout(titleWin, 0);
  wtimeout(logWin, 0);
  wtimeout(chatWin, 0);
  wtimeout(commandWin, 0);
  wtimeout(playerWin, 0);
  nodelay(titleWin, true);
  nodelay(logWin, true);
  nodelay(chatWin, true);
  nodelay(commandWin, true);
  nodelay(playerWin, true);
  
  // Setup color if we haz it
  if (has_colors())
  {
    start_color();
    use_default_colors();
    init_pair(TEXT_COLOR_RED, COLOR_RED, -1);
    init_pair(TEXT_COLOR_GREEN, COLOR_GREEN, -1);
    init_pair(TEXT_COLOR_YELLOW, COLOR_YELLOW, -1);
    init_pair(TEXT_COLOR_BLUE, COLOR_BLUE, -1);
    init_pair(TEXT_COLOR_MAGENTA, COLOR_MAGENTA, -1);
    init_pair(TEXT_COLOR_CYAN, COLOR_CYAN, -1);
    init_pair(TEXT_COLOR_WHITE, COLOR_WHITE, -1);
    init_pair(TEXT_COLOR_INVERSE, COLOR_BLACK, COLOR_WHITE);
    
    wattron(titleWin, COLOR_PAIR(TEXT_COLOR_CYAN));
    wattron(playerWin, COLOR_PAIR(TEXT_COLOR_WHITE));
  }  
  
  // Write our border lines on the regular stdscr
  attron(COLOR_PAIR(TEXT_COLOR_WHITE));
  
  /* Draw dividers */  
  /* These mvaddch calls are cast to void to avoid an unused return value
     warning in clang. If there's a better way to do it, feel free to change
     things. */
  // log bar
  for (int x = 0; x < COLS; x++)
    (void)mvaddch(logBar, x, '=');

  // chat bar
  for(int x = 0; x < COLS - (playerWidth + 1); x++)
    (void)mvaddch(chatBar, x, '=');

  // help bar
  const char* helpStr = "Help:/help History:<UP>|<DOWN> Delete:<BACKSACE> CLEAR:<HOME>";
  mvaddstr(helpBar, 0, helpStr);

  // command bar
  for (int x = 0; x < COLS; x++)
    (void)mvaddch(commandBar, x, '=');

  // playerlist column divider
  for (int y = logBar + 1; y < commandBar; y++)
    (void)mvaddch(y, COLS - (playerWidth + 1), '|');

  attroff(COLOR_PAIR(TEXT_COLOR_MAGENTA));
  
  // Write the window labels
  attron(COLOR_PAIR(TEXT_COLOR_WHITE));
  attron(WA_BOLD);
  mvprintw(logBar, 2, " Log ");
  mvprintw(logBar, COLS - 15, " Players ");
  mvprintw(chatBar, 2, " Chat ");
  attroff(COLOR_PAIR(TEXT_COLOR_WHITE));
  attroff(WA_BOLD);
  refresh();
  
  // Now shove our logo in at the top
  log(LogType::LOG_INFO, "Title", "  /\\/\\ (_)_ __   ___ ___  ___ _ ____   _____ _ __ ");
  log(LogType::LOG_INFO, "Title", " /    \\| | '_ \\ / _ | __|/ _ \\ '__\\ \\ / / _ \\ '__|");
  log(LogType::LOG_INFO, "Title", "/ /\\/\\ \\ | | | |  __|__ \\  __/ |   \\ V /  __/ |   ");
  log(LogType::LOG_INFO, "Title", "\\/    \\/_|_| |_|\\___|___/\\___|_|    \\_/ \\___|_|  ");

  wmove(titleWin, logBar - 1, 50);
  wattron(titleWin, COLOR_PAIR(TEXT_COLOR_WHITE));
  wprintw(titleWin, ("v"+this->version).c_str());
  wattroff(titleWin, COLOR_PAIR(TEXT_COLOR_WHITE));
  wrefresh(titleWin);

  keypad(commandWin, true);

}

void CursesScreen::init(std::string version)
{
  this->version = version;
  commandBuf = "";
  
  initscr(); // Start NCurses
  timeout(0); // Non blocking
  //noecho();
  echo();
  refresh();

  CursesScreen::initWindows();
  CursesScreen::redrawPlayerList();
}

void CursesScreen::redraw()
{
  timeout(0);
  refresh();
  CursesScreen::initWindows();
  CursesScreen::redrawPlayerList();
  
  wclear(commandWin);
  waddstr(commandWin, commandBuf.c_str());
  wrefresh(commandWin);
}

std::string CursesScreen::prevCommand()
{
  std::string str = "";
  if (prevCommands.size() > 0)
  {
    str = prevCommands.top();
    prevCommands.pop();
    nextCommands.push(str);
  }
  return str;
}

std::string CursesScreen::nextCommand()
{
  std::string str = "";
  if (nextCommands.size() > 0)
  {
    str = nextCommands.top();
    nextCommands.pop();
    prevCommands.push(str);
  }
  return str;
}

void CursesScreen::addCommand(std::string str)
{
  while (!prevCommands.empty())
  {
    nextCommands.push(prevCommands.top());
    prevCommands.pop();
  }
  nextCommands.push(str);
}

/* This is where the keyboard interactions are handled. */
bool CursesScreen::hasCommand()
{
  bool running = true;
  std::string str = "";
  int c;

  do{
    c = wgetch(commandWin);
    switch(c)
    {
      case ERR:
      case KEY_RIGHT:
      case KEY_END:
        running = false;
        break;
      case KEY_RESIZE:
        CursesScreen::redraw();
        wdeleteln(commandWin);
        wclear(commandWin);
        waddstr(commandWin, commandBuf.c_str());
        running = false;
        break;
      case KEY_UP:
      case KEY_PPAGE:
        str = nextCommand();
        wdeleteln(commandWin);
        wclear(commandWin);
        if (str == "")
        {
          waddstr(commandWin, commandBuf.c_str());
        }
        else
        {
          waddstr(commandWin, str.c_str());
          commandBuf = str;
        }
        break;
      case KEY_DOWN:
      case KEY_NPAGE:
        str = prevCommand();
        wdeleteln(commandWin);
        wclear(commandWin);
        waddstr(commandWin, str.c_str());
        commandBuf = str;
        break;
      case KEY_ENTER:
      case '\n':
      case '\r':
        wdeleteln(commandWin);
        wclear(commandWin);
        command = commandBuf;
        addCommand(commandBuf);
        commandBuf = "";
        return true;
        break;
      case KEY_HOME:
        wdeleteln(commandWin);
        wclear(commandWin);
        commandBuf = "";
        running = false;
        break;
      case KEY_BACKSPACE:
      case KEY_LEFT:
      case KEY_SDC:
      case KEY_DC:
      case '\b':
        if (commandBuf.length() > 0)
          commandBuf.erase(commandBuf.length()-1,1);
        wdeleteln(commandWin);
        wclear(commandWin);
        waddstr(commandWin, commandBuf.c_str());
        running = false;
        break;
      default:
        commandBuf += c;
    }
  } while(running);

  return false;
}

std::string CursesScreen::getCommand()
{
  std::string str = command;
  command = "";
  return str;
}

WINDOW* CursesScreen::createWindow(int width, int height, int startx, int starty)
{  
  WINDOW *local_win;

  local_win = newwin(height, width, starty, startx);
  scrollok(local_win, 1);    // Fine to scroll
  wrefresh(local_win);  

  return local_win;
}

void CursesScreen::destroyWindow(WINDOW *local_win)
{  
  /* box(local_win, ' ', ' '); : This won't produce the desired
   * result of erasing the window. It will leave it's four corners 
   * and so an ugly remnant of window. 
   */
  wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
  /* The parameters taken are 
   * 1. win: the window on which to operate
   * 2. ls: character to be used for the left side of the window 
   * 3. rs: character to be used for the right side of the window 
   * 4. ts: character to be used for the top side of the window 
   * 5. bs: character to be used for the bottom side of the window 
   * 6. tl: character to be used for the top left corner of the window 
   * 7. tr: character to be used for the top right corner of the window 
   * 8. bl: character to be used for the bottom left corner of the window 
   * 9. br: character to be used for the bottom right corner of the window
   */
  wrefresh(local_win);
  delwin(local_win);
}

void CursesScreen::end()
{
  // Kill our windows
  destroyWindow(titleWin);
  destroyWindow(logWin);
  destroyWindow(chatWin);
  destroyWindow(playerWin);
  
  // Stop NCurses
  endwin();
}

void CursesScreen::log(LogType::LogType type, const std::string& source, const std::string& message)
{
  WINDOW *window;
  
  if (source == "Chat")
  {
    window = chatWin;
  }
  else if (source == "Command")
  {
    window = commandWin;
  }
  else if (source == "Players")
  {
    window = playerWin;
  }
  else if (source == "Title")
  {
    window = titleWin;
  }
  else
  {
    window = logWin;
  }

  // Set the color
  if (type == LogType::LOG_ERROR)
  {
    wattron(window, COLOR_PAIR(TEXT_COLOR_RED));
  }

  // Get the cursor so we can indent a bit
  int x, y;
  getyx(window, y, x);

  if (type != LogType::LOG_ERROR && (window == chatWin || window == logWin))
  {
    waddstr(window, "\n");
    wattron(window, WA_BOLD);
    waddstr(window, ("[" + currentTimestamp(true) + "] ").c_str());
    wattroff(window, WA_BOLD);
    waddstr(window, (message).c_str());
  }
  else if (window == titleWin || window == playerWin)
  {
    waddstr(window, (message).c_str());
    waddstr(window, "\n");
  }
  else 
  {
    waddstr(window, "\n");
    waddstr(window, (message).c_str());
  }
    
  // Turn off color again
  if (type == LogType::LOG_ERROR)
  {
    wattroff(window, COLOR_PAIR(TEXT_COLOR_RED));
  }

  wrefresh(window);  
}

#include <algorithm>

void CursesScreen::redrawPlayerList()
{
  // Clear the playerlist
  wclear(playerWin);

  // Now fill it up!
  if(usernames.size() == 0) 
  {
    log(LogType::LOG_INFO, "Players", "No active players");
  }
  else
  {
    for (std::vector<std::string>::const_iterator username = usernames.begin();
         username != usernames.end();
         ++username)
    {
      log(LogType::LOG_INFO, "Players", *username);
    }
  }
}

void CursesScreen::updatePlayerList(bool joining, const char *username)
{
  // Update the list
  if (joining)
  {
    usernames.push_back(username);
  } 
  else
  {
    std::vector<std::string>::iterator element = std::find(usernames.begin(), usernames.end(), std::string(username));

    if (element != usernames.end())
    {
      usernames.erase(element);
    }
  }

  // Redraw the list
  CursesScreen::redrawPlayerList();
}

bool logPost(int type, const char* source, const char* message)
{
  screen->log((LogType::LogType)type, source, message);
  return false;
}

static const unsigned int SERVER_CONSOLE_UID = -1;

bool checkForCommand()
{
  if (screen->hasCommand())
  {
    // Now handle this command as normal
    mineserver->chat.handleMessage("[Server]", screen->getCommand().c_str());
  }
  return false;
}

bool on_player_login(const char *username)
{
  screen->updatePlayerList(true, username);
  return false;
}

bool on_player_quit(const char *username)
{
  screen->updatePlayerList(false, username);
  return false;
}

PLUGIN_API_EXPORT void CALLCONVERSION cursesui_init(mineserver_pointer_struct* mineserver_temp)
{
  mineserver = mineserver_temp;

  if (mineserver->plugin.getPluginVersion(pluginName) > 0)
  {
    mineserver->logger.log(LogType::LOG_INFO, logSource, "The " PLUGIN_NAME " plugin is already loaded");
    return;
  }

  mineserver->plugin.setPluginVersion(pluginName, pluginVersion);

  screen = new CursesScreen();
  screen->init("1.2"); // TODO This should query mineserver for the version
  mineserver->plugin.addCallback("LogPost", (void *)logPost);
  mineserver->plugin.addCallback("Timer200", (void *)checkForCommand);
  mineserver->plugin.addCallback("PlayerLoginPost", (void *)on_player_login);
  mineserver->plugin.addCallback("PlayerQuitPost", (void *)on_player_quit);

  mineserver->logger.log(LogType::LOG_INFO, logSource, "Loaded " PLUGIN_NAME);
}

PLUGIN_API_EXPORT void CALLCONVERSION cursesui_shutdown(void)
{
  screen->end();
  //mineserver = NULL;
}
