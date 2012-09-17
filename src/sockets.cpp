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

#ifdef _WIN32
#include <cstdlib>
#include <winsock2.h>
typedef int socklen_t;
#else
#include <netdb.h>   // for gethostbyname()
#include <netinet/tcp.h> // for TCP constants
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
#include "config.h"


extern int setnonblock(int fd);

#ifndef WIN32
#define SOCKET_ERROR -1
#endif

static const size_t BUFSIZE = 2048;
static std::tr1::array<uint8_t, BUFSIZE> BUF;
static std::tr1::array<uint8_t, BUFSIZE> BUFCRYPT;
static char* const cpBUF = reinterpret_cast<char*>(BUF.data());
static uint8_t* const upBUF = BUF.data();

static char* cpBUFCRYPT = reinterpret_cast<char*>(BUFCRYPT.data());


bool client_write(User *user)
{
  //If there is data in the output buffer, crypt it before writing
  if (!user->buffer.getWriteEmpty())
  {
    std::vector<char> buf;
    
    user->buffer.getWriteData(buf);
    
    //More glue - Fador
    if(user->crypted)
    {
      //We might have to write some data uncrypted ToDo: fix
      if(user->uncryptedLeft)
      {
        user->bufferCrypted.addToWrite((uint8_t *)buf.data(),user->uncryptedLeft);
      }
      int p_len = buf.size()-user->uncryptedLeft, f_len = 0;
      if(p_len)
      {
        uint8_t *buffer = (uint8_t *)malloc(p_len+1);
        EVP_EncryptUpdate(&user->en, (uint8_t *)buffer, &p_len, (const uint8_t *)buf.data()+user->uncryptedLeft, buf.size()-user->uncryptedLeft);
        int written = p_len + f_len;
        user->bufferCrypted.addToWrite((uint8_t *)buffer,written);
        free(buffer);
      }
      user->uncryptedLeft = 0;
    }
    else
    {
      user->bufferCrypted.addToWrite((uint8_t *)buf.data(),buf.size());
      user->uncryptedLeft = 0;
    }

    //free(outBuf);
    user->buffer.clearWrite(buf.size());
  }

  //We have crypted data ready to be written
  if(!user->bufferCrypted.getWriteEmpty())
  {
    std::vector<char> buf;
    user->bufferCrypted.getWriteData(buf);

    //Try to write the whole buffer
    const int written = send(user->fd, buf.data(), buf.size(), 0);

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
        return false;
      }
    }
    else
    {
      //Remove written amount from the buffer
      user->bufferCrypted.clearWrite(written);
    }

    //If we couldn't write everything at once, add EV_WRITE event calling this function again..
    if (!user->bufferCrypted.getWriteEmpty())
    {
      event_add(user->getWriteEvent(), NULL);
      return false;
    }
  }
  return true;
}

extern "C" void client_callback(int fd, short ev, void* arg)
{
  User* user = reinterpret_cast<User*>(arg);

  if (ev & EV_READ)
  {
    int read = 1;

    //Data must be decrypted if we are in crypted mode
    if(user->crypted)
    {
      read = recv(fd, cpBUFCRYPT, BUFSIZE, 0);
      int p_len = read, f_len = 0;  
      EVP_DecryptUpdate(&user->de, upBUF, &p_len, (const uint8_t *)cpBUFCRYPT, read);
      read = p_len + f_len;
    }
    else
    {
      read = recv(fd, cpBUF, BUFSIZE, 0);
    }

    if (read == 0)
    {
      if(user->nick.size())
      {
        LOG2(INFO, "User "+ user->nick + " disconnected by closing socket");
      }
      else
      {
        LOG2(INFO, "Socket closed");
      }
      delete user;
      return;
    }

    if (read == SOCKET_ERROR)
    {
      LOG2(INFO, "Socket error");
      delete user;
      return;
    }

    //Keep track on incoming data, can timeout inactive users
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
          event_add(user->getReadEvent(), NULL);
          return;
        }

        if (disconnecting) // disconnect -- player gone
        {
          if(user->nick.size())
          {
            LOG2(INFO, "User "+ user->nick + " disconnected normally");
          }
          delete user;
          return;
        }
      }
      else if (ServerInstance->packetHandler()->packets[user->action].len == PACKET_DOES_NOT_EXIST)
      {
        std::ostringstream str;
        str << "Unknown packet: 0x" << std::hex << (unsigned int)(user->action);
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
          event_add(user->getReadEvent(), NULL);
          return;
        }

        //Call specific function
        ServerInstance->packetHandler()->packets[user->action].function(user);
      }
    } // while(user->buffer)
  } //End reading

  //Write data to user socket
  if(!client_write(user))
  {
    return;
  }  

  //Add EV_READ event again
  event_add(user->getReadEvent(), NULL);
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

  //Keep delay minimum: more (smaller) packets -> less lag
  int one = 1;
  setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&one, sizeof(int));

  event_set(client->getReadEvent(), client_fd, EV_READ, client_callback, client);
  event_set(client->getWriteEvent(), client_fd, EV_WRITE, client_callback, client);
  event_add(client->getReadEvent(), NULL);
}

//Opens a socket and connects to the host/port
int socket_connect(char* host, int port)
{
  struct hostent* hp;
  struct sockaddr_in addr;
  int on = 1, sock;

  if ((hp = gethostbyname(host)) == NULL)
  {
    return 0;
  }

  memmove(&addr.sin_addr, hp->h_addr, hp->h_length);
  addr.sin_port = htons(port);
  addr.sin_family = AF_INET;
  sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  //Set 5s timeout
  struct timeval tv;
  tv.tv_sec = 5;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval));
  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(struct timeval));
  setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(int));

  if (sock == -1 || connect(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1)
  {
    return 0;
  }

  return sock;
}

//Connects to session server and verifies the client
//Will store output thread-safely to array which is read in the main loop
void *user_validation_thread(void *arg)
{
  Mineserver::userValidation *user = reinterpret_cast<Mineserver::userValidation *>(arg);
  user->valid=false;
  std::string url = "/game/checkserver.jsp?user=" + user->user->nick + "&serverId=" + user->user->generateDigest();
  LOG(INFO, "Packets", "Validating " + user->user->nick + " against session.minecraft.net: ");

  std::string http_request = "GET " + url + " HTTP/1.1\r\n"
                            + "Host: session.minecraft.net\r\n"
                            + "Connection: close\r\n\r\n";

  int fd = socket_connect((char*)"session.minecraft.net", 80);
  if (fd)
  {
    send(fd, http_request.c_str(), http_request.length(), 0);

    #define BUFFER_SIZE 1024
    char* buffer = new char[BUFFER_SIZE];
    std::string stringbuffer;

    while (int received = recv(fd, buffer, BUFFER_SIZE - 1, 0) != 0)
    {
      stringbuffer += std::string(buffer);
    }
    delete[] buffer;
#ifdef WIN32
    closesocket(fd);
#else
    close(fd);
#endif
    //session server will return "YES" if user is valid, have to skip HTTP headers
    if ((stringbuffer.size() >= 3 && stringbuffer.find("\r\n\r\nYES", 0) != std::string::npos))
    {  
      user->valid = true;
    }
  }
  pthread_mutex_lock(&ServerInstance->m_validation_mutex);
  ServerInstance->validatedUsers.push_back(*user);
  pthread_mutex_unlock(&ServerInstance->m_validation_mutex);
  user->user = NULL;
  delete user;
  pthread_exit(NULL);
  return NULL;
}
