#include <vector>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <deque>
#include <algorithm>
#include <sys/stat.h> 
#ifdef WIN32
  #include <winsock2.h>
  #include <direct.h>
  #define ZLIB_WINAPI
#endif
#include <zlib.h>
#include "constants.h"

#include "logger.h"
#include "tools.h"
#include "map.h"
#include "user.h"
#include "nbt.h"
#include "chat.h"


std::vector<User *> Users;


User::User(int sock, uint32 EID)
{
  this->action=0;
  this->waitForData=false;
  this->fd=sock;
  this->UID=EID;
  this->logged=false;
  // ENABLED FOR DEBUG
  this->admin=true;

  this->pos.x=Map::get().spawnPos.x;
  this->pos.y=Map::get().spawnPos.y;
  this->pos.z=Map::get().spawnPos.z;
}

bool User::changeNick(std::string nick, std::deque<std::string> admins)
{
  this->nick=nick;
  
  // Check adminstatus
  for(unsigned int i=0;i<admins.size();i++) {
    if(admins[i] == nick) {
        this->admin=true;
    }
  }      
  
  return true;
}

User::~User()
{
  if(this->nick.size())
  {
    //Send signal to everyone that the entity is destroyed
    uint8 entityData[5];
    entityData[0]=0x1d; //Destroy entity;
    putSint32(&entityData[1], this->UID);
    this->sendOthers(&entityData[0],5);
  }
}

// Kick player
bool User::kick(std::string kickMsg) 
{
  int len = kickMsg.size();
  uint8 *data = new uint8 [3+len];
  
  data[0] = 0xff;
  putSint16(&data[1],len);
  for(unsigned int i=0;i<kickMsg.size();i++) data[i+3]= kickMsg[i];
  
  bufferevent_write(this->buf_ev, data,len+3);
  
  std::cout << this->nick << " kicked. Reason: " << kickMsg << std::endl;

  delete [] data;
  return true;
}

bool User::loadData()
{
  std::string infile=Map::get().mapDirectory+"/players/"+this->nick+".dat";

  struct stat stFileInfo;
  if(stat(infile.c_str(), &stFileInfo) != 0)
  {
    return false;
  }

   // Read gzipped map file
  gzFile mapfile=gzopen(infile.c_str(), "rb");
  uint8 *uncompressedData = new uint8 [ALLOCATE_NBTFILE];
  gzread(mapfile, uncompressedData, ALLOCATE_NBTFILE);
  gzclose(mapfile);

  NBT_struct userdata;

  TAG_Compound(uncompressedData, &userdata, true);

  delete [] uncompressedData;

  //Load position
  NBT_list *Pos=get_NBT_list(&userdata, "Pos");
  if(Pos!=0 && Pos->length==3)
  {
    pos.x=*(double *)Pos->items[0];
    pos.y=*(double *)Pos->items[1];
    pos.z=*(double *)Pos->items[2];
  }

  //Load rotation
  NBT_list *Rotation=get_NBT_list(&userdata, "Rotation");
  if(Rotation!=0 && Rotation->length==2)
  {
    pos.yaw=*(float *)Rotation->items[0];
    pos.pitch=*(float *)Rotation->items[1];
  }

  //Load inventory
  NBT_list *Inventory=get_NBT_list(&userdata, "Inventory");
  if(Inventory!=0)
  {
    for(int i=0;i<Inventory->length;i++)
    {
      NBT_struct *item=(NBT_struct *)Inventory->items[i];
      char count;      
      get_NBT_value(item, "Count", &count);
      char slot;
      get_NBT_value(item, "Slot", &slot);      
      sint16 damage;
      get_NBT_value(item, "Damage", &damage);
      sint16 item_id;
      get_NBT_value(item, "id", &item_id);

      //Main inventory slot
      if(slot>=0 && slot<=35)
      {
        inv.main[(uint8)slot].count=count;
        inv.main[(uint8)slot].health=damage;
        inv.main[(uint8)slot].type=item_id;
      }
      //Crafting      
      else if(slot>=80 && slot <= 83)
      {
        inv.crafting[(uint8)slot-80].count=count;
        inv.crafting[(uint8)slot-80].health=damage;
        inv.crafting[(uint8)slot-80].type=item_id;
      }
      //Equipped
      else if(slot>=100 && slot <= 103)
      {
        inv.equipped[(uint8)slot-100].count=count;
        inv.equipped[(uint8)slot-100].health=damage;
        inv.equipped[(uint8)slot-100].type=item_id;
      }
    }
  }

  freeNBT_struct(&userdata);

  return true;
}

