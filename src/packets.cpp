#include <stdlib.h>
#ifdef WIN32
  #define _CRTDBG_MAP_ALLOC
  #include <crtdbg.h>
  #include <conio.h>
  #include <winsock2.h>
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <string.h>
#endif

#include <sys/types.h>
#include <fcntl.h>
#include <cstdio>
#include <deque>
#include <iostream>
#include <event.h>
#include <fstream>
#include <ctime>
#include <cmath>
#include <vector>
#include <zlib.h>
#include <stdint.h>

#include "constants.h"

#include "logger.h"

#include "sockets.h"
#include "tools.h"
#include "map.h"
#include "user.h"
#include "chat.h"
#include "config.h"
#include "nbt.h"
#include "packets.h"


void PacketHandler::initPackets()
{  

  //Len 0
  packets[PACKET_KEEP_ALIVE]                = Packets(0,&PacketHandler::keep_alive);
  //Variable len
  packets[PACKET_LOGIN_REQUEST]             = Packets(-1, &PacketHandler::login_request);
  //Variable len
  packets[PACKET_HANDSHAKE]                 = Packets(PACKET_VARIABLE_LEN,&PacketHandler::handshake);
  packets[PACKET_CHAT_MESSAGE]              = Packets(PACKET_VARIABLE_LEN,&PacketHandler::chat_message);
  packets[PACKET_PLAYER_INVENTORY]          = Packets(PACKET_VARIABLE_LEN,&PacketHandler::player_inventory);
  packets[PACKET_PLAYER]                    = Packets( 1,&PacketHandler::player);
  packets[PACKET_PLAYER_POSITION]           = Packets(33,&PacketHandler::player_position);
  packets[PACKET_PLAYER_LOOK]               = Packets( 9,&PacketHandler::player_look);
  packets[PACKET_PLAYER_POSITION_AND_LOOK]  = Packets(41,&PacketHandler::player_position_and_look);
  packets[PACKET_PLAYER_DIGGING]            = Packets(11,&PacketHandler::player_digging);
  packets[PACKET_PLAYER_BLOCK_PLACEMENT]    = Packets(12,&PacketHandler::player_block_placement);
  packets[PACKET_HOLDING_CHANGE]            = Packets( 6,&PacketHandler::holding_change);
  packets[PACKET_ARM_ANIMATION]             = Packets( 5,&PacketHandler::arm_animation);
  packets[PACKET_PICKUP_SPAWN]              = Packets(22,&PacketHandler::pickup_spawn);
  packets[PACKET_DISCONNECT]                = Packets(PACKET_VARIABLE_LEN,&PacketHandler::disconnect);

}

// Keep Alive (http://mc.kev009.com/wiki/Protocol#Keep_Alive_.280x00.29)
void PacketHandler::keep_alive(uint8 *data, User *user)
{
  //No need to do anything
}

