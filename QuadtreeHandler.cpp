#include"QuadtreeHandler.hpp"
using namespace DirectX;

Quadtree::Quadtree(XMVECTOR newMin, XMVECTOR newMax, int level) {
	root = new Node();
	constructNode(newMin, newMax, level,root);
}

Quadtree::~Quadtree() {

}
void Quadtree::constructNode(DirectX::XMVECTOR newMin, DirectX::XMVECTOR newMax, int level,Node* parent) {
	if (level <= maxLevel) {
		float xmiddle = (XMVectorGetX(newMin) + XMVectorGetX(newMax)) / 2;
		float zmiddle = (XMVectorGetZ(newMin) + XMVectorGetZ(newMax)) / 2;
		parent->boxMin = newMin;
		parent->boxMax = newMax;
		parent->levels = level;
		for (int i = 0; i < 4; i++) {
			parent->children[i] = new Node();
		}
		constructNode(newMin, XMVectorSet(xmiddle, XMVectorGetY(newMax), zmiddle, 0), level + 1,parent->children[0]);
		constructNode(XMVectorSet(XMVectorGetX(newMin), XMVectorGetY(newMin), zmiddle, 0), XMVectorSet(xmiddle, XMVectorGetY(newMax), XMVectorGetZ(newMax), 0), level + 1, parent->children[1]);
		constructNode(XMVectorSet(xmiddle, XMVectorGetY(newMin), XMVectorGetZ(newMin), 0), XMVectorSet(XMVectorGetX(newMax), XMVectorGetY(newMax), zmiddle, 0), level + 1, parent->children[2]);
		constructNode(XMVectorSet(xmiddle, XMVectorGetY(newMin), zmiddle, 0), newMax, level + 1, parent->children[3]);
	}
}
void Quadtree::storeObjects(UINT index,DirectX::XMVECTOR vertex,Node* parent) {
	if (parent->levels < maxLevel) {
		float xmiddle = (XMVectorGetX(parent->boxMin) + XMVectorGetX(parent->boxMax)) / 2;
		float zmiddle = (XMVectorGetZ(parent->boxMin) + XMVectorGetZ(parent->boxMax)) / 2;
		if (XMVectorGetX(vertex) < xmiddle) {
			if (XMVectorGetZ(vertex) < zmiddle) {
				storeObjects(index,vertex, parent->children[0]);
			}
			else {
				storeObjects(index,vertex, parent->children[1]);
			}
		}
		else {
			if (XMVectorGetZ(vertex) < zmiddle) {
				storeObjects(index,vertex, parent->children[2]);
			}
			else {
				storeObjects(index,vertex, parent->children[3]);
			}
		}
	}
	else {
		parent->objects.push_back(index);
	}
}

std::vector<UINT> Quadtree::getObjects(Node* child) {
	std::vector<UINT> objectsToReturn;
	if (frustum.checkVisible(child->boxMin,child-> boxMax)) {
		if (child->levels < maxLevel) {
			for (int i = 0; i < 4; i++) {
				objectsToReturn.insert(objectsToReturn.end(), getObjects(child->children[i]).begin(), getObjects(child->children[i]).end());
			}
		}
		else {
			objectsToReturn.insert(objectsToReturn.end(),child->objects.begin(), child->objects.end());
		}
	}
	return objectsToReturn;
}