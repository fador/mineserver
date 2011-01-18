#ifndef _HOOK_H
#define _HOOK_H

#include <vector>
#include <utility>
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
  virtual void addIdentifiedCallback(void* identifier, void* function) {}
  virtual bool hasCallback(void* function) { return false; }
  virtual void remCallback(void* function) {}
  virtual bool doUntilTrueVA(va_list vl) { return false; }
  virtual bool doUntilFalseVA(va_list vl) { return false; }
  virtual void doAllVA(va_list vl) {}
};

template <class R>
class Hook0 : public Hook
{
public:
  typedef R (*fatype_t)();
  typedef R (*fitype_t)(void*);

  fatype_t getCallback(int n)
  {
    return m_callbacks[n].second;
  }

  void* getCallbackIdentifier(int n)
  {
    return m_callbacks[n].first;
  }

  std::vector<std::pair<void*,void*> >* getCallbacks()
  {
    return &m_callbacks;
  }

  int numCallbacks()
  {
    return m_callbacks.size();
  }

  bool hasCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if ((ia)->second == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(fatype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  bool hasCallback(fitype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, void* function)
  {
    m_callbacks.push_back(std::pair<void*,void*>(identifier,function));
  }

  void addIdentifiedCallback(void* identifier, fatype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, fitype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addCallback(void* function)
  {
    addIdentifiedCallback(NULL, function);
  }

  void addCallback(fatype_t function)
  {
    addCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->second == function)
      {
        m_callbacks.erase(ia);
        break;
      }
    }
  }

  void remCallback(fatype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(fitype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void doAll()
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        (reinterpret_cast<fatype_t>(ia->second))();
      }
      else
      {
        (reinterpret_cast<fitype_t>(ia->second))(ia->first);
      }
    }
  }

  void doAllVA(va_list vl)
  {


    doAll();
  }

  bool doUntilTrue()
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if ((reinterpret_cast<fatype_t>(ia->second))())
        {
          return true;
        }
      }
      else
      {
        if ((reinterpret_cast<fitype_t>(ia->second))(ia->first))
        {
          return true;
        }
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
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if (!(reinterpret_cast<fatype_t>(ia->second))())
        {
          return true;
        }
      }
      else
      {
        if (!(reinterpret_cast<fitype_t>(ia->second))(ia->first))
        {
          return true;
        }
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
    std::pair<void*,void*>* cb = &(m_callbacks[n]);

    if (cb->first == NULL)
    {
      return (reinterpret_cast<fatype_t>(cb->second))();
    }
    else
    {
      return (reinterpret_cast<fitype_t>(cb->second))(cb->first);
    }
  }

  R doThisVA(int n, va_list vl)
  {


    return doThis(n);
  }

private:
  std::vector<std::pair<void*,void*> > m_callbacks;
};

template <class R, class A1>
class Hook1 : public Hook
{
public:
  typedef R (*fatype_t)(A1);
  typedef R (*fitype_t)(void*, A1);

  fatype_t getCallback(int n)
  {
    return m_callbacks[n].second;
  }

  void* getCallbackIdentifier(int n)
  {
    return m_callbacks[n].first;
  }

  std::vector<std::pair<void*,void*> >* getCallbacks()
  {
    return &m_callbacks;
  }

  int numCallbacks()
  {
    return m_callbacks.size();
  }

  bool hasCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if ((ia)->second == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(fatype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  bool hasCallback(fitype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, void* function)
  {
    m_callbacks.push_back(std::pair<void*,void*>(identifier,function));
  }

  void addIdentifiedCallback(void* identifier, fatype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, fitype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addCallback(void* function)
  {
    addIdentifiedCallback(NULL, function);
  }

  void addCallback(fatype_t function)
  {
    addCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->second == function)
      {
        m_callbacks.erase(ia);
        break;
      }
    }
  }

  void remCallback(fatype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(fitype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void doAll(A1 a1)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        (reinterpret_cast<fatype_t>(ia->second))(a1);
      }
      else
      {
        (reinterpret_cast<fitype_t>(ia->second))(ia->first, a1);
      }
    }
  }

  void doAllVA(va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));

    doAll(a1);
  }

  bool doUntilTrue(A1 a1)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if ((reinterpret_cast<fatype_t>(ia->second))(a1))
        {
          return true;
        }
      }
      else
      {
        if ((reinterpret_cast<fitype_t>(ia->second))(ia->first, a1))
        {
          return true;
        }
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
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if (!(reinterpret_cast<fatype_t>(ia->second))(a1))
        {
          return true;
        }
      }
      else
      {
        if (!(reinterpret_cast<fitype_t>(ia->second))(ia->first, a1))
        {
          return true;
        }
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
    std::pair<void*,void*>* cb = &(m_callbacks[n]);

    if (cb->first == NULL)
    {
      return (reinterpret_cast<fatype_t>(cb->second))(a1);
    }
    else
    {
      return (reinterpret_cast<fitype_t>(cb->second))(cb->first, a1);
    }
  }

  R doThisVA(int n, va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));

    return doThis(n, a1);
  }

private:
  std::vector<std::pair<void*,void*> > m_callbacks;
};

template <class R, class A1, class A2>
class Hook2 : public Hook
{
public:
  typedef R (*fatype_t)(A1, A2);
  typedef R (*fitype_t)(void*, A1, A2);

  fatype_t getCallback(int n)
  {
    return m_callbacks[n].second;
  }

  void* getCallbackIdentifier(int n)
  {
    return m_callbacks[n].first;
  }

  std::vector<std::pair<void*,void*> >* getCallbacks()
  {
    return &m_callbacks;
  }

  int numCallbacks()
  {
    return m_callbacks.size();
  }

