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

#include "screen.h"

//#define _DEBUG 

Screen* Screen::_instance;

Screen::Screen() {
}
void Screen::init(std::string version) {
	
	// Start our NCurses session
	initscr();
  timeout(0);   // Non blocking?			
  //noecho();
  echo();
	refresh();
	
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
  nodelay(title,true);
  nodelay(generalLog,true);
  nodelay(chatLog,true);
  nodelay(commandLog,true);
  nodelay(playerList,true);

	// Setup color if we haz it
	if(has_colors())
	{
		start_color();
		init_pair(TEXT_COLOR_RED, COLOR_RED, COLOR_BLACK);
		init_pair(TEXT_COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
		init_pair(TEXT_COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
		init_pair(TEXT_COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
		init_pair(TEXT_COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(TEXT_COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
		init_pair(TEXT_COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
		init_pair(TEXT_COLOR_INVERSE, COLOR_BLACK, COLOR_WHITE);
		
		wattron(title, COLOR_PAIR(TEXT_COLOR_GREEN));
		wattron(playerList, COLOR_PAIR(TEXT_COLOR_CYAN));
	}

	// Write our border lines on the regular stdscr
	attron(COLOR_PAIR(TEXT_COLOR_MAGENTA));
	for(int x = 0; x < COLS; x++)               // Top row
	{
		mvaddch(titleHeight - 1, x, '=');
	}
	for(int x = 0; x < COLS-21; x++)            // Middle row
	{
		mvaddch(logHeight + titleHeight, x, '=');
	}
	for(int x = 0; x < COLS; x++)               // Bottom row
	{
		mvaddch((logHeight + chatHeight + titleHeight + 1), x, '=');
	}	
	for(int y = 5; y < (logHeight + chatHeight + titleHeight + 1); y++)          // Far col divider
	{
		mvaddch(y, COLS-21, '|');
	} 
	attroff(COLOR_PAIR(TEXT_COLOR_MAGENTA));
	
	// Write the window labels
	attron(COLOR_PAIR(TEXT_COLOR_WHITE));
	mvprintw(titleHeight - 1, 2, " Log ");
	mvprintw(titleHeight - 1, COLS - 15, " Players ");
	mvprintw(logHeight + titleHeight, 2, " Chat ");
	mvprintw((logHeight + chatHeight + titleHeight + 1), 2, " Command History ");
	attroff(COLOR_PAIR(TEXT_COLOR_WHITE));
	refresh();
	
	// Now shove our logo in at the top
	log(LOG_TITLE, "  /\\/\\ (_)_ __   ___ ___  ___ _ ____   _____ _ __ ");
	log(LOG_TITLE, " /    \\| | '_ \\ / _ | __|/ _ \\ '__\\ \\ / / _ \\ '__|");
	log(LOG_TITLE, "/ /\\/\\ \\ | | | |  __|__ \\  __/ |   \\ V /  __/ |   ");
	log(LOG_TITLE, "\\/    \\/_|_| |_|\\___|___/\\___|_|    \\_/ \\___|_|  ");
	wmove(title, 3, 50);
	wattron(title, COLOR_PAIR(TEXT_COLOR_YELLOW));
	wprintw(title, ("v" + version).c_str());
	wattroff(title, COLOR_PAIR(TEXT_COLOR_YELLOW));
	wrefresh(title);
	
	// Fill up the command window
//  log(LOG_COMMAND, "");
//  log(LOG_COMMAND, "");
//  log(LOG_COMMAND, "");
//  log(LOG_COMMAND, "");
//  log(LOG_COMMAND, "");   	

}
bool Screen::hasCommand()
{

  // Get the chars in the buffer
  wmove(commandLog, 4, currentCommand.size() + 1);
  char readchar;
  bool run=true;
  do
  {
    readchar=wgetch(commandLog);
    // Add to our string buffer
    if(readchar!=ERR)
    {
      //Backspace
      if(readchar == '\b')
      {
        currentCommand.erase(currentCommand.end()-1);
      }
      else if(readchar == '\n')
      {
        run = false;
      }
      else
      {
        currentCommand += readchar;
      }

    }
    else
    {
      run = false;
    }
  }
  while(run);
  //int crlfEntered = wgetnstr(commandLog, commandBuffer, 80);
  wmove(commandLog, 4, currentCommand.size() + 1);
  wrefresh(commandLog);


  
  // Check if we've got a full command waiting
  if(readchar == '\n')//crlfEntered == OK)
    return true;
  else
    return false;
}
std::string Screen::getCommand()
{
  // Get a copy of the current command, clear it and return the copy
  std::string tempCmd = currentCommand;
  currentCommand.clear();
  log(LOG_COMMAND, "");
  return tempCmd;
}
WINDOW* Screen::createWindow(int width, int height, int startx, int starty)
{	
	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
//	box(local_win, 0 , 0);		// Border the window		
	scrollok(local_win, 1);		// Fine to scroll
	wrefresh(local_win);	

	return local_win;
}
void Screen::destroyWindow(WINDOW *local_win)
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
void Screen::end()
{
	// Kill our windows
	destroyWindow(title);
	destroyWindow(generalLog);
	destroyWindow(chatLog);
	destroyWindow(playerList);
	
	// Stop NCurses
	endwin();
}
void Screen::log(std::string message)
{
	// Default to general log
	this->log(LOG_GENERAL, message);
}
void Screen::log(int logType, std::string message)
{
	WINDOW *window = title;
	if(logType == LOG_GENERAL) { window = generalLog; }
	if(logType == LOG_ERROR) { window = generalLog; }	
	if(logType == LOG_CHAT) { window = chatLog; }
	if(logType == LOG_PLAYERS) { window = playerList; }
	if(logType == LOG_COMMAND) { window = commandLog; }
  	
	// Set the color
	if(logType == LOG_ERROR) {	wattron(window, COLOR_PAIR(TEXT_COLOR_RED));	}
	
	// Get the cursor so we can indent a bit
	int x, y;
	getyx(window, y, x);

	// Print the message to the correct window
	if (window == chatLog)
	{
	  // Timestamp
    time_t rawTime = time(NULL);
    struct tm *Tm  = localtime(&rawTime);
    std::string timeStamp (asctime(Tm));
    timeStamp = timeStamp.substr(11, 5);
    
    // Display the timestamp
    waddstr(window, (timeStamp + " ").c_str());
    
    // Check for special messages
    if(message.substr(0,3) == "[!]" || message.substr(0,3) == "[@]")    // Server or AdminChat message
    {
      wattron(window, COLOR_PAIR(TEXT_COLOR_RED));  
      waddstr(window, message.substr(0,3).c_str());
      wattroff(window, COLOR_PAIR(TEXT_COLOR_RED));  
      if(message.substr(0,3) == "[!]") { wattron(window, COLOR_PAIR(TEXT_COLOR_GREEN)); }      
      waddstr(window, (message.substr(3) + "\n").c_str());
      if(message.substr(0,3) == "[!]") { wattroff(window, COLOR_PAIR(TEXT_COLOR_GREEN)); }          
    }
    else if(message.substr(0, 1) == "*")                                // Emotes
    {
      wattron(window, COLOR_PAIR(TEXT_COLOR_YELLOW));  
      waddstr(window, (message + "\n").c_str());
      wattroff(window, COLOR_PAIR(TEXT_COLOR_YELLOW));  
    }
  	else                                                                // Regular chat
  	{
  	   waddstr(window, (message + "\n").c_str());
  	}    
	}
	else
	{
		waddstr(window, (message + "\n").c_str());
	}
		
	// Turn off color again
	if(logType == LOG_ERROR) {	wattroff(window, COLOR_PAIR(TEXT_COLOR_RED)); }
	
	wrefresh(window);	
	
//  wprintw(commandLog, "> ", 5, 0);
//  wmove(commandLog, 5, 2);
//  wrefresh(commandLog);
	
}
void Screen::updatePlayerList(std::vector<User *> users)
{
	// Clear the playerlist
	wclear(playerList);
	
	// Now fill it up!
	if(users.size() == 0)
	{
		Screen::get()->log(LOG_PLAYERS, "No players");
	}
	else
	{
		for(unsigned int i = 0; i < users.size(); i++)
	  {
			Screen::get()->log(LOG_PLAYERS, users[i]->nick);
	  }
	}
}


