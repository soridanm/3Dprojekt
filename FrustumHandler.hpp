#ifndef FRUSTUMHANDLER_HPP
#define FRUSTUMHANDLER_HPP
#include "GlobalResources.hpp"

struct Plane {
	float a;
	float b;
	float c;
	float d;
};
class FrustumHandler {
public:
	FrustumHandler(DirectX::XMFLOAT4X4 projection= DirectX::XMFLOAT4X4(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0), DirectX::XMFLOAT4X4 view= DirectX::XMFLOAT4X4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
	~FrustumHandler();

	bool checkVisible(DirectX::XMVECTOR, DirectX::XMVECTOR);
	Plane planes[6];
};



#endif