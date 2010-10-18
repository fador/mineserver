#include <TcpSocket.h>
#include <ISocketHandler.h>

class DisplaySocket : public TcpSocket
{
public:
	DisplaySocket(ISocketHandler& );
    void OnDisconnect();
    void OnAccept();
	void OnRead();
};
