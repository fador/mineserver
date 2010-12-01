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

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <deque>
#include <fstream>
#include <vector>
#include <ctime>
#include <math.h>
#ifdef WIN32
  #include <winsock2.h>
#else
  #include <netinet/in.h>
  #include <string.h>
#endif

#include "logger.h"
#include "constants.h"

#include "tools.h"
#include "map.h"
#include "user.h"
#include "chat.h"
#include "config.h"
#include "physics.h"


Chat* Chat::mChat;

void Chat::free()
{
   if (mChat)
   {
      delete mChat;
      mChat = 0;
   }
}

Chat::Chat()
{
  registerStandardCommands();
}

void Chat::registerCommand(std::deque<std::string> words, ChatCommand command, bool adminOnly)
{
  // Loop thru all the words for this command
  std::string currentWord;
  while(!words.empty())
  {
    currentWord = words[0];
    words.pop_front();
  
    if(adminOnly)
      adminCommands[currentWord] = command;
    else
      userCommands[currentWord] = command;
  }
}

bool Chat::checkMotd(std::string motdFile)
{
  //
  // Create motdfile is it doesn't exist
  //
  std::ifstream ifs(motdFile.c_str());

  // If file does not exist
  if(ifs.fail())
  {
    std::cout << "> Warning: " << motdFile << " not found. Creating..." << std::endl;

    std::ofstream motdofs(motdFile.c_str());
    motdofs << MOTD_CONTENT << std::endl;
    motdofs.close();
  }

  ifs.close();

  return true;
}

bool Chat::loadAdmins(std::string adminFile)
{
  // Clear current admin-vector
  admins.clear();

  // Read admins to deque
  std::ifstream ifs(adminFile.c_str());

  // If file does not exist
  if(ifs.fail())
  {
    std::cout << "> Warning: " << adminFile << " not found. Creating..." << std::endl;

    std::ofstream adminofs(adminFile.c_str());
    adminofs << ADMIN_CONTENT << std::endl;
    adminofs.close();

    return true;
  }

  std::string temp;
  while(getline(ifs, temp))
  {
    // If not commentline
    if(temp[0] != COMMENTPREFIX)
      admins.push_back(temp);
  }
  ifs.close();

  std::cout << "Loaded admins from " << adminFile << std::endl;

  return true;
}

bool Chat::loadBanned(std::string bannedFile)
{
  // Clear current banned-vector
  banned.clear();

  // Read banned to deque
  std::ifstream ifs(bannedFile.c_str());

  // If file does not exist
  if(ifs.fail())
  {
    std::cout << "> Warning: " << bannedFile << " not found. Creating..." << std::endl;

    std::ofstream bannedofs(bannedFile.c_str());
    bannedofs << BANNED_CONTENT << std::endl;
    bannedofs.close();

    return true;
  }

  std::string temp;
  while(getline(ifs, temp))
  {
    // If not commentline
    if(temp[0] != COMMENTPREFIX)
      banned.push_back(temp);
  }
  ifs.close();

  std::cout << "Loaded banned users from " << bannedFile << std::endl;

  return true;
}

bool Chat::loadWhitelist(std::string whitelistFile)
{
  // Clear current whitelist-vector
  whitelist.clear();

  // Read whitelist to deque
  std::ifstream ifs(whitelistFile.c_str());

  // If file does not exist
  if(ifs.fail())
  {
    std::cout << "> Warning: " << whitelistFile << " not found. Creating..." << std::endl;

    std::ofstream whitelistofs(whitelistFile.c_str());
    whitelistofs << WHITELIST_CONTENT << std::endl;
    whitelistofs.close();

    return true;
  }

  std::string temp;
  while(getline(ifs, temp))
  {
    // If not commentline
    if(temp[0] != COMMENTPREFIX)
      whitelist.push_back(temp);
  }
  ifs.close();

  std::cout << "Loaded whitelisted users from " << whitelistFile << std::endl;

  return true;
}

