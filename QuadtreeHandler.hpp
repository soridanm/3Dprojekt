#ifndef QUADTREEHANDLER_HPP
#define QUADTREEHANDLER_HPP
#include "GlobalResources.hpp"
struct Node{
	bool isLeaf;
	DirectX::XMVECTOR boxMin;
	DirectX::XMVECTOR boxMax;
	Node* children[4];
	
};

class QuadtreeHandler {
public:


};


#endif
