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
  uint32 i = 0;

  User *user=(User *)arg;

  int read = 1;

  uint8 *buf = new uint8[2048];

  //Push data to buffer
  while(read)
  {
    if(read = bufferevent_read(incoming, buf, 2048))
    {
      for(int i = 0;i<read;i++)
      {
        user->buffer.push_back(buf[i]);
      }
    }
  }

  delete [] buf;

  while(user->buffer.size()>0)
  {
    // If not waiting more data
    if(!user->waitForData)
    {
      user->action = user->buffer.front();
      user->buffer.pop_front();
      //printf("Action: 0x%x\n", user->action);
    }
    else
    {
       user->waitForData = false;
    }

    //Variable len package
    if(PacketHandler::get().packets[user->action].len==PACKET_VARIABLE_LEN)
    {
      //Call specific function
      int (PacketHandler::*varLenFunction)(User *)=PacketHandler::get().packets[user->action].varLenFunction;
      int curpos=(PacketHandler::get().*varLenFunction)(user);
      if(curpos==PACKET_NEED_MORE_DATA)
      {
        user->waitForData = true;
        return;
      }
    }
    if(PacketHandler::get().packets[user->action].len==PACKET_DOES_NOT_EXIST)
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
      if(user->buffer.size()<PacketHandler::get().packets[user->action].len)
      {
        user->waitForData = true;
        return;
      }
      //Store data to dynamic array
      uint8 *data = new uint8 [PacketHandler::get().packets[user->action].len];
      if(PacketHandler::get().packets[user->action].len)
      {
        std::copy(user->buffer.begin(),user->buffer.begin()+PacketHandler::get().packets[user->action].len,data);
      }

      //Call specific function
      void (PacketHandler::*function)(uint8 *, User *)=PacketHandler::get().packets[user->action].function;
      (PacketHandler::get().*function)(data,user);

      //Release data array
      delete [] data;

      //Package completely received, remove from buffer
      user->buffer.erase(user->buffer.begin(), user->buffer.begin()+PacketHandler::get().packets[user->action].len);
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
  if (client_fd < 0)
    {
      LOG("Client: accept() failed");
      return;
    }
  User *client = addUser(client_fd,generateEID());
  setnonblock(client_fd);

  client->buf_ev = bufferevent_new(client_fd,
                                   buf_read_callback,
                                   buf_write_callback,
                                   buf_error_callback,
                                   client);

  bufferevent_enable(client->buf_ev, EV_READ);
}
