/*
g++ -c command.cpp
g++ -shared command.o -o command.so

copy command.so to Mineserver bin directory.


*/

#include <string>
#include <deque>
#include <ctime>
#include <sstream>
#include <cstdlib>

#include "../../src/plugin_api.h"

#include "command.h"

#define PLUGIN_COMMAND_VERSION 1.1
const char CHATCMDPREFIX   = '/';
mineserver_pointer_struct* mineserver;

std::string dtos( double n )
{
  std::ostringstream result;
  result << n;
  return result.str();
}

std::deque<std::string> parseCmd(std::string cmd)
{
  int del;
  std::deque<std::string> temp;

  while(cmd.length() > 0)
  {
    while(cmd[0] == ' ')
    {
      cmd = cmd.substr(1);
    }

    del = cmd.find(' ');

    if(del > -1)
    {
      temp.push_back(cmd.substr(0, del));
      cmd = cmd.substr(del+1);
    }
    else
    {
      temp.push_back(cmd);
      break;
    }
  }

  if(temp.empty())
  {
    temp.push_back("empty");
  }
  return temp;
}

typedef void (*CommandCallback)(std::string nick, std::string, std::deque<std::string>);

struct Command
{
  std::deque<std::string> names;
  std::string arguments;
  std::string description;
  CommandCallback callback;  
  Command(std::deque<std::string> _names, std::string _arguments, std::string _description, CommandCallback _callback) 
    : names(_names),arguments(_arguments),description(_description),callback(_callback) {}
};

typedef std::map<std::string, Command*> CommandList;
CommandList m_Commands;




void registerCommand(Command* command)
{
  // Loop thru all the words for this command
  std::string currentWord;
  std::deque<std::string> words = command->names;
  while(!words.empty())
  {
    currentWord = words[0];
    words.pop_front();
    m_Commands[currentWord] = command;
  }
}



bool chatPreFunction(const std::string& user, std::string msg)
{
  if(msg.size() == 0)
  {
    return false;
  }

  char prefix = msg[0];

  mineserver->screen.log("Command Plugin got from "+user+": " + msg);

  if(prefix == CHATCMDPREFIX)
  {
    // Timestamp
    time_t rawTime = time(NULL);
    struct tm* Tm  = localtime(&rawTime);
    std::string timeStamp (asctime(Tm));
    timeStamp = timeStamp.substr(11, 5);

    std::deque<std::string> cmd = parseCmd(msg.substr(1));

    std::string command = cmd[0];
    cmd.pop_front();

    // User commands
    CommandList::iterator iter;
    if((iter = m_Commands.find(command)) != m_Commands.end())
    {
      iter->second->callback(user, command, cmd);
      return true;
    }
  }

  return false;
}


bool isValidItem(int id)
{
  if(id < 1)  // zero or negative items are all invalid
  {
    return false;
  }

  if(id > 91 && id < 256)  // these are undefined blocks and items
  {
    return false;
  }

  if(id == 2256 || id == 2257)  // records are special cased
  {
    return true;
  }

  if(id > 350)  // high items are invalid
  {
    return false;
  }

  return true;
}

int roundUpTo(int x, int nearest)
{
  return (((x + (nearest - 1)) / nearest) * nearest );
}

void giveItemsSelf(std::string user, std::string command, std::deque<std::string> args)
{
  if(args.size() == 1 || args.size() == 2)
  {
    int itemId = 0;

    //First check if item is a number
    itemId = atoi(args[0].c_str());

    // Check item validity
    if(isValidItem(itemId))
    {
      position_struct *player_pos = mineserver->user.getPosition(user);

      if(player_pos != NULL)
      {
        int itemCount = 1, itemStacks = 1;

        if(args.size() == 2)
        {
          itemCount = atoi(args[1].c_str());
          if(itemCount>1024) itemCount=1024;
          // If multiple stacks
          itemStacks = roundUpTo(itemCount, 64) / 64;
          itemCount  -= (itemStacks-1) * 64;
        }

        int amount = 64;
        for(int i = 0; i < itemStacks; i++)
        {
          // if last stack
          if(i == itemStacks - 1)
          {
            amount = itemCount;
          }

          mineserver->map.createPickupSpawn((int)player_pos->x,(int)player_pos->y,(int)player_pos->z,itemId,amount,0,user);
        }
      }
    }
    else
    {
      mineserver->chat.sendmsgTo(user, "Not a valid item");      
    }
  }
  else
  {
    mineserver->chat.sendmsgTo(user, "Usage: /igive item [count]");
  }
}

void home(std::string user, std::string command, std::deque<std::string> args)
{
  mineserver->chat.sendmsgTo(user, "Teleported you home!");
  int x,y,z;
  mineserver->map.getSpawn(&x,&y,&z);
  mineserver->user.teleport(user,x, y + 2, z);
}

void setTime(std::string user, std::string command, std::deque<std::string> args)
{
  if(args.size() == 1)
  {
    std::string timeValue = args[0];

    // Check for time labels
    if(timeValue == "day" || timeValue == "morning")
    {
      timeValue = "24000";
    }
    else if (timeValue == "dawn")
    {
      timeValue = "22500";
    }
    else if (timeValue == "noon")
    {
      timeValue = "6000";
    }
    else if (timeValue == "dusk")
    {
      timeValue = "12000";
    }
    else if (timeValue == "night" || timeValue == "midnight")
    {
      timeValue = "18000";
    }

    mineserver->map.setTime(timeValue);


    mineserver->chat.sendmsgTo(user, "World time changed.");
  }
  else
  {
    mineserver->chat.sendmsgTo(user, "Usage: /settime time (time = 0-24000)");
  }
}


PLUGIN_API_EXPORT void command_init(mineserver_pointer_struct* mineserver_temp)
{
  mineserver = mineserver_temp;
  if (mineserver->plugin.getPluginVersion("command") > 0)
  {
    mineserver->screen.log("command is already loaded v." +dtos(mineserver->plugin.getPluginVersion("command")));
    return;
  }
  mineserver->screen.log("Loaded \"command\"!");

  mineserver->plugin.setPluginVersion("command", PLUGIN_COMMAND_VERSION);

  mineserver->callback.add_hook("ChatPre", (void *)chatPreFunction);

  registerCommand(new Command(parseCmd("igive i"), "<id/alias> [count]", "Gives self [count] pieces of <id/alias>. By default [count] = 1", giveItemsSelf));
  registerCommand(new Command(parseCmd("home"), "", "Teleport to map spawn location", home));
  registerCommand(new Command(parseCmd("settime"), "<time>", "Sets server time. (<time> = 0-24000, 0 & 24000 = day, ~15000 = night)", setTime));  
}

PLUGIN_API_EXPORT void command_shutdown(void)
{
  if (mineserver->plugin.getPluginVersion("command") <= 0)
  {
    mineserver->screen.log("command is not loaded!");
    return;
  }

}


