#include "../../src/plugin_api.h"
#include "../../src/logtype.h"

PLUGIN_API_EXPORT void test_2_init(mineserver_pointer_struct* mineserver)
{
  mineserver->logger.log(LogType::LOG_INFO, "plugin.test_2", "test_2_init");
}

PLUGIN_API_EXPORT void test_2_shutdown(void)
{
}
