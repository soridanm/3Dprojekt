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
	bool checkCube(float, float, float, float,float,float);
private:
	Plane planes[6];
};



#endif