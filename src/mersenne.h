/*
  Random number generator class
  =============================
  Created - Sarah "Voodoo Doll" White (2006/01/24)
*/

#ifndef _MERSENNE_H
#define _MERSENNE_H

class Random {
  // Arbitrary constants that work well
  static const int           N = 624;
  static const int           M = 397;
  static const unsigned long MATRIX_A = 0x9908b0dfUL;
  static const unsigned long UPPER_MASK = 0x80000000UL;
  static const unsigned long LOWER_MASK = 0x7fffffffUL;
  static const unsigned long MAX = 0xffffffffUL;

  unsigned long x[N]; // Random number pool
  int           next; // Current pool index
public:
  Random(unsigned long seed = 1) : next(0) { seedgen(seed); }

  // Return a uniform deviate in the range [0,1)
  double uniform();
  // Return a uniform deviate in the range [0,hi)
  unsigned uniform(unsigned hi);
  // Return a uniform deviate in the range [lo,hi)
  unsigned uniform(unsigned lo, unsigned hi);
private:
  void seedgen(unsigned long seed);
  unsigned long randgen();
};

#endif
