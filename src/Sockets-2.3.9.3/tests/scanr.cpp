#include <TcpSocket.h>
#include <SocketHandler.h>
#include <StdoutLog.h>
#include <iostream>


class tSocket : public TcpSocket
{
public:
	tSocket(ISocketHandler& h, const std::string& host, int port) : TcpSocket(h), m_host(host)
	{
		SetLineProtocol();
		Open(host, port);
	}

	void OnLine(const std::string& line)
	{
		printf("%s :: ", GetRemoteAddress().c_str());
		for (size_t i = 0; i < line.size(); i++)
		{
			if (isprint(line[i]) || line[i] == 13 || line[i] == 10)
			{
				printf("%c", line[i]);
			}
		}
		printf("\n");
	}

private:
	std::string m_host;
};


int main(int argc, char *argv[])
{
	std::string host = argc > 1 ? argv[1] : "www.alhem.net";
	int port = argc > 2 ? atoi(argv[2]) : 80;
	SocketHandler h;
	for (int i = 1; i < 255; i++)
	{
		char host[40];
		sprintf(host, "10.170.23.%d", i);
		tSocket *p = new tSocket(h, host, 23);
		h.Add(p);
	}
	while (h.GetCount())
	{
		h.Select(1, 0);
	}
}


