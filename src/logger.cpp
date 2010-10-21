//
// Mineserver logger.cpp
//

#include <cstdio>
//#include <cstdlib>
#include <iostream>
#include <string>
#include "logger.h"

Logger &Logger::get()
{
  static Logger instance;
  return instance;
}

// Log to terminal
void Logger::log(std::string msg) 
{
    std::cout << "> " << msg << std::endl;
}