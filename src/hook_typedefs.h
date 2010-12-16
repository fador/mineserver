#ifndef _HOOK_TYPEDEFS_H
#define _HOOK_TYPEDEFS_H

struct arglist {};
struct functor {};

struct arglist0 : arglist
{
  arglist0()
  {
  }
};
template <class R>
struct functor0 : functor
{
  typedef R rtype;
  typedef arglist0 atype;
  typedef R (*ftype)();

  static R call(ftype function, arglist0* args)
  {
    return function();
  }
};

template <class A0>
struct arglist1 : arglist
{
  A0 a0;
  arglist1(A0 _a0)
  {
    a0 = _a0;
  }
};
template <class R, class A0>
struct functor1 : functor
{
  typedef R rtype;
  typedef arglist1<A0> atype;
  typedef R (*ftype)(A0 a0);

  static R call(ftype function, arglist1<A0>* args)
  {
    return function(args->a0);
  }
};

template <class A0, class A1>
struct arglist2 : arglist
{
  A0 a0;
  A1 a1;
  arglist2(A0 _a0, A1 _a1)
  {
    a0 = _a0;
    a1 = _a1;
  }
};
template <class R, class A0, class A1>
struct functor2 : functor
{
  typedef R rtype;
  typedef arglist2<A0, A1> atype;
  typedef R (*ftype)(A0 a0, A1 a1);

  static R call(ftype function, arglist2<A0, A1>* args)
  {
    return function(args->a0, args->a1);
  }
};

template <class A0, class A1, class A2>
struct arglist3 : arglist
{
  A0 a0;
  A1 a1;
  A2 a2;
  arglist3(A0 _a0, A1 _a1, A2 _a2)
  {
    a0 = _a0;
    a1 = _a1;
    a2 = _a2;
  }
};
template <class R, class A0, class A1, class A2>
struct functor3 : functor
{
  typedef R rtype;
  typedef arglist3<A0, A1, A2> atype;
  typedef R (*ftype)(A0 a0, A1 a1, A2 a2);

  static R call(ftype function, arglist3<A0, A1, A2>* args)
  {
    return function(args->a0, args->a1, args->a2);
  }
};

template <class A0, class A1, class A2, class A3>
struct arglist4 : arglist
{
  A0 a0;
  A1 a1;
  A2 a2;
  A3 a3;
  arglist4(A0 _a0, A1 _a1, A2 _a2, A3 _a3)
  {
    a0 = _a0;
    a1 = _a1;
    a2 = _a2;
    a3 = _a3;
  }
};
template <class R, class A0, class A1, class A2, class A3>
struct functor4 : functor
{
  typedef R rtype;
  typedef arglist4<A0, A1, A2, A3> atype;
  typedef R (*ftype)(A0 a0, A1 a1, A2 a2, A3 a3);

  static R call(ftype function, arglist4<A0, A1, A2, A3>* args)
  {
    return function(args->a0, args->a1, args->a2, args->a3);
  }
};

template <class A0, class A1, class A2, class A3, class A4>
struct arglist5 : arglist
{
  A0 a0;
  A1 a1;
  A2 a2;
  A3 a3;
  A4 a4;
  arglist5(A0 _a0, A1 _a1, A2 _a2, A3 _a3, A4 _a4)
  {
    a0 = _a0;
    a1 = _a1;
    a2 = _a2;
    a3 = _a3;
    a4 = _a4;
  }
};
template <class R, class A0, class A1, class A2, class A3, class A4>
struct functor5 : functor
{
  typedef R rtype;
  typedef arglist5<A0, A1, A2, A3, A4> atype;
  typedef R (*ftype)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4);

  static R call(ftype function, arglist5<A0, A1, A2, A3, A4>* args)
  {
    return function(args->a0, args->a1, args->a2, args->a3, args->a4);
  }
};

