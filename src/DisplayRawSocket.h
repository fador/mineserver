#include <TcpSocket.h>
#include <ISocketHandler.h>

class DisplayRawSocket : public TcpSocket
{
public:
	DisplayRawSocket(ISocketHandler& );

	void OnRawData(const char *,size_t);
};