bool User::saveData()
{
  std::string outfile=Map::get().mapDirectory+"/players/"+this->nick+".dat";
  // Try to create parent directories if necessary
  struct stat stFileInfo;
  if (stat(outfile.c_str(), &stFileInfo) != 0)
  {
    std::string outdir = Map::get().mapDirectory+"/players";

    if (stat(outdir.c_str(), &stFileInfo) != 0)
    { 
#ifdef WIN32
      if (_mkdir(outdir.c_str()) == -1)
#else
      if (mkdir(outdir.c_str(), 0755) == -1)
#endif
      {
        return false;
      }
    }
  }

  NBT_struct playerdata;
  NBT_value nbtvalue;

  //OnGround
  nbtvalue.type=TAG_BYTE;
  nbtvalue.name="OnGround";
  nbtvalue.value=(void *)new char;
  *(char *)nbtvalue.value=1;
  playerdata.values.push_back(nbtvalue);

  //Air
  nbtvalue.type=TAG_SHORT;
  nbtvalue.name="Air";
  nbtvalue.value=(void *)new sint16;
  *(sint16 *)nbtvalue.value=300;
  playerdata.values.push_back(nbtvalue);

  //AttackTime
  nbtvalue.type=TAG_SHORT;
  nbtvalue.name="AttackTime";
  nbtvalue.value=(void *)new sint16;
  *(sint16 *)nbtvalue.value=0;
  playerdata.values.push_back(nbtvalue);

  //DeathTime
  nbtvalue.type=TAG_SHORT;
  nbtvalue.name="DeathTime";
  nbtvalue.value=(void *)new sint16;
  *(sint16 *)nbtvalue.value=0;
  playerdata.values.push_back(nbtvalue);

  //Fire
  nbtvalue.type=TAG_SHORT;
  nbtvalue.name="Fire";
  nbtvalue.value=(void *)new sint16;
  *(sint16 *)nbtvalue.value=-20;
  playerdata.values.push_back(nbtvalue);

  //Health
  nbtvalue.type=TAG_SHORT;
  nbtvalue.name="Health";
  nbtvalue.value=(void *)new sint16;
  *(sint16 *)nbtvalue.value=20;
  playerdata.values.push_back(nbtvalue);

  //HurtTime
  nbtvalue.type=TAG_SHORT;
  nbtvalue.name="HurtTime";
  nbtvalue.value=(void *)new sint16;
  *(sint16 *)nbtvalue.value=0;
  playerdata.values.push_back(nbtvalue);

  //FallDistance
  nbtvalue.type=TAG_FLOAT;
  nbtvalue.name="FallDistance";
  nbtvalue.value=(void *)new float;
  *(float *)nbtvalue.value=54.0;
  playerdata.values.push_back(nbtvalue);

  NBT_list nbtlist;  
  nbtlist.name="Inventory";
  nbtlist.tagId=TAG_COMPOUND;
  int invlength=0;
  for(int i=0;i<36;i++)
  {
    if(inv.main[i].count) invlength++;
  }
  for(int i=0;i<4;i++)
  {
    if(inv.crafting[i].count) invlength++;
  }
  for(int i=0;i<4;i++)
  {
    if(inv.equipped[i].count) invlength++;
  }
  nbtlist.length=invlength;
  nbtlist.items=(void **)new NBT_struct *[invlength];

  //Start with main items
  Item *slots=(Item *)&inv.main;
  char slotid=0;
  char itemslot=0;
  int index=0;
  for(int i=0;i<36+4+4;i++)
  {
    //Crafting items after main
    if(i == 36)
    {
      slots=(Item *)&inv.crafting;
      itemslot=80;
      slotid=0;
    }
    //Equipped items last
    else if(i == 36+4)
    {
      slots=(Item *)&inv.equipped;
      itemslot=100;
      slotid=0;
    }
    if(slots[(uint8)slotid].count)
    {
      NBT_struct *inventory=new NBT_struct;
    
      //Count
      nbtvalue.type=TAG_BYTE;
      nbtvalue.name="Count";
      nbtvalue.value=(void *)new char;
      *(char *)nbtvalue.value=slots[(uint8)slotid].count;
      inventory->values.push_back(nbtvalue);

      //Count
      nbtvalue.type=TAG_BYTE;
      nbtvalue.name="Slot";
      nbtvalue.value=(void *)new char;
      *(char *)nbtvalue.value=itemslot;
      inventory->values.push_back(nbtvalue);

      //Damage
      nbtvalue.type=TAG_SHORT;
      nbtvalue.name="Damage";
      nbtvalue.value=(void *)new sint16;
      *(sint16 *)nbtvalue.value=slots[(uint8)slotid].health;
      inventory->values.push_back(nbtvalue);

      //ID
      nbtvalue.type=TAG_SHORT;
      nbtvalue.name="id";
      nbtvalue.value=(void *)new sint16;
      *(sint16 *)nbtvalue.value=slots[(uint8)slotid].type;
      inventory->values.push_back(nbtvalue);


      nbtlist.items[index]=(void *)inventory;
      index++;
    }

    slotid++;
    itemslot++;
  }

  playerdata.lists.push_back(nbtlist);



  nbtlist.name="Motion";
  nbtlist.tagId=TAG_DOUBLE;
  nbtlist.length=3;
  nbtlist.items=(void **)new double *[3];
  nbtlist.items[0]=(void *)new double;
  *(double *)nbtlist.items[0]=0;
  nbtlist.items[1]=(void *)new double;
  *(double *)nbtlist.items[1]=0;
  nbtlist.items[2]=(void *)new double;
  *(double *)nbtlist.items[2]=0;
  playerdata.lists.push_back(nbtlist);

  nbtlist.name="Pos";
  nbtlist.tagId=TAG_DOUBLE;
  nbtlist.length=3;
  nbtlist.items=(void **)new double *[3];
  nbtlist.items[0]=(void *)new double;
  *(double *)nbtlist.items[0]=pos.x;
  nbtlist.items[1]=(void *)new double;
  *(double *)nbtlist.items[1]=pos.y;
  nbtlist.items[2]=(void *)new double;
  *(double *)nbtlist.items[2]=pos.z;
  playerdata.lists.push_back(nbtlist);

  nbtlist.name="Rotation";
  nbtlist.tagId=TAG_FLOAT;
  nbtlist.length=2;
  nbtlist.items=(void **)new float *[2];
  nbtlist.items[0]=(void *)new float;
  *(float *)nbtlist.items[0]=pos.yaw;
  nbtlist.items[1]=(void *)new float;
  *(float *)nbtlist.items[1]=pos.pitch;
  playerdata.lists.push_back(nbtlist);

  // Save gzipped player file
  uint8 *uncompressedData = new uint8 [ALLOCATE_NBTFILE];
  int dumpsize=dumpNBT_struct(&playerdata, uncompressedData);
  gzFile mapfile2=gzopen(outfile.c_str(), "wb");
  gzwrite(mapfile2, uncompressedData, dumpsize);
  gzclose(mapfile2);

  freeNBT_struct(&playerdata);

  delete [] uncompressedData;

  return true;

}

