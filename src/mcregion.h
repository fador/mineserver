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

#ifndef _MCREGION_H_
#define _MCREGION_H_

enum { VERSION_GZIP = 1, VERSION_DEFLATE };

class RegionFile
{
private:
  static const uint32_t SECTOR_BYTES = 4096;
  static const uint32_t SECTOR_INTS = SECTOR_BYTES / 4;
  static const uint32_t CHUNK_HEADER_SIZE = 5;

  FILE* regionFile;
  uint32_t fileLength;

  uint32_t offsets[SECTOR_INTS];
  int timestamps[SECTOR_INTS];
  int sizeDelta; //Not used
  std::vector<bool> sectorFree;

public:

  int x, z; //The loaded chunk

  RegionFile();
  ~RegionFile();

  bool openFile(std::string mapDir, int32_t x, int32_t z);
  bool writeChunk(uint8_t* chunkdata, uint32_t datalen, int32_t x, int32_t z);
  bool readChunk(uint8_t* chunkdata, uint32_t* datalen, int32_t x, int32_t z);

private:

  /* is this an invalid chunk coordinate? */
  inline bool outOfBounds(int x, int z) const
  {
    return x < 0 || x >= 32 || z < 0 || z >= 32;
  }

  //Get chunk offset from the table
  inline uint32_t getOffset(int x, int z) const
  {
    return offsets[(x + z * 32)];
  }

  //Check if the chunk at (x,z) exists
  inline bool hasChunk(int x, int z) const
  {
    return getOffset(x, z) != 0;
  }

  //Set new offset for a chunk
  void setOffset(int x, int z, int offset);

  //Set timestamp
  void setTimestamp(int x, int z, int timestamp);

  // write a chunk data to the region file at specified sector number
  void write(int sectorNumber, uint8_t* data, uint32_t datalen);
};

//For converting old mapformat to McRegion
bool convertMap(std::string mapDir);

#endif
