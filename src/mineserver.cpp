#include <stdlib.h>
#ifdef WIN32
  #define _CRTDBG_MAP_ALLOC
  #include <crtdbg.h>
  #include <conio.h>
#endif

#include <cstdio>
//#include <cstdlib>
#include <deque>
#include <iostream>

#include <SocketHandler.h>
#include <ListenSocket.h>
#include "StatusHandler.h"

#include "tri_logger.hpp"

#include "DisplaySocket.h"
#include "map.h"
#include "user.h"
#include "chat.h"

static bool quit = false;

StatusHandler h;
ListenSocket<DisplaySocket> l(h);

int main(void)
{
    uint32 starttime=time(0);

#ifdef WIN32
  _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
  _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
#endif

  initMap();
  atexit(freeMap);

  //Bind to port 25565
	if (l.Bind(25565))
	{                                                                                                        
		exit(-1);
	}
    std::cout << std::endl
              << "    _/      _/  _/                                                                                    " << std::endl
              << "   _/_/  _/_/      _/_/_/      _/_/      _/_/_/    _/_/    _/  _/_/  _/      _/    _/_/    _/  _/_/   " << std::endl
              << "  _/  _/  _/  _/  _/    _/  _/_/_/_/  _/_/      _/_/_/_/  _/_/      _/      _/  _/_/_/_/  _/_/        " << std::endl
              << " _/      _/  _/  _/    _/  _/            _/_/  _/        _/          _/  _/    _/        _/           " << std::endl
              << "_/      _/  _/  _/    _/    _/_/_/  _/_/_/      _/_/_/  _/            _/        _/_/_/  _/            " << std::endl;
    std::cout << "Version 0.1.3 by Fador(&Psoden -_-)" << std::endl << std::endl;    
	h.Add(&l);
	h.Select(1,0);
	while (!quit)
	{
		h.Select(1,0);
        if(time(0)-starttime>10)
        {
            starttime=time(0);
            std::cout << "Currently " << h.GetCount()-1 << " users in!" << std::endl;

            //If users, ping them
            if(Users.size()>0)
            {
              //0x00 package
              uint8 data=0;
              Users[0].sendAll(&data, 1);
            }
        }
        #ifdef WIN32
        if(kbhit())
            quit=1;
        #endif
	}

    
  freeMap();
  l.CloseAndDelete();

  //Windows debug
  #ifdef WIN32
      _CrtDumpMemoryLeaks();
  #endif

  return EXIT_SUCCESS;
}