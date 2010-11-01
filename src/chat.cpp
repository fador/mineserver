#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <deque>
#include <fstream>

#include "logger.h"
#include "constants.h"

#include "DisplaySocket.h"
#include "StatusHandler.h"

#include "tools.h"
#include "map.h"
#include "user.h"
#include "chat.h"
#include "config.h"

extern StatusHandler h;

Chat::Chat() 
{
  this->loadAdmins();
  
  //
  // Create motdfile is it doesn't exist
  //
  std::ifstream ifs( MOTDFILE.c_str() );
        
  // If file does not exist
  if( ifs.fail() )
  {
    std::cout << "> Warning: " << MOTDFILE << " not found." << std::endl;
    std::cout << "> Creating " << MOTDFILE << " with default message" << std::endl;
  }
  
  ifs.close();
}

bool Chat::loadAdmins() 
{
  // Read admins to deque
  std::ifstream ifs( ADMINFILE.c_str() );
  
  // If file does not exist
  if( ifs.fail() )
  {
    std::cout << "> Warning: " << ADMINFILE << " not found." << std::endl;
    std::cout << "> Creating " << ADMINFILE << std::endl;
    
    return true;
  }
  
  std::string temp;
  
  admins.clear();

  while( getline( ifs, temp ) ) {
    // If not commentline
    if(temp[0] != COMMENTPREFIX) {
        admins.push_back( temp );
    }
  }
  ifs.close();
  
  std::cout << "Loaded admins from " << ADMINFILE << std::endl;
  
  return true;
} 

bool Chat::sendUserlist( User *user ) {
      this->sendMsg(user, COLOR_BLUE + "[ Players online ]", USER);
      for(unsigned int i=0;i<Users.size();i++)
      {
          this->sendMsg(user, "> " + Users[i].nick, USER);
      }

      return true;
}

std::deque<std::string> Chat::parseCmd(std::string cmd)
{
  int del;
  std::deque<std::string> temp;
  while(cmd.length() > 0) {
    while(cmd[0] == ' ') cmd = cmd.substr(1);
    del = cmd.find(' ');
    if(del > -1)
    {
      temp.push_back( cmd.substr(0,del) );
      cmd = cmd.substr(del+1);
    } else {
      temp.push_back( cmd );
      break;
    }
  }
  if(temp.empty()) temp.push_back("empty");
  return temp;
}

bool Chat::handleMsg( User *user, std::string msg )
{
    // Timestamp
    time_t rawTime = time(NULL);
    struct tm* Tm = localtime(&rawTime);
    
    std::string timeStamp (asctime(Tm));
    
    timeStamp = timeStamp.substr(11,5);

    //
    // Chat commands
    //
    
    // Servermsg (Admin-only)
    if(msg[0] == SERVERMSGPREFIX && user->admin) 
    {
        // Decorate server message
        msg = COLOR_RED + "[!] " + COLOR_GREEN + msg.substr(1);
        this->sendMsg(user, msg, ALL);
    } 
    else if(msg[0] == CHATCMDPREFIX)
    {
        std::deque<std::string> cmd = this->parseCmd( msg.substr(1) );
        
        // Playerlist
        if(cmd[0] == "players") 
        {
          this->sendUserlist(user);
        }
        
        // About server
        else if(cmd[0] == "about") 
        {
          this->sendMsg(user, COLOR_DARK_MAGENTA + "SERVER:" + COLOR_RED + " Mineserver v." + VERSION, USER);
        }
        
        //
        // Admin commands
        //
        else if(user->admin) 
        {
          // Save map
          if(cmd[0] == "save") 
          {
            Map::get().saveWholeMap();
            this->sendMsg(user, COLOR_DARK_MAGENTA + "SERVER:" + COLOR_RED + " Saved map to disc", USER);
          }
          
          // Kick user
          else if(cmd[0] == "kick") 
          {
            cmd.pop_front();
            if( !cmd.empty() ) {
              LOG( "Kicking: " + cmd[0] );
              // Get coordinates
              User* tUser = getUserByNick(cmd[0]);
              if(tUser != false)
              {
                cmd.pop_front();
                std::string kickMsg = DEFAULTKICKMSG;
                if( !cmd.empty() ) 
                {
                  kickMsg = "";
                  while( !cmd.empty() ) 
                  {
                    kickMsg += cmd[0] + " ";
                    cmd.pop_front();
                  }
                }
                tUser->kick(kickMsg);
                LOG("Kicked!");
              }
            }
          }
          
          // Teleport to coordinates
          else if(cmd[0] == "ctp") 
          {
            cmd.pop_front();
            if( cmd.size() > 2 ) {
              LOG(user->nick + " teleport to: " + cmd[0] + " " + cmd[1] + " " + cmd[2] );
              double x = atof( cmd[0].c_str() );
              double y = atof( cmd[1].c_str() );
              double z = atof( cmd[2].c_str() );
              user->teleport(x,y,z);
            }
          }
          
          // Teleport to user
          else if(cmd[0] == "tp") 
          {
            cmd.pop_front();
            if( !cmd.empty() ) {
              LOG(user->nick + " teleport to: " + cmd[0]);
              
              // Get coordinates
              User* tUser = getUserByNick( cmd[0] );
              if(tUser != false)
              {
                user->teleport(tUser->pos.x, tUser->pos.y+2, tUser->pos.z);
              } else {
                this->sendMsg(user, COLOR_DARK_MAGENTA + "Error!" + COLOR_RED + " User " + cmd[0] + " not found (See /players)", USER);
              }
            }
          }
          
                    // Kick user
          else if(cmd[0] == "reload") 
          {
            cmd.pop_front();
            
            // Load admins
            this->loadAdmins();
            
            // Load config
            Conf::get().load(CONFIGFILE);
            
            // Note: Motd is loaded whenever new user joins
          }
          
        }
    } 
    // Normal message
    else {
        
        if(user->admin) 
        {
          msg = timeStamp + " <"+ COLOR_DARK_MAGENTA + user->nick + COLOR_WHITE + "> " + msg;
        } else 
        {
          msg = timeStamp + " <"+ user->nick + "> " + msg;
        }
        LOG(msg);

        this->sendMsg(user, msg, ALL);
    }

    return true;
}

bool Chat::sendMsg(User *user, std::string msg, int action = ALL)
{
    uint8 *tmpArray = new uint8 [msg.size()+3];
    tmpArray[0]=0x03;
    tmpArray[1]=0;
    tmpArray[2]=msg.size()&0xff;      
    for(unsigned int i=0;i<msg.size();i++) tmpArray[i+3]=msg[i]; 

    if(action == ALL) user->sendAll(&tmpArray[0],msg.size()+3);
    if(action == USER) h.SendSock(user->sock, &tmpArray[0], msg.size()+3);
    if(action == OTHERS)  user->sendOthers(&tmpArray[0], msg.size()+3);
    delete [] tmpArray;
    
    return true;
}