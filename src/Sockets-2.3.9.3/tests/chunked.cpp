#include <SocketHandler.h>
#include <TcpSocket.h>
#include <ListenSocket.h>
#include <iostream>


class Server : public TcpSocket
{
public:
	Server(ISocketHandler& h) : TcpSocket(h), m_header(true) {
		SetLineProtocol();
	}

	void OnLine(const std::string& line) {
		if (m_header && line.empty())
		{
			m_header = false;
			Send("HTTP/1.1 200 OK\r\n"
//			"Server: Microsoft-IIS/5.0\r\n"
//			"Date: Mon, 15 Aug 2005 11:39:18 GMT\r\n"
			"Content-Type: text/html\r\n"
//			"Set-Cookie: ASPSESSIONIDASTATTTA=PIMPPEJBIMEGJLHOBNPHMIDC; path=/\r\n"
//			"Cache-control: private\r\n"
			"Transfer-Encoding: chunked\r\n"
			"\r\n"
			"a\r\n"
			"XXXXXXXXXX\r\n"
			"a\r\n"
			"XXXXXXXXXX\r\n"
			"a\r\n"
			"XXXXXXXXXX\r\n"
			"a\r\n"
			"XXXXXXXXXX\r\n"
			"a\r\n"
			"XXXXXXXXXX\r\n"
			"0\r\n"
			"\r\n");
		}
	}

private:
	bool m_header;
};


int main()
{
	SocketHandler h;
	ListenSocket<Server> l(h);
	try
	{
		l.Bind(12345);
		h.Add(&l);
		while (true)
		{
			h.Select(1, 0);
		}
	}
	catch (const Exception& e)
	{
		std::cout << e.ToString() << std::endl;
	}
}


