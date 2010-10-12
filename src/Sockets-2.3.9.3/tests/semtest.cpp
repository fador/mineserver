#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#include <Semaphore.h>
#include <Mutex.h>
#include <Thread.h>
#include <Utility.h>
#include <Lock.h>


class MyThread : public Thread
{
public:
	void Run() {
		printf("Thread\n");
	}
};


class SemLock
{
public:
	SemLock(Semaphore& sem) : m_sem(sem) {
		m_sem.Wait();
	}
	~SemLock() {
		m_sem.Post();
	}

private:
	Semaphore& m_sem;
};


/**
 * Return time difference between two struct timeval's, in seconds
 * \param t0 start time
 * \param t	end time
 */
double Diff(struct timeval t0,struct timeval t)
{
	t.tv_sec -= t0.tv_sec;
	t.tv_usec -= t0.tv_usec;
	if (t.tv_usec < 0)
	{
		t.tv_usec += 1000000;
		t.tv_sec -= 1;
	}
	return t.tv_sec + (double)t.tv_usec / 1000000;
}


static	int val = 0;

void lock(Mutex& m, int i)
{
	Lock l(m);
	val += i;
}


void lock(Semaphore& s, int i)
{
	SemLock l(s);
	val += i;
}
#endif // WIN32


int main()
{
#ifndef _WIN32
	Mutex mutex;
	Semaphore sema(1);
	struct timeval start;
	struct timeval tt;
	double d;

	Utility::GetTime(&start);
	for (int i = 0; i < 100000; i++)
		lock(mutex, i);
	Utility::GetTime(&tt);
	d = Diff(start, tt);
	printf("%.4f sec\n", d);

	Utility::GetTime(&start);
	for (int i = 0; i < 100000; i++)
		lock(sema, i);
	Utility::GetTime(&tt);
	d = Diff(start, tt);
	printf("%.4f sec\n", d);
#endif
}


