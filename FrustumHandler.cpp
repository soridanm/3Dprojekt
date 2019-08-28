#include "pch.h"

#include "FrustumHandler.hpp"

/*============================================================================
*						Public functions
*===========================================================================*/
FrustumHandler::FrustumHandler(DirectX::XMFLOAT4X4 projection, DirectX::XMFLOAT4X4 view) 
{

	DirectX::XMFLOAT4X4 VP;

	//create frustummatrix
	DirectX::XMStoreFloat4x4(
		&VP, 
		DirectX::XMMatrixMultiply((DirectX::XMLoadFloat4x4(&view)), DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&projection)))
	);

	//near plane
	planes[0].a = VP._14 + VP._13;
	planes[0].b = VP._24 + VP._23;
	planes[0].c = VP._34 + VP._33;
	planes[0].d = VP._44 + VP._43;

	//far plane
	planes[1].a = VP._14 - VP._13;
	planes[1].b = VP._24 - VP._23;
	planes[1].c = VP._34 - VP._33;
	planes[1].d = VP._44 - VP._43;

	//left plane
	planes[2].a = VP._14 + VP._11;
	planes[2].b = VP._24 + VP._21;
	planes[2].c = VP._34 + VP._31;
	planes[2].d = VP._44 + VP._41;

	//right plane
	planes[3].a = VP._14 - VP._11;
	planes[3].b = VP._24 - VP._21;
	planes[3].c = VP._34 - VP._31;
	planes[3].d = VP._44 - VP._41;

	//top plane
	planes[4].a = VP._14 - VP._12;
	planes[4].b = VP._24 - VP._22;
	planes[4].c = VP._34 - VP._32;
	planes[4].d = VP._44 - VP._42;
	//have to normalize

	//bottom plane
	planes[5].a = VP._14 + VP._12;
	planes[5].b = VP._24 + VP._22;
	planes[5].c = VP._34 + VP._32;
	planes[5].d = VP._44 + VP._42;

	//normalize all the planes
	for (int i = 0; i < 6; i++)
	{
		float length = sqrt((planes[i].a*planes[i].a) + (planes[i].b*planes[i].b) + (planes[i].c*planes[i].c));
		if (length == 0)
		{
			return;
		}
		planes[i].a /= length;
		planes[i].b /= length;
		planes[i].c /= length;
		planes[i].d /= length;
	}
}

FrustumHandler::~FrustumHandler() 
{}

bool FrustumHandler::checkVisible(DirectX::XMVECTOR boxMin, DirectX::XMVECTOR boxMax) 
{
	using DirectX::operator-;

	bool isVisible = true;

	//getting points for the corners used in the diagonals
	float xMin, xMax, yMin, yMax, zMin, zMax,xMiddle,yMiddle,zMiddle;
	xMin = DirectX::XMVectorGetX(boxMin);
	xMax = DirectX::XMVectorGetX(boxMax);
	yMin = DirectX::XMVectorGetY(boxMin);
	yMax = DirectX::XMVectorGetY(boxMax);
	zMin = DirectX::XMVectorGetZ(boxMin);
	zMax = DirectX::XMVectorGetZ(boxMax);
	xMiddle = (xMin + xMax) / 2;
	yMiddle = (yMin + yMax) / 2;
	zMiddle = (zMin + zMax) / 2;

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
	diagonal[0] = corner2 - corner1;
	diagonal[1] = corner4 - corner3;
	diagonal[2] = corner6 - corner5;
	diagonal[3] = corner8 - corner7;
	diagonal[4] = corner1 - corner2;
	diagonal[5] = corner3 - corner4;
	diagonal[6] = corner5 - corner6;
	diagonal[7] = corner7 - corner8;


	DirectX::XMVECTOR mostResembling;
	for (int i = 0; i < 6; i++)
	{
		float highDot = 0.0f;

		// Find which diagonal resembles the normal of this plane the most
		for (int j = 0; j < 8; j++)
		{
			// All centerToCorner are the same length, therefore they don't need to be normalized for this to work
			float tempDot = planes[i].a*diagonal[j].m128_f32[0] + planes[i].b*diagonal[j].m128_f32[1] + planes[i].c*diagonal[j].m128_f32[2];
			if (tempDot > highDot)
			{
				highDot = tempDot;
				mostResembling = diagonal[j];
			}
		}

		DirectX::XMVECTOR point = 
		{ 
			xMiddle - mostResembling.m128_f32[0], 
			yMiddle - mostResembling.m128_f32[1],
			zMiddle - mostResembling.m128_f32[2], 
			1.0f 
		};

		// Check if a point is in front of plane
		float testPoint = planes[i].a*point.m128_f32[0] + planes[i].b*point.m128_f32[1] + planes[i].c*point.m128_f32[2] - planes[i].d;
		if (testPoint > 0.0f)
		{
			isVisible = false;
		}
	}

	return isVisible;
}