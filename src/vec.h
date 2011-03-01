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

#ifndef _VEC_H
#define _VEC_H

#if defined(__clang__) || defined(__gcc__)
#define VEC_VECTORISED
#endif

class vec
{
private:
#ifdef VEC_VECTORISED
  typedef int ivec __attribute__((vector_size(16)));
  union
  {
    int arrayValue[4];
    ivec vectorValue;
  } data;
  vec(ivec vector)
  {
    data.vectorValue = vector;
  }
#else
  union
  {
    int arrayValue[3];
  } data;
#endif
public:
  vec(int x, int y, int z)
  {
    data.arrayValue[0] = x;
    data.arrayValue[1] = y;
    data.arrayValue[2] = z;
  }
  vec(const int values[3])
  {
    data.arrayValue[0] = values[0];
    data.arrayValue[1] = values[1];
    data.arrayValue[2] = values[2];
  }
  vec()
  {
    data.arrayValue[0] = 0;
    data.arrayValue[1] = 0;
    data.arrayValue[2] = 0;
  }
  vec(const vec& ov)
  {
#ifdef VEC_VECTORISED
    data.vectorValue   = ov.data.vectorValue;
#else
    data.arrayValue[0] = ov.data.arrayValue[0];
    data.arrayValue[1] = ov.data.arrayValue[1];
    data.arrayValue[2] = ov.data.arrayValue[2];
#endif
  }

  vec& operator=(const vec& ov)
  {
#ifdef VEC_VECTORISED
    data.vectorValue   = ov.data.vectorValue;
#else
    data.arrayValue[0] = ov.data.arrayValue[0];
    data.arrayValue[1] = ov.data.arrayValue[1];
    data.arrayValue[2] = ov.data.arrayValue[2];
#endif
    return *this;
  }

  int& x()
  {
    return data.arrayValue[0];
  }
  int& y()
  {
    return data.arrayValue[1];
  }
  int& z()
  {
    return data.arrayValue[2];
  }
  const int& x() const
  {
    return data.arrayValue[0];
  }
  const int& y() const
  {
    return data.arrayValue[1];
  }
  const int& z() const
  {
    return data.arrayValue[2];
  }

  vec operator+(const vec& ov) const
  {
#ifdef VEC_VECTORISED
    return vec(data.vectorValue + ov.data.vectorValue);

#else
    return vec(x() + ov.x(),
               y() + ov.y(),
               z() + ov.z());

#endif
  }
  vec operator-(const vec& ov) const
  {
#ifdef VEC_VECTORISED
    return vec(data.vectorValue - ov.data.vectorValue);

#else
    return vec(x() - ov.x(),
               y() - ov.y(),
               z() - ov.z());

#endif
  }
  vec operator-() const
  {
#ifdef VEC_VECTORISED
    return vec(-data.vectorValue);

#else
    return vec(-x(),
               -y(),
               -z());

#endif
  }

  vec& operator+=(const vec& ov)
  {
    return *this = *this + ov;
  }
  vec& operator-=(const vec& ov)
  {
    return *this = *this - ov;
  }

  int squareLength() const
  {
    return x() * x() + y() * y() + z() * z();
  }

  static int squareDistance(const vec& x, const vec& y)
  {
    return (x - y).squareLength();
  }
};

#endif
