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

#ifdef __WIN32__
#include <cstdlib>
#include <winsock2.h>
typedef int socklen_t;
#else
#include <netdb.h>       // for gethostbyname()
#include <netinet/tcp.h> // for TCP constants
#endif

#include <string>

/// FIXME: When this works
#ifdef __GNUC__
std::string to_string(int i){
    char buffer[512];
    itoa(i,buffer, 10);
    return buffer;
}

#endif

#include <sys/stat.h>
#include <fstream>

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
static std::array<uint8_t, BUFSIZE> BUF;
static std::array<uint8_t, BUFSIZE> BUFCRYPT;
static char* const cpBUF = reinterpret_cast<char*>(BUF.data());
static uint8_t* const upBUF = BUF.data();

static char* cpBUFCRYPT = reinterpret_cast<char*>(BUFCRYPT.data());

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

#ifdef DEBUG
        printf("Packet from %s, id = 0x%hx \n", user->nick.c_str(), user->action);
#endif

        if (disconnecting) // disconnect -- player gone
        {
          delete user;
          return;
        }
      }
      else if (ServerInstance->packetHandler()->packets[user->action].len == PACKET_DOES_NOT_EXIST)
      {
        using namespace std;
        stringstream str;
        str << "Unknown action: 0x" << std::hex << (unsigned int)(user->action);
        LOG2(DEBUG, str.str());


        str.str().clear();

        str << "0x" << std::hex << (unsigned int)(user->action);
        string hex = str.str();

        string fname = string("packet_") + hex + "_";

        struct stat st;
        for(int i=0;;i++){
            string tmp = fname + to_string(i) + ".txt";
            if(!stat(tmp.c_str(),&st))
                fname = tmp;
        }
        str.str().clear();
        str << "dumping packet data to: " << fname;
        LOG2(DEBUG, str.str());

        ofstream file(fname);

        for(int i= user->buffer.m_readPos; i< user->buffer.m_readBuffer.size(); i++){
            file << "0x" << std::hex << user->buffer.m_readBuffer[i] << ' ';
            if( i % 16 == 15)
                file<< '\n';
        }

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

#ifdef DEBUG
        printf("Packet from %s, id = 0x%hx \n", user->nick.c_str(), user->action);
#endif

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
    
    //More glue - Fador
    if(user->crypted)
    {      
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


  if(!user->bufferCrypted.getWriteEmpty())
  {
    std::vector<char> buf;
    user->bufferCrypted.getWriteData(buf);

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
      user->bufferCrypted.clearWrite(written);
    }

    //If we couldn't write everything at once, add EV_WRITE event calling this function again..
    if (!user->bufferCrypted.getWriteEmpty())
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

  struct timeval tv;
  tv.tv_sec = 5;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval));
  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(struct timeval));

  setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(int));

  if (sock == -1)
  {
    return 0;
  }

  if (connect(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1)
  {
    return 0;
  }

  return sock;
}

void *user_validation_thread(void *arg)
{
  Mineserver::userValidation *user = reinterpret_cast<Mineserver::userValidation *>(arg);
  user->valid=false;
  std::string url = "/game/checkserver.jsp?user=" + user->user->nick + "&serverId=" + user->user->generateDigest();
  LOG(INFO, "Packets", "Validating " + user->user->nick + " against minecraft.net: ");

  std::string http_request = "GET " + url + " HTTP/1.1\r\n"
                              + "Host: session.minecraft.net\r\n"
                              + "Connection: close\r\n\r\n";

  int fd = socket_connect((char*)"session.minecraft.net", 80);
  if (fd)
  {
#ifdef WIN32
      send(fd, http_request.c_str(), http_request.length(), 0);
#else
      write(fd, http_request.c_str(), http_request.length());
#endif

#define BUFFER_SIZE 1024
      char* buffer = new char[BUFFER_SIZE];
      std::string stringbuffer;

#ifdef WIN32
      while (int received = recv(fd, buffer, BUFFER_SIZE - 1, 0) != 0)
      {
#else
      while (read(fd, buffer, BUFFER_SIZE - 1) != 0)
      {
#endif
        stringbuffer += std::string(buffer);
      }
      delete[] buffer;
#ifdef WIN32
      closesocket(fd);
#else
      close(fd);
#endif

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
