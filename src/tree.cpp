#include "tree.h"


Tree::Tree(sint32 x, sint32 y, sint32 z)
{
	_x = x, _y = y, _z = z;
}


Tree::~Tree(void)
{
}


void Tree::Set(sint32 xloc, sint32 yloc, sint32 zloc, int blocktype, char metadata) {
	_x = xloc, _y = yloc, _z = zloc, _type = blocktype, _meta = metadata;
}
void Tree::Update() {
	Mineserver::get()->map()->setBlock(_x, _y, _z, _type, _meta);
	Mineserver::get()->map()->sendBlockChange(_x, _y, _z, _type, _meta);
}

void Tree::Generate() {
	GenerateTrunk();
	//Generate the branch section.
	//Branch branch(_x,_y,_z);
	//m_Branches.push_back(branch);
	GenerateCanopy();
	//Place these in the treeBlocks list.
	m_treeBlocks.push_back(m_Trunk);
	m_treeBlocks.push_back(m_Canopy);
	
	//m_treeBlocks.push_back(m_Branches);
	
	
	

	//Push to client
	for(std::vector<std::vector<ITree> >::iterator section = m_treeBlocks.begin(); section != m_treeBlocks.end(); ++section) {
		for(std::vector<ITree>::iterator block = section->begin(); block != section->end(); ++block) {
			block->Update();
		}
	}
}

void Tree::GenerateTrunk() {
	//Generate the trunk section.
	for(int i = 0; i < (rand() % MAX_TRUNK) + MIN_TRUNK; i++)  {
		Trunk trunk(_x,_y+i,_z);
		m_Trunk.push_back(trunk);
	}
}

void Tree::GenerateCanopy() {
	//Generate the canopy section.
	for(int i = 0; i < (rand() % MAX_CANOPY) + MIN_CANOPY; i++) {
		Canopy canopy(m_Trunk.back());
		canopy.SetY(canopy.GetY() + i);
		m_Canopy.push_back(canopy);
	}
}