template <class A0, class A1, class A2, class A3, class A4, class A5>
struct arglist6 : arglist
{
  A0 a0;
  A1 a1;
  A2 a2;
  A3 a3;
  A4 a4;
  A5 a5;
  arglist6(A0 _a0, A1 _a1, A2 _a2, A3 _a3, A4 _a4, A5 _a5)
  {
    a0 = _a0;
    a1 = _a1;
    a2 = _a2;
    a3 = _a3;
    a4 = _a4;
    a5 = _a5;
  }
};
template <class R, class A0, class A1, class A2, class A3, class A4, class A5>
struct functor6 : functor
{
  typedef R rtype;
  typedef arglist6<A0, A1, A2, A3, A4, A5> atype;
  typedef R (*ftype)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5);

  static R call(ftype function, arglist6<A0, A1, A2, A3, A4, A5>* args)
  {
    return function(args->a0, args->a1, args->a2, args->a3, args->a4, args->a5);
  }
};

template <class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct arglist7 : arglist
{
  A0 a0;
  A1 a1;
  A2 a2;
  A3 a3;
  A4 a4;
  A5 a5;
  A6 a6;
  arglist7(A0 _a0, A1 _a1, A2 _a2, A3 _a3, A4 _a4, A5 _a5, A6 _a6)
  {
    a0 = _a0;
    a1 = _a1;
    a2 = _a2;
    a3 = _a3;
    a4 = _a4;
    a5 = _a5;
    a6 = _a6;
  }
};
template <class R, class A0, class A1, class A2, class A3, class A4, class A5, class A6>
struct functor7 : functor
{
  typedef R rtype;
  typedef arglist7<A0, A1, A2, A3, A4, A5, A6> atype;
  typedef R (*ftype)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6);

  static R call(ftype function, arglist7<A0, A1, A2, A3, A4, A5, A6>* args)
  {
    return function(args->a0, args->a1, args->a2, args->a3, args->a4, args->a5, args->a6);
  }
};

template <class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct arglist8 : arglist
{
  A0 a0;
  A1 a1;
  A2 a2;
  A3 a3;
  A4 a4;
  A5 a5;
  A6 a6;
  A7 a7;
  arglist8(A0 _a0, A1 _a1, A2 _a2, A3 _a3, A4 _a4, A5 _a5, A6 _a6, A7 _a7)
  {
    a0 = _a0;
    a1 = _a1;
    a2 = _a2;
    a3 = _a3;
    a4 = _a4;
    a5 = _a5;
    a6 = _a6;
    a7 = _a7;
  }
};
template <class R, class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct functor8 : functor
{
  typedef R rtype;
  typedef arglist8<A0, A1, A2, A3, A4, A5, A6, A7> atype;
  typedef R (*ftype)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7);

  static R call(ftype function, arglist8<A0, A1, A2, A3, A4, A5, A6, A7>* args)
  {
    return function(args->a0, args->a1, args->a2, args->a3, args->a4, args->a5, args->a6, args->a7);
  }
};

template <class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct arglist9 : arglist
{
  A0 a0;
  A1 a1;
  A2 a2;
  A3 a3;
  A4 a4;
  A5 a5;
  A6 a6;
  A7 a7;
  A8 a8;
  arglist9(A0 _a0, A1 _a1, A2 _a2, A3 _a3, A4 _a4, A5 _a5, A6 _a6, A7 _a7, A8 _a8)
  {
    a0 = _a0;
    a1 = _a1;
    a2 = _a2;
    a3 = _a3;
    a4 = _a4;
    a5 = _a5;
    a6 = _a6;
    a7 = _a7;
    a8 = _a8;
  }
};
template <class R, class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct functor9 : functor
{
  typedef R rtype;
  typedef arglist9<A0, A1, A2, A3, A4, A5, A6, A7, A8> atype;
  typedef R (*ftype)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8);

  static R call(ftype function, arglist9<A0, A1, A2, A3, A4, A5, A6, A7, A8>* args)
  {
    return function(args->a0, args->a1, args->a2, args->a3, args->a4, args->a5, args->a6, args->a7, args->a8);
  }
};

