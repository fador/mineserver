#include <HttpdSocket.h>
#include <SocketHandler.h>
#include <ListenSocket.h>
#include <StdoutLog.h>
#include <HttpPostSocket.h>
#include <HttpPutSocket.h>
#include <HttpdForm.h>


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
		std::string name;
		std::string value;
		GetForm() -> getfirst(name, value);
		while (name.size())
		{
fprintf(stderr, "%s: '%s'\n", name.c_str(), value.c_str());
			GetForm() -> getnext(name, value);
		}
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
		Send("<html></html>");
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
	HttpPostSocket sock(h, "http://localhost:1028/postdata");
	sock.AddField("name", "value");
	sock.Open();
	h.Add(&sock);
	while (h.GetCount())
	{
		h.Select(1, 0);
	}
}


