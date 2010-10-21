
#ifndef _MAP_H
#define _MAP_H

#include "user.h"
#include <map>
#include "nbt.h"

class Map
{
private:
    
  Map() {};
  ~Map()
  {
    //Free all memory
    for (std::map<int, std::map<int, NBT_struct> >::const_iterator it = maps.begin(); it != maps.end(); ++it)
    {
      for (std::map<int, NBT_struct>::const_iterator it2 = maps[it->first].begin(); it2 != maps[it->first].end(); ++it2)
      {
        //TODO: Free memory!
        //delete [] maps[it->first][it2->first].fullData;
      }
    }
  };

  std::string mapDirectory;
  
public:

  coord spawnPos;
  std::map<int, std::map<int, NBT_struct> > maps;

  void initMap();
  void freeMap();
  void sendToUser(User *user, int x, int z);

  static Map &getInstance();
};



#endif