bool User::updatePos(double x, double y, double z, double stance)
{
    if(nick.size() && logged)
    {
      //Do we send relative or absolute move values
      if(0)//abs(x-this->pos.x)<127
        //&& abs(y-this->pos.y)<127
        //&& abs(z-this->pos.z)<127)
      {
        uint8 movedata[8];
        movedata[0]=0x1f; //Relative move
        putUint32(&movedata[1],this->UID);
        movedata[5]=(char)(x-this->pos.x);
        movedata[6]=(char)(y-this->pos.y);
        movedata[7]=(char)(z-this->pos.z);
        this->sendOthers(&movedata[0],8);
      }
      else
      {          
        this->pos.x=x;
        this->pos.y=y;
        this->pos.z=z;
        this->pos.stance=stance;
        uint8 teleportData[19];
        teleportData[0]=0x22; //Teleport
        putSint32(&teleportData[1],this->UID);
        putSint32(&teleportData[5],(int)(this->pos.x*32));
        putSint32(&teleportData[9],(int)(this->pos.y*32));
        putSint32(&teleportData[13],(int)(this->pos.z*32));
        teleportData[17]=(char)this->pos.yaw;
        teleportData[18]=(char)this->pos.pitch;
        this->sendOthers(&teleportData[0],19);
      }
    
      //Chunk position changed, check for map updates
      if((int)(x/16) != curChunk.x ||(int)(z/16) != curChunk.z)
      {
        //This is not accurate chunk!!
        curChunk.x=(int)(x/16);
        curChunk.z=(int)(z/16);

        for(int mapx=-viewDistance+curChunk.x;mapx<=viewDistance+curChunk.x;mapx++)
        {
          for(int mapz=-viewDistance+curChunk.z;mapz<=viewDistance+curChunk.z;mapz++)
          {
            addQueue(mapx,mapz);
          }
        }

        for(unsigned int i=0;i<mapKnown.size();i++)
        {
          //If client has map data more than viesDistance+1 chunks away, remove it
          if(mapKnown[i].x<curChunk.x-viewDistance-1 ||
             mapKnown[i].x>curChunk.x+viewDistance+1 ||
             mapKnown[i].z<curChunk.z-viewDistance-1 ||
             mapKnown[i].z>curChunk.z+viewDistance+1)
          {
            addRemoveQueue(mapKnown[i].x,mapKnown[i].z);              
          }
        }
      }
    }

    this->pos.x=x;
    this->pos.y=y;
    this->pos.z=z;
    this->pos.stance=stance;
    return true;
}

