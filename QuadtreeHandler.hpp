#pragma once
#ifndef QUADTREEHANDLER_HPP
#define QUADTREEHANDLER_HPP
#include "GlobalResources.hpp"

class QuadNode {
public:
	DirectX::XMVECTOR vecBoxMin;
	DirectX::XMVECTOR vecBoxMax;

	QuadNode* children[4];
};

#endif