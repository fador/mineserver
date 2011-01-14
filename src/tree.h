#ifndef _TREE_H_
#define _TREE_H_

#include "constants.h"
#include "mineserver.h"
#include "map.h"
#include "vec.h"
#include <stack>

enum { MAX_TRUNK = 8, MIN_TRUNK = 4, MAX_CANOPY = 3, MIN_CANOPY = 1 };

class ITree {
public:
  ITree() { }
  virtual ~ITree() { }
  virtual void update() {
    Mineserver::get()->map()->setBlock(_x, _y, _z, _type, _meta);
    Mineserver::get()->map()->sendBlockChange(_x, _y, _z, _type, _meta);
  }
  virtual void setY(int32_t y) { _y = y; }
  virtual const int32_t getY(void) { return _y; }
  virtual const vec location(void) { return vec(_x,_y,_z); }
  virtual const int type(void) { return _type; }
protected:
  int32_t _x;
  int32_t _y;
  int32_t _z;
  int _type;
  char _meta;
};

class Trunk : public ITree 
{
public:
  Trunk(int32_t x, int32_t y, int32_t z) { _x = x, _y = y, _z = z, _type = BLOCK_LOG, _meta = 0; }
  ~Trunk() { }
protected:
};

class Branch : public ITree
{
public:
  Branch(int32_t x, int32_t y, int32_t z) { _x = x, _y = y, _z = z, _type = BLOCK_LOG, _meta = 0; }
  ~Branch() { }
protected:

};



class Canopy : public ITree 
{
public:
  Canopy(int32_t x, int32_t y, int32_t z) { _x = x, _y = y, _z = z, _type = BLOCK_LEAVES, _meta = 0; }
  Canopy(ITree trunk) { vec loc = trunk.location(); _x = loc.x(), _y = loc.y(), _z = loc.z(), _type = BLOCK_LEAVES, _meta = 0; }
  ~Canopy() { }
protected:
};



class Tree : public ITree
{
public:
  Tree(int32_t x, int32_t y, int32_t z);
  void generate(void);
  ~Tree(void);
protected:
  void set(int32_t xloc, int32_t yloc, int32_t zloc, int blocktType, char metaData);
private:
  std::stack<std::stack<ITree > > m_treeBlocks;
  std::stack<ITree> m_Trunk;
  std::stack<ITree> m_Branches;
  std::stack<ITree> m_Canopy;
  int m_trunkHeight;
  int m_canopyHeight;
  void generateTrunk();
  void generateCanopy();
};

#endif
