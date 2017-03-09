#include"QuadtreeHandler.hpp"
using namespace DirectX;

Node::Node(XMVECTOR newMin, XMVECTOR newMax, int level) {
	if (level < maxLevel) {
		float xmiddle = (XMVectorGetX(newMin) + XMVectorGetX(newMax)) / 2;
		float zmiddle = (XMVectorGetZ(newMin) + XMVectorGetZ(newMax)) / 2;
		children[0] = new Node(newMin, XMVectorSet(xmiddle, XMVectorGetY(newMax), zmiddle, 0), level + 1);
		children[1] = new Node(XMVectorSet(XMVectorGetX(newMin), XMVectorGetY(newMin), zmiddle, 0), XMVectorSet(xmiddle, XMVectorGetY(newMax), XMVectorGetZ(newMax), 0), level + 1);
		children[2] = new Node(XMVectorSet(xmiddle, XMVectorGetY(newMin), XMVectorGetZ(newMin), 0), XMVectorSet(XMVectorGetX(newMax), XMVectorGetY(newMax), zmiddle, 0), level + 1);
		children[3] = new Node(XMVectorSet(xmiddle, XMVectorGetY(newMin), zmiddle, 0), newMax, level + 1);
	}
}

Node::~Node() {

}

void Node::storeObjects(DirectX::XMFLOAT4 box) {
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

std::vector<DirectX::XMFLOAT4> Node::getObjects(Node* child) {
	std::vector<DirectX::XMFLOAT4> objectsToReturn;
	if (FrustumHandler::checkVisible(boxMin, boxMax)) {
		if (levels < maxLevel) {
			for (int i = 0; i < 4; i++) {
				objectsToReturn.insert(objectsToReturn.end(), getObjects(children[i]).begin(), getObjects(children[i]).end());
			}
		}
		else {
			objectsToReturn.insert(objectsToReturn.end(),objects.begin(), objects.end());
		}
	}
	return objectsToReturn;
}