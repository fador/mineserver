#include <EventHandler.h>
#include <TcpSocket.h>
#include <Utility.h>
#include <IEventOwner.h>
#include <ListenSocket.h>


class eHandler : public EventHandler
{
public:
	eHandler() : EventHandler() {}
	~eHandler() {
	}
};


#define ID_STOP_SOCKET 1
#define ID_STOP_LISTEN 2


class eSocket : public TcpSocket, public IEventOwner
{
public:
	eSocket(ISocketHandler& h) : TcpSocket(h), IEventOwner(static_cast<eHandler&>(h)), m_listen_socket(NULL), m_server(false) {
		SetLineProtocol();
	}
	~eSocket() {
	}

	bool OnConnectRetry() {
		printf("Retrying connect\n");
		if (GetConnectionRetries() == 3)
		{
			printf("Creating ListenSocket\n");
			m_listen_socket = new ListenSocket<eSocket>(Handler());
			if (m_listen_socket -> Bind(12345))
			{
				printf("Bind port 12345 failed\n");
			}
			m_listen_socket -> SetDeleteByHandler();
			Handler().Add( m_listen_socket );
		}
		return true;
	}

	void OnAccept() {
		m_events[AddEvent(5, 0)] = ID_STOP_SOCKET;
		m_server = true;
	}

	void OnConnect() {
		m_events[AddEvent(10, 0)] = ID_STOP_LISTEN;
	}

	void OnReconnect() {
		m_events[AddEvent(10, 0)] = ID_STOP_LISTEN;
	}

	void OnEvent(int id) {
		if (m_events[id] == ID_STOP_SOCKET && m_server)
		{
		printf("Event: Server disconnect\n");
			SetCloseAndDelete();
		}
		if (m_events[id] == ID_STOP_LISTEN && !m_server)
		{
		printf("Event: Stop listensocket\n");
			if (m_listen_socket)
			{
				m_listen_socket -> SetCloseAndDelete();
				m_listen_socket = NULL;
			}
		}
	}

	void OnLine(const std::string& line) {
		printf("  Incoming data: %s\n", line.c_str());
	}

	void OnDelete() {
		printf("eSocket::OnDelete(), server: %s\n", m_server ? "true" : "false");
	}

	void OnDisconnect() {
		printf("Disconnect, server: %s\n", m_server ? "true" : "false");
	}

private:
	ListenSocket<eSocket> *m_listen_socket;
	std::map<int, int> m_events;
	bool m_server;
};


class Sender : public IEventOwner
{
public:
	Sender(IEventHandler& h, TcpSocket& ref) : IEventOwner(h), m_socket(ref), m_count(1) {
	}

	void OnEvent(int id) {
		if (static_cast<eHandler&>(GetEventHandler()).Valid(&m_socket))
			m_socket.Send("Event#" + Utility::l2string(m_count++) + "\n");
		AddEvent(1, 0);
	}

private:
	TcpSocket& m_socket;
	int m_count;
};


int main(int argc, char *argv[])
{
	eHandler h;
	eSocket sock(h);
	sock.SetConnectTimeout(3);
	sock.SetConnectionRetry(-1);
#ifdef ENABLE_RECONNECT
	sock.SetReconnect();
#endif
	sock.Open("localhost", 12345);
	h.Add( &sock );
	Sender send(h, sock);
	h.AddEvent( &send, 1, 0 );
	h.EventLoop();
}

