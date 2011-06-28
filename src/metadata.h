#ifndef _METAINFO_H
#define _MATAINFO_H

#include <vector>
#include "packets.h"
#include "tr1.h"
#include TR1INCLUDE(memory)

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
  private:
    int8_t val;
};

typedef std::tr1::shared_ptr<MetaDataElem> MetaDataElemPtr;

/* The MetaInfo class for storing and sending metainfo packages */
class MetaData
{
  friend Packet& operator<<(Packet& p, const MetaData& m);
  public:
    void set(MetaDataElem* p);
  private:
    std::vector<MetaDataElemPtr> m_elem;
};

#endif
