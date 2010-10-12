#include <SocketHandler.h>
#include <TcpSocket.h>


class GetHttp : public TcpSocket
{
public:
	GetHttp(ISocketHandler& h, const char *request) : TcpSocket(h)
	, m_request(request) {}

	void OnConnect() {
		printf("... connected\n");
		Send( m_request );
	}

	void OnRawData( const char *buf, size_t len ) {
		printf("... data\n");
		if (len > 0) {
			std::string tmp;
			tmp.resize( len );
			memcpy( &tmp[0], buf, len );
			m_response += tmp;
		}
	}

	const std::string& Response() {
		return m_response;
	}

private:
	std::string m_request;
	std::string m_response;
};


std::string get_http(const char *host, int port, const char *request)
{
	SocketHandler h;
	GetHttp sock(h, request);
	sock.Open( host, port );
	h.Add(&sock);
	while (h.GetCount()) {
		h.Select(1, 0);
	}
	return sock.Response();
}


int main(int argc, char *argv[])
{
	std::string zz = get_http("www.alhem.net", 80, "GET /index.html HTTP/1.0\r\n"
		"Host: www.alhem.net\r\n"
		"Connection: close\r\n"
		"\r\n");
	printf("%s\n%d\n", zz.c_str(), zz.size());
}