template <class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct arglist10 : arglist
{
  A0 a0;
  A1 a1;
  A2 a2;
  A3 a3;
  A4 a4;
  A5 a5;
  A6 a6;
  A7 a7;
  A8 a8;
  A9 a9;
  arglist10(A0 _a0, A1 _a1, A2 _a2, A3 _a3, A4 _a4, A5 _a5, A6 _a6, A7 _a7, A8 _a8, A9 _a9)
  {
    a0 = _a0;
    a1 = _a1;
    a2 = _a2;
    a3 = _a3;
    a4 = _a4;
    a5 = _a5;
    a6 = _a6;
    a7 = _a7;
    a8 = _a8;
    a9 = _a9;
  }
};
template <class R, class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct functor10 : functor
{
  typedef R rtype;
  typedef arglist10<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9> atype;
  typedef R (*ftype)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9);

  static R call(ftype function, arglist10<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9>* args)
  {
    return function(args->a0, args->a1, args->a2, args->a3, args->a4, args->a5, args->a6, args->a7, args->a8, args->a9);
  }
};

template <class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct arglist11 : arglist
{
  A0 a0;
  A1 a1;
  A2 a2;
  A3 a3;
  A4 a4;
  A5 a5;
  A6 a6;
  A7 a7;
  A8 a8;
  A9 a9;
  A10 a10;
  arglist11(A0 _a0, A1 _a1, A2 _a2, A3 _a3, A4 _a4, A5 _a5, A6 _a6, A7 _a7, A8 _a8, A9 _a9, A10 _a10)
  {
    a0 = _a0;
    a1 = _a1;
    a2 = _a2;
    a3 = _a3;
    a4 = _a4;
    a5 = _a5;
    a6 = _a6;
    a7 = _a7;
    a8 = _a8;
    a9 = _a9;
    a10 = _a10;
  }
};
template <class R, class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct functor11 : functor
{
  typedef R rtype;
  typedef arglist11<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10> atype;
  typedef R (*ftype)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10);

  static R call(ftype function, arglist11<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>* args)
  {
    return function(args->a0, args->a1, args->a2, args->a3, args->a4, args->a5, args->a6, args->a7, args->a8, args->a9, args->a10);
  }
};

template <class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct arglist12 : arglist
{
  A0 a0;
  A1 a1;
  A2 a2;
  A3 a3;
  A4 a4;
  A5 a5;
  A6 a6;
  A7 a7;
  A8 a8;
  A9 a9;
  A10 a10;
  A11 a11;
  arglist12(A0 _a0, A1 _a1, A2 _a2, A3 _a3, A4 _a4, A5 _a5, A6 _a6, A7 _a7, A8 _a8, A9 _a9, A10 _a10, A11 _a11)
  {
    a0 = _a0;
    a1 = _a1;
    a2 = _a2;
    a3 = _a3;
    a4 = _a4;
    a5 = _a5;
    a6 = _a6;
    a7 = _a7;
    a8 = _a8;
    a9 = _a9;
    a10 = _a10;
    a11 = _a11;
  }
};
template <class R, class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
struct functor12 : functor
{
  typedef R rtype;
  typedef arglist12<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11> atype;
  typedef R (*ftype)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11);

  static R call(ftype function, arglist12<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>* args)
  {
    return function(args->a0, args->a1, args->a2, args->a3, args->a4, args->a5, args->a6, args->a7, args->a8, args->a9, args->a10, args->a11);
  }
};

template <class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct arglist13 : arglist
{
  A0 a0;
  A1 a1;
  A2 a2;
  A3 a3;
  A4 a4;
  A5 a5;
  A6 a6;
  A7 a7;
  A8 a8;
  A9 a9;
  A10 a10;
  A11 a11;
  A12 a12;
  arglist13(A0 _a0, A1 _a1, A2 _a2, A3 _a3, A4 _a4, A5 _a5, A6 _a6, A7 _a7, A8 _a8, A9 _a9, A10 _a10, A11 _a11, A12 _a12)
  {
    a0 = _a0;
    a1 = _a1;
    a2 = _a2;
    a3 = _a3;
    a4 = _a4;
    a5 = _a5;
    a6 = _a6;
    a7 = _a7;
    a8 = _a8;
    a9 = _a9;
    a10 = _a10;
    a11 = _a11;
    a12 = _a12;
  }
};
template <class R, class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
struct functor13 : functor
{
  typedef R rtype;
  typedef arglist13<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12> atype;
  typedef R (*ftype)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12);

  static R call(ftype function, arglist13<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>* args)
  {
    return function(args->a0, args->a1, args->a2, args->a3, args->a4, args->a5, args->a6, args->a7, args->a8, args->a9, args->a10, args->a11, args->a12);
  }
};

