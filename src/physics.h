#ifndef _PHYSICS_H
#define _PHYSICS_H

enum { TYPE_WATER, TYPE_LAVA };
enum { M0, M1, M2, M3, M4, M5, M6, M7, M_FALLING };

struct SimBlock {
  uint8 id;
  int x;
  int y;
  int z;
  uint8 meta;
  
  SimBlock(uint8 id, int x, int y, int z, uint8 meta)
  {
    this->id = id;
    this->x=x;
    this->y=y;
    this->z=z;
    this->meta=meta;
  }
};

struct Sim {
  char type;
  std::vector<SimBlock> blocks;
  
  Sim(char type, SimBlock initblock)
  {
    this->type=type;
    this->blocks.push_back(initblock);
  }
};

class Physics
{
  private:
    Physics() {};
    std::vector<Sim> simList;
  public:
    static Physics &get();
    bool update();
    bool addSimulation(int x, int y, int z);
};

#endif
