
#ifndef _MAP_H
#define _MAP_H

#include "user.h"
#include <map>


typedef struct
{
  int x;
  int z;
  uint8 *fullData;
  uint8 *blocks;
  uint8 *metadata;
  uint8 *blocklight;
  uint8 *skylight;
  int datasize;
} storedMap;

class Map
{
private:
    
  Map() {};
  ~Map()
  {
    //Free all memory
    for (std::map<int, std::map<int, storedMap>>::const_iterator it = maps.begin(); it != maps.end(); ++it)
    {
      for (std::map<int, storedMap>::const_iterator it2 = maps[it->first].begin(); it2 != maps[it->first].end(); ++it2)
      {
        delete [] maps[it->first][it2->first].fullData;
      }
    }
  };
  std::string mapDirectory;
  
public:

  coord spawnPos;
  std::map<int, std::map<int, storedMap>> maps;

  void initMap();
  void freeMap();
  void sendToUser(User *user, int x, int z);

  static Map &getInstance();
};



#endif
