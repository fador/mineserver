#include "../../src/plugin_api.h"

PLUGIN_API_EXPORT void test_2_init(mineserver_pointer_struct* mineserver)
{
  mineserver->logger.log("test_2_init");
}

PLUGIN_API_EXPORT void test_2_shutdown(void)
{
}
