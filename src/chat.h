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

#ifndef _CHAT_H
#define _CHAT_H

class Chat
{
public:
  enum MessageTarget
  {
    ALL,
    USER,
    OTHERS,
    ADMINS
  };
  typedef void (*ChatCommand)(User *, std::string, std::deque<std::string> );
  //Chat();
  std::deque<std::string> admins;
  std::deque<std::string> banned;
  std::deque<std::string> whitelist;
  bool handleMsg( User *user, std::string msg );
  bool sendMsg( User *user, std::string msg, MessageTarget action = ALL );
  bool sendUserlist( User *user );
  bool loadAdmins(std::string adminFile);
  bool loadBanned(std::string bannedFile);
  bool loadWhitelist(std::string whitelistFile);
  bool checkMotd(std::string motdFile);
  void registerCommand(std::deque<std::string> words, ChatCommand command, bool adminOnly);
  static Chat* get()
  {
    if(!mChat) {
      mChat = new Chat();
    }
    return mChat;
  }
  void free();
private:
  static Chat *mChat;
  typedef std::map<std::string, ChatCommand> CommandList;
  CommandList userCommands;
  CommandList adminCommands;
  Chat();
  void registerStandardCommands();
  std::deque<std::string> parseCmd(std::string cmd);
};

#endif
