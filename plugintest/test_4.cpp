#include <string.h>
#include <cstring>
#include <cstdio>

#include "../../src/plugin_api.h"
#include "../../src/logtype.h"
#include "../../src/hook.h"

mineserver_pointer_struct* mineserver = NULL;

bool test_4a_callback()
{
  mineserver->logger.log(LogType::LOG_INFO, "plugin.test_4a", "test_4a_callback called");

  int i_a = 0, i_b = 0, i_c = 0;
  bool b_a = false, b_b = false;
  char c_a[100], c_b[100], c_c[100];

  // b_a should be true (1), i_a should be 1
  b_a = mineserver->plugin.doUntilTrue("test_4", &i_a);
  // b_b should be false (0), i_b should be 2
  b_b = mineserver->plugin.doUntilFalse("test_4", &i_b);
  // i_c should be 2
  mineserver->plugin.doAll("test_4", &i_c);

  sprintf(c_a, "b_a = %d, i_a = %d", b_a, i_a);
  sprintf(c_b, "b_b = %d, i_b = %d", b_b, i_b);
  sprintf(c_c, "i_c = %d", i_c);

  mineserver->logger.log(LogType::LOG_INFO, "plugin.test_4a", "Callback results:");
  mineserver->logger.log(LogType::LOG_INFO, "plugin.test_4a", c_a);
  mineserver->logger.log(LogType::LOG_INFO, "plugin.test_4a", c_b);
  mineserver->logger.log(LogType::LOG_INFO, "plugin.test_4a", c_c);

  return true;
}

PLUGIN_API_EXPORT void test_4a_init(mineserver_pointer_struct* _mineserver)
{
  mineserver = _mineserver;

  mineserver->logger.log(LogType::LOG_INFO, "plugin.test_4a", "test_4a_init");
  mineserver->plugin.setHook("test_4", new Hook1<bool,int*>);
  mineserver->plugin.addCallback("Timer1000", reinterpret_cast<void*>(&test_4a_callback));
}

PLUGIN_API_EXPORT void test_4a_shutdown(void)
{
}

bool test_4b_callback(int* ptr)
{
  mineserver->logger.log(LogType::LOG_INFO, "plugin.test_4b", "test_4b_callback called");

  ++(*ptr);
  return true;
}

PLUGIN_API_EXPORT void test_4b_init(mineserver_pointer_struct* _mineserver)
{
  mineserver = _mineserver;

  mineserver->logger.log(LogType::LOG_INFO, "plugin.test_4b", "test_4b_init");
  mineserver->plugin.addCallback("test_4", reinterpret_cast<void*>(&test_4b_callback));
}

PLUGIN_API_EXPORT void test_4b_shutdown(void)
{
}

bool test_4c_callback(int* ptr)
{
  mineserver->logger.log(LogType::LOG_INFO, "plugin.test_4c", "test_4c_callback called");

  ++(*ptr);
  return true;
}

PLUGIN_API_EXPORT void test_4c_init(mineserver_pointer_struct* _mineserver)
{
  mineserver = _mineserver;

  mineserver->logger.log(LogType::LOG_INFO, "plugin.test_4c", "test_4c_init");
  mineserver->plugin.addCallback("test_4", reinterpret_cast<void*>(&test_4c_callback));
}

PLUGIN_API_EXPORT void test_4c_shutdown(void)
{
}