bool User::updateLook(float yaw, float pitch)
{
  
    uint8 lookdata[7];
    lookdata[0]=0x20;
    putUint32(&lookdata[1],this->UID);
    lookdata[5]=(char)(yaw);
    lookdata[6]=(char)(pitch);          
    this->sendOthers(&lookdata[0],7);
      
    this->pos.yaw=yaw;
    this->pos.pitch=pitch;
    return true;
}

bool User::sendOthers(uint8* data,uint32 len)
{
  for(unsigned int i=0;i<Users.size();i++)
  {
    if(Users[i]->fd!=this->fd && Users[i]->logged)
    {
      bufferevent_write(Users[i]->buf_ev, data,len);
    }
  }
  return true;
}

bool User::sendAll(uint8* data,uint32 len)
{
  for(unsigned int i=0;i<Users.size();i++)
  {
    if(Users[i]->fd && Users[i]->logged)
    {
      bufferevent_write(Users[i]->buf_ev, data,len);
    }
  }
  return true;
}

bool User::sendAdmins(uint8* data,uint32 len)
{
  for(unsigned int i=0;i<Users.size();i++)
  {
    if(Users[i]->fd && Users[i]->logged && Users[i]->admin)
    {
      bufferevent_write(Users[i]->buf_ev, data,len);
    }
  }
  return true;
}

