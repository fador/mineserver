#ifndef _HOOK_H
#define _HOOK_H

#include <vector>

#include "hook_typedefs.h"

template <class T>
class Hook
{
public:
  typedef T type;

  typename T::ftype getCallback(int n)
  {
    return functions[n];
  }

  std::vector<typename T::ftype>* getCallbacks()
  {
    return &functions;
  }

  int numCallbacks()
  {
    return functions.size();
  }

  void addCallback(typename T::ftype function)
  {
    functions.push_back(function);
  }

  void remCallback(typename T::ftype function)
  {
    int i = 0;
    int s = functions.size();
    for (;i<s;i++)
    {
      if (functions[i] == function)
      {
        functions.erase(i);
        break;
      }
    }
  }

  void doAll(typename T::atype* args)
  {
    int i = 0;
    int s = functions.size();
    for (;i<s;i++)
    {
      T::call(functions[i], args);
    }
  }

  bool doOne(typename T::atype* args)
  {
    int i = 0;
    int s = functions.size();
    for (;i<s;i++)
    {
      if (T::call(functions[i], args))
      {
        return true;
      }
    }
    return false;
  }

  typename T::rtype doThis(int n, typename T::atype* args)
  {
    return T::call(functions[n], args);
  }
private:
  std::vector<typename T::ftype> functions;
};

#endif
