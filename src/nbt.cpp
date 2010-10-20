
#include <SocketHandler.h>
#include <ListenSocket.h>
#include <iostream>
#include <fstream>
#include <deque>

#include "tools.h"
#include "DisplaySocket.h"
#include "StatusHandler.h"
#include "map.h"

//NBT level file reading
//More info: http://www.minecraft.net/docs/NBT.txt
int readTag(uint8* input, int inputlen,uint8* output, int* outputlen,std::string TAG, int *pointer=0)
{
  int level=0;
  int curpos=0;
  int outputPos=0;
  bool end=false;

  while(!end && curpos<inputlen)
  {
    std::string name;
    int inputType=input[curpos];
    curpos++;
    if(inputType!=0x00)
    {
      int strLen=getUint16(&input[curpos]);
      curpos+=2;

      while(strLen)
      {
        name+=input[curpos];
        strLen--;
        curpos++;
      }

      //if(TAG==name) std::cout << "Name: " << name << " Curpos: " << curpos << " Level: " << level << std::endl;
    }
    


    if(inputType==0x00)
    {
      level--;
      if(level<=0)
      {
        return curpos;
      }
    }
    
    //Tag_Byte
    else if(inputType==0x01)
    {

      //Read data
      //...
      curpos++;
    }
    //TAG_short
    else if(inputType==0x02)
    {
      int data=getUint16(&input[curpos]);
      curpos+=2;
    }
    //TAG_Int
    else if(inputType==0x03)
    {
      int data=getUint32(&input[curpos]);
      curpos+=4;
    }

    //TAG_Long
    else if(inputType==0x04)
    {
      curpos+=8;
    }
    //TAG_Float
    else if(inputType==0x05)
    {
      curpos+=4;
    }
    //TAG_Double
    else if(inputType==0x06)
    {
      curpos+=8;
    }
    //TAG_Byte_Array
    else if(inputType==0x07)
    {
      int dataLen=getUint32(&input[curpos]);
      //Read the data...
      curpos+=4;

      if(TAG==name)
      {
        if(pointer!=0)
        {
          *pointer=curpos;
          return 0;
        }
        *outputlen=dataLen;
        while(dataLen)
        {
          output[outputPos]=input[curpos];
          dataLen--;
          curpos++;
          outputPos++;
        }
        return -1;
      }
      else curpos+=dataLen;
      
    }
    //TAG_String
    else if(inputType==0x08)
    {
      int strLen=getUint16(&input[curpos]);
      curpos+=2;
      std::string data;
      while(strLen)
      {
        name+=input[curpos];
        strLen--;
        curpos++;
      }
    }
    //Tag_List
    else if(inputType==0x09)
    {
      int listType=input[curpos];
      curpos++;
      int listLen=getUint32(&input[curpos]);
      curpos+=4;
      //Read list
      if(listLen)
      {
        //std::cout << "List type: " << listType << " ListLen: " << listLen << std::endl;
        while(listLen)
        {
          if(listType==0x05)
          {
             curpos+=4;
          }
          if(listType==0x06)
          {
             curpos+=8;
          }
          if(listType==0x0a)
          {
            curpos+=readTag(&input[curpos], inputlen-curpos,&output[outputPos], outputlen,TAG);
          }
          listLen--;
        }
      }
      
    }
    //Compound   
    else if(inputType==0x0a)
    {
      level++;

      //std::cout << "Compound: " << name << std::endl;

    }
    /*
    else
    {
        printf("Unknown Data: 0x%x curpos: %i\r\n",input[curpos],curpos);
        curpos++;
    }
    */
    
  }

  return -1;
}
