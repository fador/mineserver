%module MineServer
%ignore getMineServer();
%feature("autodoc", "1");
%include "std_string.i"
using namespace std;
typedef std::string String;
%{
#include "python_plugin_api.h"

%}
%include "python_plugin_api.h"
%pythoncode %{
def setUpMSPointer(ms):
    get_MS().setMineServer(ms)
%}
