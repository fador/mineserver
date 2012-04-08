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

#ifndef _CHAT_H
#define _CHAT_H

#include <deque>
#include <string>

class User;

class Chat
{
public:
  enum MessageTarget
  {
    ALL,
    USER,
    OTHERS,
    ADMINS,
    OPS,
    GUESTS
  };

  Chat();
  ~Chat();

  bool handleMsg(User* user, std::string msg);
  void handleServerMsg(User* user, std::string msg, const std::string& timeStamp);
  void handleAdminChatMsg(User* user, std::string msg, const std::string& timeStamp);
  void handleChatMsg(User* user, std::string msg, const std::string& timeStamp);

  bool sendMsg(User* user, std::string msg, MessageTarget action = ALL);
  bool sendUserlist(User* user);
  void sendHelp(User* user, std::deque<std::string> args);

  void handleCommand(User* user, std::string msg, const std::string& timeStamp);

private:
  std::deque<std::string> parseCmd(std::string cmd);
  std::string adminPassword;
};

#endif
