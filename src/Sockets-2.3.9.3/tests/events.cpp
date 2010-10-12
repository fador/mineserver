#include <EventHandler.h>
#include <TcpSocket.h>
#include <IEventOwner.h>
#include <ListenSocket.h>
#include <StdoutLog.h>
#include <iostream>
#include <Debug.h>


double start;

double gettime()
{
	struct timeval tv;
	Utility::GetTime(&tv);
	return tv.tv_sec + (double)tv.tv_usec / 1000000;
}


class MyEvHandler : public EventHandler
{
public:
	MyEvHandler() : EventHandler() {}
	MyEvHandler(StdLog *p) : EventHandler(p) {}

};


class EvThread : public Thread
{
public:
	EvThread(Socket *p) : m_socket(p) {}

	void Run();

private:
	Socket *m_socket;
};


class EvSocket : public TcpSocket, public IEventOwner
{
public:
	EvSocket(ISocketHandler& h) : TcpSocket(h)
	, IEventOwner( static_cast<MyEvHandler&>(h) ) {
	}

	void OnEvent(int id) {
		std::cout << "OnEvent" << std::endl;
		std::cout << gettime() - start << std::endl;
		SetCloseAndDelete();
	}

	void OnAccept() {
		std::cout << "Incoming" << std::endl;
	}

	void OnConnect() {
		std::cout << "Connected" << std::endl;
		EvThread *thr = new EvThread( this );
		thr -> SetDeleteOnExit();
	}

	void OnDelete() {
		std::cout << "EvSocket::OnDelete()" << std::endl;
	}
};


void EvThread::Run()
{
	Socket *p0 = (Socket *)m_socket;
	EvSocket *p = dynamic_cast<EvSocket *>(p0);
	if (p)
	{
#ifdef _WIN32
		Sleep( 5000 );
#else
		sleep(10);
#endif
		std::cout << "Add event" << std::endl;
		start = gettime();
		p -> AddEvent(1, 50000);
	}
	else
	{
		std::cout << "Thread: not an EvSocket" << std::endl;
	}
}


//------------------------------------------------------------
// myTimer class
//------------------------------------------------------------
class myTimer : public IEventOwner
{
public:
	// Prototype of user call back function
	typedef void (*myTimer_cb)(myTimer* user_func, void* user_id);

	myTimer(IEventHandler& h, long ssec, long susec,
		long psec, long pusec, myTimer_cb func, void* id);
	~myTimer();

private:
	void OnEvent(int id);
	long ssec_;
	long susec_;
	long psec_;
	long pusec_;
	myTimer_cb user_func_;
	void* user_id_;
	bool periodic_;
};


//------------------------------------------------------------
myTimer::myTimer(IEventHandler& h, long ssec, long susec,
	long psec, long pusec, myTimer_cb user_func,
	void* user_id) :
	IEventOwner(h),
	ssec_(ssec),
	susec_(susec),
	psec_(psec),
	pusec_(pusec),
	user_func_(user_func),
	user_id_(user_id),
	periodic_(psec != 0 || pusec != 0)
{
	Debug deb("myTimer()");
	AddEvent(ssec_, susec_);
}


//------------------------------------------------------------
myTimer::~myTimer()
{
	Debug deb("~myTimer()");
	ClearEvents();
}


//------------------------------------------------------------
void myTimer::OnEvent(int id)
{
	Debug deb("myTimer::OnEvent()");
	if (periodic_)
	{
		AddEvent(psec_, pusec_);
	}

	user_func_(this, user_id_);

	if (!periodic_)
	{
		delete this;
	}
}


//------------------------------------------------------------
static void myfunc(myTimer* t, void* user_id)
{
	Debug deb("myTimer::myfunc()");
	deb << "Event " << ((char*)user_id) << Debug::endl();
}


/*
int main(int argc, char *argv[])
{
	EventHandler h;

	// Example 1: Create a 2 sec one-shot timer.
	// No need to save the pointer as it will delete
	// itself upon expiry.
	new myTimer(h, 2, 0, 0, 0, myfunc, (void*)"My Timer 0");

	// Example 2: Create a 1 sec periodic timer.
	// Save the pointer because we can stop the timer
	// later with a "delete mytimer".
	myTimer* mytimer = new myTimer(h, 0, 0, 1, 0, myfunc,
		(void*)"My Timer 1");

	// Drop into the event handler ..
	h.EventLoop();
}
*/


int main(int argc, char *argv[])
{
	StdoutLog log;
	MyEvHandler h(&log);
	ListenSocket<EvSocket> l(h);
	l.Bind("localhost", 12344);
	h.Add(&l);

	EvSocket sock(h);
	sock.Open("localhost", 12344);
	h.Add(&sock);

	// Example 1: Create a 2 sec one-shot timer.
	// No need to save the pointer as it will delete
	// itself upon expiry.
	new myTimer(h, 2, 0, 0, 0, myfunc, (void*)"My Timer 0");

	// Example 2: Create a 1 sec periodic timer.
	// Save the pointer because we can stop the timer
	// later with a "delete mytimer".
	myTimer* mytimer = new myTimer(h, 0, 0, 1, 0, myfunc,
		(void*)"My Timer 1");

	h.EventLoop();
}


