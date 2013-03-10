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

#include "metadata.h"

void MetaDataElemByte::output(Packet& p) const
{
  int header = idx; /* 0 << 5 + idx */
  p << (int8_t)header << (int8_t)val;
}

MetaDataElemByte::MetaDataElemByte(int8_t _idx, int8_t _val)
{
  idx = _idx;
  val = _val;
}

Packet& operator<<(Packet& p, const MetaData& m)
{
  for (std::vector<MetaDataElemPtr>::const_iterator i = m.m_elem.begin(); i != m.m_elem.end(); ++i)
  {
    (*i)->output(p);
  }
  p << (int8_t)127;
  return p;
}

void MetaData::set(MetaDataElem* el)
{
  for (std::vector<MetaDataElemPtr>::iterator i = m_elem.begin(); i != m_elem.end(); ++i)
  {
    if ((*i)->idx == el->idx)
    {
      i->reset(el);
      return;
    }
  }
  m_elem.push_back(MetaDataElemPtr(el));
}

MetaDataElemPtr MetaData::get(int8_t idx)
{
  for (std::vector<MetaDataElemPtr>::iterator i = m_elem.begin(); i != m_elem.end(); ++i)
  {
    if ((*i)->idx == idx)
    {
      return *i;
    }
  }
  return MetaDataElemPtr();
}
