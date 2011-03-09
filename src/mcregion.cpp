/*
   Copyright (c) 2011, The Mineserver Project
   All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of the The Mineserver Project nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 Original code: http://pastebin.com/niWTqLvk
*/

/*
struct RegionFile {
    struct ChunkOffset {
        int sector_number;
        char sector_count;
    } chunk_offsets[1024];

    struct ChunkTimestamp {
        int timestamp;
    } chunk_timestamps[1024];
    
    struct Sector {
        int length;
        char version;
        char data[length - 1];
    } sectors[file_length / 4096 - 1];
}
*/

#ifdef WIN32
#include <conio.h>
#include <WinSock2.h>
#include <direct.h>
#else
#include <netinet/in.h>
#endif
#include <sys/stat.h>
#include <fstream>
#include <vector>
#include <string>
#include <stdint.h>
#include <time.h>
#include <iostream>

#include "constants.h"
#include "mcregion.h"
#include "tools.h"
#include "logger.h"
#include "nbt.h"

RegionFile::RegionFile(): sizeDelta(0)
{

}

RegionFile::~RegionFile()
{

}

bool RegionFile::openFile(std::string mapDir, int32_t chunkX, int32_t chunkZ)
{
  std::string strchunkZ;
  std::string strchunkX;
  //std::cout << "openFile(" << chunkX << "," << chunkZ << ")";
  my_itoa(abs(chunkZ >> 5),strchunkZ,10);
  if((chunkZ < 0)) strchunkZ = "-" + strchunkZ;
  my_itoa(abs(chunkX >> 5),strchunkX,10);
  if((chunkX < 0)) strchunkX = "-" + strchunkX;
  regionFile.open(std::string(mapDir + "/region/r." + strchunkX + "." + strchunkZ + ".mcr").c_str(), std::ios::binary | std::ios_base::out | std::ios_base::in | std::ios_base::app);

  if(!regionFile.is_open())
  {
    std::cout << "Failed to open file " << std::string(mapDir + "/region/r." + strchunkX + "." + strchunkZ + ".mcr") << std::endl;
    return false;
  }

  //Store file length
  regionFile.seekg (0, std::ios::end);
  fileLength = (uint32_t)regionFile.tellg();
  regionFile.seekg (0, std::ios::beg);


  //New file?
  if(fileLength < SECTOR_BYTES)
  {
    int zeroInt = 0;
    //Offsets
    for (int i = 0; i < SECTOR_INTS; i++)
    {
      regionFile.write(reinterpret_cast<const char *>(&zeroInt),4);
    }
    //Timestamps
    for (int i = 0; i < SECTOR_INTS; i++)
    {
      regionFile.write(reinterpret_cast<const char *>(&zeroInt),4);
    }

    //Get new size
    regionFile.seekg (0, std::ios::end);
    fileLength = (uint32_t)regionFile.tellg();
    regionFile.seekg (0, std::ios::beg);
  }
  //Not multiple of 4096
  if((fileLength & 0xffff) != 0)
  {
    int zeroInt = 0;
    for (uint32_t i = 0; i < 0xfff-(fileLength & 0xfff); i++)
    {
      regionFile.write(reinterpret_cast<const char *>(&zeroInt),1);
    }
    //Get new size
    regionFile.seekg (0, std::ios::end);
    fileLength = (uint32_t)regionFile.tellg();
    regionFile.seekg (0, std::ios::beg);
  }
  int sectorCount = fileLength/SECTOR_BYTES;

  sectorFree.resize(sectorCount, true);
  //Offsets
  sectorFree[0] = false;
  //Timestamps
  sectorFree[1] = false;

  regionFile.seekg (0, std::ios::beg);
  //Read sectors and mark used
  for (uint32_t i = 0; i < SECTOR_INTS; i++)
  {
    uint32_t offset = 0;
    regionFile.read(reinterpret_cast<char *>(&offset),4);
    offset = ntohl(offset);
    offsets[i] = offset;
    if (offset != 0 && (offset >> 8) + (offset & 0xFF) <= sectorFree.size())
    {
      for (uint32_t sectorNum = 0; sectorNum < (offset & 0xFF); sectorNum++)
      {
        sectorFree[(offset >> 8) + sectorNum] = false;
      }
    }
  }
  //Read timestamps  
  for (uint32_t i = 0; i < SECTOR_INTS; ++i)
  {
    uint32_t lastModValue = 0;
    regionFile.read(reinterpret_cast<char *>(&lastModValue),4);
    lastModValue = ntohl(lastModValue);
    timestamps[i] = lastModValue;
  }
  return true;
}

