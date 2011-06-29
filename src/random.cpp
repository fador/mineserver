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

//#include <iostream>
//#include <iomanip>
#include <limits>
#include <ctime>

#ifdef WIN32
#include <wincrypt.h>
#else
#include <fstream>
#endif

#include "random.h"

seed_type prng_seed = 0;

MyRNG prng;

MyUniform m_uniformUINT(0, std::numeric_limits<MyUniform::result_type>::max());

void initPRNG()
{
  bool seedsuccess = false;

#ifdef WIN32
  HCRYPTPROV hCryptProv;
  BYTE* pbData = reinterpret_cast<BYTE*>(&prng_seed);

  if (CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0))
  {
    if (CryptGenRandom(hCryptProv, sizeof(prng_seed), pbData))
    {
      seedsuccess = true;
    }
    CryptReleaseContext(hCryptProv, 0);
  }
#else
  std::ifstream urandom("/dev/urandom");
  if (urandom)
  {
    urandom.read(reinterpret_cast<char*>(&prng_seed), sizeof(prng_seed));
    seedsuccess = true;
  }
#endif

  if (!seedsuccess)
  {
    prng_seed = std::time(NULL);
  }

  //std::cout << "Seeding the PRNG with: 0x" << std::hex << std::uppercase << std::setfill('0')
  //          << std::setw(2 * sizeof(MyRNG::result_type)) << prng_seed << std::endl;

  prng.seed(prng_seed);
}

double uniform01()
{
  return double(m_uniformUINT(prng)) / double(std::numeric_limits<MyUniform::result_type>::max());
}
