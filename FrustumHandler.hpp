/**
* Course: DV1542 - 3D-Programming
* Authors: Viktor Enfeldt, Peter Meunier
*
* File: FrustumHandler.hpp
*
* Class summary:
*	Constructs a mFrustum to test against the quadtree.
*/

#ifndef FRUSTUMHANDLER_HPP
#define FRUSTUMHANDLER_HPP

#include "GlobalResources.hpp"

struct Plane {
	float a;
	float b;
	float c;
	float d;
};

class FrustumHandler {
public:
	FrustumHandler( //do with VP
		DirectX::XMFLOAT4X4 projection = DirectX::XMFLOAT4X4(),
		DirectX::XMFLOAT4X4 view = DirectX::XMFLOAT4X4()
	);
	~FrustumHandler();

	bool checkVisible(DirectX::XMVECTOR boxMin, DirectX::XMVECTOR boxMax);

	Plane planes[6];
};



#endif