#ifndef VEHICLE_H
#define VEHICLE_H

#include <../../home/matthew/Desktop/mineserver/include/entity.h>

//I really didn't want to call this class "Object". It just sounds too generic.
//It's spawned with PACKET_ADD_OBJECT
class Vehicle : public Entity
{
public:
    Vehicle();
    Vehicle(int8_t type, int32_t x,int32_t y,int32_t z);
    
    virtual void spawnToAll();
};

#endif // VEHICLE_H
