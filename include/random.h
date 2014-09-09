/*
   Copyright (c) 2011, The Mineserver Project
   All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of the The Mineserver Project nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MY_RANDOM_H
#define MY_RANDOM_H

/// <Eximius> @MY_RANDOM_H : There seemed to be a conflict with another define of unknown origin
///          (don't use names starting with underscores... compilers are stl libraries like that too)


#include "tr1.h"
#ifdef __APPLE__
#include <tr1/random>
#else
#include TR1INCLUDE(random)
#endif

#include <random>

// This is our core PRNG engine. The Mersenne Twister is both fast and good.

typedef std::mt19937 MyRNG;
/**
#if (defined(__GNUC__))
 * GCC TR1 implementation has a bug, which doesn't produce a correct
 * distribution on 64bit-machines when using mt19937 and uniform_int for 64bit
 * integers together. Because GCC TR1 implementation is frozen, this bug won't
 * be fixed and we need to limit uniform_int to 32bit integer
typedef std::uniform_int<uint32_t> MyUniform;
#else
typedef std::uniform_int<MyRNG::result_type> MyUniform;
#endif
*/ /// not sure! but my right brain says to wing it

typedef std::uniform_int_distribution<MyRNG::result_type> MyUniform;

// Ideally we would have "typedef MyRNG::result_type seed_type", but mt19937 is broken.
// Set this to whatever MyRNG::seed(seed_type) needs to be.

typedef unsigned long int seed_type;


/// Global objects: The PRNG, its seed value, and a 32-bit uniform distribution.
/// Other distribution objects are instantiated locally as needed.

extern MyRNG prng;
extern seed_type prng_seed;
extern MyUniform m_uniformUINT;


/// A function to initialize the PRNG.

void initPRNG();


/// Some global accessor functions for useful distributions.

inline MyRNG::result_type uniformUINT()
{
  return m_uniformUINT(prng);
}

inline MyRNG::result_type uniformUINT(MyUniform::result_type min, MyUniform::result_type max)
{
  MyUniform uni(min, max);
  return uni(prng);
}

inline uint8_t uniformUINT8(uint8_t min, uint8_t max)
{
  std::uniform_int_distribution<uint16_t> uni(min, max);
  return uint8_t(uni(prng));
}

double uniform01();

#define MAKE_UNIFORM_DIST(A,B) static inline unsigned int uniform_ ## A ## _ ## B()\
{\
  MyUniform uni(A, B);\
  return uni(prng);\
}\

#endif
