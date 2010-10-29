#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <fstream>

#include "logger.h"
#include "constants.h"

#include "config.h"

Conf &Conf::get()
{
  static Conf instance;
  return instance;
}

// Load/reload configuration
bool Conf::load(std::string configFile) 
{
  std::ifstream ifs( configFile.c_str() );
  
  if( ifs.fail() )
  {
    LOG("Failed to open configurationfile");
    return false;
  }
  
  std::string temp;
  
  // Empty configvector (to allow configuration reload)
  confSet.empty();

  while( getline( ifs, temp ) ) 
  {
    // If not commentline
    if(temp[0] != CONFIGPREFIX) 
    {
      int del;
      std::vector<std::string> line;
      while(temp.length() > 0) 
      {
        while(temp[0] == ' ') temp = temp.substr(1);
        del = temp.find(' ');
        if(del > -1)
        {
          line.push_back( temp.substr(0,del) );
          temp = temp.substr(del+1);
        } else {
          line.push_back( temp );
          break;
        }
      }
      
      // Add to configuration
      if(line.size() > 1) 
      {
        // If string -> append
        std::string text;
        if(line[1][0] == '"') 
        {
          for(int i = 1; i < line.size(); i++ ) 
          {
            text += line[i] + " ";
          }
          text = text.substr(1, text.length()-2);
        } else 
        {
          text = line[1];
        }
        confSet.push_back( pair<line[0], text> );
      }
    }
  }
  ifs.close();
  
  return true;
}



// Save configuration
bool Conf::save(std::string configFile) 
{
  /* Read config to vector
  std::ifstream ifs( configFile.c_str() );
  std::string temp;
  
  confSet.empty();

  while( getline( ifs, temp ) ) {
    // If not commentline
    if(temp.substr(0,1) != "#") {
        confSet.push_back( temp );
    }
  }
  ifs.close();*/
  return true;
}


// Return value
std::string Conf::value(std::string name) 
{
    if( confSet.find(name) != std::map::end() ) 
    {
      return confSet[name];
    } else
    {
      return false;
    }
}