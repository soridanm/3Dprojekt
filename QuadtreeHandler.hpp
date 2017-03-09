#ifndef QUADTREEHANDLER_HPP
#define QUADTREEHANDLER_HPP
#include "GlobalResources.hpp"
#include "FrustumHandler.hpp"
#include <vector>

class Node {
public:

	int levels;
	int maxLevel = 5;
	DirectX::XMVECTOR boxMin;
	DirectX::XMVECTOR boxMax;
	Node* children[4];
	std::vector<DirectX::XMFLOAT4>objects;
	Node(DirectX::XMVECTOR newMin=DirectX::XMVectorZero(), DirectX::XMVECTOR newMax = DirectX::XMVectorZero(), int level=5);
	~Node();
	std::vector<DirectX::XMFLOAT4> getObjects(Node*);
	void storeObjects(DirectX::XMFLOAT4);

	//what more is needed?
	//store geometry
};


#endif