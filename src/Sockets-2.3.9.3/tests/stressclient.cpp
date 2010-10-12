/**
 **  \file stressclient.cpp
 **  \date  2006-10-02
 **  \author grymse@alhem.net
**/
/*
Copyright (C) 2006  Anders Hedstrom

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifdef _WIN32
#pragma warning(disable:4786)
#endif
#include <StdoutLog.h>
#include <ListenSocket.h>
#include <SocketHandlerEp.h>
#include <TcpSocket.h>
#include <Utility.h>
#ifndef _WIN32
#include <signal.h>
#include <stdint.h>
#else
typedef __int64 int64_t;
#endif
#include <HttpGetSocket.h>
#include <iostream>


static  int             g_ant = 0;
static  double          g_min_time = 10000;
static  double          g_max_time = 0;
static  double          g_tot_time = 0;

static  int             g_ant2 = 0;
static  double          g_min_time2 = 10000;
static  double          g_max_time2 = 0;
static  double          g_tot_time2 = 0;

static  int64_t         gBytesIn = 0;
static  int64_t         gBytesOut = 0;

static	int		samples = 0;

static  int             g_tot_ant = 0;
static  double          g_tot_min_t = 0;
static  double          g_tot_max_t = 0;
static  double          g_tot_tot_t = 0;

static  int             g_tot_ant2 = 0;
static  double          g_tot_min_t2 = 0;
static  double          g_tot_max_t2 = 0;
static  double          g_tot_tot_t2 = 0;

static  int64_t         tot_gBytesIn = 0;
static  int64_t         tot_gBytesOut = 0;

static  double          tot_rt = 0;

static  bool            gQuit = false;
static  size_t          g_max_connections = 0;
static  std::string     gHost = "localhost";
static  port_t          gPort = 2222;
static  bool            g_b_flood = false;
static  bool            g_b_off = false;
static  bool            g_b_limit = false;
static  bool            g_b_repeat = false;
static  std::string     g_data;
static  size_t          g_data_size = 1024;
static  bool            g_b_stop = false;
#ifdef HAVE_OPENSSL
static  bool            g_b_ssl = false;
#endif
static  bool            g_b_instant = false;
static  struct timeval  g_t_start;


/**
 * Return time difference between two struct timeval's, in seconds
 * \param t0 start time
 * \param t  end time
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


void gettime(struct timeval *p, struct timezone *)
{
#ifdef _WIN32
  FILETIME ft; // Contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
  GetSystemTimeAsFileTime(&ft);
  uint64_t tt;
  memcpy(&tt, &ft, sizeof(tt));
  tt /= 10;
  p->tv_sec = tt / 1000000;
  p->tv_usec = tt % 1000000;
#else
  gettimeofday(p, NULL);
#endif
}


void print_tot()
{
  printf("c ----- %6d (%.4f/%.4f/%.4f)", g_tot_ant / samples,  g_tot_min_t / samples,  g_tot_max_t / samples,  g_tot_tot_t /  g_tot_ant);

  printf("  r --- %7d (%.4f/%.4f/%.4f)", g_tot_ant2 / samples, g_tot_min_t2 / samples, g_tot_max_t2 / samples, g_tot_tot_t2 / g_tot_ant2);

  double mbi = (double)tot_gBytesIn / 1024 / samples;
  mbi /= 1024;
  mbi /= tot_rt / samples;
  printf("  -/in %11lld",  tot_gBytesIn / samples);
  printf(" (%5.2f MB/s)",       mbi);

  double mbo = (double)tot_gBytesOut / 1024 / samples;
  mbo /= 1024;
  mbo /= tot_rt / samples;
  printf("  -/out %11lld", tot_gBytesOut / samples);
  printf(" (%5.2f MB/s)",       mbo);

  printf("  %5.2f s\n",         tot_rt / samples);

}


void printreport()
{
  struct timeval tv;
  gettime(&tv, NULL);
  double rt = Diff(g_t_start, tv);
  g_t_start = tv;
  //
//  printf("connect %6d (%.4f/%.4f/%.4f)", g_ant,  g_min_time,  g_max_time,  g_tot_time /  g_ant);
  g_tot_ant += g_ant;
  g_tot_min_t += g_min_time;
  g_tot_max_t += g_max_time;
  g_tot_tot_t += g_tot_time;

//  printf("  reply %7d (%.4f/%.4f/%.4f)", g_ant2, g_min_time2, g_max_time2, g_tot_time2 / g_ant2);
  g_tot_ant2 += g_ant2;
  g_tot_min_t2 += g_min_time2;
  g_tot_max_t2 += g_max_time2;
  g_tot_tot_t2 += g_tot_time2;

  double mbi = (double)gBytesIn / 1024;
  mbi /= 1024;
  mbi /= rt;
//  printf("  b/in %11lld",  gBytesIn);
//  printf(" (%5.2f MB/s)",       mbi);
  tot_gBytesIn += gBytesIn;

  double mbo = (double)gBytesOut / 1024;
  mbo /= 1024;
  mbo /= rt;
//  printf("  b/out %11lld", gBytesOut);
//  printf(" (%5.2f MB/s)",       mbo);
  tot_gBytesOut += gBytesOut;

//  printf("  %5.2f s\n",         rt);
  tot_rt += rt;

  ++samples;
  print_tot();
}


void printreport_reset()
{
  printreport();
  //
  g_min_time = 10000;
  g_max_time = 0;
  g_tot_time = 0;
  g_ant = 0;
  g_min_time2 = 10000;
  g_max_time2 = 0;
  g_tot_time2 = 0;
  g_ant2 = 0;
  gBytesIn = gBytesOut = 0;
}


class MySocket : public TcpSocket
{
public:
  MySocket(ISocketHandler& h,bool one) : TcpSocket(h), m_b_client(false), m_b_one(one), m_b_created(false), m_b_active(false) {
    gettime(&m_create, NULL);
    SetLineProtocol();
#ifdef HAVE_OPENSSL
    if (g_b_ssl)
      EnableSSL();
#endif
    if (g_max_connections && !m_b_one && Handler().GetCount() >= g_max_connections)
    {
      fprintf(stderr, "\nConnection limit reached: %d, continuing in single connection stress mode\n", (int)g_max_connections);
      if (g_b_off)
        printreport_reset();
      g_b_limit = true;
      m_b_one = true;
      //
      g_b_flood = g_b_repeat;
    }
#ifndef USE_EPOLL
    if (!m_b_one && Handler().GetCount() >= Handler().MaxCount() - 17)
    {
      fprintf(stderr, "\nFD_SETSIZE connection limit reached: %d, continuing in single connection stress mode\n", (int)Handler().GetCount());
      if (g_b_off)
        printreport_reset();
      g_b_limit = true;
      m_b_one = true;
      //
      g_b_flood = g_b_repeat;
    }
#endif
  }
  ~MySocket() {
  }

  void OnConnect() {
    gettime(&m_connect, NULL);
    m_b_active = true;
    {
      double tconnect = Diff(m_create, m_connect);
      //
      g_min_time = tconnect < g_min_time ? tconnect : g_min_time;
      g_max_time = tconnect > g_max_time ? tconnect : g_max_time;
      g_tot_time += tconnect;
      g_ant += 1;
    }
    SendBlock();
    m_b_client = true;
  }

  void SendBlock() {
    gettime(&m_send, NULL);
    Send(g_data + "\n");
  }

  void OnLine(const std::string& line) {
    gettime(&m_reply, NULL);
    m_b_active = true;
    {
      double treply = Diff(m_send, m_reply);
      //
      g_min_time2 = treply < g_min_time2 ? treply : g_min_time2;
      g_max_time2 = treply > g_max_time2 ? treply : g_max_time2;
      g_tot_time2 += treply;
      g_ant2 += 1;
    }
    //
    if (line != g_data)
    {
      fprintf(stderr, "\n%s\n%s\n", line.c_str(), g_data.c_str());
      fprintf(stderr, "(reply did not match data - exiting)\n");
      exit(-1);
    }
    //
    gBytesIn += GetBytesReceived(true);
    gBytesOut += GetBytesSent(true);
    if (m_b_one)
    {
      SetCloseAndDelete();
    }
    else
    if (g_b_repeat && g_b_limit)
    {
      SendBlock();
    }
    // add another
    if (!m_b_created && (!g_b_limit || !g_b_off) && !g_b_instant)
    {
      MySocket *p = new MySocket(Handler(), m_b_one);
      p -> SetDeleteByHandler();
      p -> Open(gHost, gPort);
      Handler().Add(p);
      m_b_created = true;
    }
  }

  bool IsActive() {
    bool b = m_b_active;
    m_b_active = false;
    return b;
  }

private:
  bool m_b_client;
  bool m_b_one;
  bool m_b_created;
  bool m_b_active;
  struct timeval m_create;
  struct timeval m_connect;
  struct timeval m_send;
  struct timeval m_reply;
};


class MyHttpSocket : public HttpGetSocket
{
public:
  MyHttpSocket(ISocketHandler& h,const std::string& url) : HttpGetSocket(h,url), m_url(url) {
    gettime(&m_create, NULL);
    AddResponseHeader("content-length", Utility::l2string(g_data_size));
  }
  ~MyHttpSocket() {}

  void OnConnect() {
    gettime(&m_connect, NULL);
    {
      double tconnect = Diff(m_create, m_connect);
      //
      g_min_time = tconnect < g_min_time ? tconnect : g_min_time;
      g_max_time = tconnect > g_max_time ? tconnect : g_max_time;
      g_tot_time += tconnect;
      g_ant += 1;
    }
    gettime(&m_send, NULL);

    // send request header
    HttpGetSocket::OnConnect();

    // send body
    Send(g_data);
  }

  void OnContent() {
//    std::cout << GetContentLength() << std::endl;
    gettime(&m_reply, NULL);
    {
      double treply = Diff(m_send, m_reply);
      //
      g_min_time2 = treply < g_min_time2 ? treply : g_min_time2;
      g_max_time2 = treply > g_max_time2 ? treply : g_max_time2;
      g_tot_time2 += treply;
      g_ant2 += 1;
    }
    gBytesIn += GetBytesReceived(true);
    gBytesOut += GetBytesSent(true);
    CreateNew();
  }
  void CreateNew() {
    if (g_b_off)
      return;
    MyHttpSocket *p = new MyHttpSocket(Handler(), m_url);
    p -> SetDeleteByHandler();
    Handler().Add(p);
    SetCloseAndDelete();
  }

  void OnDelete() {
  }

private:
  std::string m_url;
  struct timeval m_create;
  struct timeval m_connect;
  struct timeval m_send;
  struct timeval m_reply;
};


int connectors = 0;

class ConnectorSocket : public TcpSocket
{
public:
  ConnectorSocket(ISocketHandler& h) : TcpSocket(h), m_state(0) {
  }
  ~ConnectorSocket() {}

  void OnConnect() {
    SetTimeout(5);
    connectors += 1;
    std::cout << "Connected: " << connectors << std::endl;
  }

  void OnTimeout() {
    SetTimeout(5);
    if (!m_state)
    {
      m_state = 1;
    }
    else
    {
      ConnectorSocket *s = new ConnectorSocket(Handler());
      s -> Open(gHost, gPort);
      s -> SetDeleteByHandler();
      Handler().Add(s);
    }
  }

private:
  int m_state;
};


#ifndef _WIN32
void sigint(int)
{
  printreport();
  gQuit = true;
}


void sigusr1(int)
{
  g_b_flood = true;
}


void sigusr2(int)
{
  printreport_reset();
}
#endif


class MyHandler : public SocketHandlerEp
{
public:
  MyHandler() : SocketHandlerEp() {
  }
  MyHandler(StdoutLog *p) : SocketHandlerEp(p) {
  }
  ~MyHandler() {
  }
  void Flood() {
    for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
    {
      Socket *p0 = it -> second;
      MySocket *p = dynamic_cast<MySocket *>(p0);
      if (p)
      {
        p -> SendBlock();
      }
    }
  }
  void Report() {
    int ant = 0;
    int act = 0;
    for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
    {
      MySocket *p = dynamic_cast<MySocket *>(it -> second);
      if (p)
      {
        ant++;
        if (p -> IsActive())
        {
          act++;
        }
      }
    }
//    printf("  Number of //stress// sockets: %d  Active: %d\n", ant, act);
  }
};


int main(int argc,char *argv[])
{
  bool many = false;
  bool one = false;
  bool enableLog = false;
  bool http = false;
  bool connector = false;
  std::string url;
  time_t report_period = 10;
  for (int i = 1; i < argc; i++)
  {
    if (!strcmp(argv[i], "-many"))
      many = true;
    if (!strcmp(argv[i], "-one"))
      one = true;
    if (*argv[i] == '-' && strlen(argv[i]) > 1 && isdigit(argv[i][1]) )
      g_max_connections = atoi(argv[i] + 1);
    if (!strcmp(argv[i], "-host") && i < argc - 1)
      gHost = argv[++i];
    if (!strcmp(argv[i], "-port") && i < argc - 1)
      gPort = atoi(argv[++i]);
    if (!strcmp(argv[i], "-off"))
      g_b_off = true;
    if (!strcmp(argv[i], "-repeat"))
      g_b_repeat = true;
    if (!strcmp(argv[i], "-size") && i < argc - 1)
      g_data_size = atoi(argv[++i]);
    if (!strcmp(argv[i], "-log"))
      enableLog = true;
    if (!strcmp(argv[i], "-time") && i < argc - 1)
      report_period = atoi(argv[++i]);
    if (!strcmp(argv[i], "-stop"))
      g_b_stop = true;
#ifdef HAVE_OPENSSL
    if (!strcmp(argv[i], "-ssl"))
      g_b_ssl = true;
#endif
    if (!strcmp(argv[i], "-instant"))
      g_b_instant = true;
    if (!strcmp(argv[i], "-http"))
      http = true;
    if (!strcmp(argv[i], "-url") && i < argc - 1)
      url = argv[++i];
    if (!strcmp(argv[i], "-connector"))
      connector = true;
  }
  if (argc < 2 || (!many && !one && !g_max_connections && !http && !connector) )
  {
    printf("Usage: %s [mode] [options]\n", *argv);
    printf("  Modes (only use one of these):\n");
    printf("    -many      start max number of connections\n");
    printf("    -one       open - close - repeat\n");
    printf("    -nn        open nn connections, then start -one mode\n");
    printf("    -http      send/receive http request/response\n");
    printf("  Options:\n");
    printf("    -host xx   host to connect to\n");
    printf("    -port nn   port number to connection to\n");
    printf("    -off       turn off new connections when connection limit reached\n");
    printf("    -repeat    send new block when reply is received\n");
    printf("    -size nn   size of block to send, default is 1024 bytes\n");
    printf("    -log       enable debug log\n");
    printf("    -time nn   time between reports, default 10s\n");
    printf("    -stop      stop after time elapsed\n");
    printf("    -instant   create all sockets at once\n");
#ifdef HAVE_OPENSSL
    printf("    -ssl       use ssl\n");
#endif
    printf("    -url xx    url to use in http mode (default http://<host>:<port>/)\n");
    printf("    -connector Use connector stress test\n");
    exit(-1);
  }
  fprintf(stderr, "Using data size: %d bytes\n", (int)g_data_size);
  std::string chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  while (g_data.size() < g_data_size)
  {
    g_data += chars[rand() % chars.size()];
  }
#ifndef _WIN32
  signal(SIGINT, sigint);
  signal(SIGUSR1, sigusr1);
  signal(SIGUSR2, sigusr2);
  signal(SIGPIPE, SIG_IGN);
#endif
  StdoutLog *log = enableLog ? new StdoutLog() : NULL;
  MyHandler h(log);
  if (connector)
  {
    report_period = 4;
    ConnectorSocket *s = new ConnectorSocket(h);
    s -> Open(gHost, gPort);
    s -> SetDeleteByHandler();
    h.Add(s);
  }
  else
  if (http)
  {
    if (!url.size())
    {
      url = "http://" + gHost + ":" + Utility::l2string(gPort) + "/";
    }
    MyHttpSocket *s = new MyHttpSocket(h, url);
    s -> SetDeleteByHandler();
    h.Add(s);
  }
  else
  if (g_b_instant)
  {
    for (size_t i = 0; i < g_max_connections; i++)
    {
      MySocket *s = new MySocket(h, one);
      s -> SetDeleteByHandler();
      s -> Open(gHost, gPort);
      h.Add(s);
    }
    g_b_limit = true;
  }
  else
  {
    MySocket *s = new MySocket(h, one);
    s -> SetDeleteByHandler();
    s -> Open(gHost, gPort);
    h.Add(s);
  }
  time_t t = time(NULL);
  gettime(&g_t_start, NULL);
  while (!gQuit)
  {
    h.Select(1, 0);
    if (g_b_flood)
    {
      fprintf(stderr, "\nFlooding\n");
      h.Flood();
      g_b_flood = false;
    }
    if (time(NULL) - t >= report_period) // report
    {
      t = time(NULL);
      printreport_reset();
      h.Report();
      if (g_b_stop)
      {
        gQuit = true;
      }
    }
  }
  fprintf(stderr, "\nExiting...\n");
  if (log)
  {
//    delete log;
  }
  return 0;
}


