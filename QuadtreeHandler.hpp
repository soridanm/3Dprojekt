#ifndef QUADTREEHANDLER_HPP
#define QUADTREEHANDLER_HPP
#include "GlobalResources.hpp"
#include "FrustumHandler.hpp"
#include <vector>
template<typename T>
class Node {
public:

	int levels;
	int maxLevel = 5;
	DirectX::XMVECTOR boxMin;
	DirectX::XMVECTOR boxMax;
	Node* children[4];
	std::vector<T>objects;
	Node(DirectX::XMVECTOR newMin, DirectX::XMVECTOR newMax, int level);
	~Node();
	std::vector<T> getObjects(Node*);

	//what more is needed?
	//traverse nodes
	//store geometry
};


#endif