bool User::addQueue(int x, int z)
{
  coord newMap={x,0,z};

  for(unsigned int i=0;i<mapQueue.size();i++)
  {
    // Check for duplicates
    if(mapQueue[i].x==newMap.x && mapQueue[i].z==newMap.z)
      return false;
  }

  for(unsigned int i=0;i<mapKnown.size();i++)
  {
    //Check for duplicates
    if(mapKnown[i].x==newMap.x && mapKnown[i].z==newMap.z)
      return false;
  }

  this->mapQueue.push_back(newMap);

  return true;
}

bool User::addRemoveQueue(int x, int z)
{
  coord newMap={x,0,z};

  this->mapRemoveQueue.push_back(newMap);

  return true;
}

bool User::addKnown(int x, int z)
{
  coord newMap={x,0,z};
  this->mapKnown.push_back(newMap);

  return true;
}

bool User::delKnown(int x, int z)
{
  
  for(unsigned int i=0;i<mapKnown.size();i++)
  {
    if(mapKnown[i].x==x && mapKnown[i].z==z)
    {
      mapKnown.erase(mapKnown.begin()+i);
      return true;
    }
  }

  return false;
}

bool SortVect(const coord &first, const coord &second)
{
  return (first.x-Map::get().spawnPos.x/16)*(first.x-Map::get().spawnPos.x/16)+(first.z-Map::get().spawnPos.z/16)*(first.z-Map::get().spawnPos.z/16) 
          < (second.x-Map::get().spawnPos.x/16)*(second.x-Map::get().spawnPos.x/16)+(second.z-Map::get().spawnPos.z/16)*(second.z-Map::get().spawnPos.z/16);
}

bool User::popMap()
{
  //If map in queue, push it to client
  while(this->mapRemoveQueue.size())
  {
    uint8 preChunk[10];
    //Pre chunk
    preChunk[0]=0x32;
    putSint32(&preChunk[1], mapRemoveQueue[0].x);
    putSint32(&preChunk[5], mapRemoveQueue[0].z);
    preChunk[9]=0; //Unload chunk
    bufferevent_write(this->buf_ev, (uint8 *)&preChunk[0], 10);

    //Delete from known list
    delKnown(mapRemoveQueue[0].x, mapRemoveQueue[0].z);

    //Remove from queue
    mapRemoveQueue.erase(mapRemoveQueue.begin());

    //return true;
  }

  return false;
}

bool User::pushMap()
{
  //Dont send all at once
  int maxcount=10;
  // If map in queue, push it to client
  while(this->mapQueue.size() > 0 && maxcount>0)
  {
    maxcount--;
    // Sort by distance from center
    sort(mapQueue.begin(),mapQueue.end(),SortVect);

    Map::get().sendToUser(this,mapQueue[0].x, mapQueue[0].z);

    // Add this to known list
    addKnown(mapQueue[0].x, mapQueue[0].z);

    // Remove from queue
    mapQueue.erase(mapQueue.begin());
  }

  return true;
}

bool User::teleport(double x, double y, double z)
{      
  uint8 teleportdata[42]={0};
  int curpos=0;
  teleportdata[curpos]=0x0d;
  curpos++;
  putDouble(&teleportdata[curpos],x); //X
  curpos+=8;
  putDouble(&teleportdata[curpos],y);  //Y
  curpos+=8;
  putDouble(&teleportdata[curpos], 0.0); //Stance
  curpos+=8;
  putDouble(&teleportdata[curpos],z); //Z
  curpos+=8;

  putFloat(&teleportdata[curpos], 0.0);
  curpos+=4;
  putFloat(&teleportdata[curpos], 0.0);
  curpos+=4;
  teleportdata[curpos] = 0; //On Ground
  bufferevent_write(this->buf_ev, (char *)&teleportdata[0], 42);

  //Also update pos for other players
  updatePos(x,y,z,0);
  return true;
}

