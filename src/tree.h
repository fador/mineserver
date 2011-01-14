#ifndef _TREE_H_
#define _TREE_H_

#include "constants.h"
#include "mineserver.h"
#include "map.h"
#include "vec.h"
#include <stack>

enum { MAX_TRUNK = 13, MIN_TRUNK = 4, MAX_CANOPY = 3, MIN_CANOPY = 2 ,
       BRANCHING_HEIGHT= 6, BRANCHING_CHANCE = 7,
TREE_MIN_SPACE = 5 };// Lucky 7 for good branches ^^

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
  uint8_t _type;
  char _meta;
};

class Trunk : public ITree 
{
public:
  Trunk(int32_t x, int32_t y, int32_t z,char meta=0 ) { _x = x, _y = y, _z = z, _type = BLOCK_LOG, _meta = meta; update();}
  ~Trunk() { }
protected:
};

class Canopy : public ITree 
{
public:
  Canopy(int32_t x, int32_t y, int32_t z,char meta=0) { _x = x, _y = y, _z = z, _type = BLOCK_LEAVES, _meta = meta; update();}
  ~Canopy() { }
protected:
};

class Tree : public ITree
{
public:
  Tree(int32_t x, int32_t y, int32_t z,uint8_t limit = MAX_TRUNK);
  void generate(uint8_t);
  ~Tree(void);

  char darkness; //For different tree types :)
protected:
  void set(int32_t xloc, int32_t yloc, int32_t zloc, int blocktType, char metaData);
private:
  Trunk* m_Branch[256]; // 1KB on x86 and 2KB on x86_64 Faster than stack or vector tho :)
                        // With full array of allocated classes it rounds up to...
                        // 3.6KB on x86 :F 4.6KB on x86_64
                        // it is a good enough buffer for absolutely MASSIVE MASSIVE TREES
                        // Like in Avatar *_*
  uint8_t n_branches;
  void generateCanopy();
  void generateBranches(Trunk*);
};

#endif
