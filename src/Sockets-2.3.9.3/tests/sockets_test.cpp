#ifdef _WIN32
#pragma warning(disable:4786)
#endif
#include <StdoutLog.h>
#include <SocketHandler.h>
#include <TcpSocket.h>
#include <ListenSocket.h>
#include <Utility.h>
#include <Parse.h>
#include <HttpGetSocket.h>
#include <Socket.h>
#include <HttpDebugSocket.h>
#include <iostream>

#ifdef SOCKETS_NAMESPACE
using namespace SOCKETS_NAMESPACE;
#endif


class MyHandler : public SocketHandler
{
public:
	MyHandler(StdLog *p) : SocketHandler(p),m_done(false),m_quit(false) {}
	~MyHandler() {}

	void List(TcpSocket *p) {
		for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
		{
			Socket *p0 = (*it).second;
#ifdef ENABLE_POOL
			if (dynamic_cast<ISocketHandler::PoolSocket *>(p0))
			{
				p -> Send("PoolSocket\n");
			}
			else
#endif
			if (dynamic_cast<HttpGetSocket *>(p0))
			{
				p -> Send("HttpGetSocket\n");
			}
			else
			if (dynamic_cast<TcpSocket *>(p0))
			{
				p -> Send("TcpSocket\n");
			}
			else
			{
				p -> Send("Some kind of Socket\n");
			}
		}
	}
	void SetQuit() { m_quit = true; }
	bool Quit() { return m_quit; }
	void CheckHtml() {
		if (m_done)
		{
			if (m_ok)
				printf("Html OK:\n%s\n", m_html.c_str());
			else
				printf("Html Failed\n");
			m_done = false;
		}
	}

	std::string m_html;
	bool m_ok;
	bool m_done;

private:
	bool m_quit;
};


class MySocket : public TcpSocket
{
public:
	MySocket(ISocketHandler& h) : TcpSocket(h) {
	}
	void OnAccept() {
		int port = GetParent() -> GetPort();
		Send("I'm the server at port " + 
			Utility::l2string(port) + "\n");
		SetCloseAndDelete();
	}
};


class hSocket : public HttpGetSocket
{
public:
	hSocket(ISocketHandler& h,const std::string& x,const std::string& y) : HttpGetSocket(h,x,y) {}

	void OnConnect() {
		printf("hSocket::OnConnect\n");
		HttpGetSocket::OnConnect();
	}
};


class OrderSocket : public TcpSocket
{
public:
	OrderSocket(ISocketHandler& h) : TcpSocket(h) {
		SetLineProtocol();
	}
	Socket *Create() {
		Handler().LogError(this, "Create", 0, "OrderSocket", LOG_LEVEL_INFO);
		return new OrderSocket(Handler());
	}
	void OnAccept() {
		Send("Cmd (get,quit,list,stop,detach,count,resolve <name>)>");
	}
	void OnLine(const std::string& line) {
		Parse pa(line);
		std::string cmd = pa.getword();
		std::string arg = pa.getrest();
		try
		{
			if (cmd == "get")
			{
				HttpGetSocket *p = new hSocket(Handler(), arg, "tmpfile.html");
				p -> SetHttpVersion("HTTP/1.1");
				p -> AddResponseHeader("Connection", "keep-alive");
				p -> SetDeleteByHandler();
				Handler().Add( p );
				Send("Reading url '" + arg + "'\n");
			}
			else
			if (cmd == "quit")
			{
				Send("Goodbye!\n");
				SetCloseAndDelete();
			}
			else
			if (cmd == "list")
			{
				dynamic_cast<MyHandler&>(Handler()).List( this );
			}
			else
			if (cmd == "stop")
			{
				dynamic_cast<MyHandler&>(Handler()).SetQuit();
			}
			else
			if (cmd == "resolve")
			{
				//Resolve( arg );
				ipaddr_t a;
				if (Utility::u2ip(arg, a))
				{
					std::string tmp;
					Utility::l2ip(a, tmp);
					Send("Resolved: " + tmp + "\n");
				}
				else
				{
					Send("Resolve failed: " + arg + "\n");
				}
			}
			else
	/*
			if (cmd == "reverse")
			{
				ipaddr_t a;
				Utility::u2ip(arg, a); // ip -> ipaddr_t
				int id = Socket::Resolve(a, 0);
				Send("Resolve id = " + Utility::l2string(id) + "\n");
			}
			else
	*/
	#ifdef ENABLE_DETACH
			if (cmd == "detach")
			{
				if (!Detach())
				{
					Send("Detach() call failed\n");
				}
				else
				{
					Send("Ok.\n");
				}
			}
			else
	#endif
			if (cmd == "count")
			{
				Send("Socket count: " + Utility::l2string( (long)Handler().GetCount()) + "\n");
			}
			else
			{
				Send("Huh?\n");
			}
		}
		catch (const std::exception& e)
		{
			Send(e.what() + std::string("\n"));
		}
		Send("Cmd>");
	}
	void OnDelete() {
		printf("OrderSocket::OnDelete()\n");
	}
#ifdef ENABLE_RESOLVER
	void OnResolved(int id,ipaddr_t a,port_t port)
	{
	}
	void OnResolved(int id,const std::string& name,port_t port)
	{
		Send("Resolve id " + Utility::l2string(id) + " = " + name + "\n");
	}
/*
	void OnResolved(const char *p,size_t l) {
		printf("OnResolved, %d bytes:\n", l);
		for (size_t i = 0; i < l; i++)
		{
			unsigned char c = p[i];
			if (isprint(c))
				printf("%c",c);
			else
				printf("<%02X>",c);
		}
		printf("\n");
	}
*/
#endif
#ifdef ENABLE_DETACH
	void OnDetached() {
		Send("\nDetached.\nCmd>");
	}
#endif
};


