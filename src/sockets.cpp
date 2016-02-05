/*
   Copyright (c) 2013, The Mineserver Project
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

#include <string>

#include <sys/stat.h>
#include <fstream>

#include <cerrno>
#include <sstream>
#include <algorithm>


#include <array>

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
#include "extern.h"


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


bool client_write(User *user)
{
  if (user->deleting) return false;
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
        user->deleting = true;
        ServerInstance->usersToRemove().insert(user);
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
  if (user->deleting) return;

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
      user->deleting = true;
      ServerInstance->usersToRemove().insert(user);
      return;
    }

    if (read == SOCKET_ERROR)
    {
      LOG2(INFO, "Socket error");
      user->deleting = true;
      ServerInstance->usersToRemove().insert(user);
      return;
    }

    //Keep track on incoming data, can timeout inactive users
    user->lastData = std::time(NULL);

    user->buffer.addToRead(upBUF, read);
    user->buffer.reset();
    MS_VarInt packetLen;
    uint32_t varint_len;
    uint32_t buffer_pos = 0;

    // Handle exceptions caused by varint reading
    try
    {
      while (user->buffer >> (MS_VarInt&)packetLen)
      {  
        varint_len = user->buffer.m_readPos-buffer_pos;      

        // Check if we have the data in buffer
        if (!user->buffer.haveData(static_cast<int64_t>(packetLen)))
        {
          user->waitForData = true;
          event_add(user->getReadEvent(), NULL);
          return;
        }
        // Packet data has been received, call the function
        else
        {
          // Store packetlen to be used in the parsing functions (replaced later if compression used..)
          user->packetLen = (int32_t)packetLen;
          // Handle compressed incoming data
          if (user->compression)
          {
            MS_VarInt uncompressed_size;
            int32_t cur_pos = user->buffer.m_readPos;
            user->buffer >> (MS_VarInt&)uncompressed_size;

            // Hopefully a rare occasion when the packet size exceeds the compression threshold
            if (uncompressed_size != 0)
            {
              uLongf written = uncompressed_size;
              uLong sourceLen = (int)packetLen.val-(user->buffer.m_readPos-cur_pos);
              uint8_t* inbuffer = new uint8_t[sourceLen];
              uint8_t* buffer = new uint8_t[uncompressed_size+1];

              user->buffer.getData(inbuffer,sourceLen);           

              user->buffer.removePacketLen((uint32_t)packetLen+varint_len);
              int ret = uncompress(buffer, &written, inbuffer, sourceLen);
              if (ret < Z_OK || written != uncompressed_size) {
                LOG2(DEBUG, "Uncompress error");              
                buffer_pos = user->buffer.m_readPos;
                continue;
              }

              user->buffer.addToReadBegin(buffer, written);
              packetLen.val = written;
              varint_len = 0;
              delete[] inbuffer;
              delete[] buffer;

              // Replace the current packet length
              user->packetLen = (int32_t)uncompressed_size;
            }
          }
          MS_VarInt action;
          user->buffer >> (MS_VarInt&)action;
          user->action = (uint8_t)static_cast<int64_t>(action);

          if (ServerInstance->packetHandler()->packets[user->gameState][user->action].len == PACKET_DOES_NOT_EXIST) {
            std::ostringstream str;
            str << "Unknown packet: 0x" << std::hex << (unsigned int)(user->action);
            LOG2(DEBUG, str.str());
            user->buffer.removePacketLen((uint32_t)packetLen+varint_len);

            buffer_pos = user->buffer.m_readPos;
            continue;
          }
          user->packetsPerSecond++;
  #ifdef DEBUG
          printf("Packet from %s, state = 0x%hx, id = 0x%hx \n", user->nick.c_str(), user->gameState, user->action);
  #endif

          //Call specific function
          ServerInstance->packetHandler()->packets[user->gameState][user->action].function(user);

          const bool disconnecting = user->action == 0xFF;

          if (disconnecting) // disconnect -- player gone
          {
            if (user->nick.size()) {
              LOG2(INFO, "User " + user->nick + " disconnected normally");
            }
            user->deleting = true;
            ServerInstance->usersToRemove().insert(user);
            return;
          }   
        }
        user->buffer.removePacketLen((uint32_t)packetLen+varint_len);
        buffer_pos = user->buffer.m_readPos;
      } // while(user->buffer)
    } catch (std::logic_error &e)
    {
      user->waitForData = true;
      event_add(user->getReadEvent(), NULL);
    }
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

  client->setReadEvent(event_new(ServerInstance->getEventBase(), client_fd, EV_READ, client_callback, client));
  client->setWriteEvent(event_new(ServerInstance->getEventBase(), client_fd, EV_WRITE, client_callback, client));
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
