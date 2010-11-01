
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
    
  Map()
  {  
    for(int i=0;i<256;i++) emitLight[i]=0;
    emitLight[0x0A] = 15; //Lava
    emitLight[0x0B] = 15; //Stationary Lava
    emitLight[0x27] = 1;  //Brown mushroom
    emitLight[0x32] = 14; //Torch
    emitLight[0x33] = 15; //Fire
    emitLight[0x3E] = 14; //Lit furnace
    emitLight[0x4A] = 9;  //Redstone ore (Glowing)
    emitLight[0x4C] = 7;  //Redstone Torch (On)
    emitLight[0x59] = 15; //Lightstone
    emitLight[0x5B] = 15; //Jack-O-Lantern

    for(int i=0;i<256;i++) stopLight[i]=-16;
    stopLight[0x00] = 0; //Empty
    stopLight[0x08] =-3; //Water
    stopLight[0x09] =-3; //Stationary water
    stopLight[0x12] = 0; //Leaves
    stopLight[0x14] = 0; //Glass
    stopLight[0x25] = 0; //Yellow flower
    stopLight[0x26] = 0; //Red rose
    stopLight[0x27] = 0; //Brown mushroom
    stopLight[0x28] = 0; //Red mushroom
    stopLight[0x32] = 0; //Torch
    stopLight[0x4b] = 0; //Redstone Torch (Off)
    stopLight[0x4C] = 0; //Redstone Torch (On)
    stopLight[0x4e] = 0; //Snow
    stopLight[0x4f] =-3; //Ice
  };
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

  //How blocks affect light
  int stopLight[256];

  //Blocks that emit light
  int emitLight[256];

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
  
  // Save whole map to disc (/save command)
  bool saveWholeMap();

  // Generate light maps for chunk
  bool generateLightMaps(int x, int z);

  //Release/save map chunk
  bool releaseMap(int x, int z);

  //Light get/set
  bool getBlockLight(int x, int y, int z, uint8 *blocklight, uint8 *skylight);
  bool setBlockLight(int x, int y, int z, uint8 blocklight, uint8 skylight, uint8 setLight);
  bool lightmapStep(int x, int y, int z, int light);
  bool blocklightmapStep(int x, int y, int z, int light);

  //Block value/meta get/set
  bool getBlock(int x, int y, int z, uint8 *type, uint8 *meta);
  bool setBlock(int x, int y, int z, char type, char meta);

  bool sendBlockChange(int x, int y, int z, char type, char meta);

  bool sendPickupSpawn(spawnedItem item);

  static Map &get();
};



#endif