template <class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
struct arglist14 : arglist
{
  A0 a0;
  A1 a1;
  A2 a2;
  A3 a3;
  A4 a4;
  A5 a5;
  A6 a6;
  A7 a7;
  A8 a8;
  A9 a9;
  A10 a10;
  A11 a11;
  A12 a12;
  A13 a13;
  arglist14(A0 _a0, A1 _a1, A2 _a2, A3 _a3, A4 _a4, A5 _a5, A6 _a6, A7 _a7, A8 _a8, A9 _a9, A10 _a10, A11 _a11, A12 _a12, A13 _a13)
  {
    a0 = _a0;
    a1 = _a1;
    a2 = _a2;
    a3 = _a3;
    a4 = _a4;
    a5 = _a5;
    a6 = _a6;
    a7 = _a7;
    a8 = _a8;
    a9 = _a9;
    a10 = _a10;
    a11 = _a11;
    a12 = _a12;
    a13 = _a13;
  }
};
template <class R, class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
struct functor14 : functor
{
  typedef R rtype;
  typedef arglist14<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13> atype;
  typedef R (*ftype)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13);

  static R call(ftype function, arglist14<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13>* args)
  {
    return function(args->a0, args->a1, args->a2, args->a3, args->a4, args->a5, args->a6, args->a7, args->a8, args->a9, args->a10, args->a11, args->a12, args->a13);
  }
};

template <class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
struct arglist15 : arglist
{
  A0 a0;
  A1 a1;
  A2 a2;
  A3 a3;
  A4 a4;
  A5 a5;
  A6 a6;
  A7 a7;
  A8 a8;
  A9 a9;
  A10 a10;
  A11 a11;
  A12 a12;
  A13 a13;
  A14 a14;
  arglist15(A0 _a0, A1 _a1, A2 _a2, A3 _a3, A4 _a4, A5 _a5, A6 _a6, A7 _a7, A8 _a8, A9 _a9, A10 _a10, A11 _a11, A12 _a12, A13 _a13, A14 _a14)
  {
    a0 = _a0;
    a1 = _a1;
    a2 = _a2;
    a3 = _a3;
    a4 = _a4;
    a5 = _a5;
    a6 = _a6;
    a7 = _a7;
    a8 = _a8;
    a9 = _a9;
    a10 = _a10;
    a11 = _a11;
    a12 = _a12;
    a13 = _a13;
    a14 = _a14;
  }
};
template <class R, class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
struct functor15 : functor
{
  typedef R rtype;
  typedef arglist15<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14> atype;
  typedef R (*ftype)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14);

  static R call(ftype function, arglist15<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14>* args)
  {
    return function(args->a0, args->a1, args->a2, args->a3, args->a4, args->a5, args->a6, args->a7, args->a8, args->a9, args->a10, args->a11, args->a12, args->a13, args->a14);
  }
};

