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

#include <math.h>
#include "tree.h"
#include "mineserver.h"

#include "tools.h"

#define Branch(x,y,z) Trunk* v = new Trunk(x,y,z);m_Branch[n_branches] = v;n_branches++;generateBranches(v);

Tree::Tree(int32_t x, int32_t y, int32_t z, uint8_t limit)
{
    n_branches = 0;
    _x = x, _y = y, _z = z;
    this->generate(limit);

}


Tree::~Tree(void)
{
}
void Tree::generate(uint8_t limit)
{
    uint8_t darkness=1;

    srand((uint32_t)time(NULL));

    uint8_t m_trunkHeight = getRandInt(MIN_TRUNK,limit);

    bool smalltree=false;

    //in this implementation we generate the trunk and as we do we call branching and canopy
    if(m_trunkHeight<BRANCHING_HEIGHT)
    {
        smalltree=true;
        darkness=0;
    }
    uint8_t th=m_trunkHeight-1;
    uint8_t i;
    for(i = 0; i < th; i++)
    {
        if(smalltree)
        {
            Trunk* v = new Trunk(_x,_y+i,_z,darkness);
            if(i>=MIN_TRUNK-1){
                m_Branch[n_branches]= v;
                n_branches++;
            }
            else
            {
                delete v;
            }
        }
        else
        {
            Trunk* v = new Trunk(_x,_y+i,_z,darkness);
            if(i>BRANCHING_HEIGHT-1)
            {
                generateBranches(v);
            }
            else
            {
                delete v;
            }
        }
    }
    Trunk* v = new Trunk(_x,_y+i,_z,darkness);
    m_Branch[n_branches]= v;
    n_branches++;
    generateBranches(v);
    generateCanopy();
}
//I STRONGLY RECOMMEND TO USE Trunk Rather than a new unneeded class
//maybe just a class Wood?
void Tree::generateBranches(Trunk* wrap)
{
    vec loc = wrap->location();

    int32_t posx = loc.x();
    uint8_t posy = loc.y();
    int32_t posz = loc.z();

    uint8_t schanse = BRANCHING_CHANCE / (posy - _y);
    //Not much point to loop here
    //or make a function for the inside of the if.
    if(rand() % schanse == 0){
        Branch(posx+1,posy,posz);
    }
    if(rand() % schanse == 0){
        Branch(posx-1,posy,posz);
    }
    if(rand() % schanse == 0){
        Branch(posx,posy,posz+1);
    }
    if(rand() % schanse == 0 ){
        Branch(posx,posy,posz-1);
    }
    if(rand() % schanse == 0 ){
        Branch(posx,posy+1,posz)
    }
}

void Tree::generateCanopy(){
    uint8_t blocktype;
    uint8_t meta;
    uint8_t canopySize;
    vec loc;

    uint8_t canopy_darkness = 0;
    //Not much point making less code with a while/for loop
    //since compiled this is alot faster
    if(rand() % 15 ==0){
        canopy_darkness++;
    }
    if(rand() % 15 ==0){
        canopy_darkness++;
    }
    if(rand() % 15 ==0){
        canopy_darkness++;
    }
    //I'm Not Proud of this looping.
    for(uint8_t i=0;i<n_branches;i++){
        canopySize = getRandInt(MIN_CANOPY,MAX_CANOPY);
        loc = m_Branch[i]->location();
        delete m_Branch[i];

        int32_t posx = loc.x();
        uint8_t posy = loc.y();
        int32_t posz = loc.z();

        for(int8_t xi=(-canopySize);xi<=canopySize;xi++){
            for(int8_t yi=(-canopySize);yi<=canopySize;yi++){
                for(int8_t zi=(-canopySize);zi<=canopySize;zi++){
                    if(sqrt(xi^2+yi^2+zi^2) <= canopySize){
                        int32_t temp_posx = posx+xi;
                        uint8_t temp_posy = posy+yi;
                        int32_t temp_posz = posz+zi;
                        Mineserver::get()->map()->getBlock(temp_posx,temp_posy,temp_posz,&blocktype,&meta);
                        if(blocktype== BLOCK_AIR){
                            Canopy u(temp_posx,temp_posy,temp_posz,canopy_darkness);
                        }
                    }
                }
            }
        }
    }
}