  bool hasCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if ((ia)->second == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(fatype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  bool hasCallback(fitype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, void* function)
  {
    m_callbacks.push_back(std::pair<void*,void*>(identifier,function));
  }

  void addIdentifiedCallback(void* identifier, fatype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, fitype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addCallback(void* function)
  {
    addIdentifiedCallback(NULL, function);
  }

  void addCallback(fatype_t function)
  {
    addCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->second == function)
      {
        m_callbacks.erase(ia);
        break;
      }
    }
  }

  void remCallback(fatype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(fitype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void doAll(A1 a1, A2 a2)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        (reinterpret_cast<fatype_t>(ia->second))(a1, a2);
      }
      else
      {
        (reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2);
      }
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
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if ((reinterpret_cast<fatype_t>(ia->second))(a1, a2))
        {
          return true;
        }
      }
      else
      {
        if ((reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2))
        {
          return true;
        }
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
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if (!(reinterpret_cast<fatype_t>(ia->second))(a1, a2))
        {
          return true;
        }
      }
      else
      {
        if (!(reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2))
        {
          return true;
        }
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
    std::pair<void*,void*>* cb = &(m_callbacks[n]);

    if (cb->first == NULL)
    {
      return (reinterpret_cast<fatype_t>(cb->second))(a1, a2);
    }
    else
    {
      return (reinterpret_cast<fitype_t>(cb->second))(cb->first, a1, a2);
    }
  }

  R doThisVA(int n, va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));

    return doThis(n, a1, a2);
  }

private:
  std::vector<std::pair<void*,void*> > m_callbacks;
};

template <class R, class A1, class A2, class A3>
class Hook3 : public Hook
{
public:
  typedef R (*fatype_t)(A1, A2, A3);
  typedef R (*fitype_t)(void*, A1, A2, A3);

  fatype_t getCallback(int n)
  {
    return m_callbacks[n].second;
  }

  void* getCallbackIdentifier(int n)
  {
    return m_callbacks[n].first;
  }

  std::vector<std::pair<void*,void*> >* getCallbacks()
  {
    return &m_callbacks;
  }

  int numCallbacks()
  {
    return m_callbacks.size();
  }

  bool hasCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if ((ia)->second == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(fatype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  bool hasCallback(fitype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, void* function)
  {
    m_callbacks.push_back(std::pair<void*,void*>(identifier,function));
  }

  void addIdentifiedCallback(void* identifier, fatype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, fitype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addCallback(void* function)
  {
    addIdentifiedCallback(NULL, function);
  }

  void addCallback(fatype_t function)
  {
    addCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->second == function)
      {
        m_callbacks.erase(ia);
        break;
      }
    }
  }

  void remCallback(fatype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(fitype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void doAll(A1 a1, A2 a2, A3 a3)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        (reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3);
      }
      else
      {
        (reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3);
      }
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
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if ((reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3))
        {
          return true;
        }
      }
      else
      {
        if ((reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3))
        {
          return true;
        }
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
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if (!(reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3))
        {
          return true;
        }
      }
      else
      {
        if (!(reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3))
        {
          return true;
        }
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
    std::pair<void*,void*>* cb = &(m_callbacks[n]);

    if (cb->first == NULL)
    {
      return (reinterpret_cast<fatype_t>(cb->second))(a1, a2, a3);
    }
    else
    {
      return (reinterpret_cast<fitype_t>(cb->second))(cb->first, a1, a2, a3);
    }
  }

  R doThisVA(int n, va_list vl)
  {
    A1 a1 = static_cast<A1>(va_arg(vl, typename va_widened<A1>::t));
    A2 a2 = static_cast<A2>(va_arg(vl, typename va_widened<A2>::t));
    A3 a3 = static_cast<A3>(va_arg(vl, typename va_widened<A3>::t));

    return doThis(n, a1, a2, a3);
  }

private:
  std::vector<std::pair<void*,void*> > m_callbacks;
};

template <class R, class A1, class A2, class A3, class A4>
class Hook4 : public Hook
{
public:
  typedef R (*fatype_t)(A1, A2, A3, A4);
  typedef R (*fitype_t)(void*, A1, A2, A3, A4);

  fatype_t getCallback(int n)
  {
    return m_callbacks[n].second;
  }

  void* getCallbackIdentifier(int n)
  {
    return m_callbacks[n].first;
  }

  std::vector<std::pair<void*,void*> >* getCallbacks()
  {
    return &m_callbacks;
  }

  int numCallbacks()
  {
    return m_callbacks.size();
  }

