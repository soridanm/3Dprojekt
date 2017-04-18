/**
* Course: DV1542 - 3D-Programming
* Authors: Viktor Enfeldt, Peter Meunier
*
* File: QuadreeHandler.hpp
*
* Class summary:
*	Basic quadree that doesn't split any geometry and is only run on startup.
*	"Stores" the static objects in the scene by saving their individual arrays
*	of indexes to their locations in ObjectHandler.mStaticObjects in the leaves
*	of the tree. Note that parent nodes do not store any object indexes.
*
*	The "depth" of the quadtree is specified by NR_OF_QUADREE_LEVELS in 
*	GlobalSettings.hpp
*/

#ifndef QUADTREEHANDLER_HPP
#define QUADTREEHANDLER_HPP

#include "GlobalResources.hpp"
#include "GlobalSettings.hpp"

#include "FrustumHandler.hpp"

struct Node
{
	int levels;
	DirectX::XMVECTOR boxMin;
	DirectX::XMVECTOR boxMax;
	Node* children[4];
	std::vector<UINT>objects;

	// Override the new and delete operators to ensure correct heap allignment
	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

	void operator delete(void* p)
	{
		_mm_free(p);
	}
};

class Quadtree 
{
public:
	Quadtree(
		DirectX::XMVECTOR newMin = DirectX::XMVectorZero(), 
		DirectX::XMVECTOR newMax = DirectX::XMVectorZero(), 
		int level = NR_OF_QUADREE_LEVELS);
	~Quadtree();

	void constructNode(DirectX::XMVECTOR newMin, DirectX::XMVECTOR newMax, int level, Node* parent);
	void storeObjects(UINT index, DirectX::XMVECTOR vector, Node* node);

	std::vector<UINT> getVisibleObjectIndices(Node* node);

	Node* root = new Node();
	FrustumHandler mFrustum = FrustumHandler();
	int maxLevel = NR_OF_QUADREE_LEVELS;
};


#endif