bool Chat::sendUserlist(User *user)
{
  this->sendMsg(user, COLOR_BLUE + "[ " + dtos(Users.size()) + " players online ]", USER);

  for(unsigned int i = 0; i < Users.size(); i++)
  {
	std::string playerDesc = "> " + Users[i]->nick;
	if(Users[i]->muted)
		playerDesc += COLOR_YELLOW + " (muted)";
	if(Users[i]->dnd)
		playerDesc += COLOR_YELLOW + " (dnd)";
	
    this->sendMsg(user, playerDesc, USER);
  }

  return true;
}

std::deque<std::string> Chat::parseCmd(std::string cmd)
{
  int del;
  std::deque<std::string> temp;

  while(cmd.length() > 0)
  {
    while(cmd[0] == ' ')
      cmd = cmd.substr(1);

    del = cmd.find(' ');

    if(del > -1)
    {
      temp.push_back(cmd.substr(0, del));
      cmd = cmd.substr(del+1);
    }
    else
    {
      temp.push_back(cmd);
      break;
    }
  }

  if(temp.empty())
    temp.push_back("empty");

  return temp;
}

bool Chat::handleMsg(User *user, std::string msg)
{
  // Timestamp
  time_t rawTime = time(NULL);

  struct tm *Tm  = localtime(&rawTime);

  std::string timeStamp (asctime(Tm));
  timeStamp = timeStamp.substr(11, 5);

  //
  // Chat commands
  //

  // Servermsg (Admin-only)
  if(msg[0] == SERVERMSGPREFIX && user->admin)
  {
    // Decorate server message
    msg = COLOR_RED + "[!] " + COLOR_GREEN + msg.substr(1);
    this->sendMsg(user, msg, ALL);
  }

  // Adminchat
  else if(msg[0] == ADMINCHATPREFIX && user->admin)
  {
    msg = timeStamp + " @@ <"+ COLOR_DARK_MAGENTA + user->nick + COLOR_WHITE + "> " + msg.substr(1);
    this->sendMsg(user, msg, ADMINS);
  }

  // Command
  else if(msg[0] == CHATCMDPREFIX)
  {
    std::deque<std::string> cmd = this->parseCmd(msg.substr(1));

    std::string command         = cmd[0];
    cmd.pop_front();

    // User commands
    CommandList::iterator iter;
    if((iter = userCommands.find(command)) != userCommands.end())
      iter->second(user, command, cmd);
    else if(user->admin && (iter = adminCommands.find(command)) != adminCommands.end())
      iter->second(user, command, cmd);
  }
  // Normal message
  else
  {
		if(user->isAbleToCommunicate("chat") == false) {
			return true;
		}
    else {
      if(user->admin) 
        msg = timeStamp + " <"+ COLOR_DARK_MAGENTA + user->nick + COLOR_WHITE + "> " + msg;
      else 
        msg = timeStamp + " <"+ user->nick + "> " + msg;
    }
      
    LOG(msg);

    this->sendMsg(user, msg, ALL);
    
  }

  return true;
}

bool Chat::sendMsg(User *user, std::string msg, MessageTarget action)
{
  size_t tmpArrayLen = msg.size()+3;
  uint8 *tmpArray    = new uint8[tmpArrayLen];

  tmpArray[0] = 0x03;
  tmpArray[1] = 0;
  tmpArray[2] = msg.size()&0xff;

  for(unsigned int i = 0; i < msg.size(); i++)
    tmpArray[i+3] = msg[i];

  switch(action)
  {
  case ALL:
    user->sendAll(tmpArray, tmpArrayLen);
    break;

  case USER:
   	user->buffer.addToWrite(tmpArray, tmpArrayLen);
    break;

  case ADMINS:
    user->sendAdmins(tmpArray, tmpArrayLen);
    break;

  case OTHERS:
    user->sendOthers(tmpArray, tmpArrayLen);
    break;
  }

  delete[] tmpArray;

  return true;
}
