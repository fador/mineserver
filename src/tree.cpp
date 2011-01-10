#include "tree.h"

#include "tools.h"

Tree::Tree(int32_t x, int32_t y, int32_t z)
{
  _x = x, _y = y, _z = z;
}


Tree::~Tree(void)
{
}


void Tree::set(int32_t xloc, int32_t yloc, int32_t zloc, int blockType, char metaData)
{
  _x = xloc, _y = yloc, _z = zloc, _type = blockType, _meta = metaData;
}

void Tree::generate()
{
  srand((uint32_t)time(NULL));
  generateTrunk();
  //Generate the branch section.
  //Branch branch(_x,_y,_z);
  //m_Branches.push_back(branch);
  generateCanopy();
  //Place these in the treeBlocks list.
  
  m_treeBlocks.push(m_Canopy);
  m_treeBlocks.push(m_Trunk);
  
  //m_treeBlocks.push_back(m_Branches);
  while(!m_treeBlocks.empty())
  {
    std::stack<ITree> section = m_treeBlocks.top();
    while(!section.empty())
    {
      section.top().update();
      section.pop();
    }
    m_treeBlocks.pop();
  }
}

void Tree::generateTrunk()
{
  //Generate the trunk section.
  m_trunkHeight = getRandInt(MIN_TRUNK,MAX_TRUNK);
  for(int i = 0; i < m_trunkHeight; i++)
  {
    Trunk trunk(_x,_y+i,_z);
    m_Trunk.push(trunk);
  }
  assert(m_Trunk.size() >= MIN_TRUNK && m_Trunk.size() <= MAX_TRUNK);
}

void Tree::generateCanopy()
{
  //Generate the canopy section.
  m_canopyHeight = getRandInt(MIN_CANOPY,MAX_CANOPY);
  for(int i = 0; i < m_canopyHeight; i++)
  {
    Canopy canopy(m_Trunk.top());
    canopy.setY(canopy.getY()+1 + i);
    m_Canopy.push(canopy);
  }
  assert(m_Canopy.size() >= MIN_CANOPY && m_Canopy.size() <= MAX_CANOPY);
}