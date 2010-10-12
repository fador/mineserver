#ifndef _STATUSHANDLER_H
#define _STATUSHANDLER_H

#include <SocketHandler.h>
#include <TcpSocket.h>


class StatusHandler : public SocketHandler
{
public:
	StatusHandler();

    void SendAll(std::string data);
    void SendSock(SOCKET sock, void *data, int len);

};


#endif // _STATUSHANDLER_H