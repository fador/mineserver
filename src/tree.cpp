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
  generateTrunk();
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
  m_canopyHeight = CANOPY_HEIGHT;

  int topX = m_Trunk.top().getX();
  int topY = m_Trunk.top().getY();
  int topZ = m_Trunk.top().getZ();

  for (int y = 0; y < m_canopyHeight; y++)
  {
    int r = 2;
    if (m_canopyHeight - y < 2)
    {
      r = 1;
    }

    for (int x = -r; x <= r; x++)
    {
      for (int z = -r; z <= r; z++)
      {
        int useLoc = getRandInt(0, 100);
        int yLoc = topY + y - m_canopyHeight + 2;

        // Probability of skipping a piece of canopy - based on trial and error.
        int prob = 100 - (3 * (abs(x) + abs(z) + 1) * (3 * y + 1));

        // Don't overwrite the trunk, and skip some spots at random based on probability above
        if ((x == 0 && z == 0 && yLoc <= topY) || useLoc > prob)
        {
          continue;
        }                

        Canopy canopy(topX + x, yLoc, topZ + z);
        m_Canopy.push(canopy);
      }
    }

  }
}