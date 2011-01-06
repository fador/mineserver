#include "../../src/plugin_api.h"
#include "../../src/hook.h"

PLUGIN_API_EXPORT void test_3_init(mineserver_pointer_struct* mineserver)
{
  mineserver->screen.log("test_3_init");
  mineserver->plugin.setHook("test", new Hook0<void>);
}

PLUGIN_API_EXPORT void test_3_shutdown(void)
{
}
