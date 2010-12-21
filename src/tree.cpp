#include "tree.h"


Tree::Tree(sint32 x, sint32 y, sint32 z)
{
	_x = x, _y = y, _z = z;
}


Tree::~Tree(void)
{
}


void Tree::set(sint32 xloc, sint32 yloc, sint32 zloc, int blockType, char metaData) {
	_x = xloc, _y = yloc, _z = zloc, _type = blockType, _meta = metaData;
}

void Tree::generate() {
	generateTrunk();
	//Generate the branch section.
	//Branch branch(_x,_y,_z);
	//m_Branches.push_back(branch);
	generateCanopy();
	//Place these in the treeBlocks list.
	
	m_treeBlocks.push(m_Canopy);
	m_treeBlocks.push(m_Trunk);
	
	//m_treeBlocks.push_back(m_Branches);
	
	while(!m_treeBlocks.empty()) {
		std::stack<ITree> section = m_treeBlocks.top();
		while(!section.empty()) {
			section.top().Update();
			section.pop();
		}
		m_treeBlocks.pop();
	}
}

void Tree::generateTrunk() {
	//Generate the trunk section.
	for(int i = 0; i < MIN_TRUNK; i++)  {
		Trunk trunk(_x,_y+i,_z);
		m_Trunk.push(trunk);
	}
	for(int i = MIN_TRUNK; i < (rand() % MAX_TRUNK); i++)  {
		Trunk trunk(_x,_y+i,_z);
		m_Trunk.push(trunk);
	}
}

void Tree::generateCanopy() {
	//Generate the canopy section.
	for(int i = 0; i < MIN_CANOPY; i++) {
		Canopy canopy(m_Trunk.top());
		canopy.SetY(canopy.GetY() + i);
		m_Canopy.push(canopy);
	}
	for(int i = MIN_CANOPY; i < (rand() % MAX_CANOPY); i++) {
		Canopy canopy(m_Trunk.top());
		canopy.SetY(canopy.GetY() + i);
		m_Canopy.push(canopy);
	}
}