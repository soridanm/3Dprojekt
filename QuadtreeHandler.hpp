#ifndef QUADTREEHANDLER_HPP
#define QUADTREEHANDLER_HPP
#include "GlobalResources.hpp"
#include "FrustumHandler.hpp"
#include <vector>

class Quadtree {
public:
	Node* root;
	FrustumHandler frustum;
	int maxLevel = 5;
	Quadtree(DirectX::XMVECTOR newMin=DirectX::XMVectorZero(), DirectX::XMVECTOR newMax = DirectX::XMVectorZero(), int level=5);
	~Quadtree();
	void constructNode(DirectX::XMVECTOR newMin, DirectX::XMVECTOR newMax, int level,Node* parent);
	std::vector<UINT> getObjects(Node*);
	void storeObjects(UINT, DirectX::XMVECTOR , Node*);

	std::vector<Vertex> getVertexes(Node*);
	ID3D11Buffer* quadVertBuffer = nullptr;
	ID3D11Buffer* quadIndexBuffer = nullptr;
	ID3D11Buffer* quadtreeWorldBuffer = nullptr;
	//ID3D11Buffer* quadtreeWorldBufferData = nullptr; //DATA IS NOT A POINTER
	void createQuadLines(ID3D11Device*);
	int nrOfvertexes;
};


#endif