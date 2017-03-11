#include"QuadtreeHandler.hpp"
using namespace DirectX;

Quadtree::Quadtree(XMVECTOR newMin, XMVECTOR newMax, int level) {
	constructNode(newMin, newMax, level,&node);
}

Quadtree::~Quadtree() {

}
void Quadtree::constructNode(DirectX::XMVECTOR newMin, DirectX::XMVECTOR newMax, int level,Node* parent) {
	if (level < maxLevel) {
		float xmiddle = (XMVectorGetX(newMin) + XMVectorGetX(newMax)) / 2;
		float zmiddle = (XMVectorGetZ(newMin) + XMVectorGetZ(newMax)) / 2;
		parent->boxMin = newMin;
		parent->boxMax = newMax;
		parent->levels = level;
		constructNode(newMin, XMVectorSet(xmiddle, XMVectorGetY(newMax), zmiddle, 0), level + 1,parent->children[0]);
		constructNode(XMVectorSet(XMVectorGetX(newMin), XMVectorGetY(newMin), zmiddle, 0), XMVectorSet(xmiddle, XMVectorGetY(newMax), XMVectorGetZ(newMax), 0), level + 1, parent->children[1]);
		constructNode(XMVectorSet(xmiddle, XMVectorGetY(newMin), XMVectorGetZ(newMin), 0), XMVectorSet(XMVectorGetX(newMax), XMVectorGetY(newMax), zmiddle, 0), level + 1, parent->children[2]);
		constructNode(XMVectorSet(xmiddle, XMVectorGetY(newMin), zmiddle, 0), newMax, level + 1, parent->children[3]);
	}
}
void Quadtree::storeObjects(DirectX::XMFLOAT4 box) {
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
}

std::vector<DirectX::XMFLOAT4> Quadtree::getObjects(Node* child) {
	std::vector<DirectX::XMFLOAT4> objectsToReturn;
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