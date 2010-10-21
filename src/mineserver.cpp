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

#include "constants.h"

#include <SocketHandler.h>
#include <ListenSocket.h>
#include "StatusHandler.h"

#include "logger.h"

#include "DisplaySocket.h"
#include "tools.h"
#include "map.h"
#include "user.h"
#include "chat.h"

static bool quit = false;

StatusHandler h;
ListenSocket<DisplaySocket> l(h);

int main(void)
{
    uint32 starttime=(uint32)time(0);
    uint32 tick=(uint32)time(0);

#ifdef WIN32
  _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
  _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
#endif

  Map::getInstance().initMap();
  //atexit(freeMap);

  //Bind to port
  if (l.Bind(PORT))
  {
    std::cout << "Unable to Bind port!" << std::endl;
    exit(-1);
  }
    std::cout << std::endl
              << "    _/      _/  _/                                                                                    " << std::endl
              << "   _/_/  _/_/      _/_/_/      _/_/      _/_/_/    _/_/    _/  _/_/  _/      _/    _/_/    _/  _/_/   " << std::endl
              << "  _/  _/  _/  _/  _/    _/  _/_/_/_/  _/_/      _/_/_/_/  _/_/      _/      _/  _/_/_/_/  _/_/        " << std::endl
              << " _/      _/  _/  _/    _/  _/            _/_/  _/        _/          _/  _/    _/        _/           " << std::endl
              << "_/      _/  _/  _/    _/    _/_/_/  _/_/_/      _/_/_/  _/            _/        _/_/_/  _/            " << std::endl;
    std::cout << "Version " << VERSION <<" by Fador & Psoden" << std::endl << std::endl;    
  h.Add(&l);
  h.Select(1,0);
  while (!quit)
  {
    h.Select(1,0);
    if(time(0)-starttime>10)
    {
      starttime=(uint32)time(0);
      //Logger::get().log("Currently " + h.GetCount()-1 + " users in!");
      std::cout << "Currently " << h.GetCount()-1 << " users in!" << std::endl;

      //If users, ping them
      if(Users.size()>0)
      {
        //0x00 package
        uint8 data=0;
        Users[0].sendAll(&data, 1);

        //Send server time (after dawn)
        uint8 data3[9]={0x04, 0x00, 0x00, 0x00,0x00,0x00,0x00,0x0e,0x00};
        Users[0].sendAll((uint8 *)&data3[0], 9);
      }
    }

    //Every second
    if(time(0)-tick>0)
    {
      tick=(uint32)time(0);
      //Loop users
      for(unsigned int i=0;i<Users.size();i++)
      {
        for(uint8 j=0;j<5;j++)
        {
          //Push new map data        
          Users[i].pushMap();

          //Remove map far away
          Users[i].popMap();
        }

        if(Users[i].logged)
        {
          Users[i].logged=false;
          //Send "On Ground" signal
          char data6[2]={0x0A, 0x01};
          h.SendSock(Users[i].sock, (char *)&data6[0], 2);

          //We need the first map part quickly
          Users[i].addQueue(0,0);
          Users[i].pushMap();

          //Teleport player
          Users[i].teleport(0,70,0); 
          
          for(int x=-Users[i].viewDistance;x<=Users[i].viewDistance;x++)
          {
            for(int z=-Users[i].viewDistance;z<=Users[i].viewDistance;z++)
            {
              Users[i].addQueue(x,z);
            }
          }

          //Spawn this user to others
          Users[i].spawnUser(0,70*32,0);
          //Spawn other users for connected user
          Users[i].spawnOthers();
        }
      }
    }
    #ifdef WIN32
    if(_kbhit())
        quit=1;
    #endif
  }

    
  Map::getInstance().freeMap();
  l.CloseAndDelete();

  //Windows debug
  #ifdef WIN32
      _CrtDumpMemoryLeaks();
  #endif

  return EXIT_SUCCESS;
}