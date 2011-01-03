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

#include "../../plugin_api.h"

#include "../../MineServer_wrap.cxx"

using namespace std;

class Script
{
    public:
        Script(Mineserver* mineserver, string name);
        Script(const Script &last);
        ~Script();
        PyObject* callPyFunc(const char* name);
        void load(string ModName);
    private:
        Mineserver * m_mineserver;
        PyObject* mod;
};

class PyScript
{
    public:
        PyScript(Mineserver* mineserver);
        ~PyScript();
        void callbackLogin(bool temp, User* user, bool* kick, std::string* reason);
        int getdir(string dir, vector<string> &files); 
        PyObject* callAllFunc(const char* func);
        static PyObject* MineServer_get_version(PyObject *self, PyObject *args);
    private:
        Mineserver* m_mineserver;
        vector<Script*> scripts;
};

void PyScript::callbackLogin(bool temp, User* user, bool* kick, std::string* reason){
    callAllFunc("player_login");
}

Script::Script(Mineserver* mine, string name): m_mineserver(mine)
{
    load(name);
}

Script::Script(const Script &last){
    mod = last.mod;
}

void Script::load(string ModName){
    PyObject* pName;
    pName = PyString_FromString(ModName.c_str());
    mod = PyImport_Import(pName);
    Py_DECREF(pName);
    callPyFunc("init");
    m_mineserver->screen()->log(ModName);
}

Script::~Script(){
 
}

int PyScript::getdir (string dir, vector<string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        return errno;
    }

    while ((dirp = readdir(dp)) != NULL) {
        files.push_back(string(dirp->d_name));
    }
    closedir(dp);
    return 0;
}

PyObject* Script::callPyFunc(const char* name){
    PyObject  *pValue, *pFunc, *pArgs;
    if(mod != NULL) {
        pFunc = PyObject_GetAttrString(mod, name);
        if(pFunc && PyCallable_Check(pFunc)){
            pArgs = PyTuple_New(0);
            pValue = PyObject_CallObject(pFunc, pArgs);
            Py_DECREF(pArgs);
            if(pValue!=NULL){
                Py_XDECREF(pFunc);
                return pValue;
            }
            Py_DECREF(pValue);
        } else {
            if(PyErr_Occurred())
                PyErr_Print();
        }
        Py_XDECREF(pFunc);
    }
    return NULL;
}

PyScript::PyScript(Mineserver* mineserver) : m_mineserver(mineserver)
{
//    mineserver->plugin()->hookLogin.addCallback(&PyScript::callbackLogin);
    string dir = string("python");
    string filename;
    size_t found,found2,found3;
    vector<string> files = vector<string>();
    getdir(dir, files);
    scripts = vector<Script*>();
    Py_Initialize();
    init_MineServer();
    PyRun_SimpleString("import sys; sys.path.insert(0,\"python\"); sys.path.insert(0,\".\");");
    PyRun_SimpleString("import MineServer");

    for (unsigned int i = 0; i < files.size(); i++){
        found = files[i].rfind(string(".py"));
        found2 = files[i].rfind(string(".pyc"));
        found3 = files[i].find(string("."));
        if(found!=string::npos && found2==string::npos && found3!=0){
            m_mineserver->screen()->log(files[i]);
            filename = files[i].substr(0,found);
            Script thescript = Script(mineserver,filename);
            scripts.push_back(&thescript);
        }
    }
}

PyObject* PyScript::callAllFunc(const char* func){
    unsigned int i;
    for(i = 0; i<scripts.size(); i++){
        Script *sc = scripts.at(i);
        sc->callPyFunc(func);
    }
    return NULL;
}

PyScript::~PyScript(){
    callAllFunc("deinit");
    Py_Finalize();
}

extern "C" void PyScript_init(Mineserver* mineserver){
    if(mineserver->plugin()->hasPointer("PyScript")){
        mineserver->screen()->log("PyScript already loaded!");
        return;
    }
    mineserver->plugin()->setPointer("PyScript", new PyScript(mineserver));
    mineserver->screen()->log("PyScript loaded!");
}

extern "C" void PyScript_shutdown(Mineserver* mineserver){
    if(mineserver->plugin()->hasPointer("PyScript")){
        PyScript* plugin = (PyScript*)mineserver->plugin()->getPointer("PyScript");
        mineserver->plugin()->remPointer("PyScript");
        delete plugin;
    }
    mineserver->screen()->log("PyScript unloaded!");
}


int main(int argc, char **argv)
{
    return 0;
}


