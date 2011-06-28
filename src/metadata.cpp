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
