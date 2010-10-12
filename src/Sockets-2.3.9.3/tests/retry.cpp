#include <TcpSocket.h>
#include <SocketHandler.h>
#include <ListenSocket.h>


	bool quit = false;

/*
	virtual bool OnConnectRetry();
-	void SetRetryClientConnect(bool x = true);
-	bool RetryClientConnect();
	void SetConnectionRetry(int n);
	int GetConnectionRetry();
	void IncreaseConnectionRetries();
	int GetConnectionRetries();
	void ResetConnectionRetries();
*/
class RetrySocket : public TcpSocket
{
public:
	RetrySocket(ISocketHandler& h) : TcpSocket(h) {
		SetConnectTimeout(2);
		SetConnectionRetry(-1);
	}

	bool OnConnectRetry() {
		printf("Connection attempt#%d\n", GetConnectionRetries());
		if (GetConnectionRetries() == 3)
		{
			ListenSocket<RetrySocket> *l = new ListenSocket<RetrySocket>(Handler());
			if (l -> Bind(12345))
			{
				printf("Bind port 12345 failed\n");
			}
			l -> SetDeleteByHandler();
			Handler().Add(l);
		}
		return true;
	}

	void OnConnect() {
		printf("Connected\n");
		printf("GetRemoteAddress(): %s\n", GetRemoteAddress().c_str());
		printf("Remote address: %s\n", GetRemoteSocketAddress() -> Convert(false).c_str());
		printf("Remote address: %s\n", GetRemoteSocketAddress() -> Convert(true).c_str());
		SetCloseAndDelete();
	}

	void OnDelete() {
		quit = true;
	}
};


int main(int argc, char *argv[])
{
	SocketHandler h;
	RetrySocket sock(h);
	sock.Open("localhost", 12345);
	h.Add(&sock);
	while (!quit)
	{
		h.Select(0, 200000);
	}
}


