#define _CRTDBG_MAPALLOC
#include <stdarg.h>
#include "DisplaySocket.h"
#include "StatusHandler.h"


StatusHandler::StatusHandler()
:SocketHandler()
{
}


#define SIZE 5000


void StatusHandler::SendAll(std::string data)
{

    for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
	{
		Socket *p0 = (*it).second;
		DisplaySocket *p = dynamic_cast<DisplaySocket *>(p0);
		if (p)
		{
			p->Send(data);
		}
	}


}

void StatusHandler::SendSock(SOCKET sock, void *data, int len)
{

		Socket *p0 = m_sockets[sock];
		DisplaySocket *p = dynamic_cast<DisplaySocket *>(p0);
		if (p)
		{
			p->SendBuf((const char *)data, len);
		}
}
