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

#ifdef WIN32
#include <cstdlib>
#define NOMINMAX
#include <winsock2.h>
typedef int socklen_t;
#endif

#include <cerrno>
#include <sstream>
#include <algorithm>

#include "tr1.h"
#include TR1INCLUDE(array)

#include "sockets.h"
#include "tools.h"
#include "logger.h"
#include "constants.h"
#include "user.h"
#include "map.h"
#include "chat.h"
#include "nbt.h"
#include "mineserver.h"
#include "packets.h"


extern int setnonblock(int fd);

#ifndef WIN32
#define SOCKET_ERROR -1
#endif

static const size_t BUFSIZE = 2048;
static std::tr1::array<uint8_t, BUFSIZE> BUF;
static char* const cpBUF = reinterpret_cast<char*>(BUF.data());
static uint8_t* const upBUF = BUF.data();

extern "C" void client_callback(int fd, short ev, void* arg)
{
  User* user = reinterpret_cast<User*>(arg);

  if (ev & EV_READ)
  {
    int read = 1;

    read = recv(fd, cpBUF, BUFSIZE, 0);

    if (read == 0)
    {
      LOG2(INFO, "Socket closed properly");

      delete user;
      return;
    }

    if (read == SOCKET_ERROR)
    {
      LOG2(INFO, "Socket had no data to read");

      delete user;
      return;
    }

    user->lastData = std::time(NULL);

    user->buffer.addToRead(upBUF, read);

    user->buffer.reset();

    while (user->buffer >> (int8_t&)user->action)
    {      
      //Variable len package
      if (ServerInstance->packetHandler()->packets[user->action].len == PACKET_VARIABLE_LEN)
      {
        //Call specific function
        const bool disconnecting = user->action == 0xFF;
        const int curpos = ServerInstance->packetHandler()->packets[user->action].function(user);

        if (curpos == PACKET_NEED_MORE_DATA)
        {
          user->waitForData = true;
          event_set(user->GetEvent(), fd, EV_READ, client_callback, user);
          event_add(user->GetEvent(), NULL);
          return;
        }

        if (disconnecting) // disconnect -- player gone
        {
          delete user;
          return;
        }
      }
      else if (ServerInstance->packetHandler()->packets[user->action].len == PACKET_DOES_NOT_EXIST)
      {
        std::ostringstream str;
        str << "Unknown action: 0x" << std::hex << (unsigned int)(user->action);
        LOG2(DEBUG, str.str());

        delete user;
        return;
      }
      //Constant len packets
      else
      {
        //Check that the buffer has enough data before calling the function
        if (!user->buffer.haveData(ServerInstance->packetHandler()->packets[user->action].len))
        {
          user->waitForData = true;
          event_set(user->GetEvent(), fd, EV_READ, client_callback, user);
          event_add(user->GetEvent(), NULL);
          return;
        }

        //Call specific function
        ServerInstance->packetHandler()->packets[user->action].function(user);
      }
    } // while(user->buffer)
  }

  //If there is data in the output buffer, try to write it
  if (!user->buffer.getWriteEmpty())
  {
    std::vector<char> buf;
    user->buffer.getWriteData(buf);

    //Try to write the whole buffer
    const int written = send(fd, buf.data(), buf.size(), 0);

    //Handle errors
    if (written == SOCKET_ERROR)
    {
#ifdef WIN32
#define ERROR_NUMBER WSAGetLastError()
      if ((ERROR_NUMBER != WSATRY_AGAIN && ERROR_NUMBER != WSAEINTR && ERROR_NUMBER != WSAEWOULDBLOCK))
#else
#define ERROR_NUMBER errno
      if ((errno != EAGAIN && errno != EINTR))
#endif
      {
        LOG2(ERROR, "Error writing to client, tried to write " + dtos(buf.size()) + " bytes, code: " + dtos(ERROR_NUMBER));

        delete user;
        return;
      }

    }
    else
    {
      //Remove written amount from the buffer
      user->buffer.clearWrite(written);
    }

    //If we couldn't write everything at once, add EV_WRITE event calling this function again..
    if (!user->buffer.getWriteEmpty())
    {
      event_set(user->GetEvent(), fd, EV_WRITE | EV_READ, client_callback, user);
      event_add(user->GetEvent(), NULL);
      return;
    }
  }

  //Add EV_READ event again
  event_set(user->GetEvent(), fd, EV_READ, client_callback, user);
  event_add(user->GetEvent(), NULL);
}

extern "C" void accept_callback(int fd, short ev, void* arg)
{
  sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);

  const int client_fd = accept(fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);

  if (client_fd < 0)
  {
    LOGLF("Client: accept() failed");
    return;
  }

  User* const client = new User(client_fd, Mineserver::generateEID());
  setnonblock(client_fd);

  event_set(client->GetEvent(), client_fd, EV_WRITE | EV_READ, client_callback, client);
  event_add(client->GetEvent(), NULL);
}
