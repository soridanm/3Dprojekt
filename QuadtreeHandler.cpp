#include"QuadtreeHandler.hpp"
using namespace DirectX; //NO

Quadtree::Quadtree(XMVECTOR newMin, XMVECTOR newMax, int level) {
	root = new Node();
	constructNode(newMin, newMax, level,root);
}

Quadtree::~Quadtree() {

}

void Quadtree::constructNode(DirectX::XMVECTOR newMin, DirectX::XMVECTOR newMax, int level,Node* parent) {
	float xmiddle = (XMVectorGetX(newMin) + XMVectorGetX(newMax)) / 2.0f;
	float zmiddle = (XMVectorGetZ(newMin) + XMVectorGetZ(newMax)) / 2.0f;
	parent->boxMin = newMin;
	parent->boxMax = newMax;
	parent->levels = level;
	if (level < maxLevel) {
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
	return objectsToReturn;
}

std::vector<Vertex> Quadtree::getVertexes(Node* parent) {
	std::vector<Vertex> toReturn;
		if (parent->levels < maxLevel) {
			for (int i = 0; i < 4; i++) {
				std::vector<Vertex> toGet=getVertexes(parent->children[i]);
				toReturn.insert(toReturn.end(), toGet.begin(), toGet.end());
			}
		}
			//getting points for the corners used in the diagonals
			float xMin, xMax, yMin, yMax, zMin, zMax;
			xMin = DirectX::XMVectorGetX(parent->boxMin);
			xMax = DirectX::XMVectorGetX(parent->boxMax);
			yMin = DirectX::XMVectorGetY(parent->boxMin);
			yMax = DirectX::XMVectorGetY(parent->boxMax);
			zMin = DirectX::XMVectorGetZ(parent->boxMin);
			zMax = DirectX::XMVectorGetZ(parent->boxMax);

			//seting corners used for diagonals
			DirectX::XMFLOAT3 corner[8];
			DirectX::XMStoreFloat3(&corner[0],parent->boxMin);
			DirectX::XMStoreFloat3(&corner[6],parent->boxMax);
			corner[3] = DirectX::XMFLOAT3(xMin, yMin, zMax);
			corner[5] = DirectX::XMFLOAT3(xMax, yMax, zMin);
			corner[7] = DirectX::XMFLOAT3(xMin, yMax, zMax);
			corner[1] = DirectX::XMFLOAT3(xMax, yMin, zMin);
			corner[4] = DirectX::XMFLOAT3(xMin, yMax, zMin);
			corner[2] = DirectX::XMFLOAT3(xMax, yMin, zMax);
			Vertex vertex[8];
			for (int i = 0; i < 8; i++) {
				vertex[i].pos = corner[i];
				vertex[i].normal= DirectX::XMFLOAT3(0.f,0.f,0.f);
				vertex[i].texCoord = DirectX::XMFLOAT2(0.f, 0.f);
				toReturn.push_back(vertex[i]);
			}

			return toReturn;
}

void Quadtree::createQuadLines(ID3D11Device* Dev) {
	std::vector<Vertex> vertexes;
	vertexes = getVertexes(root);
	std::vector<DWORD> drawOrder(vertexes.size() * 2);
	nrOfvertexes = vertexes.size() * 2;
	
	for (DWORD k = 0; k <(vertexes.size()*2-16); k += 16) {
			drawOrder[k] = k;//bottom left
			drawOrder[k + 1] = (k+1);//top left
			drawOrder[k + 2] = (k+2);//bottom right
			drawOrder[k + 3] = (3 + k);
			drawOrder[k + 4] = ( k);
			drawOrder[k + 5] = (4 + k);
			drawOrder[k + 6] = (5 + k);
			drawOrder[k + 7] = (6 + k);
			drawOrder[k + 8] = (7 + k);
			drawOrder[k + 9] = (4 + k);
			drawOrder[k + 10] = (5 + k);
			drawOrder[k + 11] = (1 + k);
			drawOrder[k + 12] = (2 + k);
			drawOrder[k + 13] = (6 + k);
			drawOrder[k + 14] = (7 + k);
			drawOrder[k + 15] = (3 + k);
		}


	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD)*vertexes.size() * 2;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &drawOrder[0];
	Dev->CreateBuffer(&indexBufferDesc, &iinitData, &quadIndexBuffer);

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex)*vertexes.size();
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = &vertexes[0];
	Dev->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &quadVertBuffer);
}