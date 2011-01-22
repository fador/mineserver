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

void CursesScreen::init(std::string version)
{
  initscr(); // Start NCurses
  timeout(0); // Non blocking
  //noecho();
  echo();
  refresh();

  currentCommandHistoryIndex = 0;
  nextCommandHistoryIndex = 0;
  for (int i = 0; i < commandHistorySize; i++)
  {
    commandHistory[i].clear();
  }

  // Work out our dimensions - 7 for top row - 7 for bottom command row
  int titleHeight = 5;
  int commandHeight = 5;
  int logHeight = int((LINES - titleHeight - commandHeight) / 2);
  int chatHeight = LINES - titleHeight - commandHeight - logHeight - 3;
  
  // Create our windows
  title = createWindow(COLS, titleHeight, 0, 0);
  generalLog = createWindow(COLS - 21, logHeight, 0, titleHeight);
  chatLog = createWindow(COLS - 21, chatHeight, 0, logHeight + titleHeight + 1);
  commandLog = createWindow(COLS, 5, 0, LINES - 5);  
  playerList = createWindow(20, LINES - 14, COLS - 20, 5);

  // Make sure nothing waits for input
  wtimeout(title, 0);
  wtimeout(generalLog, 0);
  wtimeout(chatLog, 0);
  wtimeout(commandLog, 0);
  wtimeout(playerList, 0);
  nodelay(title, true);
  nodelay(generalLog, true);
  nodelay(chatLog, true);
  nodelay(commandLog, true);
  nodelay(playerList, true);

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
    
    wattron(title, COLOR_PAIR(TEXT_COLOR_CYAN));
    wattron(playerList, COLOR_PAIR(TEXT_COLOR_WHITE));
  }

  // Write our border lines on the regular stdscr
  attron(COLOR_PAIR(TEXT_COLOR_WHITE));

  // TODO: These mvaddch calls are cast to void to avoid an unused return value
  // warning in clang. If there's a better way to do it, feel free to change
  // things.

  // Top row
  for (int x = 0; x < COLS; x++)
    (void)mvaddch(titleHeight - 1, x, '=');

  // Middle row
  for(int x = 0; x < COLS - 21; x++)
    (void)mvaddch(logHeight + titleHeight, x, '=');

  // Bottom row
  for (int x = 0; x < COLS; x++)
    (void)mvaddch((logHeight + chatHeight + titleHeight + 1), x, '=');

  // Far column divider
  for (int y = 5; y < (logHeight + chatHeight + titleHeight + 1); y++)
    (void)mvaddch(y, COLS - 21, '|');

  attroff(COLOR_PAIR(TEXT_COLOR_MAGENTA));
  
  // Write the window labels
  attron(COLOR_PAIR(TEXT_COLOR_WHITE));
  attron(WA_BOLD);
  mvprintw(titleHeight - 1, 2, " Log ");
  mvprintw(titleHeight - 1, COLS - 15, " Players ");
  mvprintw(logHeight + titleHeight, 2, " Chat ");
  mvprintw((logHeight + chatHeight + titleHeight + 1), 2, " Command History ");
  attroff(COLOR_PAIR(TEXT_COLOR_WHITE));
  attroff(WA_BOLD);
  refresh();
  
  // Now shove our logo in at the top
  log(LogType::LOG_INFO, "Title", "  /\\/\\ (_)_ __   ___ ___  ___ _ ____   _____ _ __ ");
  log(LogType::LOG_INFO, "Title", " /    \\| | '_ \\ / _ | __|/ _ \\ '__\\ \\ / / _ \\ '__|");
  log(LogType::LOG_INFO, "Title", "/ /\\/\\ \\ | | | |  __|__ \\  __/ |   \\ V /  __/ |   ");
  log(LogType::LOG_INFO, "Title", "\\/    \\/_|_| |_|\\___|___/\\___|_|    \\_/ \\___|_|  ");

  wmove(title, 3, 50);
  wattron(title, COLOR_PAIR(TEXT_COLOR_WHITE));
  wprintw(title, ("v" + version).c_str());
  wattroff(title, COLOR_PAIR(TEXT_COLOR_WHITE));
  wrefresh(title);

  keypad(commandLog, true);

  // Initalise playerlist
  wclear(playerList);

  // Now fill it up!
  if(usernames.size() == 0) 
  {
    log(LogType::LOG_INFO, "Players", "No active players");
  }

  // Fill up the command window