bool RegionFile::writeChunk(uint8_t *chunkdata, uint32_t datalen, int32_t x, int32_t z)
{
   
  x = x & 31;
  z = z & 31;

  uint32_t offset = getOffset(x, z);
  uint32_t sectorNumber = offset >> 8;
  uint32_t sectorsAllocated = offset & 0xFF;
  uint32_t sectorsNeeded = (datalen + CHUNK_HEADER_SIZE) / SECTOR_BYTES + 1;


  // maximum chunk size is 1MB
  if (sectorsNeeded >= 256)
  {
    return false;
  }

  //Current space is large enought
  if (sectorNumber != 0 && sectorsAllocated == sectorsNeeded)
  {
    write(sectorNumber, chunkdata, datalen);
  }
  //Need more space!
  else
  {
    //Free current sectors
    for (uint32_t i = 0; i < sectorsAllocated; i++)
    {
      sectorFree[sectorNumber + i] = true;
    }

    int runStart = -1;
    for(uint32_t i = 0; i < sectorFree.size(); i++)
    {
      if(sectorFree[i])
      {
        runStart = i;
        break;
      }
    }
    uint32_t runLength = 0;
    if (runStart != -1)
    {
      for (uint32_t i = runStart; i < sectorFree.size(); i++)
      {
        //Not first?
        if (runLength != 0)
        {
          if (sectorFree[i]) runLength++;
          else runLength = 0;
        }
        //Reset on first
        else if (sectorFree[i])
        {
          runStart = i;
          runLength = 1;
        }

        //We have the space
        if (runLength >= sectorsNeeded)
        {
          break;
        }
      }
    }

    //Did we find the space we need?
    if (runLength >= sectorsNeeded)
    {
      sectorNumber = runStart;
      setOffset(x, z, (sectorNumber << 8) | sectorsNeeded);

      //Reserve space
      for (uint32_t i = 0; i < sectorsNeeded; i++)
      {
        sectorFree[sectorNumber + i] = false;
      }
      //Write data
      write(sectorNumber, chunkdata, datalen);
    }
    //If no space, grow file
    else
    {
      regionFile.seekp(0, std::ios::end);
      sectorNumber = sectorFree.size();
      for (uint32_t i = 0; i < sectorsNeeded; i++)
      {
        char zerobyte = 0;
        for(uint32_t b = 0; b < SECTOR_BYTES; b++)
        {
          regionFile.write(&zerobyte, 1);
        }
        sectorFree.push_back(false);
      }
      sizeDelta += SECTOR_BYTES * sectorsNeeded;

      //Write chunk data
      write(sectorNumber, chunkdata, datalen);

      setOffset(x, z, (sectorNumber << 8) | sectorsNeeded);
    }
  }
  setTimestamp(x, z, (int)time(NULL));

  return true;
}

bool RegionFile::readChunk(uint8_t *chunkdata, uint32_t *datalen, int32_t x, int32_t z)
{
  x = x & 31;
  z = z & 31;

  //std::cout << "readChunk(" << x << "," << z << ");" << std::endl;

  int offset = getOffset(x, z);
  //std::cout << "offset: " << offset << std::endl;

  //Chunk not found
  if (offset == 0)
  {
    return false;
  }
  uint32_t sectorNumber = offset >> 8;
  uint32_t numSectors = offset & 0xFF;

  //Invalid sector
  if (sectorNumber + numSectors > sectorFree.size())
  {
    //std::cout << "Invalid sector " << offset << std::endl;
    return false;
  }

  regionFile.seekg(sectorNumber * SECTOR_BYTES);
  uint32_t length = 0;
  regionFile.read(reinterpret_cast<char *>(&length),4);
  length = ntohl(length);

  //Invalid length?
  if(length > SECTOR_BYTES * numSectors)
  {
    //std::cout << "Invalid length" << std::endl;
    return false;
  }

  char version;
  regionFile.read(&version,1);
  //TODO: do something with version?

  *datalen = length;
  regionFile.read((char *)chunkdata, length);

  //std::cout << "Read " << *datalen << " bytes!" << std::endl;
  return true;

}


