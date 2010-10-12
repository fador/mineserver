#include <TcpSocket.h>
#include <SocketHandler.h>
#include <ListenSocket.h>
#include <Utility.h>

#define BUFSZ 7024

#ifdef _WIN32
#define MIN(a,b) (a<b?a:b)

#endif

static	int quit = 0;
static	size_t lim = 120000;


class CopySocket : public TcpSocket
{
public:
	CopySocket(ISocketHandler& h) : TcpSocket(h), m_fil(NULL), m_out(NULL), m_b_server(false), m_sz(0) {
		SetTransferLimit(60000);
	}
	CopySocket(ISocketHandler& h, size_t sz) : TcpSocket(h), m_fil(NULL), m_out(NULL), m_b_server(false), m_sz(sz) {
		SetTransferLimit(60000);
		quit++;
	}
	CopySocket(ISocketHandler& h, const std::string& filename) : TcpSocket(h), m_filename(filename), m_fil(NULL), m_out(NULL), m_b_server(false), m_sz(0) {
		SetTransferLimit(60000);
		quit++;
	}
	~CopySocket() {
		quit--;
	}

	void OnConnect() {
		Utility::GetTime(&m_start);
		if (!m_filename.size())
		{
			Send("\n");
			char buf[BUFSZ];
			int n = MIN(m_sz, BUFSZ);
			while (n > 0 && GetOutputLength() < lim)
			{
				SendBuf(buf, n);
				m_sz -= n;
				n = MIN(m_sz, BUFSZ);
			}
			if (!n)
			{
				SetCloseAndDelete();
			}
			else
			{
				SendBuf(buf, n);
				m_sz -= n;
			}
			return;
		}
		size_t x = 0;
		for (size_t i = 0; i < m_filename.size(); i++)
			if (m_filename[i] == '/')
				x = i + 1;
		Send(m_filename.substr(x) + "\n");
		m_fil = fopen(m_filename.c_str(), "rb");
		if (m_fil)
		{
			char buf[BUFSZ];
			int n = fread(buf, 1, BUFSZ, m_fil);
			while (n > 0 && GetOutputLength() < lim)
			{
				SendBuf(buf, n);
				n = fread(buf, 1, BUFSZ, m_fil);
			}
			if (!n)
			{
				SetCloseAndDelete();
				fclose(m_fil);
				m_fil = NULL;
			}
			else
			{
				SendBuf(buf, n);
			}
		}
	}

	void OnWriteComplete() {
	}

	void OnTransferLimit() {
		if (!m_filename.size())
		{
			char buf[BUFSZ];
			int n = MIN(m_sz, BUFSZ);
			while (n > 0 && GetOutputLength() < lim)
			{
				SendBuf(buf, n);
				m_sz -= n;
				n = MIN(m_sz, BUFSZ);
			}
			if (!n)
			{
				SetCloseAndDelete();
			}
			else
			{
				SendBuf(buf, n);
				m_sz -= n;
			}
			return;
		}
		if (m_fil)
		{
			char buf[BUFSZ];
			int n = fread(buf, 1, BUFSZ, m_fil);
			while (n > 0 && GetOutputLength() < lim)
			{
				SendBuf(buf, n);
				n = fread(buf, 1, BUFSZ, m_fil);
			}
			if (!n)
			{
				SetCloseAndDelete();
				fclose(m_fil);
				m_fil = NULL;
			}
			else
			{
				SendBuf(buf, n);
			}
		}
	}

	void OnAccept() {
		Utility::GetTime(&m_start);
		m_b_server = true;
		SetLineProtocol();
		DisableInputBuffer();
	}

	void OnLine(const std::string& line) {
		if (line.size())
			m_out = fopen(line.c_str(), "wb");
		SetLineProtocol(false);
		DisableInputBuffer();
	}

	void OnDelete() {
		struct timeval stop;
		Utility::GetTime(&stop);
		stop.tv_sec -= m_start.tv_sec;
		stop.tv_usec -= m_start.tv_usec;
		if (stop.tv_usec < 0)
		{
			stop.tv_usec += 1000000;
			stop.tv_sec -= 1;
		}
		double t = stop.tv_usec;
		t /= 1000000;
		t += stop.tv_sec;
		printf("OnDelete: %s\n", m_b_server ? "SERVER" : "CLIENT");
		printf("  Time: %ld.%06ld (%f)\n", stop.tv_sec, stop.tv_usec, t);
		double r = GetBytesReceived();
		printf("  bytes in: %lld (%f Mbytes/sec)\n", GetBytesReceived(), r / t / 1000000);
		double s = GetBytesSent();
		printf("  bytes out: %lld (%f Mbytes/sec)\n", GetBytesSent(), s / t / 1000000);
		printf("\n");
		if (m_out)
			fclose(m_out);
	}

	void OnRawData(const char *buf, size_t len) {
		if (m_out)
			fwrite(buf, 1, len, m_out);
	}

private:
	std::string m_filename;
	FILE *m_fil;
	FILE *m_out;
	bool m_b_server;
	struct timeval m_start;
	size_t m_sz;
};


int main(int argc, char *argv[])
{
	std::string host = "127.0.0.1";
	int port = 12344;
	std::list<std::string> filenames;

	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-host") && i < argc - 1)
			host = argv[++i];
		else
		if (!strcmp(argv[i], "-port") && i < argc - 1)
			port = atoi(argv[++i]);
		else
		if (!strcmp(argv[i], "-h"))
		{
			fprintf(stderr, "Usage: %s [-host <host>] [-port <port>] [<file to send>]\n", *argv);
			fprintf(stderr, "  Will run as host only if <file to send> isn't specified.\n");
			fprintf(stderr, "  host default: 127.0.0.1\n");
			fprintf(stderr, "  port default: 12344\n");
			return 0;
		}
		else
		{
			filenames.push_back( argv[i] );
		}
	}

	try
	{
		SocketHandler h;
		ListenSocket<CopySocket> l(h);
		if (filenames.empty())
		{
			if (l.Bind( port ) != 0)
			{
				fprintf(stderr, "Bind() port %d failed - exiting\n", port);
				return -1;
			}
			h.Add(&l);
		}
		for (std::list<std::string>::iterator it = filenames.begin(); it != filenames.end(); it++)
		{
			std::string filename = *it;
			size_t sz = atol(filename.c_str());
			if (sz)
			{
				CopySocket *sock = new CopySocket(h, sz);
				sock -> SetDeleteByHandler();
				sock -> Open(host, port);
				h.Add(sock);
			}
			else
			{
				CopySocket *sock = new CopySocket(h, filename);
				sock -> SetDeleteByHandler();
				sock -> Open(host, port);
				h.Add(sock);
			}
		}
		if (!filenames.size())
		{
			fprintf(stderr, "Starting as server only, listening on port %d\n", port);
			quit++;
		}
		while (quit > 0)
		{
			h.Select(5, 0);
		}
		return 0;
	}
	catch (const Exception& e)
	{
		printf("%s\n", e.ToString().c_str());
	}
}


