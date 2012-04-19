#include <stdint.h>

class Entity {
public:
  int32_t UID;
  int8_t type;
  int32_t x, y, z;
  bool spawned;

  virtual void spawnToAll() = 0;
  void deSpawnToAll();
  
protected:
  Entity();
  Entity(int8_t type, int32_t x, int32_t y, int32_t z);
};