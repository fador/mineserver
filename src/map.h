
#ifndef _MAP_H
#define _MAP_H


class Map
{
private:
    
  Map() {};
  ~Map() {};
  std::string mapDirectory;
public:
  void initMap();
  void freeMap();



  static Map &getInstance();
};



#endif