//  log(LOG_COMMAND, "");
//  log(LOG_COMMAND, "");
//  log(LOG_COMMAND, "");
//  log(LOG_COMMAND, "");
//  log(LOG_COMMAND, "");     

  commandX = 0;
}

bool CursesScreen::hasCommand()
{
  int readchar;
  bool running = true;

  // Get the chars in the buffer
  wmove(commandLog, 4, commandX + 1);

  do
  {
    readchar = wgetch(commandLog); // FIXME

    // Add to our string buffer
    if (readchar != ERR)
    {
      if (readchar == '\b') // Backspace
      {
        if (!currentCommand.empty())
        {
          currentCommand.erase(commandX - 1, 1);
          wdelch(commandLog);
          --commandX;
        }
      }
      else if (readchar == '\n')
      {
        running = false;
      }
      else if (readchar == KEY_LEFT)
      {
        if (commandX > 0)
        {
          --commandX;
        }
      }
      else if (readchar == KEY_RIGHT)
      {
        if ( commandX < currentCommand.size() )
        {
          ++commandX;
        }
      }
      else if (readchar == KEY_DOWN)
      {
        // Get the next command in the history
        currentCommandHistoryIndex = (currentCommandHistoryIndex + 1) % commandHistorySize;

        if (currentCommandHistoryIndex == nextCommandHistoryIndex)
        {
          // At the start of the history
          currentCommand = "";
          if ((--currentCommandHistoryIndex) < 0)
          {
            currentCommandHistoryIndex += CursesScreen::commandHistorySize;
          }
        }
        else
        {
          currentCommand = commandHistory[currentCommandHistoryIndex];
        }

        // Render the command to the screen
        wdeleteln(commandLog);
        wmove(commandLog, 4, 1);
        waddstr(commandLog, currentCommand.c_str());
        commandX = currentCommand.size();
      }
      else if (readchar == KEY_UP)
      {
        // Get the previous command from the command history.
        if (!commandHistory[currentCommandHistoryIndex].empty())
        {
          currentCommand = commandHistory[currentCommandHistoryIndex];
        }

        if (currentCommandHistoryIndex != nextCommandHistoryIndex)
        {
          // Set it up to go to the previous command next time.
          currentCommandHistoryIndex = currentCommandHistoryIndex - 1;
          if (currentCommandHistoryIndex < 0)
          {
            currentCommandHistoryIndex += CursesScreen::commandHistorySize;
          }
        }

        if (!currentCommand.empty())
        {
          // Render the command to the screen.
          wdeleteln(commandLog);
          wmove(commandLog, 4, 1);
          waddstr(commandLog, currentCommand.c_str());
          commandX = currentCommand.size();
        }
      }
      else
      {
        currentCommand += readchar;
        ++commandX;
      }
    }
    else
      running = false;
  } while(running);

  //int crlfEntered = wgetnstr(commandLog, commandBuffer, 80);

  // Check if we've got a full command waiting
  if (readchar == '\n')//crlfEntered == OK)
  {
    wmove(commandLog, 3, commandX + 1);
    waddstr(commandLog, currentCommand.substr(commandX).c_str());
    return true;
  }

  wmove(commandLog, 4, commandX + 1);
  wrefresh(commandLog);
  return false;
}

