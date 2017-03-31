#include"QuadtreeHandler.hpp"

Quadtree::Quadtree(DirectX::XMVECTOR newMin, DirectX::XMVECTOR newMax, int level) {
	root = new Node();
	constructNode(newMin, newMax, level,root);
}

Quadtree::~Quadtree() {

}

void Quadtree::constructNode(DirectX::XMVECTOR newMin, DirectX::XMVECTOR newMax, int level,Node* parent) {
	float xmiddle = (DirectX::XMVectorGetX(newMin) + DirectX::XMVectorGetX(newMax)) / 2.0f;
	float zmiddle = (DirectX::XMVectorGetZ(newMin) + DirectX::XMVectorGetZ(newMax)) / 2.0f;
	parent->boxMin = newMin;
	parent->boxMax = newMax;
	parent->levels = level;
	if (level < maxLevel) {
		for (int i = 0; i < 4; i++) {
			parent->children[i] = new Node();
		}
		constructNode(newMin, DirectX::XMVectorSet(xmiddle, DirectX::XMVectorGetY(newMax), zmiddle, 0), level + 1,parent->children[0]);
		constructNode(DirectX::XMVectorSet(DirectX::XMVectorGetX(newMin), DirectX::XMVectorGetY(newMin), zmiddle, 0), DirectX::XMVectorSet(xmiddle, DirectX::XMVectorGetY(newMax), DirectX::XMVectorGetZ(newMax), 0), level + 1, parent->children[1]);
		constructNode(DirectX::XMVectorSet(xmiddle, DirectX::XMVectorGetY(newMin), DirectX::XMVectorGetZ(newMin), 0), DirectX::XMVectorSet(DirectX::XMVectorGetX(newMax), DirectX::XMVectorGetY(newMax), zmiddle, 0), level + 1, parent->children[2]);
		constructNode(DirectX::XMVectorSet(xmiddle, DirectX::XMVectorGetY(newMin), zmiddle, 0), newMax, level + 1, parent->children[3]);
	}
}

void Quadtree::storeObjects(UINT index,DirectX::XMVECTOR vertex,Node* parent) {
	if (parent->levels < maxLevel) {
		float xmiddle = (DirectX::XMVectorGetX(parent->boxMin) + DirectX::XMVectorGetX(parent->boxMax)) / 2;
		float zmiddle = (DirectX::XMVectorGetZ(parent->boxMin) + DirectX::XMVectorGetZ(parent->boxMax)) / 2;
		if (DirectX::XMVectorGetX(vertex) < xmiddle) {
			if (DirectX::XMVectorGetZ(vertex) < zmiddle) {
				storeObjects(index,vertex, parent->children[0]);
			}
			else {
				storeObjects(index,vertex, parent->children[1]);
			}
		}
		else {
			if (DirectX::XMVectorGetZ(vertex) < zmiddle) {
				storeObjects(index,vertex, parent->children[2]);
			}
			else {
				storeObjects(index,vertex, parent->children[3]);
			}
		}
	}
	else {
		if (std::find(parent->objects.begin(), parent->objects.end(), index) == parent->objects.end()) {
			parent->objects.push_back(index);
		}
	}
}

std::vector<UINT> Quadtree::getObjects(Node* parent) {
	std::vector<UINT> objectsToReturn;
	if (frustum.checkVisible(parent->boxMin, parent-> boxMax)) {
		std::vector<UINT> currentObjects;
		if (parent->levels < maxLevel) {
			for (int i = 0; i < 4; i++) {
				currentObjects = getObjects(parent->children[i]);
				objectsToReturn.insert(objectsToReturn.end(), currentObjects.begin(), currentObjects.end());
			}
		}
		else {
			objectsToReturn.insert(objectsToReturn.end(), parent->objects.begin(), parent->objects.end());
		}
	}
	std::sort(objectsToReturn.begin(), objectsToReturn.end());
	objectsToReturn.erase(std::unique(objectsToReturn.begin(), objectsToReturn.end()), objectsToReturn.end());

	return objectsToReturn;
}