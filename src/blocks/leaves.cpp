/*
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

#include <cmath>
#include "leaves.h"
#include "../map.h"
#include "../mineserver.h"
#include "../constants.h"
#include "../plugin.h"

inline int32_t abs(int32_t a){ // <cmath> <math.h> and even stdlib (which is included in basic block) doesn't work here for some reason =/
    if(a<0) return -a;
    return a;
}
bool BlockLeaves::affectedBlock(int block){
    if(block == BLOCK_LEAVES) return true;
    else return false;
}
BlockLeaves::BlockLeaves(){
    decaying.reserve(64);
}
bool BlockLeaves::onBroken(User* user, int8_t status, int32_t x, int8_t y, int32_t z, int map, int8_t direction){
    for(uint16_t i=0;i<decaying.size();i++){
        if(decaying[i].x == x && decaying[i].y == y && decaying[i].z == z && decaying[i].map){
            decaying.erase(decaying.begin()+i);
        }
    }
    return true;
}

void BlockLeaves::onNeighbourBroken(User* user, int16_t oldblock, int32_t x, int8_t y, int32_t z, int map, int8_t direction){
    if(oldblock == BLOCK_WOOD || oldblock == BLOCK_LEAVES){
        for(uint16_t i=0;i<decaying.size();i++){
            if(decaying[i].x == x && decaying[i].y == y && decaying[i].z == z && decaying[i].map){
                return;
            }
        }
        uint8_t block,meta;
        for (int8_t xi=(-2);xi<=2;xi++){
            for (int8_t yi=(-2);yi<=2;yi++){
                for (int8_t zi=(-2);zi<=2;zi++){
                    if (abs((int32_t)xi)+abs((int32_t)yi)+abs((int32_t)zi) <= 3){
                        Mineserver::get()->map(map)->getBlock(x+xi,y+yi,z+zi,&block,&meta);
                        if(block == BLOCK_WOOD)
                            return;
                    }
                }
            }
        }
        decaying.push_back(Decay(time(0),x,y,z,map));
    }
}
inline void decayIt(Decay decaying){
    uint8_t block,meta;
    BlockBasic* blockcb;
    //this->notifyNeighbours(decaying[0].x,decaying[0].y,decaying[0].z,decaying[0].map,"onNeighbourBroken",0,BLOCK_LEAVES,0); // <--- USE THIS WHEN IT's FIXED
    for(int8_t xoff=-1;xoff<=1;xoff++){
        for(int8_t yoff=-1;yoff<=1;yoff++){
            for(int8_t zoff=-1;zoff<=1;zoff++){
                Mineserver::get()->map(decaying.map)->getBlock(decaying.x +xoff,decaying.y+yoff,decaying.z+zoff,&block,&meta);
                for(uint16_t i =0 ; i<Mineserver::get()->plugin()->getBlockCB().size(); i++){
                    blockcb = Mineserver::get()->plugin()->getBlockCB()[i];
                    if(blockcb!=NULL) {
                        if(blockcb->affectedBlock(block))
                            blockcb->onNeighbourBroken(0,BLOCK_LEAVES,decaying.x +xoff,decaying.y+yoff,decaying.z+zoff,decaying.map,0);
                    }
                }
            }
        }
    }
    for(uint16_t i =0 ; i<Mineserver::get()->plugin()->getBlockCB().size(); i++){
        blockcb = Mineserver::get()->plugin()->getBlockCB()[i];
        if(blockcb!=NULL) {
            if(blockcb->affectedBlock(BLOCK_LEAVES)){
                blockcb->onBroken(0,0,decaying.x,decaying.y,decaying.z,decaying.map,0);
            }
        }
    }
}

void BlockLeaves::timer200(){
    while(decaying.size() > 0){
        if((time(NULL) - decaying[0].decayStart) >= 5){
            decayIt(decaying[0]);
            decaying.erase(decaying.begin());
        }
        else return;
    }
}