template <class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15>
struct arglist16 : arglist
{
  A0 a0;
  A1 a1;
  A2 a2;
  A3 a3;
  A4 a4;
  A5 a5;
  A6 a6;
  A7 a7;
  A8 a8;
  A9 a9;
  A10 a10;
  A11 a11;
  A12 a12;
  A13 a13;
  A14 a14;
  A15 a15;
  arglist16(A0 _a0, A1 _a1, A2 _a2, A3 _a3, A4 _a4, A5 _a5, A6 _a6, A7 _a7, A8 _a8, A9 _a9, A10 _a10, A11 _a11, A12 _a12, A13 _a13, A14 _a14, A15 _a15)
  {
    a0 = _a0;
    a1 = _a1;
    a2 = _a2;
    a3 = _a3;
    a4 = _a4;
    a5 = _a5;
    a6 = _a6;
    a7 = _a7;
    a8 = _a8;
    a9 = _a9;
    a10 = _a10;
    a11 = _a11;
    a12 = _a12;
    a13 = _a13;
    a14 = _a14;
    a15 = _a15;
  }
};
template <class R, class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15>
struct functor16 : functor
{
  typedef R rtype;
  typedef arglist16<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15> atype;
  typedef R (*ftype)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15);

  static R call(ftype function, arglist16<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15>* args)
  {
    return function(args->a0, args->a1, args->a2, args->a3, args->a4, args->a5, args->a6, args->a7, args->a8, args->a9, args->a10, args->a11, args->a12, args->a13, args->a14, args->a15);
  }
};

template <class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15, class A16>
struct arglist17 : arglist
{
  A0 a0;
  A1 a1;
  A2 a2;
  A3 a3;
  A4 a4;
  A5 a5;
  A6 a6;
  A7 a7;
  A8 a8;
  A9 a9;
  A10 a10;
  A11 a11;
  A12 a12;
  A13 a13;
  A14 a14;
  A15 a15;
  A16 a16;
  arglist17(A0 _a0, A1 _a1, A2 _a2, A3 _a3, A4 _a4, A5 _a5, A6 _a6, A7 _a7, A8 _a8, A9 _a9, A10 _a10, A11 _a11, A12 _a12, A13 _a13, A14 _a14, A15 _a15, A16 _a16)
  {
    a0 = _a0;
    a1 = _a1;
    a2 = _a2;
    a3 = _a3;
    a4 = _a4;
    a5 = _a5;
    a6 = _a6;
    a7 = _a7;
    a8 = _a8;
    a9 = _a9;
    a10 = _a10;
    a11 = _a11;
    a12 = _a12;
    a13 = _a13;
    a14 = _a14;
    a15 = _a15;
    a16 = _a16;
  }
};
template <class R, class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15, class A16>
struct functor17 : functor
{
  typedef R rtype;
  typedef arglist17<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16> atype;
  typedef R (*ftype)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16);

  static R call(ftype function, arglist17<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16>* args)
  {
    return function(args->a0, args->a1, args->a2, args->a3, args->a4, args->a5, args->a6, args->a7, args->a8, args->a9, args->a10, args->a11, args->a12, args->a13, args->a14, args->a15, args->a16);
  }
};

template <class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15, class A16, class A17>
struct arglist18 : arglist
{
  A0 a0;
  A1 a1;
  A2 a2;
  A3 a3;
  A4 a4;
  A5 a5;
  A6 a6;
  A7 a7;
  A8 a8;
  A9 a9;
  A10 a10;
  A11 a11;
  A12 a12;
  A13 a13;
  A14 a14;
  A15 a15;
  A16 a16;
  A17 a17;
  arglist18(A0 _a0, A1 _a1, A2 _a2, A3 _a3, A4 _a4, A5 _a5, A6 _a6, A7 _a7, A8 _a8, A9 _a9, A10 _a10, A11 _a11, A12 _a12, A13 _a13, A14 _a14, A15 _a15, A16 _a16, A17 _a17)
  {
    a0 = _a0;
    a1 = _a1;
    a2 = _a2;
    a3 = _a3;
    a4 = _a4;
    a5 = _a5;
    a6 = _a6;
    a7 = _a7;
    a8 = _a8;
    a9 = _a9;
    a10 = _a10;
    a11 = _a11;
    a12 = _a12;
    a13 = _a13;
    a14 = _a14;
    a15 = _a15;
    a16 = _a16;
    a17 = _a17;
  }
};
template <class R, class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15, class A16, class A17>
struct functor18 : functor
{
  typedef R rtype;
  typedef arglist18<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17> atype;
  typedef R (*ftype)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17);

  static R call(ftype function, arglist18<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17>* args)
  {
    return function(args->a0, args->a1, args->a2, args->a3, args->a4, args->a5, args->a6, args->a7, args->a8, args->a9, args->a10, args->a11, args->a12, args->a13, args->a14, args->a15, args->a16, args->a17);
  }
};

