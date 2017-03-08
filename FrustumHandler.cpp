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
	bool isVisible = true;

	//getting points for the corners used in the diagonals
	float xMin, xMax, yMin, yMax, zMin, zMax;
	xMin = DirectX::XMVectorGetX(boxMin);
	xMax = DirectX::XMVectorGetX(boxMax);
	yMin = DirectX::XMVectorGetY(boxMin);
	yMax = DirectX::XMVectorGetY(boxMax);
	zMin = DirectX::XMVectorGetZ(boxMin);
	zMax = DirectX::XMVectorGetZ(boxMax);

	//seting corners used for diagonals
	DirectX::XMVECTOR corner1, corner2, corner3, corner4, corner5, corner6, corner7, corner8;
	corner1 = boxMin;
	corner2 = boxMax;
	corner3 = DirectX::XMVectorSet(xMin, yMin, zMax, 0);
	corner4 = DirectX::XMVectorSet(xMax, yMax, zMin, 0);
	corner5 = DirectX::XMVectorSet(xMin, yMax, zMax, 0);
	corner6 = DirectX::XMVectorSet(xMax, yMin, zMin, 0);
	corner7 = DirectX::XMVectorSet(xMin, yMax, zMin, 0);
	corner8 = DirectX::XMVectorSet(xMax, yMin, zMax, 0);


	DirectX::XMVECTOR diagonal[8];
	using DirectX::operator-;
	diagonal[0] = corner2 - corner1;
	diagonal[1] = corner4 - corner3;
	diagonal[2] = corner6 - corner5;
	diagonal[3] = corner8 - corner7;
	diagonal[4] = corner1 - corner2;
	diagonal[5] = corner3 - corner4;
	diagonal[6] = corner5 - corner6;
	diagonal[7] = corner7 - corner8;

	//normals from planes point inwards
	for (int i = 0; i < 6; i++) {
		int diagonalCheck = 0;
		float dot = 0;
		DirectX::XMVECTOR plane =DirectX::XMVectorSet(planes[i].a, planes[i].b, planes[i].c, planes[i].d);
		for (int j = 0; j < 8; j++) {
			float temp = DirectX::XMVectorGetX(DirectX::XMVector3Dot(diagonal[j], plane));
			if (temp > dot) {
				diagonalCheck = j;
				dot = temp;
			}
		}
		if (diagonalCheck == 0 && DirectX::XMVectorGetX(DirectX::XMPlaneDot(plane, corner2)) < 0 ) {
				isVisible = false;
		}
		if (diagonalCheck == 1 && DirectX::XMVectorGetX(DirectX::XMPlaneDot(plane, corner4)) < 0) {
			isVisible = false;
		}
		if (diagonalCheck == 2 && DirectX::XMVectorGetX(DirectX::XMPlaneDot(plane, corner6)) < 0) {
			isVisible = false;
		}
		if (diagonalCheck == 3 && DirectX::XMVectorGetX(DirectX::XMPlaneDot(plane, corner8)) < 0) {
			isVisible = false;
		}
		if (diagonalCheck == 4 && DirectX::XMVectorGetX(DirectX::XMPlaneDot(plane, corner1)) < 0) {
			isVisible = false;
		}
		if (diagonalCheck == 5 && DirectX::XMVectorGetX(DirectX::XMPlaneDot(plane, corner3)) < 0) {
			isVisible = false;
		}
		if (diagonalCheck == 6 && DirectX::XMVectorGetX(DirectX::XMPlaneDot(plane, corner5)) < 0) {
			isVisible = false;
		}
		if (diagonalCheck == 7 && DirectX::XMVectorGetX(DirectX::XMPlaneDot(plane, corner7)) < 0) {
			isVisible = false;
		}
	}
	return isVisible;
}