std::string CursesScreen::getCommand()
{
  // Get a copy of the current command, clear it and return the copy
  std::string command = currentCommand;
  currentCommand.clear();
  commandX = 0;
  if (!command.empty())
  {
    wmove(commandLog, 4, command.size() + 1);
    log(LogType::LOG_INFO, "Command", "");
    currentCommandHistoryIndex =  nextCommandHistoryIndex;
    commandHistory[nextCommandHistoryIndex++] = command;
    nextCommandHistoryIndex = nextCommandHistoryIndex % CursesScreen::commandHistorySize;    
  }
  return command;
}

WINDOW* CursesScreen::createWindow(int width, int height, int startx, int starty)
{  
  WINDOW *local_win;

  local_win = newwin(height, width, starty, startx);
//  box(local_win, 0 , 0);    // Border the window    
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
  destroyWindow(title);
  destroyWindow(generalLog);
  destroyWindow(chatLog);
  destroyWindow(playerList);

  // Stop NCurses
  endwin();
}

/*void Screen::log(std::string message)
{
  // Default to general log
  this->log(LOG_GENERAL, message);
}*/

void CursesScreen::log(LogType::LogType type, const std::string& source, const std::string& message)
{
  WINDOW *window = generalLog;

  if (source == "Chat")
  {
    window = chatLog;
  }
  else if (source == "Command")
  {
    window = commandLog;
  }
  else if (source == "Players")
  {
    window = playerList;
  }
  else if (source == "Title")
  {
    window = title;
  }

  // Set the color
  if (type == LogType::LOG_ERROR)
  {
    wattron(window, COLOR_PAIR(TEXT_COLOR_RED));
  }

  // Get the cursor so we can indent a bit
  int x, y;
  getyx(window, y, x);

/*
  // Print the message to the correct window
  if (window == chatLog)
  {
    // Display the timestamp
    waddstr(window, (currentTimestamp(false) + " ").c_str());

    // Check for special messages
    if (message.substr(0,3) == "[!]" || message.substr(0,3) == "[@]")    // Server or AdminChat message
    {
      wattron(window, COLOR_PAIR(TEXT_COLOR_RED));  
      waddstr(window, message.substr(0,3).c_str());
      wattroff(window, COLOR_PAIR(TEXT_COLOR_RED));  
      if(message.substr(0,3) == "[!]") { wattron(window, COLOR_PAIR(TEXT_COLOR_GREEN)); }      
      waddstr(window, (message.substr(3) + "\n").c_str());
      if(message.substr(0,3) == "[!]") { wattroff(window, COLOR_PAIR(TEXT_COLOR_GREEN)); }          
    }
    else if (message.substr(0, 1) == "*") // Emotes
    {
      wattron(window, COLOR_PAIR(TEXT_COLOR_YELLOW));  
      waddstr(window, (message + "\n").c_str());
      wattroff(window, COLOR_PAIR(TEXT_COLOR_YELLOW));  
    }
    else // Regular chat
    {
       waddstr(window, (message + "\n").c_str());
    }
  }
*/

  if (type != LogType::LOG_ERROR && (window == chatLog || window == generalLog))
  {
    wattron(window, WA_BOLD);
    waddstr(window, ("[" + currentTimestamp(true) + "] ").c_str());
    wattroff(window, WA_BOLD);
    waddstr(window, (message + "\n").c_str());
  }
  else
  {
    waddstr(window, (message + "\n").c_str());
  }
    
  // Turn off color again
  if (type == LogType::LOG_ERROR)
  {
    wattroff(window, COLOR_PAIR(TEXT_COLOR_RED));
  }

  wrefresh(window);  
  
//  wprintw(commandLog, "> ", 5, 0);
//  wmove(commandLog, 5, 2);
//  wrefresh(commandLog);
  
}

#include <algorithm>

void CursesScreen::updatePlayerList(bool joining, const char *username)
{
  // Update the list
  if (joining)
  {
    usernames.push_back(username);
  } else
  {
    std::vector<std::string>::iterator element = std::find(usernames.begin(), usernames.end(), std::string(username));

    if (element != usernames.end())
    {
      usernames.erase(element);
    }
  }

  // Redraw the list

  // Clear the playerlist
  wclear(playerList);

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
  mineserver = NULL;
}
