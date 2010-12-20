#ifndef _TREE_H
#define _TREE_H

#include "tools.h"
#include "map.h"
#include "screen.h"
#include "vec.h"
#include <stack>

enum { MAX_TRUNK = 13, MIN_TRUNK = 4, MAX_CANOPY = 3, MIN_CANOPY = 1 };

class ITree {
	public:
		ITree() { }
		virtual ~ITree() { }
		virtual void ITree::Update() {
			Map::get()->setBlock(_x, _y, _z, _type, _meta);
			Map::get()->sendBlockChange(_x, _y, _z, _type, _meta);
		}
		virtual void ITree::SetY(sint32 y) { _y = y; }
		virtual const sint32 ITree::GetY(void) { return _y; }
		virtual const vec ITree::Location() { return vec(_x,_y,_z); }
		virtual const int ITree::Type() { return _type; }
	protected:
		sint32 _x;
		sint32 _y;
		sint32 _z;
		int _type;
		char _meta;

};

class Trunk : public ITree 
{
	public:
		Trunk(sint32 x, sint32 y, sint32 z) { _x = x, _y = y, _z = z, _type = BLOCK_LOG, _meta = 0; }
		~Trunk() { }
	protected:
};

class Branch : public ITree
{
	public:
		Branch(sint32 x, sint32 y, sint32 z) { _x = x, _y = y, _z = z, _type = BLOCK_LOG, _meta = 0; }
		~Branch() { }
	protected:

};



class Canopy : public ITree 
{
	public:
		Canopy(sint32 x, sint32 y, sint32 z) { _x = x, _y = y, _z = z, _type = BLOCK_LEAVES, _meta = 0; }
		Canopy(ITree trunk) { vec loc = trunk.Location(); _x = loc.x(), _y = loc.y(), _z = loc.z(), _type = BLOCK_LEAVES, _meta = 0; }
		~Canopy() { }
	protected:
};



class Tree : public ITree
{
	public:
		Tree(sint32 x, sint32 y, sint32 z);
		void Tree::Generate(void);
		~Tree(void);
	protected:
		void Tree::Set(sint32 xloc, sint32 yloc, sint32 zloc, int blocktype, char metadata);
	private:
		std::stack<std::stack<ITree>> m_treeBlocks;
		std::stack<ITree> m_Trunk;
		std::stack<ITree> m_Branches;
		std::stack<ITree> m_Canopy;
		void Tree::GenerateTrunk();
		void Tree::GenerateCanopy();
};

#endif