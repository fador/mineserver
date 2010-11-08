#ifndef _MAPGEN_H
#define _MAPGEN_H

class MapGen
{
  private:
    MapGen() {};
    unsigned int noiseSeed;
    double psRand(double x,double y);
    double interpolate(double a,double b,double x);
  public:
    static MapGen &get();
    bool setSeed(unsigned int seed);
    unsigned int getSeed();
    double noise(double x,double y);
};

#endif
