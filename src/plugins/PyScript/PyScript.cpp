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

using namespace std;

#ifndef SWIG
mineserver_pointer_struct* ms;
#endif

Script* the_script;

void checkPyErr(){
    if(PyErr_Occurred()){
        PyErr_Print();
        int h;
        cin >> h;
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

void Script::load(string ModName){
    modname = ModName;
    PyObject *pName,*Args;
    pName = PyString_FromString(ModName.c_str());
    mod = PyImport_Import(pName);
    checkPyErr();
    Py_DECREF(pName);
    Args = PyTuple_New(0);
    callPyFunc("init",Args);
    checkPyErr();
    Py_DECREF(Args);
}

Script::~Script(){
 
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

PyObject* Script::callPyFunc(const char* name, PyObject* Args){
    PyObject  *pValue, *pFunc;
    if(mod != NULL) {
        pFunc = PyObject_GetAttrString(mod, name);
        if(pFunc && PyCallable_Check(pFunc)){
            pValue = PyObject_CallObject(pFunc, Args);
            if(pValue!=NULL){
                Py_XDECREF(pFunc);
                return pValue;
            }
            Py_XDECREF(pValue);
        } else {
            checkPyErr();
        }
        Py_XDECREF(pFunc);
    }else{
        string str1 = "Module ", str2 = " is null";
        m_mineserver->logger.log(6,"Plugin",(str1+modname+str2).c_str());
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

PLUGIN_API_EXPORT void CALLCONVERSION PyScript_init(mineserver_pointer_struct* mineserver){
    ms  = mineserver;
    getMS()->setMineServer(mineserver);
}

PLUGIN_API_EXPORT void CALLCONVERSION set_name(const char* name){
    std::string script_name = std::string(name);
    size_t pos = script_name.rfind("\\");
    while(pos != string::npos){
        script_name.replace(pos,1,"/"); // Silently ignore backslashes
        pos = script_name.find("\\");
    }
    Py_Initialize();
    //init_MineServer();
    PyRun_SimpleString("import sys;");
    PyRun_SimpleString("sys.path.insert(0,\".\");");
    string f_slash ("/");
    string path_to = script_name.substr(
        0,
        script_name.rfind(f_slash));
    string c1 ("sys.path.insert(0,\"");
    string c2 ("\");");
    string command (c1+path_to+c2);
    PyRun_SimpleString(command.c_str());
    size_t start = script_name.rfind("/")+1;
    size_t len = script_name.rfind(".") - start;
    string filename = script_name.substr(start, len);
    PyRun_SimpleString("import MineServer");
    Script newScript = Script(ms,filename);
    the_script = &newScript;
}

PLUGIN_API_EXPORT void CALLCONVERSION PyScript_shutdown(void){
}
