#define PVERSION 1.1

#include <Python.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string>
#include <errno.h>
#include <iostream>
#include <dirent.h>
#include <sys/types.h>

#include "../../plugin_api.h"
#include "python_plugin_api.h"
#include "PyScript.h"
#include "MineServer_wrap.cxx"

using namespace std;

mineserver_pointer_struct* safe;
mineserver_pointer_struct* ms;
PyObject* mod;


Script* the_script;

void checkPyErr()
{
  if (PyErr_Occurred())
  {
    PyErr_Print();
  }
}


//PyScript* mainPS;

//void PyScript::callbackLogin(bool temp, User* user, bool* kick, std::string* reason){
//    callAllFunc("player_login");
//}

Script::Script(mineserver_pointer_struct* mine, string name): m_mineserver(mine)
{
  load(name);
}

//Script::Script(const Script &last){
//    mod = last.mod;
//}

void Script::load(string ModName)
{
  modname = ModName;
  PyObject* pName, *Args, *Arg;
  pName = PyString_FromString(ModName.c_str());
  ::mod = PyImport_Import(pName);
  checkPyErr();
  Py_DECREF(pName);

  Arg = SWIG_NewPointerObj((void*)m_mineserver, SWIGTYPE_p_mineserver_pointer_struct, 1);
  Args = Py_BuildValue("(O)", Arg);

  callPyFunc("setUpMSPointer", Args);
  Py_XDECREF(Args);
  checkPyErr();
  PyObject* Args2;
  Args2 = PyTuple_New(0);
  callPyFunc("init", Args2);
}

Script::~Script()
{

}

//int PyScript::getdir (string dir, vector<string> &files)
//{
//    DIR *dp;
//    struct dirent *dirp;
//    if((dp  = opendir(dir.c_str())) == NULL) {
//        return errno;
//    }
//
//    while ((dirp = readdir(dp)) != NULL) {
//        files.push_back(string(dirp->d_name));
//    }
//    closedir(dp);
//    return 0;
//}

PyObject* Script::callPyFunc(const char* name, PyObject* Args)
{
  PyObject*  pValue, *pFunc;
  if (::mod != NULL)
  {
    pFunc = PyObject_GetAttrString(::mod, name);
    if (pFunc && PyCallable_Check(pFunc))
    {
      pValue = PyObject_CallObject(pFunc, Args);
      if (pValue != NULL)
      {
        Py_XDECREF(pFunc);
        return pValue;
      }
      Py_XDECREF(pValue);
    }
    else
    {
      checkPyErr();
    }
    Py_XDECREF(pFunc);
  }
  else
  {
    string str1 = "Module ", str2 = " is null";
    m_mineserver->logger.log(6, "Plugin", (str1 + modname + str2).c_str());
  }
  return NULL;
}

//PyScript::PyScript(mineserver_pointer_struct* mineserver) : m_mineserver(mineserver)
//{
////    mineserver->plugin()->hookLogin.addCallback(&PyScript::callbackLogin);
//    string dir = string("python");
//    string filename;
//    size_t found,found2,found3;
//    vector<string> files = vector<string>();
//    getdir(dir, files);
//    scripts = vector<Script*>();
//    Py_Initialize();
//    //init_MineServer();
//    PyRun_SimpleString("import sys; sys.path.insert(0,\"python\"); sys.path.insert(0,\".\");");
//    PyRun_SimpleString("import MineServer");

//    for (unsigned int i = 0; i < files.size(); i++){
//        found = files[i].rfind(string(".py"));
//        found2 = files[i].rfind(string(".pyc"));
//        found3 = files[i].find(string("."));
//        if(found!=string::npos && found2==string::npos && found3!=0){
//            m_mineserver->logger.log(6,"Plugin",("Loading : "+files[i]).c_str());
//            filename = files[i].substr(0,found);
//            Script thescript = Script(mineserver,filename);
//            scripts.push_back(&thescript);
//        }
//    }
//}

//PyObject* PyScript::callAllFunc(const char* func, PyObject* Args){
//    unsigned int i;
//    for(i = 0; i<scripts.size(); i++){
//        Script *sc = scripts.at(i);
//        sc->callPyFunc(func,Args);
//    }
//    return NULL;
//}

//PyScript::~PyScript(){
//    PyObject* Args;
//    Args=PyTuple_New(0);
//    callAllFunc("deinit",Args);
//    Py_DECREF(Args);
//    Py_Finalize();
//}

mineserver_pointer_struct* getMineServer()
{
  if ((int)::ms == 0)
  {
    cout << "Passing zero pointer as MineServer struct! SEGFAULT COMMING" << endl;
  }
  return ::ms;
}

