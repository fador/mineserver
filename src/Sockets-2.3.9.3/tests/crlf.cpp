#include <TcpSocket.h>
#include <SocketHandler.h>
#include <ListenSocket.h>


static	bool quit = false;


class IOSocket : public TcpSocket
{
public:
	IOSocket(ISocketHandler& h) : TcpSocket(h) {
		SetLineProtocol();
	}
	~IOSocket() {}

	void OnConnect() {
		Utility::GetTime(&m_start);
		while (GetOutputLength() < 500000)
		{
			Send("zzzzzzzzzmmmmmmmmm1234lkkk54\r\n");
			Send("zzzzzzzzzmmmmmmmmm1234lkkk543\r\n");
			Send("zzzzzzzzzmmmmmmmmm1234lkkk54\r\n");
			Send("zzzzzzzzzmmmmmmmmm1234lkkk543\r\n");
			Send("zzzzzzzzzmmmmmmmmm1234lkkk54\r\n");
			Send("zzzzzzzzzmmmmmmmmm1234lkkk543\r\n");
			Send("zzzzzzzzzmmmmmmmmm1234lkkk54\r\n");
			Send("zzzzzzzzzmmmmmmmmm1234lkkk543\r\n");
		}
//		Send("\r\n");
	}

	void OnAccept() {
		SetLineProtocol();
	}

	void OnDelete() {
		printf("OnDelete\n");
	}

	void OnWriteComplete() {
printf("OnWriteComplete\n");
		struct timeval tv;
		Utility::GetTime(&tv);

		tv.tv_sec -= m_start.tv_sec;
		tv.tv_usec -= m_start.tv_usec;
		Utility::GetTime(&m_start);
		if (tv.tv_usec < 0)
		{
			tv.tv_usec += 1000000;
			tv.tv_sec -= 1;
		}
		double t = tv.tv_usec;
		t /= 1000000;
		t += tv.tv_sec;
		printf("  Time: %ld.%06ld (%f)\n", tv.tv_sec, tv.tv_usec, t);
		double r = GetBytesReceived();
		printf("  bytes in: %lld (%f Mbytes/sec)\n", GetBytesReceived(true), r / t / 1000000);
		double s = GetBytesSent();
		printf("  bytes out: %lld (%f Mbytes/sec)\n", GetBytesSent(true), s / t / 1000000);
		printf("\n");

		while (GetOutputLength() < 500000)
		{
			Send("zzzzzzzzzmmmmmmmmm1234lkkk54\r\n");
			Send("zzzzzzzzzmmmmmmmmm1234lkkk543\r\n");
			Send("zzzzzzzzzmmmmmmmmm1234lkkk54\r\n");
			Send("zzzzzzzzzmmmmmmmmm1234lkkk543\r\n");
			Send("zzzzzzzzzmmmmmmmmm1234lkkk54\r\n");
			Send("zzzzzzzzzmmmmmmmmm1234lkkk543\r\n");
			Send("zzzzzzzzzmmmmmmmmm1234lkkk54\r\n");
			Send("zzzzzzzzzmmmmmmmmm1234lkkk543\r\n");
		}
//		Send("\r\n");
	}

	void OnLine(const std::string& line) {
		if (line == "")
		{
			struct timeval tv;
			Utility::GetTime(&tv);

			tv.tv_sec -= m_start.tv_sec;
			tv.tv_usec -= m_start.tv_usec;
			Utility::GetTime(&m_start);
			if (tv.tv_usec < 0)
			{
				tv.tv_usec += 1000000;
				tv.tv_sec -= 1;
			}
			double t = tv.tv_usec;
			t /= 1000000;
			t += tv.tv_sec;
			printf("  Time: %ld.%06ld (%f)\n", tv.tv_sec, tv.tv_usec, t);
			double r = GetBytesReceived();
			printf("  bytes in: %lld (%f Mbytes/sec)\n", GetBytesReceived(true), r / t / 1000000);
			double s = GetBytesSent();
			printf("  bytes out: %lld (%f Mbytes/sec)\n", GetBytesSent(true), s / t / 1000000);
			printf("\n");

		}
		else
		if (line != "zzzzzzzzzmmmmmmmmm1234lkkk54" && 
			line != "zzzzzzzzzmmmmmmmmm1234lkkk543")
		{
printf("\n-------------------------------------------------------\n");
			for (size_t i = 0; i < line.size(); i++)
				if (!isprint(line[i]))
					printf("<%d>", line[i]);
				else
					printf("%c", line[i]);
			printf("\n");
fflush(stdout);
exit(-1);
			quit = true;
		}
	}

	struct timeval m_start;
};


int main(int argc, char *argv[])
{
	SocketHandler h;
	ListenSocket<IOSocket> l(h);
	l.Bind(12344);
	h.Add(&l);
	IOSocket sock(h);
	sock.Open("192.168.7.4", 12344);
	h.Add(&sock);
	while (!quit)
	{
		h.Select(1, 0);
	}
}


