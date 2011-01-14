#include "tree.h"
#include "mineserver.h"

#include "tools.h"

#define Branch(x,y,z) Trunk* v = new Trunk(x,y,z);m_Branch[n_branches] = v;n_branches++;generateBranches(v);

Tree::Tree(int32_t x, int32_t y, int32_t z, uint8_t limit)
{
    _x = x, _y = y, _z = z;
    this->generate(limit);
}


Tree::~Tree(void)
{
}
void Tree::generate(uint8_t limit)
{
    n_branches=0;
    darkness=1;

    srand((uint32_t)time(NULL));

    uint8_t m_trunkHeight = getRandInt(MIN_TRUNK,limit);

    bool smalltree=false;

    //in this implementation we generate the trunk and as we do we call branching and canopy
    if(m_trunkHeight<BRANCHING_HEIGHT){
        smalltree=true;
	darkness=0;
    }
    uint8_t th=m_trunkHeight-1;
    uint8_t i;
    for(i = 0; i < th; i++){
        if(smalltree){
            Trunk* v = new Trunk(_x,_y+i,_z,darkness);
            if(i>=MIN_TRUNK-1){
                m_Branch[n_branches]= v;
                n_branches++;
            }else{
                delete v;
            }
        }
        else{
            Trunk* v = new Trunk(_x,_y+i,_z,darkness);
            if(i>BRANCHING_HEIGHT-1){
                generateBranches(v);
                m_Branch[n_branches]=v;
                n_branches++;
            }else{
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
void Tree::generateBranches(Trunk* wrap){
    vec loc = wrap->location();

    uint8_t posx = loc.x();
    uint8_t posy = loc.y();
    uint8_t posz = loc.z();

    //Not much point to loop here
    //or make a function for the inside of the if.
    if(rand() % BRANCHING_CHANCE == 0){
        Branch(posx+1,posy,posz);
    }
    if(rand() % BRANCHING_CHANCE == 0){
        Branch(posx-1,posy,posz);
    }
    if(rand() % BRANCHING_CHANCE == 0){
        Branch(posx,posy,posz+1);
    }
    if(rand() % BRANCHING_CHANCE == 0 ){
        Branch(posx,posy,posz-1);
    }
    if(rand() % BRANCHING_CHANCE == 0 ){
        Branch(posx,posy+1,posz)
            }
}

void Tree::generateCanopy(){
    uint8_t blocktype;
    uint8_t meta;
    uint8_t canopySize;
    vec loc;


    char canopy_darkness;
    //Not much point making less code with a while/for loop
    //since compiled this is alot faster
    if(rand() % 50 ==0){
        canopy_darkness++;
    }
    if(rand() % 50 ==0){
        canopy_darkness++;
    }
    if(rand() % 50 ==0){
        canopy_darkness++;
    }
    //I'm Not Proud of this looping.
    for(uint8_t i=0;i<n_branches;i++){
        canopySize = getRandInt(MIN_CANOPY,MAX_CANOPY);
        loc = m_Branch[i]->location();
        delete m_Branch[i];

        uint8_t posx = loc.x();
        uint8_t posy = loc.y();
        uint8_t posz = loc.z();

        for(int8_t xi=(-canopySize);xi<=canopySize;xi++){
            for(int8_t yi=(-canopySize);yi<=canopySize;yi++){
                for(int8_t zi=(-canopySize);zi<=canopySize;zi++){
                    if(abs(xi)+abs(yi)+abs(zi) <= canopySize){
                        uint8_t temp_posx = posx+xi;
                        uint8_t temp_posy = posy+yi;
                        uint8_t temp_posz = posz+zi;
                        Mineserver::get()->map()->getBlock(temp_posx,temp_posy,temp_posz,&blocktype,&meta);
                        if(blocktype==0){
                            Canopy u = Canopy(temp_posx,temp_posy,temp_posz,canopy_darkness);
                        }
                    }
                }
            }
        }
    }
}