class TestSocket : public TcpSocket
{
public:
	TestSocket(ISocketHandler& h) : TcpSocket(h) {
		SetLineProtocol();
	}
	void OnConnect() {
		printf("TestSocket connected, sending QUIT\n");
		Send( "quit\n" );
	}
	void OnConnectFailed() {
		printf("TestSocket::OnConnectFailed\n");
		SetCloseAndDelete();
	}
	void OnLine(const std::string& line) {
		printf("TestSocket: %s\n", line.c_str());
	}
	void OnDelete() {
		printf("TestSocket::OnDelete()\n");
	}
#ifdef ENABLE_RESOLVER
	void OnResolved(int id,ipaddr_t a,port_t port) {
		printf("TestSocket::OnResolved():  %d,  %08x:%d\n", id, a, port);
		TcpSocket::OnResolved(id,a,port);
	}
#endif
};


int main()
{
	try
	{
		StdoutLog log;
		MyHandler h(&log);

#ifdef ENABLE_RESOLVER
		h.EnableResolver(9999);
#endif
//	Utility::ResolveLocal();
		printf(" *** My hostname: %s\n", Utility::GetLocalHostname().c_str());
		printf(" *** My local IP: %s\n", Utility::GetLocalAddress().c_str());

		// socks4 options
/*
		h.SetSocks4Host("127.0.0.1");
		h.SetSocks4Port(1080);
		h.SetSocks4Userid("www.alhem.net");
		h.SetSocks4TryDirect( true );
		printf("Socks4Host: %x\n", h.GetSocks4Host());
*/

		// first server
		ListenSocket<MySocket> l1(h);
		if (l1.Bind(1024))
		{
			printf("Bind 1024 failed\n");
			exit(-1);
		}
		h.Add(&l1);

		// second server
		ListenSocket<MySocket> l2(h);
		if (l2.Bind(1025))
		{
			printf("Bind 1025 failed\n");
			exit(-1);
		}
		h.Add(&l2);

		// line server
		ListenSocket<OrderSocket> l3(h);
		if (l3.Bind(1027))
		{
			printf("Bind 1027 failed\n");
			exit(-1);
		}
		h.Add(&l3);

		// http debug
		ListenSocket<HttpDebugSocket> l4(h);
		if (l4.Bind(8080))
		{
			printf("Bind 8080 failed\n");
			exit(-1);
		}
		h.Add(&l4);

		// wait for resolver to really start
#ifdef ENABLE_RESOLVER
		printf("Waiting for resolver ...");
		while (!h.ResolverReady())
			;
		printf(" resolver ready!\n");
#endif

		TestSocket ts(h);
printf(">>> TestSocket.Open\n");
		ts.Open("localhost", 1027);
printf(">>> Adding TestSocket\n");
		h.Add(&ts);

printf(">>> Enter mainloop\n");
		h.Select(0,0);
		while (!h.Quit())
		{
			h.Select(1,0);
		}
printf(">>> Leaving mainloop\n");

		return 0;
	}
	catch (const Exception& e)
	{
		std::cerr << e.ToString() << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}


