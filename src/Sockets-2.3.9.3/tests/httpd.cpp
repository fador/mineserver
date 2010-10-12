#include <HttpdSocket.h>
#include <SocketHandler.h>
#include <ListenSocket.h>
#include <StdoutLog.h>


class sSocket : public HttpdSocket
{
public:
	sSocket(ISocketHandler& h) : HttpdSocket(h) {
	}

	void Init()
	{
		if (GetParent() -> GetPort() == 443 || GetParent() -> GetPort() == 8443)
		{
#ifdef HAVE_OPENSSL
			EnableSSL();
#else
			fprintf(stderr, "SSL not available\n");
#endif
		}
	}

	void Exec()
	{
		CreateHeader();
		GenerateDocument();
	}

	void CreateHeader()
	{
		SetStatus("200");
		SetStatusText("OK");
fprintf(stderr, "Uri: '%s'\n", GetUri().c_str());
		{
			size_t x = 0;
			for (size_t i = 0; i < GetUri().size(); i++)
				if (GetUri()[i] == '.')
					x = i;
			std::string ext = GetUri().substr(x + 1);
			if (ext == "gif" || ext == "jpg" || ext == "png")
				AddResponseHeader("Content-type", "image/" + ext);
			else
				AddResponseHeader("Content-type", "text/" + ext);
		}
		AddResponseHeader("Connection", "close");
		SendResponse();
	}

	void GenerateDocument()
	{
		std::string fn = GetUri().substr(1);
		FILE *fil = fopen(fn.c_str(), "rb");
		if (fil)
		{
			char slask[1000];
			int n = fread(slask,1,1000,fil);
			while (n > 0)
			{
				SendBuf(slask, n);
				n = fread(slask,1,1000,fil);
			}
			fclose(fil);
		}
		else
		{
			SetStatus("404");
			SetStatusText("Not Found");
		}
		SetCloseAndDelete();
	}

#ifdef HAVE_OPENSSL
	void InitSSLServer()
	{
		InitializeContext("httpd", "comb.pem", "", SSLv23_method());
	}
#endif

};


int main(int argc, char *argv[])
{
	std::string host = argc > 1 ? argv[1] : "www.alhem.net";
	StdoutLog log;
	SocketHandler h(&log);
	ListenSocket<sSocket> l(h);
	if (l.Bind(1028))
	{
		printf("Bind port 1028 failed\n");
		return -1;
	}
	h.Add(&l);
	ListenSocket<sSocket> l2(h);
	if (l2.Bind(8443))
	{
		printf("Bind port 8443 failed\n");
		return -1;
	}
	h.Add(&l2);
	while (h.GetCount())
	{
		h.Select(1, 0);
	}
}


