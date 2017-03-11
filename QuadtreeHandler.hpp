#ifndef QUADTREEHANDLER_HPP
#define QUADTREEHANDLER_HPP
#include "GlobalResources.hpp"
#include "FrustumHandler.hpp"
#include <vector>

class Quadtree {
public:
	Node* node;
	FrustumHandler frustum;
	int maxLevel = 5;
	Quadtree(DirectX::XMVECTOR newMin=DirectX::XMVectorZero(), DirectX::XMVECTOR newMax = DirectX::XMVectorZero(), int level=5);
	~Quadtree();
	void constructNode(DirectX::XMVECTOR newMin, DirectX::XMVECTOR newMax, int level,Node* parent);
	std::vector<Vertex> getObjects(Node*);
	void storeObjects(Vertex,Node*);

	//what more is needed?
	//store geometry
};


#endif