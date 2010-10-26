
#ifndef _MAP_H
#define _MAP_H

#include "user.h"
#include <map>
#include "nbt.h"


struct spawnedItem
{
  int EID;
  int item;
  char count;
  int x;
  int y;
  int z;
};

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
        releaseMap(it->first, it2->first);
      }
    }
  };

  std::string mapDirectory;
  
public:

  coord spawnPos;

  //Store all maps here
  std::map<int, std::map<int, NBT_struct> > maps;

  //Store the time map chunk has been last used
  std::map<int, std::map<int, int> > mapLastused;

  //Store if map has been modified
  std::map<int, std::map<int, bool> > mapChanged;

  std::vector<spawnedItem> items;

  void initMap();
  void freeMap();
  void sendToUser(User *user, int x, int z);

  //Get pointer to struct
  NBT_struct *getMapData(int x, int z);

  //Load map chunk
  bool loadMap(int x,int z);

   //Save map chunk to disc
  bool saveMap(int x,int z);

  //Release/save map chunk
  bool releaseMap(int x, int z);

  bool getBlock(int x, int y, int z, uint8 *type, uint8 *meta);
  bool setBlock(int x, int y, int z, char type, char meta);

  bool sendBlockChange(int x, int y, int z, char type, char meta);

  bool sendPickupSpawn(spawnedItem item);

  static Map &get();
};



#endif