bool User::spawnUser(int x, int y, int z)
{
   uint8 entityData2[256];
  int curpos=0;
  entityData2[curpos]=0x14; //Named Entity Spawn
  curpos++;
  putSint32(&entityData2[curpos], this->UID);        
  curpos+=4;
  entityData2[curpos]=0;
  entityData2[curpos+1]=this->nick.size();
  curpos+=2;
  
  for(unsigned int j=0;j<this->nick.size();j++)
  {
    entityData2[curpos]=this->nick[j];
    curpos++;
  }
  
  putSint32(&entityData2[curpos],x);
  curpos+=4;
  putSint32(&entityData2[curpos],y);
  curpos+=4;
  putSint32(&entityData2[curpos],z);
  curpos+=4;
  entityData2[curpos]=0; //Rotation
  entityData2[curpos+1]=0; //Pitch
  curpos+=2;
  putSint16(&entityData2[curpos],0); //current item
  curpos+=2;
  this->sendOthers((uint8 *)&entityData2[0], curpos);
  return true;
}

bool User::spawnOthers()
{

  for(unsigned int i=0;i<Users.size(); i++)
  {
    if(Users[i]->UID!=this->UID && Users[i]->nick != this->nick)
    {
      uint8 entityData2[256];
      int curpos=0;
      entityData2[curpos]=0x14; //Named Entity Spawn
      curpos++;
      putSint32(&entityData2[curpos], Users[i]->UID);        
      curpos+=4;
      entityData2[curpos]=0;
      entityData2[curpos+1]=Users[i]->nick.size();
      curpos+=2;
  
      for(unsigned int j=0;j<Users[i]->nick.size();j++)
      {
        entityData2[curpos]=Users[i]->nick[j];
        curpos++;
      }
  
      putSint32(&entityData2[curpos],(int)(Users[i]->pos.x*32));
      curpos+=4;
      putSint32(&entityData2[curpos],(int)(Users[i]->pos.y*32));
      curpos+=4;
      putSint32(&entityData2[curpos],(int)(Users[i]->pos.z*32));
      curpos+=4;
      entityData2[curpos]=0; //Rotation
      entityData2[curpos+1]=0; //Pitch
      curpos+=2;
      putSint16(&entityData2[curpos],0); //current item
      curpos+=2;
      bufferevent_write(this->buf_ev,(uint8 *)&entityData2[0], curpos);
    }
  }
  return true;
}

User *addUser(int sock,uint32 EID)
{
  User *newuser = new User(sock,EID);
  Users.push_back(newuser);

  return newuser;
}

bool remUser(int sock)
{        
  for(int i=0;i<(int)Users.size();i++)
  {
    if(Users[i]->fd==sock)
    {
      if(Users[i]->nick.size())
      {
        Chat::get().sendMsg(Users[i], Users[i]->nick+" disconnected!", OTHERS);
        Users[i]->saveData();
      }
      delete Users[i];
      Users.erase(Users.begin()+i);
      return true;
    }
  }
  return false;
}

bool isUser(int sock)
{
  uint8 i;
  for(i=0;i<Users.size();i++)
  {
      if(Users[i]->fd==sock)
          return true;
  }
  return false;
}

//Generate random and unique entity ID
uint32 generateEID()
{
  static uint32 EID = 0;
  /*
  bool finished=false;
  srand ( time(NULL) );
          
  while(!finished)
  {
    finished=true;
    EID=rand()%0xffffff;

    for(uint8 i=0;i<Users.size();i++)
    {
      if(Users[i]->UID==EID)
      {
        finished=false;
      }
    }
    for(uint8 i=0;i<Map::get().items.size();i++)
    {
      if(Map::get().items[i].EID==EID)
      {
        finished=false;
      }
    }
  }
  */
  return ++EID;
}

//Not case-sensitive search
User *getUserByNick(std::string nick) 
{
  // Get coordinates
  for(unsigned int i=0;i<Users.size();i++)
  {
    if(strToLower(Users[i]->nick) == strToLower(nick))
    {
      return Users[i];
    }
  }
  return false;
}
