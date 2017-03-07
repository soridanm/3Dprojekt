#include"QuadtreeHandler.hpp"
using namespace DirectX;
template<typename T>
Node<T>::Node(XMVECTOR newMin, XMVECTOR newMax,int level) {
	if (level < maxLevel) {
		float xmiddle = (XMVectorGetX(newMin) + XMVectorGetX(newMax)) / 2;
		float zmiddle = (XMVectorGetZ(newMin) + XMVectorGetZ(newMax)) / 2;
		children[0]= new Node(newMin, XMVectorSet(xmiddle, XMVectorGetY(newMax), zmiddle,0), level + 1);
		children[1] = new Node(XMVectorSet(XMVectorGetX(newMin),XMVectorGetY(newMin),zmiddle,0), XMVectorSet(xmiddle, XMVectorGetY(newMax), XMVectorGetZ(newMax), 0), level + 1);
		children[2]=new Node(XMVectorSet(xmiddle, XMVectorGetY(newMin), XMVectorGetZ(newMin), 0), XMVectorSet(XMVectorGetX(newMax), XMVectorGetY(newMax), zmiddle, 0), level + 1);
		children[3] = new Node(XMVectorSet(xmiddle, XMVectorGetY(newMin), zmiddle, 0), newMax, level + 1);
	}
	else {
		objects = nullptr;
	}
}
template<typename T>
Node<T>::~Node() {

}

template<typename T>
std::vector<T> Node<T>::getObjects(Node* child) {
	std::vector<T> objectsToReturn = NULL;
		if (FrustumHandler::checkVisible(newMin, newMax)) {
			if (level < maxLevel) {
				for (int i = 0; i < 4; i++) {
					objectsToReturn += getObjects(children[i]);
				}
			}
			else {
				objectsToReturn += objects;
			}
	}
	return objectsToReturn;
}