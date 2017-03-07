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
	FrustumHandler();
	~FrustumHandler();

	void constructFrustum(float depth, DirectX::XMFLOAT4X4* projection, DirectX::XMFLOAT4X4* view);
	bool checkVisible(DirectX::XMVECTOR,DirectX::XMVECTOR);
private:
	Plane planes[6];
};



#endif