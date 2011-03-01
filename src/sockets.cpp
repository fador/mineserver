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

#ifdef WIN32
#include <stdlib.h>
#include <conio.h>
#include <winsock2.h>
typedef  int socklen_t;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#endif
#include <errno.h>
#include <iostream>
#include <fstream>
#include <deque>
#include <vector>
#include <ctime>
#include <event.h>
#include <sys/stat.h>
#include <zlib.h>

#include "tools.h"

#include "logger.h"
#include "constants.h"
#include "user.h"
#include "map.h"
#include "chat.h"
#include "nbt.h"
#include "mineserver.h"

#include "packets.h"
#include <algorithm>

extern int setnonblock(int fd);

#ifndef WIN32
#define SOCKET_ERROR -1
#endif

void client_callback(int fd,
                     short ev,
                     void* arg)
{
  User* user = (User*)arg;
  /*
  std::vector<User *>::const_iterator it = std::find (Mineserver::get()->users().begin(),
                                                      Mineserver::get()->users().end(), user);
  if(it == Mineserver::get()->users().end())
  {
    Mineserver::get()->logger()->log(LogType::LOG_INFO, "Sockets", "Using dead player!!!");
    return;
  }
  */
  if (ev & EV_READ)
  {

    int read   = 1;

    uint8_t* buf = new uint8_t[2048];

    read = recv(fd, (char*)buf, 2048, 0);
    if (read == 0)
    {
      Mineserver::get()->logger()->log(LogType::LOG_INFO, "Sockets", "Socket closed properly");

      delete user;
      user = (User*)1;
      delete[] buf;
      return;
    }

    if (read == SOCKET_ERROR)
    {
      Mineserver::get()->logger()->log(LogType::LOG_INFO, "Sockets", "Socket had no data to read");

      delete user;
      user = (User*)2;
      delete[] buf;
      return;
    }

    user->lastData = time(NULL);

    user->buffer.addToRead(buf, read);

    delete[] buf;

    user->buffer.reset();

    while (user->buffer >> (int8_t&)user->action)
    {
      //Variable len package
      if (Mineserver::get()->packetHandler()->packets[user->action].len == PACKET_VARIABLE_LEN)
      {
        //Call specific function
        int (PacketHandler::*function)(User*) =
          Mineserver::get()->packetHandler()->packets[user->action].function;
        bool disconnecting = user->action == 0xFF;
        int curpos = (Mineserver::get()->packetHandler()->*function)(user);
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
          user = (User*)4;
          return;
        }
      }
      else if (Mineserver::get()->packetHandler()->packets[user->action].len == PACKET_DOES_NOT_EXIST)
      {
        printf("Unknown action: 0x%x\n", user->action);

        delete user;
        user = (User*)3;
        return;
      }
      else
      {
        if (!user->buffer.haveData(Mineserver::get()->packetHandler()->packets[user->action].len))
        {
          user->waitForData = true;
          event_set(user->GetEvent(), fd, EV_READ, client_callback, user);
          event_add(user->GetEvent(), NULL);
          return;
        }

        //Call specific function
        int (PacketHandler::*function)(User*) = Mineserver::get()->packetHandler()->packets[user->action].function;
        (Mineserver::get()->packetHandler()->*function)(user);
      }
    } //End while
  }

  int writeLen = user->buffer.getWriteLen();
  if (writeLen)
  {
    int written = send(fd, (char*)user->buffer.getWrite(), writeLen, 0);
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
        Mineserver::get()->logger()->log(LogType::LOG_ERROR, "Socket", "Error writing to client, tried to write " + dtos(writeLen) + " bytes, code: " + dtos(ERROR_NUMBER));

        delete user;
        user = (User*)5;
        return;
      }
      else
      {
        //user->write_err_count++;
      }

    }
    else
    {
      user->buffer.clearWrite(written);
      //user->write_err_count=0;
    }

    if (user->buffer.getWriteLen())
    {
      event_set(user->GetEvent(), fd, EV_WRITE | EV_READ, client_callback, user);
      event_add(user->GetEvent(), NULL);
      return;
    }
  }

  event_set(user->GetEvent(), fd, EV_READ, client_callback, user);
  event_add(user->GetEvent(), NULL);
}

void accept_callback(int fd,
                     short ev,
                     void* arg)
{
  int client_fd;
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);


  client_fd = accept(fd,
                     (struct sockaddr*)&client_addr,
                     &client_len);
  if (client_fd < 0)
  {
    LOGLF("Client: accept() failed");
    return;
  }
  User* client = new User(client_fd, generateEID());
  setnonblock(client_fd);

  event_set(client->GetEvent(), client_fd, EV_WRITE | EV_READ, client_callback, client);
  event_add(client->GetEvent(), NULL);

}
