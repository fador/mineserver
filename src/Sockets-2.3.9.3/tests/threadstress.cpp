#include <TcpSocket.h>
#include <Exception.h>
#include <Thread.h>
#include <SocketHandler.h>
#include <ListenSocket.h>
#include <StdoutLog.h>
#include <signal.h>
#include <iostream>

#define DEB(x) x

	int count = 0;

class EchoSocket : public TcpSocket
{
public:
	EchoSocket(ISocketHandler& h) : TcpSocket(h), m_client(false) {
		SetLineProtocol();
	}

	void OnConnect() {
		m_uid = "Test";
		Send(m_uid + "\n");
		m_client = true;
	}

	void OnLine(const std::string& line) {
		if (m_client)
		{
			if (line != m_uid)
				throw Exception("Bad data returned.");
			if (m_uid.size() < 8192)
				m_uid = line + "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[random() % 26];
			else
				SetCloseAndDelete();
			Send(m_uid + "\n");
std::cout << m_uid << std::endl;
		}
		else
		{
			Send(line + "\n");
		}
	}

	void Init()
	{
		if (GetParent() -> GetPort() >= 4000)
		{
#ifdef HAVE_OPENSSL
			EnableSSL();
#else
			throw Exception("SSL not available");
#endif
		}
	}

#ifdef HAVE_OPENSSL
	void InitSSLServer()
	{
		InitializeContext("threadstress", "comb.pem", "", SSLv23_method());
	}
#endif

private:
	std::string m_uid;
	bool m_client;
};

class StressThread : public Thread
{
public:
	StressThread(port_t port) : Thread(), m_port(port) {
		SetDeleteOnExit();
		++count;
	}
	~StressThread() {
		--count;
	}

	void Run() {
		try
		{
			StdoutLog log;
			SocketHandler h(&log);
			ListenSocket<EchoSocket> l(h);
			l.Bind(m_port);
			h.Add(&l);
			EchoSocket client(h);
			client.EnableSSL();
			client.Open("localhost", m_port);
			h.Add(&client);
			while (h.GetCount() > 1)
			{
				h.Select(1, 0);
			}
			std::cout << "Port#" << m_port << ": Leaving thread" << std::endl;
		}
		catch (const Exception& e)
		{
			std::cerr << "Port#" << m_port << ": " << e.ToString() << std::endl;
		}
	}

private:
	port_t m_port;
};

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s <number of threads>\n", *argv);
		return -1;
	}
	signal(SIGPIPE, SIG_IGN);
	for (int i = 0; i < atoi(argv[1]); i++)
		new StressThread(4000 + i);
	while (count > 0)
	{
		sleep(1);
	}
}

