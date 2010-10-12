#include <HTTPSocket.h>
#include <SocketHandler.h>
#include <StdoutLog.h>


class tSocket : public HTTPSocket
{
public:
	tSocket(ISocketHandler& h, const std::string& host, int port) : HTTPSocket(h), m_host(host)
	, m_first(false)
	, m_sz(0) {
		Open(host, port);
	}

	void OnConnect() {
		Send("GET /index.html HTTP/1.0\r\n");
		Send("Host: " + m_host + "\r\n");
		Send("\r\n");
	}
	void OnDisconnect() { // or OnDelete()
		Handler().LogError(this, "Disconnect", 0, "ServerSocket", LOG_LEVEL_INFO);
	}

	void OnFirst() {
	}

	void OnHeader(const std::string& key, const std::string& value) {
		fprintf(stderr, "%s: %s\n", key.c_str(), value.c_str());
	}

	void OnHeaderComplete() {
		fprintf(stderr, "\n");
	}

	void OnData(const char *buf, size_t sz) {
printf(" %d bytes\n", sz);
		if (1||!m_first)
		{
			std::string str = buf;
			str.resize( sz );
			printf("%s", str.c_str());
			m_first = true;
		}
		m_sz += sz;
	}

	void OnDelete() {
		fprintf(stderr, "Content length: %d\n", m_sz);
	}

private:
	std::string m_host;
	bool m_first;
	size_t m_sz;
};


int main(int argc, char *argv[])
{
	std::string host = argc > 1 ? argv[1] : "www.alhem.net";
	int port = argc > 2 ? atoi(argv[2]) : 80;
	StdoutLog log;
	SocketHandler h(&log);
	tSocket sock(h, host, port);
	h.Add(&sock);
	while (h.GetCount())
	{
		h.Select(1, 0);
	}
}


