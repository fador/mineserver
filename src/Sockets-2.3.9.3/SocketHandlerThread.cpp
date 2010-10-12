/**
 **	\file SocketHandlerThread.cpp
 **	\date  2010-03-21
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2010  Anders Hedstrom

This library is made available under the terms of the GNU GPL.

If you would like to use this library in a closed-source application,
a separate license agreement is available. For information about 
the closed-source license agreement for the C++ sockets library,
please visit http://www.alhem.net/Sockets/license.html and/or
email license@alhem.net.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "SocketHandlerThread.h"
#include "Mutex.h"
#include "ISocketHandler.h"


#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


SocketHandlerThread::SocketHandlerThread(ISocketHandler& parent) : Thread(false)
, m_parent(parent)
, m_handler(NULL)
{
}


SocketHandlerThread::~SocketHandlerThread()
{
}


ISocketHandler& SocketHandlerThread::Handler()
{
  return *m_handler;
}


void SocketHandlerThread::Run()
{
  Mutex mutex;
  m_handler = m_parent.Create(mutex, m_parent);
  m_sem.Post();
  ISocketHandler& h = *m_handler;
  h.EnableRelease();
  while (IsRunning())
  {
    h.Select(1, 0);
  }
}


void SocketHandlerThread::Wait()
{
  m_sem.Wait();
}


#ifdef SOCKETS_NAMESPACE
} // namespace SOCKETS_NAMESPACE {
#endif
