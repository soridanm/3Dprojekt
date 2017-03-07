#include "FrustumHandler.hpp"

FrustumHandler::FrustumHandler() {

}
FrustumHandler::~FrustumHandler() {

}

void FrustumHandler::constructFrustum(float depth, DirectX::XMFLOAT4X4* projection, DirectX::XMFLOAT4X4* view) {
	
	DirectX::XMFLOAT4X4* matrix;

	//create frustummatrix
	DirectX::XMStoreFloat4x4(matrix, DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(view), DirectX::XMLoadFloat4x4(projection)));

	//near plane
	planes[0].a = matrix->_14 + matrix->_13;
	planes[0].b = matrix->_24 + matrix->_23;
	planes[0].c = matrix->_34 + matrix->_33;
	planes[0].d = matrix->_44 + matrix->_43;

	//far plane
	planes[0].a = matrix->_14 - matrix->_13;
	planes[0].b = matrix->_24 - matrix->_23;
	planes[0].c = matrix->_34 - matrix->_33;
	planes[0].d = matrix->_44 - matrix->_43;

	//left plane
	planes[2].a = matrix->_14 + matrix->_11;
	planes[2].b = matrix->_24 + matrix->_21;
	planes[2].c = matrix->_34 + matrix->_31;
	planes[2].d = matrix->_44 + matrix->_41;

	//right plane
	planes[3].a = matrix->_14 - matrix->_11;
	planes[3].b = matrix->_24 - matrix->_21;
	planes[3].c = matrix->_34 - matrix->_31;
	planes[3].d = matrix->_44 - matrix->_41;

	//top plane
	planes[4].a = matrix->_14 - matrix->_12;
	planes[4].b = matrix->_24 - matrix->_22;
	planes[4].c = matrix->_34 - matrix->_32;
	planes[4].d = matrix->_44 - matrix->_42;
	//have to normalize

	//bottom plane
	planes[5].a = matrix->_14 + matrix->_12;
	planes[5].b = matrix->_24 + matrix->_22;
	planes[5].c = matrix->_34 + matrix->_32;
	planes[5].d = matrix->_44 + matrix->_42;

//normalize all the planes
	for (int i = 0; i < 6; i++) {
		float length = sqrt((planes[i].a*planes[i].a) + (planes[i].b*planes[i].b) + (planes[i].c*planes[i].c));
		planes[i].a /= length;
		planes[i].b /= length;
		planes[i].c /= length;
		planes[i].d /= length;
	}
}

bool FrustumHandler::checkVisible(DirectX::XMVECTOR boxMin,DirectX::XMVECTOR boxMax) {
	bool isVisible = false;
	for (int i = 0; i < 6; i++) {

	}


	return isVisible;
}