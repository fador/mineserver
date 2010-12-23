#ifndef _HOOK_H
#define _HOOK_H

#include <vector>

class Hook {};

template <class R>
class Hook0 : public Hook
{
public:
  typedef R (*ftype)();

  ftype getCallback(int n)
  {
    return callbacks[n];
  }

  std::vector<ftype>* getCallbacks()
  {
    return &callbacks;
  }

  int numCallbacks()
  {
    return callbacks.size();
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void remCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        callbacks.erase(ia);
        break;
      }
    }
  }

  void doAll()
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      ((ftype)*ia)();
    }
  }

  bool doUntilTrue()
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (((ftype)*ia)())
      {
        return true;
      }
    }

    return false;
  }

  bool doUntilFalse()
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (!((ftype)*ia)())
      {
        return true;
      }
    }

    return false;
  }

  R doThis(int n)
  {
    return callbacks[n]();
  }
private:
  std::vector<ftype> callbacks;
};

template <class R, class A1>
class Hook1 : public Hook
{
public:
  typedef R (*ftype)(A1);

  ftype getCallback(int n)
  {
    return callbacks[n];
  }

  std::vector<ftype>* getCallbacks()
  {
    return &callbacks;
  }

  int numCallbacks()
  {
    return callbacks.size();
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void remCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        callbacks.erase(ia);
        break;
      }
    }
  }

  void doAll(A1 a1)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      ((ftype)*ia)(a1);
    }
  }

  bool doUntilTrue(A1 a1)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (((ftype)*ia)(a1))
      {
        return true;
      }
    }

    return false;
  }

  bool doUntilFalse(A1 a1)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (!((ftype)*ia)(a1))
      {
        return true;
      }
    }

    return false;
  }

  R doThis(int n, A1 a1)
  {
    return callbacks[n](a1);
  }
private:
  std::vector<ftype> callbacks;
};

template <class R, class A1, class A2>
class Hook2 : public Hook
{
public:
  typedef R (*ftype)(A1, A2);

  ftype getCallback(int n)
  {
    return callbacks[n];
  }

  std::vector<ftype>* getCallbacks()
  {
    return &callbacks;
  }

  int numCallbacks()
  {
    return callbacks.size();
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void remCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        callbacks.erase(ia);
        break;
      }
    }
  }

  void doAll(A1 a1, A2 a2)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      ((ftype)*ia)(a1, a2);
    }
  }

  bool doUntilTrue(A1 a1, A2 a2)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (((ftype)*ia)(a1, a2))
      {
        return true;
      }
    }

    return false;
  }

  bool doUntilFalse(A1 a1, A2 a2)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (!((ftype)*ia)(a1, a2))
      {
        return true;
      }
    }

    return false;
  }

  R doThis(int n, A1 a1, A2 a2)
  {
    return callbacks[n](a1, a2);
  }
private:
  std::vector<ftype> callbacks;
};

template <class R, class A1, class A2, class A3>
class Hook3 : public Hook
{
public:
  typedef R (*ftype)(A1, A2, A3);

  ftype getCallback(int n)
  {
    return callbacks[n];
  }

  std::vector<ftype>* getCallbacks()
  {
    return &callbacks;
  }

  int numCallbacks()
  {
    return callbacks.size();
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void remCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        callbacks.erase(ia);
        break;
      }
    }
  }

  void doAll(A1 a1, A2 a2, A3 a3)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      ((ftype)*ia)(a1, a2, a3);
    }
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (((ftype)*ia)(a1, a2, a3))
      {
        return true;
      }
    }

    return false;
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (!((ftype)*ia)(a1, a2, a3))
      {
        return true;
      }
    }

    return false;
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3)
  {
    return callbacks[n](a1, a2, a3);
  }
private:
  std::vector<ftype> callbacks;
};

template <class R, class A1, class A2, class A3, class A4>
class Hook4 : public Hook
{
public:
  typedef R (*ftype)(A1, A2, A3, A4);

  ftype getCallback(int n)
  {
    return callbacks[n];
  }

  std::vector<ftype>* getCallbacks()
  {
    return &callbacks;
  }

  int numCallbacks()
  {
    return callbacks.size();
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void remCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        callbacks.erase(ia);
        break;
      }
    }
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      ((ftype)*ia)(a1, a2, a3, a4);
    }
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (((ftype)*ia)(a1, a2, a3, a4))
      {
        return true;
      }
    }

    return false;
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (!((ftype)*ia)(a1, a2, a3, a4))
      {
        return true;
      }
    }

    return false;
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4)
  {
    return callbacks[n](a1, a2, a3, a4);
  }
private:
  std::vector<ftype> callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5>
class Hook5 : public Hook
{
public:
  typedef R (*ftype)(A1, A2, A3, A4, A5);

  ftype getCallback(int n)
  {
    return callbacks[n];
  }

  std::vector<ftype>* getCallbacks()
  {
    return &callbacks;
  }

