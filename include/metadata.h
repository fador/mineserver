/*
   Copyright (c) 2013, The Mineserver Project
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

#ifndef _METAINFO_H
#define _METAINFO_H

#include <vector>
#include "packets.h"
#include "tr1.h"
#ifdef __APPLE__
#include <tr1/memory>
#else
#include TR1INCLUDE(memory)
#endif

class MetaDataElem
{
  public:
    virtual void output(Packet& p) const=0;
    int8_t idx;
};

class MetaDataElemByte : public MetaDataElem
{
  public:
    MetaDataElemByte(int8_t _idx, int8_t _val);
    void output(Packet& p) const;
    int8_t val;
};

typedef std::tr1::shared_ptr<MetaDataElem> MetaDataElemPtr;

/* The MetaInfo class for storing and sending metainfo packages */
class MetaData
{
  friend Packet& operator<<(Packet& p, const MetaData& m);
  public:
    void set(MetaDataElem* p);
    MetaDataElemPtr get(int8_t idx);
  private:
    std::vector<MetaDataElemPtr> m_elem;
};

#endif
