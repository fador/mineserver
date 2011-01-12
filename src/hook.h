#ifndef _HOOK_H
#define _HOOK_H

#include <vector>
#include <cstdarg>

template <typename T> struct va_widened { typedef T t; };
template <> struct va_widened<signed char> { typedef int t; };
template <> struct va_widened<signed short> { typedef int t; };
template <> struct va_widened<signed int> { typedef int t; };
template <> struct va_widened<unsigned char> { typedef int t; };
template <> struct va_widened<unsigned short> { typedef int t; };
template <> struct va_widened<unsigned int> { typedef int t; };
template <> struct va_widened<float> { typedef double t; };
template <> struct va_widened<double> { typedef double t; };

class Hook
{
public:
  virtual void addCallback(void* function) {}
  virtual bool hasCallback(void* function) {}
  virtual void remCallback(void* function) {}
  virtual bool doUntilTrueVA(va_list vl) { return false; }
  virtual bool doUntilFalseVA(va_list vl) { return false; }
  virtual void doAllVA(va_list vl) {}
};

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

  bool hasCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(void* function)
  {
    return hasCallback(reinterpret_cast<ftype>(function));
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void addCallback(void* function)
  {
    addCallback(reinterpret_cast<ftype>(function));
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

  void remCallback(void* function)
  {
    remCallback(reinterpret_cast<ftype>(function));
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

  void doAllVA(va_list vl)
  {


    doAll();
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

  bool doUntilTrueVA(va_list vl)
  {


    return doUntilTrue();
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

  bool doUntilFalseVA(va_list vl)
  {


    return doUntilFalse();
  }

  R doThis(int n)
  {
    return callbacks[n]();
  }

  R doThisVA(int n, va_list vl)
  {


    return doThis(n);
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

  bool hasCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(void* function)
  {
    return hasCallback(reinterpret_cast<ftype>(function));
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void addCallback(void* function)
  {
    addCallback(reinterpret_cast<ftype>(function));
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

  void remCallback(void* function)
  {
    remCallback(reinterpret_cast<ftype>(function));
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

  void doAllVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));

    doAll(a1);
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

  bool doUntilTrueVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));

    return doUntilTrue(a1);
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

  bool doUntilFalseVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));

    return doUntilFalse(a1);
  }

  R doThis(int n, A1 a1)
  {
    return callbacks[n](a1);
  }

  R doThisVA(int n, va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));

    return doThis(n, a1);
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

  bool hasCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(void* function)
  {
    return hasCallback(reinterpret_cast<ftype>(function));
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void addCallback(void* function)
  {
    addCallback(reinterpret_cast<ftype>(function));
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

  void remCallback(void* function)
  {
    remCallback(reinterpret_cast<ftype>(function));
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

  void doAllVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));

    doAll(a1, a2);
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

  bool doUntilTrueVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));

    return doUntilTrue(a1, a2);
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

  bool doUntilFalseVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));

    return doUntilFalse(a1, a2);
  }

  R doThis(int n, A1 a1, A2 a2)
  {
    return callbacks[n](a1, a2);
  }

  R doThisVA(int n, va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));

    return doThis(n, a1, a2);
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

  bool hasCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(void* function)
  {
    return hasCallback(reinterpret_cast<ftype>(function));
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void addCallback(void* function)
  {
    addCallback(reinterpret_cast<ftype>(function));
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

  void remCallback(void* function)
  {
    remCallback(reinterpret_cast<ftype>(function));
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

  void doAllVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));

    doAll(a1, a2, a3);
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

  bool doUntilTrueVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));

    return doUntilTrue(a1, a2, a3);
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

  bool doUntilFalseVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));

    return doUntilFalse(a1, a2, a3);
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3)
  {
    return callbacks[n](a1, a2, a3);
  }

  R doThisVA(int n, va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));

    return doThis(n, a1, a2, a3);
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

  bool hasCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(void* function)
  {
    return hasCallback(reinterpret_cast<ftype>(function));
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void addCallback(void* function)
  {
    addCallback(reinterpret_cast<ftype>(function));
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

  void remCallback(void* function)
  {
    remCallback(reinterpret_cast<ftype>(function));
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

  void doAllVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));

    doAll(a1, a2, a3, a4);
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

  bool doUntilTrueVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));

    return doUntilTrue(a1, a2, a3, a4);
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

  bool doUntilFalseVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));

    return doUntilFalse(a1, a2, a3, a4);
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4)
  {
    return callbacks[n](a1, a2, a3, a4);
  }

  R doThisVA(int n, va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));

    return doThis(n, a1, a2, a3, a4);
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

  bool hasCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(void* function)
  {
    return hasCallback(reinterpret_cast<ftype>(function));
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void addCallback(void* function)
  {
    addCallback(reinterpret_cast<ftype>(function));
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

  void remCallback(void* function)
  {
    remCallback(reinterpret_cast<ftype>(function));
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

  void doAllVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));

    doAll(a1, a2, a3, a4, a5);
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

  bool doUntilTrueVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));

    return doUntilTrue(a1, a2, a3, a4, a5);
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

  bool doUntilFalseVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));

    return doUntilFalse(a1, a2, a3, a4, a5);
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
  {
    return callbacks[n](a1, a2, a3, a4, a5);
  }

  R doThisVA(int n, va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));

    return doThis(n, a1, a2, a3, a4, a5);
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

  bool hasCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(void* function)
  {
    return hasCallback(reinterpret_cast<ftype>(function));
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void addCallback(void* function)
  {
    addCallback(reinterpret_cast<ftype>(function));
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

  void remCallback(void* function)
  {
    remCallback(reinterpret_cast<ftype>(function));
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

  void doAllVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));
    A6 a6 = static_cast<A6>(va_arg(vl, typename va_widened<A6>::t));

    doAll(a1, a2, a3, a4, a5, a6);
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

  bool doUntilTrueVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));
    A6 a6 = static_cast<A6>(va_arg(vl, typename va_widened<A6>::t));

    return doUntilTrue(a1, a2, a3, a4, a5, a6);
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

  bool doUntilFalseVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));
    A6 a6 = static_cast<A6>(va_arg(vl, typename va_widened<A6>::t));

    return doUntilFalse(a1, a2, a3, a4, a5, a6);
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
  {
    return callbacks[n](a1, a2, a3, a4, a5, a6);
  }

  R doThisVA(int n, va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));
    A6 a6 = static_cast<A6>(va_arg(vl, typename va_widened<A6>::t));

    return doThis(n, a1, a2, a3, a4, a5, a6);
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

  bool hasCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(void* function)
  {
    return hasCallback(reinterpret_cast<ftype>(function));
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void addCallback(void* function)
  {
    addCallback(reinterpret_cast<ftype>(function));
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

  void remCallback(void* function)
  {
    remCallback(reinterpret_cast<ftype>(function));
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

  void doAllVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));
    A6 a6 = static_cast<A6>(va_arg(vl, typename va_widened<A6>::t));
    A7 a7 = static_cast<A7>(va_arg(vl, typename va_widened<A7>::t));

    doAll(a1, a2, a3, a4, a5, a6, a7);
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

  bool doUntilTrueVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));
    A6 a6 = static_cast<A6>(va_arg(vl, typename va_widened<A6>::t));
    A7 a7 = static_cast<A7>(va_arg(vl, typename va_widened<A7>::t));

    return doUntilTrue(a1, a2, a3, a4, a5, a6, a7);
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

  bool doUntilFalseVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));
    A6 a6 = static_cast<A6>(va_arg(vl, typename va_widened<A6>::t));
    A7 a7 = static_cast<A7>(va_arg(vl, typename va_widened<A7>::t));

    return doUntilFalse(a1, a2, a3, a4, a5, a6, a7);
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
  {
    return callbacks[n](a1, a2, a3, a4, a5, a6, a7);
  }

  R doThisVA(int n, va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));
    A6 a6 = static_cast<A6>(va_arg(vl, typename va_widened<A6>::t));
    A7 a7 = static_cast<A7>(va_arg(vl, typename va_widened<A7>::t));

    return doThis(n, a1, a2, a3, a4, a5, a6, a7);
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

  bool hasCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(void* function)
  {
    return hasCallback(reinterpret_cast<ftype>(function));
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void addCallback(void* function)
  {
    addCallback(reinterpret_cast<ftype>(function));
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

  void remCallback(void* function)
  {
    remCallback(reinterpret_cast<ftype>(function));
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

  void doAllVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));
    A6 a6 = static_cast<A6>(va_arg(vl, typename va_widened<A6>::t));
    A7 a7 = static_cast<A7>(va_arg(vl, typename va_widened<A7>::t));
    A8 a8 = static_cast<A8>(va_arg(vl, typename va_widened<A8>::t));

    doAll(a1, a2, a3, a4, a5, a6, a7, a8);
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

  bool doUntilTrueVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));
    A6 a6 = static_cast<A6>(va_arg(vl, typename va_widened<A6>::t));
    A7 a7 = static_cast<A7>(va_arg(vl, typename va_widened<A7>::t));
    A8 a8 = static_cast<A8>(va_arg(vl, typename va_widened<A8>::t));

    return doUntilTrue(a1, a2, a3, a4, a5, a6, a7, a8);
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

  bool doUntilFalseVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));
    A6 a6 = static_cast<A6>(va_arg(vl, typename va_widened<A6>::t));
    A7 a7 = static_cast<A7>(va_arg(vl, typename va_widened<A7>::t));
    A8 a8 = static_cast<A8>(va_arg(vl, typename va_widened<A8>::t));

    return doUntilFalse(a1, a2, a3, a4, a5, a6, a7, a8);
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
  {
    return callbacks[n](a1, a2, a3, a4, a5, a6, a7, a8);
  }

  R doThisVA(int n, va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));
    A6 a6 = static_cast<A6>(va_arg(vl, typename va_widened<A6>::t));
    A7 a7 = static_cast<A7>(va_arg(vl, typename va_widened<A7>::t));
    A8 a8 = static_cast<A8>(va_arg(vl, typename va_widened<A8>::t));

    return doThis(n, a1, a2, a3, a4, a5, a6, a7, a8);
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

  bool hasCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(void* function)
  {
    return hasCallback(reinterpret_cast<ftype>(function));
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void addCallback(void* function)
  {
    addCallback(reinterpret_cast<ftype>(function));
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

  void remCallback(void* function)
  {
    remCallback(reinterpret_cast<ftype>(function));
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

  void doAllVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));
    A6 a6 = static_cast<A6>(va_arg(vl, typename va_widened<A6>::t));
    A7 a7 = static_cast<A7>(va_arg(vl, typename va_widened<A7>::t));
    A8 a8 = static_cast<A8>(va_arg(vl, typename va_widened<A8>::t));
    A9 a9 = static_cast<A9>(va_arg(vl, typename va_widened<A9>::t));

    doAll(a1, a2, a3, a4, a5, a6, a7, a8, a9);
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

  bool doUntilTrueVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));
    A6 a6 = static_cast<A6>(va_arg(vl, typename va_widened<A6>::t));
    A7 a7 = static_cast<A7>(va_arg(vl, typename va_widened<A7>::t));
    A8 a8 = static_cast<A8>(va_arg(vl, typename va_widened<A8>::t));
    A9 a9 = static_cast<A9>(va_arg(vl, typename va_widened<A9>::t));

    return doUntilTrue(a1, a2, a3, a4, a5, a6, a7, a8, a9);
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

  bool doUntilFalseVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));
    A6 a6 = static_cast<A6>(va_arg(vl, typename va_widened<A6>::t));
    A7 a7 = static_cast<A7>(va_arg(vl, typename va_widened<A7>::t));
    A8 a8 = static_cast<A8>(va_arg(vl, typename va_widened<A8>::t));
    A9 a9 = static_cast<A9>(va_arg(vl, typename va_widened<A9>::t));

    return doUntilFalse(a1, a2, a3, a4, a5, a6, a7, a8, a9);
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
  {
    return callbacks[n](a1, a2, a3, a4, a5, a6, a7, a8, a9);
  }

  R doThisVA(int n, va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));
    A6 a6 = static_cast<A6>(va_arg(vl, typename va_widened<A6>::t));
    A7 a7 = static_cast<A7>(va_arg(vl, typename va_widened<A7>::t));
    A8 a8 = static_cast<A8>(va_arg(vl, typename va_widened<A8>::t));
    A9 a9 = static_cast<A9>(va_arg(vl, typename va_widened<A9>::t));

    return doThis(n, a1, a2, a3, a4, a5, a6, a7, a8, a9);
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

  bool hasCallback(ftype function)
  {
    typename std::vector<ftype>::iterator ia = callbacks.begin();
    typename std::vector<ftype>::iterator ib = callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (*ia == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(void* function)
  {
    return hasCallback(reinterpret_cast<ftype>(function));
  }

  void addCallback(ftype function)
  {
    callbacks.push_back(function);
  }

  void addCallback(void* function)
  {
    addCallback(reinterpret_cast<ftype>(function));
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

  void remCallback(void* function)
  {
    remCallback(reinterpret_cast<ftype>(function));
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

  void doAllVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));
    A6 a6 = static_cast<A6>(va_arg(vl, typename va_widened<A6>::t));
    A7 a7 = static_cast<A7>(va_arg(vl, typename va_widened<A7>::t));
    A8 a8 = static_cast<A8>(va_arg(vl, typename va_widened<A8>::t));
    A9 a9 = static_cast<A9>(va_arg(vl, typename va_widened<A9>::t));
    A10 a10 = static_cast<A10>(va_arg(vl, typename va_widened<A10>::t));

    doAll(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
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

  bool doUntilTrueVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));
    A6 a6 = static_cast<A6>(va_arg(vl, typename va_widened<A6>::t));
    A7 a7 = static_cast<A7>(va_arg(vl, typename va_widened<A7>::t));
    A8 a8 = static_cast<A8>(va_arg(vl, typename va_widened<A8>::t));
    A9 a9 = static_cast<A9>(va_arg(vl, typename va_widened<A9>::t));
    A10 a10 = static_cast<A10>(va_arg(vl, typename va_widened<A10>::t));

    return doUntilTrue(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
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

  bool doUntilFalseVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));
    A6 a6 = static_cast<A6>(va_arg(vl, typename va_widened<A6>::t));
    A7 a7 = static_cast<A7>(va_arg(vl, typename va_widened<A7>::t));
    A8 a8 = static_cast<A8>(va_arg(vl, typename va_widened<A8>::t));
    A9 a9 = static_cast<A9>(va_arg(vl, typename va_widened<A9>::t));
    A10 a10 = static_cast<A10>(va_arg(vl, typename va_widened<A10>::t));

    return doUntilFalse(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
  {
    return callbacks[n](a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
  }

  R doThisVA(int n, va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));
    A4 a4 = static_cast<A4>(va_arg(vl, typename va_widened<A4>::t));
    A5 a5 = static_cast<A5>(va_arg(vl, typename va_widened<A5>::t));
    A6 a6 = static_cast<A6>(va_arg(vl, typename va_widened<A6>::t));
    A7 a7 = static_cast<A7>(va_arg(vl, typename va_widened<A7>::t));
    A8 a8 = static_cast<A8>(va_arg(vl, typename va_widened<A8>::t));
    A9 a9 = static_cast<A9>(va_arg(vl, typename va_widened<A9>::t));
    A10 a10 = static_cast<A10>(va_arg(vl, typename va_widened<A10>::t));

    return doThis(n, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
  }

private:
  std::vector<ftype> callbacks;
};

#endif
