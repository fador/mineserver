#define _CRTDBG_MAPALLOC
#include "DisplayRawSocket.h"

DisplayRawSocket::DisplayRawSocket(ISocketHandler& h) : TcpSocket(h)
{
}


void DisplayRawSocket::OnRawData(const char *buf,size_t len)
{
	printf("Read %d bytes:\n",len);
	for (size_t i = 0; i < len; i++)
	{
		printf("%c",isprint(buf[i]) ? buf[i] : '.');
	}
	printf("\n");
}