  int numCallbacks()
  {
    return callbacks.size();
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void remCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        callbacks.erase(ia);
        break;
      }
    }
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      ((ftype)*ia)(a1, a2, a3, a4, a5);
    }
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (((ftype)*ia)(a1, a2, a3, a4, a5))
      {
        return true;
      }
    }

    return false;
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (!((ftype)*ia)(a1, a2, a3, a4, a5))
      {
        return true;
      }
    }

    return false;
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
  {
    return callbacks[n](a1, a2, a3, a4, a5);
  }
private:
  std::vector<ftype> callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6>
class Hook6 : public Hook
{
public:
  typedef R (*ftype)(A1, A2, A3, A4, A5, A6);

  ftype getCallback(int n)
  {
    return callbacks[n];
  }

  std::vector<ftype>* getCallbacks()
  {
    return &callbacks;
  }

  int numCallbacks()
  {
    return callbacks.size();
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void remCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        callbacks.erase(ia);
        break;
      }
    }
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      ((ftype)*ia)(a1, a2, a3, a4, a5, a6);
    }
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (((ftype)*ia)(a1, a2, a3, a4, a5, a6))
      {
        return true;
      }
    }

    return false;
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (!((ftype)*ia)(a1, a2, a3, a4, a5, a6))
      {
        return true;
      }
    }

    return false;
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
  {
    return callbacks[n](a1, a2, a3, a4, a5, a6);
  }
private:
  std::vector<ftype> callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
class Hook7 : public Hook
{
public:
  typedef R (*ftype)(A1, A2, A3, A4, A5, A6, A7);

  ftype getCallback(int n)
  {
    return callbacks[n];
  }

  std::vector<ftype>* getCallbacks()
  {
    return &callbacks;
  }

  int numCallbacks()
  {
    return callbacks.size();
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void remCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        callbacks.erase(ia);
        break;
      }
    }
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      ((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7);
    }
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7))
      {
        return true;
      }
    }

    return false;
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (!((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7))
      {
        return true;
      }
    }

    return false;
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
  {
    return callbacks[n](a1, a2, a3, a4, a5, a6, a7);
  }
private:
  std::vector<ftype> callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
class Hook8 : public Hook
{
public:
  typedef R (*ftype)(A1, A2, A3, A4, A5, A6, A7, A8);

  ftype getCallback(int n)
  {
    return callbacks[n];
  }

  std::vector<ftype>* getCallbacks()
  {
    return &callbacks;
  }

  int numCallbacks()
  {
    return callbacks.size();
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void remCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        callbacks.erase(ia);
        break;
      }
    }
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      ((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8);
    }
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8))
      {
        return true;
      }
    }

    return false;
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (!((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8))
      {
        return true;
      }
    }

    return false;
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
  {
    return callbacks[n](a1, a2, a3, a4, a5, a6, a7, a8);
  }
private:
  std::vector<ftype> callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
class Hook9 : public Hook
{
public:
  typedef R (*ftype)(A1, A2, A3, A4, A5, A6, A7, A8, A9);

  ftype getCallback(int n)
  {
    return callbacks[n];
  }

  std::vector<ftype>* getCallbacks()
  {
    return &callbacks;
  }

  int numCallbacks()
  {
    return callbacks.size();
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void remCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        callbacks.erase(ia);
        break;
      }
    }
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      ((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9))
      {
        return true;
      }
    }

    return false;
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (!((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9))
      {
        return true;
      }
    }

    return false;
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
  {
    return callbacks[n](a1, a2, a3, a4, a5, a6, a7, a8, a9);
  }
private:
  std::vector<ftype> callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