// Login request (http://mc.kev009.com/wiki/Protocol#Login_Request_.280x01.29)
int PacketHandler::login_request(User *user)
{
  //Check that we have enought data in the buffer
  if(user->buffer.size()<12)
  {
    return PACKET_NEED_MORE_DATA;
  }
  uint32 curpos = 0;
  int i;

  //Client protocol version
  uint8 tmpIntArray[4] = {0};
  for(i = 0;i<4;i++) tmpIntArray[i]=user->buffer[curpos+i]; 
  int version = getUint32(&tmpIntArray[0]);     
  curpos+=4;

  //Player name length
  uint8 tmpShortArray[2] = {0};
  for(i = 0;i<2;i++) tmpShortArray[i]=user->buffer[curpos+i]; 
  int len = getUint16(&tmpShortArray[0]);     
  curpos+=2;

  //Check for data
  if(user->buffer.size()<curpos+len+2)
  {
    return PACKET_NEED_MORE_DATA;
  }

  std::string player;
  //Read player name
  for(int pos = 0;pos<len;pos++)
  {
    player+=user->buffer[curpos+pos];
  }
  curpos+=len;

    
  //Password length
  for(i = 0;i<2;i++) tmpShortArray[i]=user->buffer[curpos+i]; 
  len = getUint16(&tmpShortArray[0]);     
  curpos+=2;
    
  std::string passwd;
  //Check for data
  if(user->buffer.size()<curpos+len)
  {
    return PACKET_NEED_MORE_DATA;
  }

  //Read password
  for(int pos = 0;pos<len;pos++)
  {
    passwd += user->buffer[curpos+pos];
  }
  curpos+=len;

  //Package completely received, remove from buffer
  user->buffer.erase(user->buffer.begin(), user->buffer.begin()+curpos);

  std::cout << "Player " << user->UID << " login v." << version <<" : " << player <<":" << passwd << std::endl;

  // If version is not 2 or 3
  if(version != 4 && version != 3 && version != 2)
  {
    user->kick(Conf::get().sValue("wrong_protocol_message"));
    return curpos;
  }
      
  // If userlimit is reached
  if((int)Users.size() >= Conf::get().iValue("userlimit"))
  {
    user->kick(Conf::get().sValue("server_full_message"));       
    return curpos;
  }
  
  user->changeNick(player, Chat::get().admins);

  //Load user data
  user->loadData();

  //Login OK package
  char data[9]={0x01, 0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  putSint32((uint8 *)&data[1],user->UID);
  bufferevent_write(user->buf_ev, (char *)&data[0], 9);

  //Send server time (after dawn)
  uint8 data3[9]={0x04, 0x00, 0x00, 0x00,0x00,0x00,0x00,0x0e,0x00};
  bufferevent_write(user->buf_ev, (char *)&data3[0], 9);

  //Inventory
  uint8 data4[7+36*5];
  data4[0]=0x05;
  putSint32(&data4[1],-1);
  data4[5]=0;
  data4[6]=36;
  int curpos2=7;
  //Send main inventory
  for(i = 0;i<36;i++)
  {
    if(user->inv.main[i].count)
    {
      putSint16(&data4[curpos2], user->inv.main[i].type);     //Type
      curpos2+=2;
      data4[curpos2]=user->inv.main[i].count;               //Count
      curpos2++;
      putSint16(&data4[curpos2], user->inv.main[i].health); //Health
      curpos2+=2;
    }
    else
    {
      //Empty slot
      putSint16(&data4[curpos2], -1);
      curpos2+=2;
    }
  }
  bufferevent_write(user->buf_ev, (char *)&data4[0], curpos2);
    

  //Send equipped inventory
  putSint32(&data4[1],-3);
  data4[6]=4;
  curpos2=7;
  for(i = 0;i<4;i++)
  {
    if(user->inv.equipped[i].count)
    {
      putSint16(&data4[curpos2], user->inv.equipped[i].type);     //Type
      curpos2+=2;
      data4[curpos2]=user->inv.equipped[i].count;               //Count
      curpos2++;
      putSint16(&data4[curpos2], user->inv.equipped[i].health); //Health
      curpos2+=2;
    }
    else
    {
      //Empty slot
      putSint16(&data4[curpos2], -1);
      curpos2+=2;
    }
  }
  bufferevent_write(user->buf_ev, (char *)&data4[0], curpos2);
  
  //Send crafting inventory
  putSint32(&data4[1],-2);
  data4[6]=4;
  curpos2=7;
  for(i = 0;i<4;i++)
  {
    if(user->inv.crafting[i].count)
    {
      putSint16(&data4[curpos2], user->inv.crafting[i].type);     //Type
      curpos2+=2;
      data4[curpos2]=user->inv.crafting[i].count;               //Count
      curpos2++;
      putSint16(&data4[curpos2], user->inv.crafting[i].health); //Health
      curpos2+=2;
    }
    else
    {
      //Empty slot
      putSint16(&data4[curpos2], -1);
      curpos2+=2;
    }
  }
  bufferevent_write(user->buf_ev, (char *)&data4[0], curpos2);

  // Send motd
  std::ifstream motdfs( MOTDFILE.c_str() );
        
  std::string temp;

  while( getline( motdfs, temp ) ) {
    // If not commentline
    if(temp[0] != COMMENTPREFIX) {
      Chat::get().sendMsg(user, temp, USER);
    }
  }
  motdfs.close();

  //Teleport player
  user->teleport(user->pos.x,user->pos.y+2,user->pos.z);

  //Put nearby chunks to queue
  for(int x=-user->viewDistance;x<=user->viewDistance;x++)
  {
    for(int z=-user->viewDistance;z<=user->viewDistance;z++)
    {
      user->addQueue((sint32)user->pos.x/16+x,(sint32)user->pos.z/16+z);
    }
  }
  // Push chunks to user
  user->pushMap();

  //Spawn this user to others
  user->spawnUser((sint32)user->pos.x*32,((sint32)user->pos.y+2)*32,(sint32)user->pos.z*32);
  //Spawn other users for connected user
  user->spawnOthers();
        
                          
  //Send "On Ground" signal
  char data6[2]={0x0A, 0x01};
  bufferevent_write(user->buf_ev, (char *)&data6[0], 2);

  user->logged = true;

  Chat::get().sendMsg(user, player+" connected!", ALL);

  return curpos;
}

int PacketHandler::handshake(User *user)
{
  if(user->buffer.size()<3)
  {
    return PACKET_NEED_MORE_DATA;
  }
  int curpos = 0;
  int i;

  //Player name length
  uint8 tmpShortArray[2] = {0};
  for(i = 0;i<2;i++) tmpShortArray[i]=user->buffer[curpos+i]; 
  int len = getSint16(&tmpShortArray[0]);     
  curpos+=2;

  //Check for data
  if(user->buffer.size()<(unsigned int)curpos+len)
  {
    return PACKET_NEED_MORE_DATA;
  }

  //Read player name
  std::string player;
  for(int pos = 0;pos<len;pos++)
  {
      player += user->buffer[curpos+pos];
  }
  curpos+=len;

  //Remove package from buffer
  user->buffer.erase(user->buffer.begin(), user->buffer.begin()+curpos);
  std::cout << "Handshake player: " << player << std::endl;

  //Send handshake package
  char data2[4]={0x02,0x00,0x01,'-'};    
  bufferevent_write(user->buf_ev, (char *)&data2[0], 4);

  return curpos;
}

int PacketHandler::chat_message(User *user)
{
  // Wait for length-short. HEHE
  if(user->buffer.size()<2)
  {
    return PACKET_NEED_MORE_DATA;
  }
      
  int curpos = 0;
  unsigned int i;
      
  uint8 tmpLenArray[2] = {0};
  for(i = 0;i<2;i++) tmpLenArray[i]=user->buffer[curpos+i]; 
  short len = getSint16(&tmpLenArray[0]); 
  curpos+=2;
      
  // Wait for whole message
  if(user->buffer.size()<(unsigned int)curpos+len)
  {
    return PACKET_NEED_MORE_DATA;
  }
      
  //Read message
  std::string msg;
  for(i = 0;i<(unsigned int)len;i++) msg += user->buffer[curpos+i];
      
  curpos += len;
      
  // Remove from buffer
  user->buffer.erase(user->buffer.begin(), user->buffer.begin()+curpos);

  Chat::get().handleMsg( user, msg );

  return curpos;
}

int PacketHandler::player_inventory(User *user)
{
  if(user->buffer.size()<14)
  {
    return PACKET_NEED_MORE_DATA;
  }

  int curpos = 0;
  unsigned int i;

  //Read inventory type (-1,-2 or -3)
  uint8 tmpIntArray[4] = {0};
  for(i = 0;i<4;i++) tmpIntArray[i]=user->buffer[curpos+i]; 
  int type = getSint32(&tmpIntArray[0]);
  curpos+=4;

  uint8 tmpShortArray[2] = {0};
  for(i = 0;i<2;i++) tmpShortArray[i]=user->buffer[curpos+i]; 
  int count = getSint16(&tmpShortArray[0]);     
  curpos+=2;

  int items = 0;
  Item *slots=NULL;

  items=count;

  switch(type)
  {
    //Main inventory
    case -1:
      //items = 36;
      memset(user->inv.main, 0, sizeof(Item)*36);
      slots=(Item *)&user->inv.main;
    break;

    //Equipped armour
    case -3:
      //items = 4;
      memset(user->inv.equipped, 0, sizeof(Item)*4);
      slots=(Item *)&user->inv.equipped;
    break;

    //Crafting slots
    case -2:
      //items = 4;
      memset(user->inv.crafting, 0, sizeof(Item)*4);
      slots=(Item *)&user->inv.crafting;
    break;
  }

  if(user->buffer.size()<(unsigned int)6+2*items)
  {
    return PACKET_NEED_MORE_DATA;
  }

  for(i = 0;i<(unsigned int)items;i++)
  {
    int j = 0;
    for(j = 0;j<2;j++) tmpShortArray[j]=user->buffer[curpos+j]; 
    int item_id = getSint16(&tmpShortArray[0]);     
    curpos+=2;

    if(user->buffer.size()<curpos+(items-i-1)*2)
    {
      return PACKET_NEED_MORE_DATA;
    }
        
    if(item_id!=-1)
    {
      if(user->buffer.size()-curpos<(items-i-1)*2+3)
      {
        return PACKET_NEED_MORE_DATA;
      }
      uint8 numberOfItems = user->buffer[curpos];
      curpos++;
          
      for(j = 0;j<2;j++) tmpShortArray[j]=user->buffer[curpos+j]; 
      int health = getSint16(&tmpShortArray[0]);     
      curpos+=2;

      //Save to user inventory      
      slots[i].type=item_id;
      slots[i].count=numberOfItems;
      slots[i].health=health;
    }
  }

  //std::cout << "Got items type " << type << std::endl;
  //Package completely received, remove from buffer
  user->buffer.erase(user->buffer.begin(), user->buffer.begin()+curpos);

  return curpos;
}

void PacketHandler::player(uint8 *data, User *user)
{
  //OnGround packet
}

void PacketHandler::player_position(uint8 *data, User *user)
{
  int curpos = 0;
  double x,y,stance,z;

  //Read double X
  x = getDouble(&data[curpos]);    
  curpos+=8;

  //Read double Y
  y = getDouble(&data[curpos]);
  curpos+=8;

  //Read double stance
  stance = getDouble(&data[curpos]);
  curpos+=8;

  //Read double Z
  z = getDouble(&data[curpos]);
  curpos+=8;

  user->updatePos(x, y, z, stance);
}

void PacketHandler::player_look(uint8 *data, User *user)
{
  float yaw,pitch;
  uint8 onground;
  int curpos = 0;
      
  yaw = getFloat(&data[curpos]);     
  curpos+=4;
      
  pitch = getFloat(&data[curpos]);     
  curpos+=4;
      
  user->updateLook(yaw, pitch);

  onground = data[curpos];
  curpos++;
}

void PacketHandler::player_position_and_look(uint8 *data, User *user)
{
  int curpos = 0;

  double x,y,stance,z;

  //Read double X
  x = getDouble(&data[curpos]);    
  curpos+=8;

  //Read double Y
  y = getDouble(&data[curpos]);
  curpos+=8;

  //Read double stance
  stance = getDouble(&data[curpos]);
  curpos+=8;

  //Read double Z
  z = getDouble(&data[curpos]);
  curpos+=8;


  float yaw,pitch;

  yaw = getFloat(&data[curpos]);     
  curpos+=4;

  pitch = getFloat(&data[curpos]);     
  curpos+=4;

  //Update user data
  user->updatePos(x, y, z, stance);
  user->updateLook(yaw, pitch);
}

void PacketHandler::player_digging(uint8 *data, User *user)
{
  int curpos = 0;
  char status = data[curpos];
  curpos++;
    
  int x = getSint32(&data[curpos]);
  curpos+=4;

  char y = data[curpos];
  curpos++;

  int z = getSint32(&data[curpos]);
  curpos+=4;


  //If block broken
  if(status == 3)
  {
    uint8 block; uint8 meta;
    if(Map::get().getBlock(x,y,z, &block, &meta))
    {          
      Map::get().sendBlockChange(x,y,z,0,0);
      Map::get().setBlock(x,y,z,0,0);

      uint8 topblock; uint8 topmeta;        
      //Destroy items on top
      if(Map::get().getBlock(x,y+1,z, &topblock, &topmeta) && (topblock == BLOCK_SNOW ||
                                                              topblock == BLOCK_BROWN_MUSHROOM ||
                                                              topblock == BLOCK_RED_MUSHROOM ||
                                                              topblock == BLOCK_YELLOW_FLOWER ||
                                                              topblock == BLOCK_RED_ROSE ||
                                                              topblock == BLOCK_SAPLING ))
      {
        Map::get().sendBlockChange(x,y+1,z,0, 0);
        Map::get().setBlock(x,y+1,z,0,0);
        //Others than snow will spawn
        if(topblock!=BLOCK_SNOW)
        {
          spawnedItem item;
          item.EID = generateEID();
          item.health=0;
          item.item=(int)topblock;
          item.count=1;

          item.x = x*32;
          item.y = (y+1)*32;
          item.z = z*32;
          //Randomize spawn position a bit
          item.x+=5+(rand()%22);
          item.z+=5+(rand()%22);
        
          Map::get().sendPickupSpawn(item);
        }
      }

      if(block!=BLOCK_SNOW && (int)block>0 && (int)block<255)
      {
        spawnedItem item;
        item.EID = generateEID();
        item.health=0;

        // Spawn drop according to BLOCKDROPS
        // Check propability 
        if(BLOCKDROPS.count(block) && BLOCKDROPS[block].probability >= rand()%10000)
        {
          item.item=BLOCKDROPS[block].item_id;
          item.count=BLOCKDROPS[block].count;
        }
        else
        {
          item.item=(int)block;
          item.count=1;
        }
        item.x = x*32;
        item.y = y*32;
        item.z = z*32;
        //Randomize spawn position a bit
        item.x+=5+(rand()%22);
        item.z+=5+(rand()%22);
        
        // If count is greater than 0
        if(item.count > 0)
          Map::get().sendPickupSpawn(item);
      }
      
      // Block physics for BLOCK_GRAVEL and BLOCK_SAND and BLOCK_SNOW
      while(Map::get().getBlock(x,y+1,z, &topblock, &topmeta) && (topblock == BLOCK_GRAVEL ||
                                                                  topblock == BLOCK_SAND ||
                                                                  topblock == BLOCK_SNOW ||
                                                                  topblock == BLOCK_BROWN_MUSHROOM ||
                                                                  topblock == BLOCK_RED_MUSHROOM ||
                                                                  topblock == BLOCK_YELLOW_FLOWER ||
                                                                  topblock == BLOCK_RED_ROSE ||
                                                                  topblock == BLOCK_SAPLING ))
      {
        // Destroy original block
        Map::get().sendBlockChange(x,y+1,z,0, 0);
        Map::get().setBlock(x,y+1,z,0,0);
        
        Map::get().setBlock(x,y,z, topblock, topmeta);
        Map::get().sendBlockChange(x,y,z, topblock, topmeta);
        
        y++;
      }
    }
  }
}

void PacketHandler::player_block_placement(uint8 *data, User *user)
{
  int curpos = 0;
  int orig_x,orig_y,orig_z;
  bool change = false;

  int blockID = getSint16(&data[curpos]);
  curpos+=2;

  int x = orig_x = getSint32(&data[curpos]);
  curpos+=4;

  int y = orig_y = user->buffer[curpos];
  curpos++;

  int z = orig_z = getSint32(&data[curpos]);
  curpos+=4;

  int direction = data[curpos];


  uint8 block;
  uint8 metadata;
  Map::get().getBlock(x,y,z, &block, &metadata);

  switch(direction)        
  {
    case 0: y--; break;
    case 1: y++; break;
    case 2: z--; break;
    case 3: z++; break;
    case 4: x--; break;
    case 5: x++; break;
  }
      
  uint8 block_direction;
  uint8 metadata_direction;
  Map::get().getBlock(x,y,z, &block_direction, &metadata_direction);


  //If placing normal block and current block is empty
  if(blockID<0xff && blockID!=-1 && ( block_direction == BLOCK_AIR || 
                                      block_direction == BLOCK_WATER ||
                                      block_direction == BLOCK_STATIONARY_WATER ||
                                      block_direction == BLOCK_LAVA ||
                                      block_direction == BLOCK_STATIONARY_LAVA ) )
  {
    // DO NOT place a block if block is ...
    if( block != BLOCK_WORKBENCH &&
        block != BLOCK_FURNACE &&
        block != BLOCK_BURNING_FURNACE &&
        block != BLOCK_CHEST &&
        block != BLOCK_JUKEBOX )
    {
      change = true;
    }
  }

  if(block == BLOCK_SNOW || block == BLOCK_TORCH) //If snow or torch, overwrite
  {
    change = true;
    x = orig_x;
    y = orig_y;
    z = orig_z;
  }

  //Door status change
  if((block == BLOCK_WOODEN_DOOR) || (block == BLOCK_IRON_DOOR))
  {
    change = true;

    blockID = block;

    //Toggle door state
    if(metadata&0x4)
    {
      metadata&=(0x8|0x3);
    }
    else
    {
      metadata|=0x4;
    }

    uint8 metadata2,block2;

    int modifier=(metadata&0x8)?-1:1;
                
    x = orig_x;
    y = orig_y;
    z = orig_z;

    Map::get().getBlock(x,y+modifier,z, &block2, &metadata2);
    if(block2 == block)
    {
      metadata2=metadata;
      if(metadata&0x8)
      {
        metadata2&=(0x7);
      }
      else
      {
        metadata2|=0x8;
      }
      
      

      Map::get().setBlock(x,y+modifier,z, block2, metadata2);
      Map::get().sendBlockChange(x,y+modifier,z,blockID, metadata2);
    }

  }
  
  // Check if player is standing there
  double intX, intZ, fracX, fracZ;
  // Check Y coordinate
  if( y == user->pos.y || y-1 == user->pos.y )
  {
    fracX = std::abs(std::modf(user->pos.x, &intX));
    fracZ = std::abs(std::modf(user->pos.z, &intZ));
    
    // Mystics
    intX--;
    intZ--;
    
    // Optimized version of the code below
    if( (z==intZ || (z==intZ+1 && fracZ<0.30) || (z==intZ-1 && fracZ>0.70)) &&
        (x==intX || (x==intX+1 && fracZ<0.30) || (x==intX-1 && fracX>0.70)) )
    {
      change = false;
    }
    
    /*if(x==intX && z==intZ)
      change = false;
      
    if(x==intX && z==intZ+1 && fracZ < 0.30)
      change = false;
      
    if(x==intX && z==intZ-1 && fracZ > 0.70)
      change = false;
      
    if(z==intZ && x==intX+1 && fracX < 0.30)
      change = false;
    
    if(z==intZ && x==intX-1 && fracX > 0.70)
      change = false;
      
    if(z==intZ-1 && x==intX-1 && fracZ > 0.70 && fracX > 0.70)
      change = false;
      
    if(z==intZ-1 && x==intX+1 && fracZ > 0.70 && fracX < 0.30)
      change = false;
      
    if(z==intZ+1 && x==intX+1 && fracZ < 0.30 && fracX < 0.30)
      change = false;

    if(z==intZ+1 && x==intX-1 && fracZ < 0.30 && fracX > 0.70)
      change = false;*/
  }

  if(change)
  {
    Map::get().setBlock(x,y,z, blockID, metadata);
    Map::get().sendBlockChange(x,y,z,blockID, metadata);
  }
}

void PacketHandler::holding_change(uint8 *data, User *user)
{
  int itemID = getUint16(&data[4]);      

  //Send holding change to others
  uint8 holdingPackage[7];
  holdingPackage[0]=0x10;
  putSint32(&holdingPackage[1], user->UID);
  putSint16(&holdingPackage[5], itemID);
  user->sendOthers(&holdingPackage[0],7);

}

void PacketHandler::arm_animation(uint8 *data, User *user)
{
  char forward = data[4];

  uint8 animationPackage[6];
  animationPackage[0]=0x12;
  putSint32(&animationPackage[1],user->UID);
  animationPackage[5]=forward;
  user->sendOthers(&animationPackage[0], 6);
}

void PacketHandler::pickup_spawn(uint8 *data, User *user)
{
  uint32 curpos=4;
  spawnedItem item;
  item.EID = generateEID();
  item.health=0;
  item.item = getSint16(&data[curpos]);
  curpos+=2;
  item.count = data[curpos];
  curpos++;

  item.x = getSint32(&data[curpos]);
  curpos+=4;
  item.y = getSint32(&data[curpos]);
  curpos+=4;
  item.z = getSint32(&data[curpos]);
  curpos+=4;
  item.spawnedBy=user->UID;

  Map::get().sendPickupSpawn(item);
}

int PacketHandler::disconnect(User *user)
{
  if(user->buffer.size()<2)
  {
    return PACKET_NEED_MORE_DATA;
  }
  int curpos = 0;
  unsigned int i;
  uint8 shortArray[2];
  for(i = 0;i<2;i++) shortArray[i]=user->buffer[i];
  int len = getSint16(&shortArray[0]);

  curpos+=2;
  // Wait for whole message
  if(user->buffer.size()<(unsigned int)curpos+len)
  {
    return PACKET_NEED_MORE_DATA;
  }
      
  //Read message
  std::string msg;
  for(i = 0;i<(unsigned int)len;i++) msg += user->buffer[curpos+i];

  curpos+=len;
  user->buffer.erase(user->buffer.begin(), user->buffer.begin()+curpos);

  bufferevent_free(user->buf_ev);
  #ifdef WIN32
    closesocket(user->fd);
  #else
    close(user->fd);
  #endif
  remUser(user->fd);

  return curpos;
}