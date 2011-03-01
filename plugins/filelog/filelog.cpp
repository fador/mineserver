/*
   Copyright (c) 2011, The Mineserver Project
   All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of the The Mineserver Project nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
  Feature List:
    * Logging chat messages to a file
    * Logging server messages to a file
    * Configuration for disabling the chat and/or server features
    * Configuration option for specificing the filename of the server and chat files.
    
  Feature list todo:
    * Linux Makefile
    * Support for splitting chat and server log files per day
    * Configration settings
    ** Split per day
    ** Support for the splitting in the filename, or just append before the .log.
    ** timestamp format (ie strftime format maybe? without the % part)

  Options
    filelog.chat.enable = true;
    filelog.chat.filename = "mineserver_chat.log";
    filelog.chat.daily_split = false;
    filelog.server.enable = true;
    filelog.server.filename = "mineserver.log";
    filelog.server.daily_split = false;
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MINESERVER_C_API
#include "../../src/plugin_api.h"

// This should mirror mineserver/src/logtype.h.
enum LogType
{
    LOG_EMERG,    /** system is unusable */
    LOG_ALERT,    /** action must be taken immediately*/ 
    LOG_CRITICAL, /** critical conditions */
    LOG_ERROR,    /** error conditions */
    LOG_WARNING,  /** warning conditions */
    LOG_NOTICE,   /** normal, but significant, condition */
    LOG_INFO,     /** informational message */
    LOG_DEBUG,    /** debug-level message */
    LOG_COUNT
};

static const char* logTypeName[] = {
  "EMERG",
  "Alert",
  "Critical",
  "Error",
  "Warning",
  "Notice",
  "Info",
  "Debug"
};

// limits, defaults
#define STR_MAXLEN     80
#define FORMAT_TIME    "%Y-%m-%d %H:%M:%S"
#define FILENAME_LOG   "mineserver.log"
#define FILENAME_CHAT  "mineserver_chat.log"

// Constants
#define PLUGIN_NAME "Filelog"
const char* pluginName = PLUGIN_NAME;
const char* logSource = "plugin.filelog";
const float pluginVersion = 1.0f;

// Variables
mineserver_pointer_struct* mineserver;
FILE *logFile;
FILE *chatFile;
const char* formatTimestamp = FORMAT_TIME;

// Configuration Helper functions
bool filelog_config_boolean(const char* key, bool defaultValue)
{
  if (!mineserver->config.has(key)) {
    return defaultValue;
  }
  return mineserver->config.bData(key);
}

const char* filelog_config_string(const char* key, const char*defaultValue)
{
  if (!mineserver->config.has(key)) {
    return defaultValue;
  }
  return mineserver->config.sData(key);
}

bool chatPost(const char* name, time_t time, const char* message)
{
  char str[STR_MAXLEN];
  struct tm* localTime;

  localTime = localtime(&time);
  strftime(str, sizeof(str), formatTimestamp, localTime);
  fprintf(chatFile, "%s <%s> %s\n", str, name, message);
  return false;
}

bool logPost(int type, const char* source, const char* message)
{
  char str[STR_MAXLEN];
  time_t t;
  struct tm* tmLocal;

  t = time(NULL);
  tmLocal = localtime(&t);
  strftime(str, sizeof(str), formatTimestamp, tmLocal);

  if (type >= LOG_COUNT || type < 0) // Unknown log type
    fprintf(logFile, "%s [%d] %s: %s\n", str, type, source, message);
  else
    fprintf(logFile, "%s [%s] %s: %s\n", str, logTypeName[type], source, message);

  return false;
}

PLUGIN_API_EXPORT void CALLCONVERSION filelog_init(mineserver_pointer_struct* mineserver_temp)
{
  mineserver = mineserver_temp;

  if (mineserver->plugin.getPluginVersion(pluginName) > 0)
  {
    mineserver->logger.log(LOG_INFO, logSource, "The " PLUGIN_NAME " plugin is already loaded");
    return;
  }

  mineserver->logger.log(LOG_INFO, logSource, "Loaded " PLUGIN_NAME);
  mineserver->plugin.setPluginVersion(pluginName, pluginVersion);

  // Check if server message logging is enabled, if the option is not specified default to true.
  if (filelog_config_boolean("filelog.server.enable", true))
  {
    const char *filename = filelog_config_string("filelog.server.filename", FILENAME_LOG);
    char *message = (char *)malloc(strlen(filename) + 12); 

    mineserver->plugin.addCallback("LogPost", (void *)logPost);
    logFile = fopen(filename, "a");   
    sprintf(message, "Logging to %s", filename);
    mineserver->logger.log(LOG_INFO, logSource, message);
    free(message);
  }
  else
  {
    mineserver->logger.log(LOG_INFO, logSource, "Logging server messages to file is disabled");
  }

  // Check if chat logging is enabled, if the option is not specified default to true.
  if (filelog_config_boolean("filelog.chat.enable", true))
  {
    const char *filename = filelog_config_string("filelog.chat.filename", FILENAME_CHAT);
    char *message = (char *)malloc(strlen(filename) + 17); 

    mineserver->plugin.addCallback("PlayerChatPost", (void *)chatPost);
    chatFile = fopen(filename, "a");    
    sprintf(message, "Logging chat to %s", filename);
    mineserver->logger.log(LOG_INFO, logSource, message);
    free(message);
  }
  else
  {
    mineserver->logger.log(LOG_INFO, logSource, "Logging chat messages to file is disabled");
  }
}


PLUGIN_API_EXPORT void CALLCONVERSION filelog_shutdown(void)
{
  if (mineserver->plugin.getPluginVersion(pluginName) <= 0)
  {
    mineserver->logger.log(LOG_INFO, logSource, PLUGIN_NAME " is not loaded!");
    return;
  }
  mineserver->logger.log(LOG_INFO, logSource, PLUGIN_NAME " has been unloaded!");
  fclose(logFile);
  fclose(chatFile);
  logFile = NULL;
  chatFile = NULL;
}