class Hook10 : public Hook
{
public:
  typedef R (*ftype)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);

  ftype getCallback(int n)
  {
    return callbacks[n];
  }

  std::vector<ftype>* getCallbacks()
  {
    return &callbacks;
  }

  int numCallbacks()
  {
    return callbacks.size();
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void remCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        callbacks.erase(ia);
        break;
      }
    }
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      ((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
    }
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10))
      {
        return true;
      }
    }

    return false;
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (!((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10))
      {
        return true;
      }
    }

    return false;
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
  {
    return callbacks[n](a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
  }
private:
  std::vector<ftype> callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
class Hook11 : public Hook
{
public:
  typedef R (*ftype)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);

  ftype getCallback(int n)
  {
    return callbacks[n];
  }

  std::vector<ftype>* getCallbacks()
  {
    return &callbacks;
  }

  int numCallbacks()
  {
    return callbacks.size();
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void remCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        callbacks.erase(ia);
        break;
      }
    }
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      ((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
    }
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11))
      {
        return true;
      }
    }

    return false;
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (!((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11))
      {
        return true;
      }
    }

    return false;
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
  {
    return callbacks[n](a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
  }
private:
  std::vector<ftype> callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
class Hook12 : public Hook
{
public:
  typedef R (*ftype)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);

  ftype getCallback(int n)
  {
    return callbacks[n];
  }

  std::vector<ftype>* getCallbacks()
  {
    return &callbacks;
  }

  int numCallbacks()
  {
    return callbacks.size();
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void remCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        callbacks.erase(ia);
        break;
      }
    }
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      ((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
    }
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12))
      {
        return true;
      }
    }

    return false;
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (!((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12))
      {
        return true;
      }
    }

    return false;
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
  {
    return callbacks[n](a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
  }
private:
  std::vector<ftype> callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
class Hook13 : public Hook
{
public:
  typedef R (*ftype)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13);

  ftype getCallback(int n)
  {
    return callbacks[n];
  }

  std::vector<ftype>* getCallbacks()
  {
    return &callbacks;
  }

  int numCallbacks()
  {
    return callbacks.size();
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void remCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        callbacks.erase(ia);
        break;
      }
    }
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      ((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
    }
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13))
      {
        return true;
      }
    }

    return false;
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (!((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13))
      {
        return true;
      }
    }

    return false;
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
  {
    return callbacks[n](a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
  }
private:
  std::vector<ftype> callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
class Hook14 : public Hook
{
public:
  typedef R (*ftype)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14);

  ftype getCallback(int n)
  {
    return callbacks[n];
  }

  std::vector<ftype>* getCallbacks()
  {
    return &callbacks;
  }

  int numCallbacks()
  {
    return callbacks.size();
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void remCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        callbacks.erase(ia);
        break;
      }
    }
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      ((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
    }
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14))
      {
        return true;
      }
    }

    return false;
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (!((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14))
      {
        return true;
      }
    }

    return false;
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
  {
    return callbacks[n](a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
  }
private:
  std::vector<ftype> callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15>
class Hook15 : public Hook
{
public:
  typedef R (*ftype)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15);

  ftype getCallback(int n)
  {
    return callbacks[n];
  }

  std::vector<ftype>* getCallbacks()
  {
    return &callbacks;
  }

  int numCallbacks()
  {
    return callbacks.size();
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void remCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        callbacks.erase(ia);
        break;
      }
    }
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      ((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
    }
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15))
      {
        return true;
      }
    }

    return false;
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (!((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15))
      {
        return true;
      }
    }

    return false;
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
  {
    return callbacks[n](a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
  }
private:
  std::vector<ftype> callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15, class A16>
class Hook16 : public Hook
{
public:
  typedef R (*ftype)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16);

  ftype getCallback(int n)
  {
    return callbacks[n];
  }

  std::vector<ftype>* getCallbacks()
  {
    return &callbacks;
  }

  int numCallbacks()
  {
    return callbacks.size();
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void remCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        callbacks.erase(ia);
        break;
      }
    }
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      ((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
    }
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16))
      {
        return true;
      }
    }

    return false;
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (!((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16))
      {
        return true;
      }
    }

    return false;
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
  {
    return callbacks[n](a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
  }
private:
  std::vector<ftype> callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15, class A16, class A17>
class Hook17 : public Hook
{
public:
  typedef R (*ftype)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17);

  ftype getCallback(int n)
  {
    return callbacks[n];
  }

  std::vector<ftype>* getCallbacks()
  {
    return &callbacks;
  }

  int numCallbacks()
  {
    return callbacks.size();
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void remCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        callbacks.erase(ia);
        break;
      }
    }
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      ((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
    }
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17))
      {
        return true;
      }
    }

    return false;
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (!((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17))
      {
        return true;
      }
    }

    return false;
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17)
  {
    return callbacks[n](a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
  }
private:
  std::vector<ftype> callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15, class A16, class A17, class A18>
class Hook18 : public Hook
{
public:
  typedef R (*ftype)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18);

  ftype getCallback(int n)
  {
    return callbacks[n];
  }

  std::vector<ftype>* getCallbacks()
  {
    return &callbacks;
  }

  int numCallbacks()
  {
    return callbacks.size();
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void remCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        callbacks.erase(ia);
        break;
      }
    }
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      ((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
    }
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18))
      {
        return true;
      }
    }

    return false;
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (!((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18))
      {
        return true;
      }
    }

    return false;
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18)
  {
    return callbacks[n](a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
  }
private:
  std::vector<ftype> callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15, class A16, class A17, class A18, class A19>
class Hook19 : public Hook
{
public:
  typedef R (*ftype)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19);

  ftype getCallback(int n)
  {
    return callbacks[n];
  }

  std::vector<ftype>* getCallbacks()
  {
    return &callbacks;
  }

  int numCallbacks()
  {
    return callbacks.size();
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void remCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        callbacks.erase(ia);
        break;
      }
    }
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      ((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
    }
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19))
      {
        return true;
      }
    }

    return false;
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (!((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19))
      {
        return true;
      }
    }

    return false;
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19)
  {
    return callbacks[n](a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
  }
private:
  std::vector<ftype> callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15, class A16, class A17, class A18, class A19, class A20>
class Hook20 : public Hook
{
public:
  typedef R (*ftype)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20);

  ftype getCallback(int n)
  {
    return callbacks[n];
  }

  std::vector<ftype>* getCallbacks()
  {
    return &callbacks;
  }

  int numCallbacks()
  {
    return callbacks.size();
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void remCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        callbacks.erase(ia);
        break;
      }
    }
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      ((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    }
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20))
      {
        return true;
      }
    }

    return false;
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (!((ftype)*ia)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20))
      {
        return true;
      }
    }

    return false;
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20)
  {
    return callbacks[n](a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
  }
private:
  std::vector<ftype> callbacks;
};

#endif