/*function... might want it in some class?*/
int getdir (std::string dir, std::vector<std::string> &files)
{
#ifdef WIN32
  HANDLE hFind = INVALID_HANDLE_VALUE;
  WIN32_FIND_DATA ffd; // File information
  dir += "\\*.*";
  hFind = FindFirstFile(dir.c_str(), &ffd);

   if (INVALID_HANDLE_VALUE == hFind) 
   {
      return 0;
   }
   
   // List all the files in the directory with some info about them.
   do
   {
     if(std::string(ffd.cFileName) != "." && std::string(ffd.cFileName) != "..")
      files.push_back(ffd.cFileName);
   }
   while (FindNextFile(hFind, &ffd) != 0);
#else
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        std::cout << "Error(" << errno << ") opening " << dir << std::endl;
        return errno;
    }

    while ((dirp = readdir(dp)) != NULL) {
      if(std::string(dirp->d_name) != "." && std::string(dirp->d_name) != "..")
        files.push_back(std::string(dirp->d_name));
    }
    closedir(dp);

#endif
    return 1;
}

bool isMapDir(std::string filename)
{
  std::string hexarray("0123456789abcdefghijklmnopqrstuvwxyz");
  if(filename.size() < 3)
  {    
    for(int ch = 0; ch < filename.size(); ch ++)
    {
      bool found = false;
      for(int check = 0; check < hexarray.length(); check++)
      {
        if(filename[ch] == hexarray[check])
        {
          found = true;
          break;
        }
      }
      if(!found) return false;
    }
    return true;
  }
  return false;
}

bool convertMap(std::string mapDir)
{
  std::cout << "Start conversion of " << mapDir << std::endl;

  struct stat stFileInfo;
  std::string regionDir = mapDir+"/region";
  if (stat(regionDir.c_str(), &stFileInfo) != 0)
  {
    std::cout << "Creating region dir" << std::endl;
  #ifdef WIN32
    if (_mkdir(std::string(regionDir).c_str()) == -1)
#else
    if (mkdir(std::string(regionDir).c_str(), 0755) == -1)
#endif
    {
      //LOG(EMERG, "Map", "Error: Could not create map directory.");

      exit(EXIT_FAILURE);
    }
  }
    std::vector<std::string> files;
    std::vector<std::string> files2;
    std::vector<std::string> files3;
    getdir(mapDir,files);

    for(int i = 0; i < files.size(); i++)
    {
      if(isMapDir(files[i]))
      {
        std::string filename = mapDir+"/"+files[i];
        getdir(filename,files2);
        for(int ii = 0; ii < files2.size(); ii++)
        {
          if(isMapDir(files2[ii]))
          {
            int32_t x,z;
            std::string filename = mapDir+"/"+files[i]+"/"+files2[ii];
            getdir(filename,files3);
            for(int j = 0; j < files3.size(); j++)
            {
              if(files3[j].length() > 7 && files3[j].substr(files3[j].length()-3) =="dat" && files3[j][0] == 'c')
              {
                std::string filename = mapDir+"/"+files[i]+"/"+files2[ii]+"/"+files3[j];
                //std::cout << filename << std::endl;
                NBT_Value* chunk = NBT_Value::LoadFromFile(filename);

                if (chunk == NULL)
                {
                  continue;
                }

                NBT_Value* level = (*chunk)["Level"];

                if (level == NULL)
                {
                  delete chunk;
                  continue;
                }

                NBT_Value* xPos = (*level)["xPos"];
                NBT_Value* zPos = (*level)["zPos"];
                if (xPos && zPos)
                {
                  x = *xPos;
                  z = *zPos;
                  uint8_t *buffer = new uint8_t[ALLOCATE_NBTFILE];
                  uint32_t len = 0;
                  chunk->SaveToMemory(buffer, &len);

                  RegionFile newRegion;
                  newRegion.openFile(mapDir, x,z);
                  newRegion.writeChunk(buffer, len, x, z);
                  std::cout << ".";
                  delete [] buffer;
                  delete chunk;
                }
                else
                {
                  delete chunk;
                  continue;
                }
              }
            }
          }
        }
      }
    }
    std::cout << "converted" << std::endl;
}