PLUGIN_API_EXPORT void CALLCONVERSION PyScript_init(mineserver_pointer_struct* mineserver)
{
  ::safe  = mineserver;
  //    getMS()->setMineServer(mineserver);
  // Timers
  mineserver->plugin.addCallback("Timer200", (void*)timer200Function);
  mineserver->plugin.addCallback("Timer1000", (void*)timer1000Function);
  mineserver->plugin.addCallback("Timer10000", (void*)timer10000Function);

  // Chat
  mineserver->plugin.addCallback("PlayerChatPre", (void*)chatPreFunction);

  // Map callbacks
  mineserver->plugin.addCallback("BlockPlacePre", (void*)blockPlaceFunction);
  mineserver->plugin.addCallback("BlockBreakPre", (void*)blockBreakFunction);

}

PLUGIN_API_EXPORT void CALLCONVERSION set_name(const char* name)
{
  std::string script_name = std::string(name);
  size_t pos = script_name.rfind("\\");
  while (pos != string::npos)
  {
    script_name.replace(pos, 1, "/"); // Silently ignore backslashes
    pos = script_name.find("\\");
  }
  //    PyImport_AppendInittab("MineServer",init_MineServer);
  Py_Initialize();
  SWIG_init();
  init_MineServer();
  ::ms =::safe;
  PyRun_SimpleString("import sys;");
  PyRun_SimpleString("sys.path.insert(0,\".\");");
  string f_slash("/");
  string path_to = script_name.substr(
                     0,
                     script_name.rfind(f_slash));
  string c1("sys.path.insert(0,\"");
  string c2("\");");
  string command(c1 + path_to + c2);
  PyRun_SimpleString(command.c_str());
  size_t start = script_name.rfind("/") + 1;
  size_t len = script_name.rfind(".") - start;
  string filename = script_name.substr(start, len);
  PyRun_SimpleString("import MineServer");

  Script newScript = Script(::ms, filename);

  the_script = &newScript;
  ::ms =::safe;
}

PLUGIN_API_EXPORT void CALLCONVERSION PyScript_shutdown(void)
{
}



bool chatPreFunction(const char* userIn, time_t timestamp, const char* msgIn)
{
  PyObject* ret, *param;
  param = Py_BuildValue("ss", userIn, msgIn);
  ret = the_script->callPyFunc("cb_chat", param);
  checkPyErr();
  Py_XDECREF(param);
  if (PyBool_Check(ret))
  {
    Py_XDECREF(ret);
    return ret == Py_True;
  }
  Py_XDECREF(ret);

  return true;
}

bool timer200Function()
{
  PyObject* param;
  param = PyTuple_New(0);
  the_script->callPyFunc("cb_timer200", param);
  checkPyErr();
  Py_XDECREF(param);
}

bool timer1000Function()
{
  PyObject* param;
  param = PyTuple_New(0);
  the_script->callPyFunc("cb_timer1000", param);
  checkPyErr();
  Py_XDECREF(param);
}

bool timer10000Function()
{
  PyObject* param;
  param = PyTuple_New(0);
  the_script->callPyFunc("cb_timer10000", param);
  checkPyErr();
  Py_XDECREF(param);
}

bool blockPlaceFunction(const char* name, int32_t x, int8_t y, int32_t z,
                        int16_t id, int8_t dir)
{
  PyObject* param, *Arg, *ret;
  PyBlock* block = get_MS()->map.get_block((int)x, (int)y, (int)z);
  Arg = SWIG_NewPointerObj((void*)block, SWIGTYPE_p_PyBlock, 1);
  param = Py_BuildValue("(sO)", name, Arg);
  ret = the_script->callPyFunc("cb_block_place", param);
  checkPyErr();
  Py_XDECREF(Arg);
  Py_XDECREF(param);
  if (ret && PyBool_Check(ret))
  {
    Py_XDECREF(ret);
    return ret == Py_True;
  }
  Py_XDECREF(ret);
  return true;
}

bool blockBreakFunction(const char* name, int x, int y, int z)
{
  PyObject* param, *Arg, *ret;
  PyBlock* block = get_MS()->map.get_block((int)x, (int)y, (int)z);
  Arg = SWIG_NewPointerObj((void*)block, SWIGTYPE_p_PyBlock, 1);
  param = Py_BuildValue("(sO)", name, Arg);
  ret = the_script->callPyFunc("cb_block_break", param);
  checkPyErr();
  Py_XDECREF(Arg);
  Py_XDECREF(param);
  if (ret && PyBool_Check(ret))
  {
    Py_XDECREF(ret);
    return ret == Py_True;
  }
  Py_XDECREF(ret);
  return true;
}




