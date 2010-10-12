#include <TcpSocket.h>
#include <SocketHandler.h>
#include <ListenSocket.h>
#include <Exception.h>
#include <Parse.h>
#include <iostream>


bool quit = false;


class ControlHandler : public SocketHandler
{
public:
  ControlHandler() : SocketHandler() {}
  ~ControlHandler() {}

  void List(TcpSocket *p);

  void StopListen();
};


class ControlSocket : public TcpSocket
{
public:
  ControlSocket(ISocketHandler& h) : TcpSocket(h) {
    SetLineProtocol();
  }

  void OnAccept() {
    SendHelp();
    Send(">> ");
  }

  void OnLine(const std::string& line) {
    Parse pa(line);
    std::string cmd = pa.getword();
    std::string arg = pa.getrest();

    ControlHandler& h = static_cast<ControlHandler&>(Handler());
    if (cmd == "help")
    {
      SendHelp();
    }
    else
    if (cmd == "quit")
    {
      SetCloseAndDelete();
      Send("Bye.\n");
      return;
    }
    else
    if (cmd == "list")
    {
      h.List(this);
    }
    else
    if (cmd == "stop")
    {
      h.StopListen();
    }
    else
    if (cmd == "bind")
    {
      port_t port = (port_t)atoi(arg.c_str());
      ListenSocket<ControlSocket> *p = new ListenSocket<ControlSocket>(Handler());
      p -> SetDeleteByHandler();
      p -> Bind(port);
      h.Add(p);
    }
    else
    {
      Send("Huh?\n");
    }
    Send(">> ");
  }

  void SendHelp() {
    Send("Commands available\n"
      " help, quit, list, stop, bind <port number>\n"
      "\n"
      " list - list all sockets\n"
      " stop - stop all listening sockets\n"
      " bind <port number> - add a new listening socket\n"
      " quit - leave\n"
      " help - show this text\n"
      "\n");
  }

private:
};


void ControlHandler::List(TcpSocket *from)
{
  for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
  {
    SOCKET s = it -> first;
    Socket *p0 = it -> second;
    ListenSocket<ControlSocket> *l = dynamic_cast<ListenSocket<ControlSocket> *>(p0);
    ControlSocket *p = dynamic_cast<ControlSocket *>(p0);
    if (l)
    {
      from -> Send(Utility::l2string(s) + ": ListenSocket on port " + Utility::l2string(l -> GetPort()) + "\n");
    }
    else
    if (p)
    {
      from -> Send(Utility::l2string(s) + ": ControlSocket\n");
    }
    else
    {
      from -> Send(Utility::l2string(s) + ": <unknown>\n");
    }
  }
}


void ControlHandler::StopListen()
{
  for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
  {
    Socket *p0 = it -> second;
    ListenSocket<ControlSocket> *l = dynamic_cast<ListenSocket<ControlSocket> *>(p0);
    if (l)
    {
      l -> SetCloseAndDelete();
    }
  }
}


int main(int argc, char *argv[])
{
  try
  {
    ControlHandler h;
    ListenSocket<ControlSocket> l(h);
    l.Bind(2000);
    h.Add(&l);
    while (!quit && h.GetCount())
    {
      h.Select();
    }
  }
  catch (const Exception& e)
  {
    std::cout << e.ToString() << std::endl;
  }
}

