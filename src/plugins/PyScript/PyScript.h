#define PVERSION 1.1

#include <Python.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string>
#include <errno.h>
#include <iostream>
#include <dirent.h>
#include <sys/types.h>

//##ifndef PyScript
//##define PyScript
#include "../../plugin_api.h"


using namespace std;

void checkPyErr();

class Script
{
public:
  Script(mineserver_pointer_struct* mineserver, string name);
  Script(const Script& last);
  ~Script();
  PyObject* callPyFunc(const char* name, PyObject* Args);
  void load(string ModName);
private:
  mineserver_pointer_struct* m_mineserver;
  //        PyObject* mod;
  string modname;
};

// Stuff only to be used in C++ plugin, not in python wrapper
#ifndef SWIG
extern mineserver_pointer_struct* ms;
mineserver_pointer_struct* getMineServer();

// Call Back defs
bool chatPreFunction(const char* userIn, time_t timestamp, const char* msgIn);
bool timer200Function();
bool timer500Function();
bool timer1000Function();
bool timer10000Function();
bool blockPlaceFunction(const char* name, int32_t x, int8_t y, int32_t z,
                        int16_t id, int8_t dir);
bool blockBreakFunction(const char* name, int x, int y, int z);




#endif

//class PyScript
//{
//    public:
//        PyScript(mineserver_pointer_struct* mineserver);
//        ~PyScript();
//        int getdir(string dir, vector<string> &files);
//        PyObject* callAllFunc(const char* func, PyObject *args);
//        static PyObject* MineServer_get_version(PyObject *self, PyObject *args);
//    private:
//        mineserver_pointer_struct* m_mineserver;
//        vector<Script*> scripts;
//};

//##endif
