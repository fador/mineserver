#include <SocketHandler.h>
#include <TcpSocket.h>
#include <StdoutLog.h>
#ifndef _WIN32
#include <signal.h>
#endif
#include <IEventOwner.h>
#include <EventHandler.h>

#define HOST "localhost"
#define PORT 2222

#define TIME_US 250000

#define DEB(x) x


static	bool quit = false;
static	EventHandler *ph = NULL;

static	int lc = 0;
static	int l2c = 0;
static	int l3c = 0;


#ifndef _WIN32
void sighandler(int s)
{
	quit = true;
	ph -> SetQuit();
}


void sigpipe(int s)
{
}
#endif


class evHandler : public EventHandler
{
public:
	evHandler() : EventHandler() {}
	evHandler(StdLog *p) : EventHandler(p) {}

	void CloseAll() {
		for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
		{
			Socket *p = it -> second;
			p -> SetCloseAndDelete();
		}
	}

};


class lSocket : public TcpSocket, public IEventOwner
{
public:
	lSocket(ISocketHandler& h, bool first = false) : TcpSocket(h), IEventOwner(static_cast<evHandler&>(h)), m_id(0), m_first(first) {
		SetLineProtocol();
		lc++;
	}
	~lSocket() {
	}

	void OnConnect() {
		if (!m_id)
		{
			m_id = AddEvent(0, TIME_US);
		}
		printf(".");
		fflush(stdout);
	}
	void OnConnectFailed() {
		printf("lSocket::OnConnectFailed\n");
	}

	void OnDelete() {
		if (m_first)
			printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"
				"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"
				"@@@@@@@@@@@@@@ OnDelete() first @@@@@@@@@@@@@@@@@@\n"
				"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"
				"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	}

	void OnEvent(int id) {
		if (id != m_id)
		{
			printf("ID MISMATCH!\n");
			exit(-1);
		}
		if (!quit)
		{
			lSocket *p = new lSocket(Handler());
			p -> SetDeleteByHandler();
			p -> Open(HOST, PORT);
			Handler().Add(p);
		}
		static_cast<evHandler&>(Handler()).SetQuit( quit );
		SetCloseAndDelete();
	}

private:
	int m_id;
	bool m_first;
};


class l2Socket : public TcpSocket, public IEventOwner
{
public:
	l2Socket(ISocketHandler& h, bool first = false) : TcpSocket(h), IEventOwner(static_cast<evHandler&>(h)), m_id(0), m_first(first) {
		SetLineProtocol();
		l2c++;
	}
	~l2Socket() {
	}

	void OnConnect() {
		printf(".");
		fflush(stdout);
	}
	void OnConnectFailed() {
		printf("l2Socket::OnConnectFailed\n");
	}

	void OnLine(const std::string& line) {
		if (!m_id)
		{
			m_id = AddEvent(0, TIME_US);
		}
	}

	void OnDelete() {
		if (m_first)
			printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"
				"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"
				"@@@@@@@@@@@@@@ OnDelete() first @@@@@@@@@@@@@@@@@@\n"
				"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"
				"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	}

	void OnEvent(int id) {
		if (id != m_id)
		{
			printf("ID MISMATCH!\n");
			exit(-1);
		}
		if (!quit)
		{
			l2Socket *p = new l2Socket(Handler());
			p -> SetDeleteByHandler();
			p -> Open(HOST, PORT);
			Handler().Add(p);
		}
		static_cast<evHandler&>(Handler()).SetQuit( quit );
		SetCloseAndDelete();
	}

private:
	int m_id;
	bool m_first;
};


class l3Socket : public TcpSocket, public IEventOwner
{
public:
	l3Socket(ISocketHandler& h, bool first = false) : TcpSocket(h), IEventOwner(static_cast<evHandler&>(h)), m_ehlo(false), m_id(0), m_first(first) {
		SetLineProtocol();
		l3c++;
	}
	~l3Socket() {
	}

	void OnConnect() {
		printf(".");
		fflush(stdout);
	}
	void OnConnectFailed() {
		printf("l3Socket::OnConnectFailed\n");
	}

	void OnLine(const std::string& line) {
		if (!m_ehlo)
		{
			Send("EHLO alhem.net\r\n");
			m_ehlo = true;
		}
		else
		if (!m_id)
		{
			m_id = AddEvent(0, TIME_US);
		}
	}

	void OnDelete() {
		if (m_first)
			printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"
				"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"
				"@@@@@@@@@@@@@@ OnDelete() first @@@@@@@@@@@@@@@@@@\n"
				"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"
				"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	}
	
	void OnEvent(int id) {
		if (id != m_id)
		{
			printf("ID MISMATCH!\n");
			exit(-1);
		}
		if (!quit)
		{
			l3Socket *p = new l3Socket(Handler());
			p -> SetDeleteByHandler();
			p -> Open(HOST, PORT);
			Handler().Add(p);
		}
		static_cast<evHandler&>(Handler()).SetQuit( quit );
		Send("QUIT\r\n");
	}

private:
	bool m_ehlo;
	int m_id;
	bool m_first;
};


int main(int argc, char *argv[])
{
#ifndef _WIN32
	signal(SIGHUP, sighandler);
	signal(SIGINT, sighandler);
	signal(SIGPIPE, sigpipe);
#endif

	time_t tstart = time(NULL);
	{
		StdoutLog log;
		{
			evHandler h(&log);
			ph = &h;
			{
				lSocket sock(h, true);
				sock.Open(HOST, PORT);
				h.Add(&sock);
				{
					l2Socket sock2(h, true);
					sock2.Open(HOST, PORT);
					h.Add(&sock2);
					{
						l3Socket sock3(h, true);
						sock3.Open(HOST, PORT);
						h.Add(&sock3);

						h.EventLoop();
						h.CloseAll();

						// %! one socket refuses to go away
						while (h.GetCount() > 1)
						{
							h.Select(1, 0);
						}
						printf("Exiting 1...\n");
						fflush(stdout);
					}
					printf("Exiting 2...\n");
					fflush(stdout);
				}
				printf("Exiting 3...\n");
				fflush(stdout);
			}
			printf("Exiting 4...\n");
			fflush(stdout);
		}
		printf("Exiting 5...\n");
		fflush(stdout);
	}
	printf("Exiting 6...\n");
	fflush(stdout);

	time_t tstop = time(NULL);

	time_t total = tstop - tstart + 1;

	printf("lc:   %d\n", lc);
	printf("l2c:  %d\n", l2c);
	printf("l3c:  %d\n", l3c);

	printf("lc:   %f\n", (double)lc / (double)total);
	printf("l2c:  %f\n", (double)l2c / (double)total);
	printf("l3c:  %f\n", (double)l3c / (double)total);
}


