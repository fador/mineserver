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

#ifdef WIN32
    #define _CRTDBG_MAP_ALLOC
    #include <stdlib.h>
    #include <crtdbg.h>
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

#include <iostream>
#include <fstream>
#include <deque>
#include <vector>
#include <ctime>
#include <event.h>
#include <sys/stat.h>
#include <zlib.h>
#include "logger.h"
#include "constants.h"

#include "tools.h"

#include "user.h"
#include "map.h"
#include "chat.h"
#include "nbt.h"
#include "packets.h"


extern int setnonblock(int fd);

void buf_write_callback(struct bufferevent *bev, void *arg)
{
}

void buf_error_callback(struct bufferevent *bev, short what, void *arg)
{
  User *client = (User *)arg;
  bufferevent_free(client->buf_ev);

#ifdef WIN32
  closesocket(client->fd);
#else
  close(client->fd);
#endif

  remUser(client->fd);
}

void buf_read_callback(struct bufferevent *incoming, void *arg)
{

  User *user = (User *)arg;

  int read   = 1;

  uint8 *buf = new uint8[2048];

  //Push data to buffer
  while(read = bufferevent_read(incoming, buf, 2048))
  {
	user->buffer.append(buf, read);
  }

  delete[] buf;

  user->buffer.reset();

  while(user->buffer >> (sint8&)user->action)
  {
    //Variable len package
    if(PacketHandler::get().packets[user->action].len == PACKET_VARIABLE_LEN)
    {
      //Call specific function
      int (PacketHandler::*function)(User *) =
        PacketHandler::get().packets[user->action].function;
	  bool disconnecting = user->action == 0xFF;
      int curpos = (PacketHandler::get().*function)(user);
      if(curpos == PACKET_NEED_MORE_DATA)
      {
        user->waitForData = true;
        return;
      }

	  if(disconnecting) // disconnect -- player gone
		  return;
    }
    else if(PacketHandler::get().packets[user->action].len == PACKET_DOES_NOT_EXIST)
    {
      printf("Unknown action: 0x%x\n", user->action);
      bufferevent_free(user->buf_ev);
      #ifdef WIN32
      closesocket(user->fd);
      #else
      close(user->fd);
      #endif
      remUser(user->fd);
    }
    else
    {
	  if(!user->buffer.haveData(PacketHandler::get().packets[user->action].len))
      {
        user->waitForData = true;
        return;
      }

	  //Call specific function
      int (PacketHandler::*function)(User *) = PacketHandler::get().packets[user->action].function;
      (PacketHandler::get().*function)(user);

    }

  } //End while

}

void accept_callback(int fd,
                     short ev,
                     void *arg)
{
  int client_fd;
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);


  client_fd = accept(fd,
                     (struct sockaddr *)&client_addr,
                     &client_len);
  if(client_fd < 0)
  {
    LOG("Client: accept() failed");
    return;
  }
  User *client = addUser(client_fd, generateEID());
  setnonblock(client_fd);

  client->buf_ev = bufferevent_new(client_fd,
                                   buf_read_callback,
                                   buf_write_callback,
                                   buf_error_callback,
                                   client);

  bufferevent_enable(client->buf_ev, EV_READ);
}
