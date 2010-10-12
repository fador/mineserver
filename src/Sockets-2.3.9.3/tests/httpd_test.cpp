#include <TcpSocket.h>
#include <IEventOwner.h>
#include <EventHandler.h>
#include <Utility.h>
#include <Exception.h>
#include <iostream>


  std::string host;
  int port = 0;
  bool https = false;
  std::string full_request;

  int antal = 500;

  int interval = 500;
  int delay = 5000;

  int connected = 0;
  int failed = 0;
  int lost = 0;

class TestSocket : public TcpSocket, public IEventOwner
{
public:
  TestSocket(class EventHandler& h, bool add = false) : TcpSocket(h), IEventOwner(h)
  , m_req_index(0) {
    if (https)
    {
      EnableSSL();
    }
    if (add)
    {
      m_event_open = AddEvent(interval / 1000, (interval % 1000) * 1000);
    }
  }
  ~TestSocket() {}

  void OnConnect() {
    connected++;
    m_event_request = AddEvent(delay / 1000, (delay % 1000) * 1000);
    printf("%9lu %5u Connected: %d\n", UniqueIdentifier(), GetSockPort(), connected);
  }

  void OnConnectFailed() {
    failed++;
    printf("%9lu %5u Failed: %d\n", UniqueIdentifier(), GetSockPort(), failed);
  }

  void OnDisconnect() {
    printf("\n");
    printf("%s\n", Utility::Stack().c_str());
    lost++;
    connected--;
    printf("%9lu %5u Lost: %d\n", UniqueIdentifier(), GetSockPort(), lost);
  }

  void OnEvent(int id) {
    if (id == m_event_open)
    {
      if (connected < antal)
      {
        TestSocket *p = new TestSocket(static_cast<EventHandler&>(Handler()));
        p -> SetDeleteByHandler();
        p -> Open(host, port);
        Handler().Add(p);
      }
      else
      {
        printf("Finished connecting. %d connected / %d failed / %d lost\n", connected, failed, lost);
      }
      m_event_open = AddEvent(interval / 1000, (interval % 1000) * 1000);
    }
    else
    if (id == m_event_request)
    {
      if (m_req_index < full_request.size())
      {
        if (full_request[m_req_index] == '\r')
        {
          printf("<CR>");
          fflush(stdout);
        }
        if (full_request[m_req_index] == '\n')
        {
          printf("<LF>");
          fflush(stdout);
        }
        SendBuf( &full_request[m_req_index], 1 );
        m_req_index++;
        m_event_request = AddEvent(delay / 1000, (delay % 1000) * 1000);
      }
      else
      {
        m_event_restart = AddEvent(3, 0);
      }
    }
    else
    if (id == m_event_restart)
    {
      m_req_index = 0;
      m_event_request = AddEvent(delay / 1000, (delay % 1000) * 1000);
    }
    else
    {
      fprintf(stderr, "%9lu %5u Unknown event id\n", UniqueIdentifier(), GetSockPort());
      SetCloseAndDelete();
    }
  }

  void OnRawData(const char *buf, size_t len) {
    printf("<%lu,%d>", UniqueIdentifier(), len);
    fflush(stdout);
  }

private:
  long m_event_open;
  long m_event_request;
  long m_event_restart;
  size_t m_req_index;
};


int main(int argc, char *argv[])
{
  try
  {
    EventHandler h;

    /*
      $ httpd_test [options] url
      
      -host        override host part of url
      -port        override port part of url
      -uri         override uri part of url
      -qs          override query string part of url

      -nn          nn number of connections

      -interval    interval between connections, in ms
      -delay       interval between each request char sent
    */
    std::string url;

    std::string uri;
    std::string qs;

    for (int i = 1; i < argc; i++)
    {
      if (!strcmp(argv[i], "-host") && i < argc - 1)
        host = argv[++i];
      else
      if (!strcmp(argv[i], "-port") && i < argc - 1)
        port = atoi(argv[++i]);
      else
      if (!strcmp(argv[i], "-uri") && i < argc - 1)
        uri = argv[++i];
      else
      if (!strcmp(argv[i], "-qs") && i < argc - 1)
        qs = argv[++i];
      else
      if (strlen(argv[i]) > 1 && argv[i][0] == '-' && isdigit(argv[i][1]))
        antal = atoi(&argv[i][1]);
      else
      if (!strcmp(argv[i], "-interval") && i < argc - 1)
        interval = atoi(argv[++i]);
      else
      if (!strcmp(argv[i], "-delay") && i < argc - 1)
        delay = atoi(argv[++i]);
      else
      if (argv[i][0] == '-')
        printf("Huh? %s\n", argv[i]);
      else
        url = argv[i];
    }
    Utility::Uri parser(url);
    if (host.empty())
      host = parser.Host();
    if (!port)
      port = parser.Port();
    if (uri.empty())
      uri = parser.UrlUri();
    if (qs.empty())
      qs = parser.QueryString();
    if (parser.Protocol() == "https")
      https = true;

    full_request = "GET " + uri;
    if (!qs.empty())
      full_request += "?" + qs;
    full_request += " HTTP/1.1\r\n";
    full_request += "Host: " + parser.Host() + "\r\n";
    full_request += "\r\n";

    printf("Host: %s\n", host.c_str());
    printf("Port: %d\n", port);
    printf("----------------------------------\n");
    printf("%s", full_request.c_str());
    printf("----------------------------------\n");

    TestSocket *p = new TestSocket(h, true);
    p -> SetDeleteByHandler();
    p -> Open(host, port);
    h.Add(p);

    h.EventLoop();
  }
  catch (const Exception& e)
  {
    std::cerr << e.ToString() << std::endl;
  }
}