  bool hasCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if ((ia)->second == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(fatype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  bool hasCallback(fitype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, void* function)
  {
    m_callbacks.push_back(std::pair<void*,void*>(identifier,function));
  }

  void addIdentifiedCallback(void* identifier, fatype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, fitype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addCallback(void* function)
  {
    addIdentifiedCallback(NULL, function);
  }

  void addCallback(fatype_t function)
  {
    addCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->second == function)
      {
        m_callbacks.erase(ia);
        break;
      }
    }
  }

  void remCallback(fatype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(fitype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        (reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4);
      }
      else
      {
        (reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4);
      }
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
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if ((reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4))
        {
          return true;
        }
      }
      else
      {
        if ((reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4))
        {
          return true;
        }
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
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if (!(reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4))
        {
          return true;
        }
      }
      else
      {
        if (!(reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4))
        {
          return true;
        }
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
    std::pair<void*,void*>* cb = &(m_callbacks[n]);

    if (cb->first == NULL)
    {
      return (reinterpret_cast<fatype_t>(cb->second))(a1, a2, a3, a4);
    }
    else
    {
      return (reinterpret_cast<fitype_t>(cb->second))(cb->first, a1, a2, a3, a4);
    }
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
  std::vector<std::pair<void*,void*> > m_callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5>
class Hook5 : public Hook
{
public:
  typedef R (*fatype_t)(A1, A2, A3, A4, A5);
  typedef R (*fitype_t)(void*, A1, A2, A3, A4, A5);

  fatype_t getCallback(int n)
  {
    return m_callbacks[n].second;
  }

  void* getCallbackIdentifier(int n)
  {
    return m_callbacks[n].first;
  }

  std::vector<std::pair<void*,void*> >* getCallbacks()
  {
    return &m_callbacks;
  }

  int numCallbacks()
  {
    return m_callbacks.size();
  }

  bool hasCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if ((ia)->second == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(fatype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  bool hasCallback(fitype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, void* function)
  {
    m_callbacks.push_back(std::pair<void*,void*>(identifier,function));
  }

  void addIdentifiedCallback(void* identifier, fatype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, fitype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addCallback(void* function)
  {
    addIdentifiedCallback(NULL, function);
  }

  void addCallback(fatype_t function)
  {
    addCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->second == function)
      {
        m_callbacks.erase(ia);
        break;
      }
    }
  }

  void remCallback(fatype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(fitype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        (reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5);
      }
      else
      {
        (reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5);
      }
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
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if ((reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5))
        {
          return true;
        }
      }
      else
      {
        if ((reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5))
        {
          return true;
        }
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
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if (!(reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5))
        {
          return true;
        }
      }
      else
      {
        if (!(reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5))
        {
          return true;
        }
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
    std::pair<void*,void*>* cb = &(m_callbacks[n]);

    if (cb->first == NULL)
    {
      return (reinterpret_cast<fatype_t>(cb->second))(a1, a2, a3, a4, a5);
    }
    else
    {
      return (reinterpret_cast<fitype_t>(cb->second))(cb->first, a1, a2, a3, a4, a5);
    }
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
  std::vector<std::pair<void*,void*> > m_callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6>
class Hook6 : public Hook
{
public:
  typedef R (*fatype_t)(A1, A2, A3, A4, A5, A6);
  typedef R (*fitype_t)(void*, A1, A2, A3, A4, A5, A6);

  fatype_t getCallback(int n)
  {
    return m_callbacks[n].second;
  }

  void* getCallbackIdentifier(int n)
  {
    return m_callbacks[n].first;
  }

  std::vector<std::pair<void*,void*> >* getCallbacks()
  {
    return &m_callbacks;
  }

  int numCallbacks()
  {
    return m_callbacks.size();
  }

  bool hasCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if ((ia)->second == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(fatype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  bool hasCallback(fitype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, void* function)
  {
    m_callbacks.push_back(std::pair<void*,void*>(identifier,function));
  }

  void addIdentifiedCallback(void* identifier, fatype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, fitype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addCallback(void* function)
  {
    addIdentifiedCallback(NULL, function);
  }

  void addCallback(fatype_t function)
  {
    addCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->second == function)
      {
        m_callbacks.erase(ia);
        break;
      }
    }
  }

  void remCallback(fatype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(fitype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        (reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6);
      }
      else
      {
        (reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6);
      }
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
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if ((reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6))
        {
          return true;
        }
      }
      else
      {
        if ((reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6))
        {
          return true;
        }
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
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if (!(reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6))
        {
          return true;
        }
      }
      else
      {
        if (!(reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6))
        {
          return true;
        }
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
    std::pair<void*,void*>* cb = &(m_callbacks[n]);

    if (cb->first == NULL)
    {
      return (reinterpret_cast<fatype_t>(cb->second))(a1, a2, a3, a4, a5, a6);
    }
    else
    {
      return (reinterpret_cast<fitype_t>(cb->second))(cb->first, a1, a2, a3, a4, a5, a6);
    }
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
  std::vector<std::pair<void*,void*> > m_callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
class Hook7 : public Hook
{
public:
  typedef R (*fatype_t)(A1, A2, A3, A4, A5, A6, A7);
  typedef R (*fitype_t)(void*, A1, A2, A3, A4, A5, A6, A7);

  fatype_t getCallback(int n)
  {
    return m_callbacks[n].second;
  }

  void* getCallbackIdentifier(int n)
  {
    return m_callbacks[n].first;
  }

  std::vector<std::pair<void*,void*> >* getCallbacks()
  {
    return &m_callbacks;
  }

  int numCallbacks()
  {
    return m_callbacks.size();
  }

  bool hasCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if ((ia)->second == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(fatype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  bool hasCallback(fitype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, void* function)
  {
    m_callbacks.push_back(std::pair<void*,void*>(identifier,function));
  }

  void addIdentifiedCallback(void* identifier, fatype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, fitype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addCallback(void* function)
  {
    addIdentifiedCallback(NULL, function);
  }

  void addCallback(fatype_t function)
  {
    addCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->second == function)
      {
        m_callbacks.erase(ia);
        break;
      }
    }
  }

  void remCallback(fatype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(fitype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        (reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7);
      }
      else
      {
        (reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7);
      }
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
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if ((reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7))
        {
          return true;
        }
      }
      else
      {
        if ((reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7))
        {
          return true;
        }
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
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if (!(reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7))
        {
          return true;
        }
      }
      else
      {
        if (!(reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7))
        {
          return true;
        }
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
    std::pair<void*,void*>* cb = &(m_callbacks[n]);

    if (cb->first == NULL)
    {
      return (reinterpret_cast<fatype_t>(cb->second))(a1, a2, a3, a4, a5, a6, a7);
    }
    else
    {
      return (reinterpret_cast<fitype_t>(cb->second))(cb->first, a1, a2, a3, a4, a5, a6, a7);
    }
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
  std::vector<std::pair<void*,void*> > m_callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
class Hook8 : public Hook
{
public:
  typedef R (*fatype_t)(A1, A2, A3, A4, A5, A6, A7, A8);
  typedef R (*fitype_t)(void*, A1, A2, A3, A4, A5, A6, A7, A8);

  fatype_t getCallback(int n)
  {
    return m_callbacks[n].second;
  }

  void* getCallbackIdentifier(int n)
  {
    return m_callbacks[n].first;
  }

  std::vector<std::pair<void*,void*> >* getCallbacks()
  {
    return &m_callbacks;
  }

  int numCallbacks()
  {
    return m_callbacks.size();
  }

  bool hasCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if ((ia)->second == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(fatype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  bool hasCallback(fitype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, void* function)
  {
    m_callbacks.push_back(std::pair<void*,void*>(identifier,function));
  }

  void addIdentifiedCallback(void* identifier, fatype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, fitype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addCallback(void* function)
  {
    addIdentifiedCallback(NULL, function);
  }

  void addCallback(fatype_t function)
  {
    addCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->second == function)
      {
        m_callbacks.erase(ia);
        break;
      }
    }
  }

  void remCallback(fatype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(fitype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        (reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8);
      }
      else
      {
        (reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8);
      }
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
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if ((reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8))
        {
          return true;
        }
      }
      else
      {
        if ((reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8))
        {
          return true;
        }
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
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if (!(reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8))
        {
          return true;
        }
      }
      else
      {
        if (!(reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8))
        {
          return true;
        }
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
    std::pair<void*,void*>* cb = &(m_callbacks[n]);

    if (cb->first == NULL)
    {
      return (reinterpret_cast<fatype_t>(cb->second))(a1, a2, a3, a4, a5, a6, a7, a8);
    }
    else
    {
      return (reinterpret_cast<fitype_t>(cb->second))(cb->first, a1, a2, a3, a4, a5, a6, a7, a8);
    }
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
  std::vector<std::pair<void*,void*> > m_callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
class Hook9 : public Hook
{
public:
  typedef R (*fatype_t)(A1, A2, A3, A4, A5, A6, A7, A8, A9);
  typedef R (*fitype_t)(void*, A1, A2, A3, A4, A5, A6, A7, A8, A9);

  fatype_t getCallback(int n)
  {
    return m_callbacks[n].second;
  }

  void* getCallbackIdentifier(int n)
  {
    return m_callbacks[n].first;
  }

  std::vector<std::pair<void*,void*> >* getCallbacks()
  {
    return &m_callbacks;
  }

  int numCallbacks()
  {
    return m_callbacks.size();
  }

  bool hasCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if ((ia)->second == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(fatype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  bool hasCallback(fitype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, void* function)
  {
    m_callbacks.push_back(std::pair<void*,void*>(identifier,function));
  }

  void addIdentifiedCallback(void* identifier, fatype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, fitype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addCallback(void* function)
  {
    addIdentifiedCallback(NULL, function);
  }

  void addCallback(fatype_t function)
  {
    addCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->second == function)
      {
        m_callbacks.erase(ia);
        break;
      }
    }
  }

  void remCallback(fatype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(fitype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        (reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9);
      }
      else
      {
        (reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9);
      }
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
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if ((reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9))
        {
          return true;
        }
      }
      else
      {
        if ((reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9))
        {
          return true;
        }
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
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if (!(reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9))
        {
          return true;
        }
      }
      else
      {
        if (!(reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9))
        {
          return true;
        }
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
    std::pair<void*,void*>* cb = &(m_callbacks[n]);

    if (cb->first == NULL)
    {
      return (reinterpret_cast<fatype_t>(cb->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
    else
    {
      return (reinterpret_cast<fitype_t>(cb->second))(cb->first, a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
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
  std::vector<std::pair<void*,void*> > m_callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
class Hook10 : public Hook
{
public:
  typedef R (*fatype_t)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);
  typedef R (*fitype_t)(void*, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);

  fatype_t getCallback(int n)
  {
    return m_callbacks[n].second;
  }

  void* getCallbackIdentifier(int n)
  {
    return m_callbacks[n].first;
  }

  std::vector<std::pair<void*,void*> >* getCallbacks()
  {
    return &m_callbacks;
  }

  int numCallbacks()
  {
    return m_callbacks.size();
  }

  bool hasCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if ((ia)->second == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(fatype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  bool hasCallback(fitype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, void* function)
  {
    m_callbacks.push_back(std::pair<void*,void*>(identifier,function));
  }

  void addIdentifiedCallback(void* identifier, fatype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, fitype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addCallback(void* function)
  {
    addIdentifiedCallback(NULL, function);
  }

  void addCallback(fatype_t function)
  {
    addCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->second == function)
      {
        m_callbacks.erase(ia);
        break;
      }
    }
  }

  void remCallback(fatype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(fitype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        (reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
      }
      else
      {
        (reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
      }
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
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if ((reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10))
        {
          return true;
        }
      }
      else
      {
        if ((reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10))
        {
          return true;
        }
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
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if (!(reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10))
        {
          return true;
        }
      }
      else
      {
        if (!(reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10))
        {
          return true;
        }
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
    std::pair<void*,void*>* cb = &(m_callbacks[n]);

    if (cb->first == NULL)
    {
      return (reinterpret_cast<fatype_t>(cb->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
    }
    else
    {
      return (reinterpret_cast<fitype_t>(cb->second))(cb->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
    }
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
  std::vector<std::pair<void*,void*> > m_callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
class Hook11 : public Hook
{
public:
  typedef R (*fatype_t)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);
  typedef R (*fitype_t)(void*, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);

  fatype_t getCallback(int n)
  {
    return m_callbacks[n].second;
  }

  void* getCallbackIdentifier(int n)
  {
    return m_callbacks[n].first;
  }

  std::vector<std::pair<void*,void*> >* getCallbacks()
  {
    return &m_callbacks;
  }

  int numCallbacks()
  {
    return m_callbacks.size();
  }

  bool hasCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if ((ia)->second == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(fatype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  bool hasCallback(fitype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, void* function)
  {
    m_callbacks.push_back(std::pair<void*,void*>(identifier,function));
  }

  void addIdentifiedCallback(void* identifier, fatype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, fitype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addCallback(void* function)
  {
    addIdentifiedCallback(NULL, function);
  }

  void addCallback(fatype_t function)
  {
    addCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->second == function)
      {
        m_callbacks.erase(ia);
        break;
      }
    }
  }

  void remCallback(fatype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(fitype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        (reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
      }
      else
      {
        (reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
      }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));

    doAll(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if ((reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11))
        {
          return true;
        }
      }
      else
      {
        if ((reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11))
        {
          return true;
        }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));

    return doUntilTrue(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if (!(reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11))
        {
          return true;
        }
      }
      else
      {
        if (!(reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11))
        {
          return true;
        }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));

    return doUntilFalse(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
  {
    std::pair<void*,void*>* cb = &(m_callbacks[n]);

    if (cb->first == NULL)
    {
      return (reinterpret_cast<fatype_t>(cb->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
    }
    else
    {
      return (reinterpret_cast<fitype_t>(cb->second))(cb->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
    }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));

    return doThis(n, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
  }

private:
  std::vector<std::pair<void*,void*> > m_callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
class Hook12 : public Hook
{
public:
  typedef R (*fatype_t)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);
  typedef R (*fitype_t)(void*, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);

  fatype_t getCallback(int n)
  {
    return m_callbacks[n].second;
  }

  void* getCallbackIdentifier(int n)
  {
    return m_callbacks[n].first;
  }

  std::vector<std::pair<void*,void*> >* getCallbacks()
  {
    return &m_callbacks;
  }

  int numCallbacks()
  {
    return m_callbacks.size();
  }

  bool hasCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if ((ia)->second == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(fatype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  bool hasCallback(fitype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, void* function)
  {
    m_callbacks.push_back(std::pair<void*,void*>(identifier,function));
  }

  void addIdentifiedCallback(void* identifier, fatype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, fitype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addCallback(void* function)
  {
    addIdentifiedCallback(NULL, function);
  }

  void addCallback(fatype_t function)
  {
    addCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->second == function)
      {
        m_callbacks.erase(ia);
        break;
      }
    }
  }

  void remCallback(fatype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(fitype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        (reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
      }
      else
      {
        (reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
      }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));

    doAll(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if ((reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12))
        {
          return true;
        }
      }
      else
      {
        if ((reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12))
        {
          return true;
        }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));

    return doUntilTrue(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if (!(reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12))
        {
          return true;
        }
      }
      else
      {
        if (!(reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12))
        {
          return true;
        }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));

    return doUntilFalse(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
  {
    std::pair<void*,void*>* cb = &(m_callbacks[n]);

    if (cb->first == NULL)
    {
      return (reinterpret_cast<fatype_t>(cb->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
    }
    else
    {
      return (reinterpret_cast<fitype_t>(cb->second))(cb->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
    }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));

    return doThis(n, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
  }

private:
  std::vector<std::pair<void*,void*> > m_callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
class Hook13 : public Hook
{
public:
  typedef R (*fatype_t)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13);
  typedef R (*fitype_t)(void*, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13);

  fatype_t getCallback(int n)
  {
    return m_callbacks[n].second;
  }

  void* getCallbackIdentifier(int n)
  {
    return m_callbacks[n].first;
  }

  std::vector<std::pair<void*,void*> >* getCallbacks()
  {
    return &m_callbacks;
  }

  int numCallbacks()
  {
    return m_callbacks.size();
  }

  bool hasCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if ((ia)->second == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(fatype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  bool hasCallback(fitype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, void* function)
  {
    m_callbacks.push_back(std::pair<void*,void*>(identifier,function));
  }

  void addIdentifiedCallback(void* identifier, fatype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, fitype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addCallback(void* function)
  {
    addIdentifiedCallback(NULL, function);
  }

  void addCallback(fatype_t function)
  {
    addCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->second == function)
      {
        m_callbacks.erase(ia);
        break;
      }
    }
  }

  void remCallback(fatype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(fitype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        (reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
      }
      else
      {
        (reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
      }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));

    doAll(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if ((reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13))
        {
          return true;
        }
      }
      else
      {
        if ((reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13))
        {
          return true;
        }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));

    return doUntilTrue(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if (!(reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13))
        {
          return true;
        }
      }
      else
      {
        if (!(reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13))
        {
          return true;
        }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));

    return doUntilFalse(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
  {
    std::pair<void*,void*>* cb = &(m_callbacks[n]);

    if (cb->first == NULL)
    {
      return (reinterpret_cast<fatype_t>(cb->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
    }
    else
    {
      return (reinterpret_cast<fitype_t>(cb->second))(cb->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
    }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));

    return doThis(n, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
  }

private:
  std::vector<std::pair<void*,void*> > m_callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
class Hook14 : public Hook
{
public:
  typedef R (*fatype_t)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14);
  typedef R (*fitype_t)(void*, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14);

  fatype_t getCallback(int n)
  {
    return m_callbacks[n].second;
  }

  void* getCallbackIdentifier(int n)
  {
    return m_callbacks[n].first;
  }

  std::vector<std::pair<void*,void*> >* getCallbacks()
  {
    return &m_callbacks;
  }

  int numCallbacks()
  {
    return m_callbacks.size();
  }

  bool hasCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if ((ia)->second == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(fatype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  bool hasCallback(fitype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, void* function)
  {
    m_callbacks.push_back(std::pair<void*,void*>(identifier,function));
  }

  void addIdentifiedCallback(void* identifier, fatype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, fitype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addCallback(void* function)
  {
    addIdentifiedCallback(NULL, function);
  }

  void addCallback(fatype_t function)
  {
    addCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->second == function)
      {
        m_callbacks.erase(ia);
        break;
      }
    }
  }

  void remCallback(fatype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(fitype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        (reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
      }
      else
      {
        (reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
      }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));

    doAll(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if ((reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14))
        {
          return true;
        }
      }
      else
      {
        if ((reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14))
        {
          return true;
        }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));

    return doUntilTrue(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if (!(reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14))
        {
          return true;
        }
      }
      else
      {
        if (!(reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14))
        {
          return true;
        }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));

    return doUntilFalse(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
  {
    std::pair<void*,void*>* cb = &(m_callbacks[n]);

    if (cb->first == NULL)
    {
      return (reinterpret_cast<fatype_t>(cb->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
    }
    else
    {
      return (reinterpret_cast<fitype_t>(cb->second))(cb->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
    }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));

    return doThis(n, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
  }

private:
  std::vector<std::pair<void*,void*> > m_callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15>
class Hook15 : public Hook
{
public:
  typedef R (*fatype_t)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15);
  typedef R (*fitype_t)(void*, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15);

  fatype_t getCallback(int n)
  {
    return m_callbacks[n].second;
  }

  void* getCallbackIdentifier(int n)
  {
    return m_callbacks[n].first;
  }

  std::vector<std::pair<void*,void*> >* getCallbacks()
  {
    return &m_callbacks;
  }

  int numCallbacks()
  {
    return m_callbacks.size();
  }

  bool hasCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if ((ia)->second == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(fatype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  bool hasCallback(fitype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, void* function)
  {
    m_callbacks.push_back(std::pair<void*,void*>(identifier,function));
  }

  void addIdentifiedCallback(void* identifier, fatype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, fitype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addCallback(void* function)
  {
    addIdentifiedCallback(NULL, function);
  }

  void addCallback(fatype_t function)
  {
    addCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->second == function)
      {
        m_callbacks.erase(ia);
        break;
      }
    }
  }

  void remCallback(fatype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(fitype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        (reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
      }
      else
      {
        (reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
      }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));

    doAll(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if ((reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15))
        {
          return true;
        }
      }
      else
      {
        if ((reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15))
        {
          return true;
        }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));

    return doUntilTrue(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if (!(reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15))
        {
          return true;
        }
      }
      else
      {
        if (!(reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15))
        {
          return true;
        }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));

    return doUntilFalse(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
  {
    std::pair<void*,void*>* cb = &(m_callbacks[n]);

    if (cb->first == NULL)
    {
      return (reinterpret_cast<fatype_t>(cb->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
    }
    else
    {
      return (reinterpret_cast<fitype_t>(cb->second))(cb->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
    }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));

    return doThis(n, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
  }

private:
  std::vector<std::pair<void*,void*> > m_callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15, class A16>
class Hook16 : public Hook
{
public:
  typedef R (*fatype_t)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16);
  typedef R (*fitype_t)(void*, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16);

  fatype_t getCallback(int n)
  {
    return m_callbacks[n].second;
  }

  void* getCallbackIdentifier(int n)
  {
    return m_callbacks[n].first;
  }

  std::vector<std::pair<void*,void*> >* getCallbacks()
  {
    return &m_callbacks;
  }

  int numCallbacks()
  {
    return m_callbacks.size();
  }

  bool hasCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if ((ia)->second == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(fatype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  bool hasCallback(fitype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, void* function)
  {
    m_callbacks.push_back(std::pair<void*,void*>(identifier,function));
  }

  void addIdentifiedCallback(void* identifier, fatype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, fitype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addCallback(void* function)
  {
    addIdentifiedCallback(NULL, function);
  }

  void addCallback(fatype_t function)
  {
    addCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->second == function)
      {
        m_callbacks.erase(ia);
        break;
      }
    }
  }

  void remCallback(fatype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(fitype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        (reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
      }
      else
      {
        (reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
      }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));
    A16 a16 = static_cast<A16>(va_arg(vl, typename va_widened<A16>::t));

    doAll(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if ((reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16))
        {
          return true;
        }
      }
      else
      {
        if ((reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16))
        {
          return true;
        }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));
    A16 a16 = static_cast<A16>(va_arg(vl, typename va_widened<A16>::t));

    return doUntilTrue(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if (!(reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16))
        {
          return true;
        }
      }
      else
      {
        if (!(reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16))
        {
          return true;
        }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));
    A16 a16 = static_cast<A16>(va_arg(vl, typename va_widened<A16>::t));

    return doUntilFalse(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
  {
    std::pair<void*,void*>* cb = &(m_callbacks[n]);

    if (cb->first == NULL)
    {
      return (reinterpret_cast<fatype_t>(cb->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
    }
    else
    {
      return (reinterpret_cast<fitype_t>(cb->second))(cb->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
    }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));
    A16 a16 = static_cast<A16>(va_arg(vl, typename va_widened<A16>::t));

    return doThis(n, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
  }

private:
  std::vector<std::pair<void*,void*> > m_callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15, class A16, class A17>
class Hook17 : public Hook
{
public:
  typedef R (*fatype_t)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17);
  typedef R (*fitype_t)(void*, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17);

  fatype_t getCallback(int n)
  {
    return m_callbacks[n].second;
  }

  void* getCallbackIdentifier(int n)
  {
    return m_callbacks[n].first;
  }

  std::vector<std::pair<void*,void*> >* getCallbacks()
  {
    return &m_callbacks;
  }

  int numCallbacks()
  {
    return m_callbacks.size();
  }

  bool hasCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if ((ia)->second == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(fatype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  bool hasCallback(fitype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, void* function)
  {
    m_callbacks.push_back(std::pair<void*,void*>(identifier,function));
  }

  void addIdentifiedCallback(void* identifier, fatype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, fitype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addCallback(void* function)
  {
    addIdentifiedCallback(NULL, function);
  }

  void addCallback(fatype_t function)
  {
    addCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->second == function)
      {
        m_callbacks.erase(ia);
        break;
      }
    }
  }

  void remCallback(fatype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(fitype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        (reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
      }
      else
      {
        (reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
      }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));
    A16 a16 = static_cast<A16>(va_arg(vl, typename va_widened<A16>::t));
    A17 a17 = static_cast<A17>(va_arg(vl, typename va_widened<A17>::t));

    doAll(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if ((reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17))
        {
          return true;
        }
      }
      else
      {
        if ((reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17))
        {
          return true;
        }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));
    A16 a16 = static_cast<A16>(va_arg(vl, typename va_widened<A16>::t));
    A17 a17 = static_cast<A17>(va_arg(vl, typename va_widened<A17>::t));

    return doUntilTrue(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if (!(reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17))
        {
          return true;
        }
      }
      else
      {
        if (!(reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17))
        {
          return true;
        }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));
    A16 a16 = static_cast<A16>(va_arg(vl, typename va_widened<A16>::t));
    A17 a17 = static_cast<A17>(va_arg(vl, typename va_widened<A17>::t));

    return doUntilFalse(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17)
  {
    std::pair<void*,void*>* cb = &(m_callbacks[n]);

    if (cb->first == NULL)
    {
      return (reinterpret_cast<fatype_t>(cb->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
    }
    else
    {
      return (reinterpret_cast<fitype_t>(cb->second))(cb->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
    }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));
    A16 a16 = static_cast<A16>(va_arg(vl, typename va_widened<A16>::t));
    A17 a17 = static_cast<A17>(va_arg(vl, typename va_widened<A17>::t));

    return doThis(n, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
  }

private:
  std::vector<std::pair<void*,void*> > m_callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15, class A16, class A17, class A18>
class Hook18 : public Hook
{
public:
  typedef R (*fatype_t)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18);
  typedef R (*fitype_t)(void*, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18);

  fatype_t getCallback(int n)
  {
    return m_callbacks[n].second;
  }

  void* getCallbackIdentifier(int n)
  {
    return m_callbacks[n].first;
  }

  std::vector<std::pair<void*,void*> >* getCallbacks()
  {
    return &m_callbacks;
  }

  int numCallbacks()
  {
    return m_callbacks.size();
  }

  bool hasCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if ((ia)->second == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(fatype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  bool hasCallback(fitype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, void* function)
  {
    m_callbacks.push_back(std::pair<void*,void*>(identifier,function));
  }

  void addIdentifiedCallback(void* identifier, fatype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, fitype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addCallback(void* function)
  {
    addIdentifiedCallback(NULL, function);
  }

  void addCallback(fatype_t function)
  {
    addCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->second == function)
      {
        m_callbacks.erase(ia);
        break;
      }
    }
  }

  void remCallback(fatype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(fitype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        (reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
      }
      else
      {
        (reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
      }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));
    A16 a16 = static_cast<A16>(va_arg(vl, typename va_widened<A16>::t));
    A17 a17 = static_cast<A17>(va_arg(vl, typename va_widened<A17>::t));
    A18 a18 = static_cast<A18>(va_arg(vl, typename va_widened<A18>::t));

    doAll(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if ((reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18))
        {
          return true;
        }
      }
      else
      {
        if ((reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18))
        {
          return true;
        }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));
    A16 a16 = static_cast<A16>(va_arg(vl, typename va_widened<A16>::t));
    A17 a17 = static_cast<A17>(va_arg(vl, typename va_widened<A17>::t));
    A18 a18 = static_cast<A18>(va_arg(vl, typename va_widened<A18>::t));

    return doUntilTrue(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if (!(reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18))
        {
          return true;
        }
      }
      else
      {
        if (!(reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18))
        {
          return true;
        }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));
    A16 a16 = static_cast<A16>(va_arg(vl, typename va_widened<A16>::t));
    A17 a17 = static_cast<A17>(va_arg(vl, typename va_widened<A17>::t));
    A18 a18 = static_cast<A18>(va_arg(vl, typename va_widened<A18>::t));

    return doUntilFalse(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18)
  {
    std::pair<void*,void*>* cb = &(m_callbacks[n]);

    if (cb->first == NULL)
    {
      return (reinterpret_cast<fatype_t>(cb->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
    }
    else
    {
      return (reinterpret_cast<fitype_t>(cb->second))(cb->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
    }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));
    A16 a16 = static_cast<A16>(va_arg(vl, typename va_widened<A16>::t));
    A17 a17 = static_cast<A17>(va_arg(vl, typename va_widened<A17>::t));
    A18 a18 = static_cast<A18>(va_arg(vl, typename va_widened<A18>::t));

    return doThis(n, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
  }

private:
  std::vector<std::pair<void*,void*> > m_callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15, class A16, class A17, class A18, class A19>
class Hook19 : public Hook
{
public:
  typedef R (*fatype_t)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19);
  typedef R (*fitype_t)(void*, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19);

  fatype_t getCallback(int n)
  {
    return m_callbacks[n].second;
  }

  void* getCallbackIdentifier(int n)
  {
    return m_callbacks[n].first;
  }

  std::vector<std::pair<void*,void*> >* getCallbacks()
  {
    return &m_callbacks;
  }

  int numCallbacks()
  {
    return m_callbacks.size();
  }

  bool hasCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if ((ia)->second == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(fatype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  bool hasCallback(fitype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, void* function)
  {
    m_callbacks.push_back(std::pair<void*,void*>(identifier,function));
  }

  void addIdentifiedCallback(void* identifier, fatype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, fitype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addCallback(void* function)
  {
    addIdentifiedCallback(NULL, function);
  }

  void addCallback(fatype_t function)
  {
    addCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->second == function)
      {
        m_callbacks.erase(ia);
        break;
      }
    }
  }

  void remCallback(fatype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(fitype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        (reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
      }
      else
      {
        (reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
      }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));
    A16 a16 = static_cast<A16>(va_arg(vl, typename va_widened<A16>::t));
    A17 a17 = static_cast<A17>(va_arg(vl, typename va_widened<A17>::t));
    A18 a18 = static_cast<A18>(va_arg(vl, typename va_widened<A18>::t));
    A19 a19 = static_cast<A19>(va_arg(vl, typename va_widened<A19>::t));

    doAll(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if ((reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19))
        {
          return true;
        }
      }
      else
      {
        if ((reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19))
        {
          return true;
        }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));
    A16 a16 = static_cast<A16>(va_arg(vl, typename va_widened<A16>::t));
    A17 a17 = static_cast<A17>(va_arg(vl, typename va_widened<A17>::t));
    A18 a18 = static_cast<A18>(va_arg(vl, typename va_widened<A18>::t));
    A19 a19 = static_cast<A19>(va_arg(vl, typename va_widened<A19>::t));

    return doUntilTrue(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if (!(reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19))
        {
          return true;
        }
      }
      else
      {
        if (!(reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19))
        {
          return true;
        }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));
    A16 a16 = static_cast<A16>(va_arg(vl, typename va_widened<A16>::t));
    A17 a17 = static_cast<A17>(va_arg(vl, typename va_widened<A17>::t));
    A18 a18 = static_cast<A18>(va_arg(vl, typename va_widened<A18>::t));
    A19 a19 = static_cast<A19>(va_arg(vl, typename va_widened<A19>::t));

    return doUntilFalse(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19)
  {
    std::pair<void*,void*>* cb = &(m_callbacks[n]);

    if (cb->first == NULL)
    {
      return (reinterpret_cast<fatype_t>(cb->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
    }
    else
    {
      return (reinterpret_cast<fitype_t>(cb->second))(cb->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
    }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));
    A16 a16 = static_cast<A16>(va_arg(vl, typename va_widened<A16>::t));
    A17 a17 = static_cast<A17>(va_arg(vl, typename va_widened<A17>::t));
    A18 a18 = static_cast<A18>(va_arg(vl, typename va_widened<A18>::t));
    A19 a19 = static_cast<A19>(va_arg(vl, typename va_widened<A19>::t));

    return doThis(n, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
  }

private:
  std::vector<std::pair<void*,void*> > m_callbacks;
};

template <class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15, class A16, class A17, class A18, class A19, class A20>
class Hook20 : public Hook
{
public:
  typedef R (*fatype_t)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20);
  typedef R (*fitype_t)(void*, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20);

  fatype_t getCallback(int n)
  {
    return m_callbacks[n].second;
  }

  void* getCallbackIdentifier(int n)
  {
    return m_callbacks[n].first;
  }

  std::vector<std::pair<void*,void*> >* getCallbacks()
  {
    return &m_callbacks;
  }

  int numCallbacks()
  {
    return m_callbacks.size();
  }

  bool hasCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if ((ia)->second == function)
      {
        return true;
      }
    }

    return false;
  }

  bool hasCallback(fatype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  bool hasCallback(fitype_t function)
  {
    return hasCallback(reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, void* function)
  {
    m_callbacks.push_back(std::pair<void*,void*>(identifier,function));
  }

  void addIdentifiedCallback(void* identifier, fatype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addIdentifiedCallback(void* identifier, fitype_t function)
  {
    addIdentifiedCallback(identifier, reinterpret_cast<void*>(function));
  }

  void addCallback(void* function)
  {
    addIdentifiedCallback(NULL, function);
  }

  void addCallback(fatype_t function)
  {
    addCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(void* function)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->second == function)
      {
        m_callbacks.erase(ia);
        break;
      }
    }
  }

  void remCallback(fatype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void remCallback(fitype_t function)
  {
    remCallback(reinterpret_cast<void*>(function));
  }

  void doAll(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        (reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
      }
      else
      {
        (reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
      }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));
    A16 a16 = static_cast<A16>(va_arg(vl, typename va_widened<A16>::t));
    A17 a17 = static_cast<A17>(va_arg(vl, typename va_widened<A17>::t));
    A18 a18 = static_cast<A18>(va_arg(vl, typename va_widened<A18>::t));
    A19 a19 = static_cast<A19>(va_arg(vl, typename va_widened<A19>::t));
    A20 a20 = static_cast<A20>(va_arg(vl, typename va_widened<A20>::t));

    doAll(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
  }

  bool doUntilTrue(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if ((reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20))
        {
          return true;
        }
      }
      else
      {
        if ((reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20))
        {
          return true;
        }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));
    A16 a16 = static_cast<A16>(va_arg(vl, typename va_widened<A16>::t));
    A17 a17 = static_cast<A17>(va_arg(vl, typename va_widened<A17>::t));
    A18 a18 = static_cast<A18>(va_arg(vl, typename va_widened<A18>::t));
    A19 a19 = static_cast<A19>(va_arg(vl, typename va_widened<A19>::t));
    A20 a20 = static_cast<A20>(va_arg(vl, typename va_widened<A20>::t));

    return doUntilTrue(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
  }

  bool doUntilFalse(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20)
  {
    typename std::vector<std::pair<void*,void*> >::iterator ia = m_callbacks.begin();
    typename std::vector<std::pair<void*,void*> >::iterator ib = m_callbacks.end();
    for (;ia!=ib;++ia)
    {
      if (ia->first == NULL)
      {
        if (!(reinterpret_cast<fatype_t>(ia->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20))
        {
          return true;
        }
      }
      else
      {
        if (!(reinterpret_cast<fitype_t>(ia->second))(ia->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20))
        {
          return true;
        }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));
    A16 a16 = static_cast<A16>(va_arg(vl, typename va_widened<A16>::t));
    A17 a17 = static_cast<A17>(va_arg(vl, typename va_widened<A17>::t));
    A18 a18 = static_cast<A18>(va_arg(vl, typename va_widened<A18>::t));
    A19 a19 = static_cast<A19>(va_arg(vl, typename va_widened<A19>::t));
    A20 a20 = static_cast<A20>(va_arg(vl, typename va_widened<A20>::t));

    return doUntilFalse(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
  }

  R doThis(int n, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20)
  {
    std::pair<void*,void*>* cb = &(m_callbacks[n]);

    if (cb->first == NULL)
    {
      return (reinterpret_cast<fatype_t>(cb->second))(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    }
    else
    {
      return (reinterpret_cast<fitype_t>(cb->second))(cb->first, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    }
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
    A11 a11 = static_cast<A11>(va_arg(vl, typename va_widened<A11>::t));
    A12 a12 = static_cast<A12>(va_arg(vl, typename va_widened<A12>::t));
    A13 a13 = static_cast<A13>(va_arg(vl, typename va_widened<A13>::t));
    A14 a14 = static_cast<A14>(va_arg(vl, typename va_widened<A14>::t));
    A15 a15 = static_cast<A15>(va_arg(vl, typename va_widened<A15>::t));
    A16 a16 = static_cast<A16>(va_arg(vl, typename va_widened<A16>::t));
    A17 a17 = static_cast<A17>(va_arg(vl, typename va_widened<A17>::t));
    A18 a18 = static_cast<A18>(va_arg(vl, typename va_widened<A18>::t));
    A19 a19 = static_cast<A19>(va_arg(vl, typename va_widened<A19>::t));
    A20 a20 = static_cast<A20>(va_arg(vl, typename va_widened<A20>::t));

    return doThis(n, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
  }

private:
  std::vector<std::pair<void*,void*> > m_callbacks;
};

#endif