template <class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15, class A16, class A17, class A18>
struct arglist19 : arglist
{
  A0 a0;
  A1 a1;
  A2 a2;
  A3 a3;
  A4 a4;
  A5 a5;
  A6 a6;
  A7 a7;
  A8 a8;
  A9 a9;
  A10 a10;
  A11 a11;
  A12 a12;
  A13 a13;
  A14 a14;
  A15 a15;
  A16 a16;
  A17 a17;
  A18 a18;
  arglist19(A0 _a0, A1 _a1, A2 _a2, A3 _a3, A4 _a4, A5 _a5, A6 _a6, A7 _a7, A8 _a8, A9 _a9, A10 _a10, A11 _a11, A12 _a12, A13 _a13, A14 _a14, A15 _a15, A16 _a16, A17 _a17, A18 _a18)
  {
    a0 = _a0;
    a1 = _a1;
    a2 = _a2;
    a3 = _a3;
    a4 = _a4;
    a5 = _a5;
    a6 = _a6;
    a7 = _a7;
    a8 = _a8;
    a9 = _a9;
    a10 = _a10;
    a11 = _a11;
    a12 = _a12;
    a13 = _a13;
    a14 = _a14;
    a15 = _a15;
    a16 = _a16;
    a17 = _a17;
    a18 = _a18;
  }
};
template <class R, class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15, class A16, class A17, class A18>
struct functor19 : functor
{
  typedef R rtype;
  typedef arglist19<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18> atype;
  typedef R (*ftype)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18);

  static R call(ftype function, arglist19<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18>* args)
  {
    return function(args->a0, args->a1, args->a2, args->a3, args->a4, args->a5, args->a6, args->a7, args->a8, args->a9, args->a10, args->a11, args->a12, args->a13, args->a14, args->a15, args->a16, args->a17, args->a18);
  }
};

template <class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15, class A16, class A17, class A18, class A19>
struct arglist20 : arglist
{
  A0 a0;
  A1 a1;
  A2 a2;
  A3 a3;
  A4 a4;
  A5 a5;
  A6 a6;
  A7 a7;
  A8 a8;
  A9 a9;
  A10 a10;
  A11 a11;
  A12 a12;
  A13 a13;
  A14 a14;
  A15 a15;
  A16 a16;
  A17 a17;
  A18 a18;
  A19 a19;
  arglist20(A0 _a0, A1 _a1, A2 _a2, A3 _a3, A4 _a4, A5 _a5, A6 _a6, A7 _a7, A8 _a8, A9 _a9, A10 _a10, A11 _a11, A12 _a12, A13 _a13, A14 _a14, A15 _a15, A16 _a16, A17 _a17, A18 _a18, A19 _a19)
  {
    a0 = _a0;
    a1 = _a1;
    a2 = _a2;
    a3 = _a3;
    a4 = _a4;
    a5 = _a5;
    a6 = _a6;
    a7 = _a7;
    a8 = _a8;
    a9 = _a9;
    a10 = _a10;
    a11 = _a11;
    a12 = _a12;
    a13 = _a13;
    a14 = _a14;
    a15 = _a15;
    a16 = _a16;
    a17 = _a17;
    a18 = _a18;
    a19 = _a19;
  }
};
template <class R, class A0, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15, class A16, class A17, class A18, class A19>
struct functor20 : functor
{
  typedef R rtype;
  typedef arglist20<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19> atype;
  typedef R (*ftype)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19);

  static R call(ftype function, arglist20<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19>* args)
  {
    return function(args->a0, args->a1, args->a2, args->a3, args->a4, args->a5, args->a6, args->a7, args->a8, args->a9, args->a10, args->a11, args->a12, args->a13, args->a14, args->a15, args->a16, args->a17, args->a18, args->a19);
  }
};

#endif
