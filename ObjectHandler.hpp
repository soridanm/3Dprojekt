#ifndef OBJECTHANDLER_HPP
#define OBJECTHANDLER_HPP

#include "GlobalResources.hpp"
	
struct materialStruct
{
	materialStruct(float r = 0.0f, float b = 0.0f, float g = 0.0f, float specPow = 128.0f)
		: specularAlbedo(r, g, b), specularPower(specPow)
	{}
	DirectX::XMFLOAT3 specularAlbedo;
	float specularPower;
};

namespace Materials
{
	static const materialStruct Black_plastic	= materialStruct(0.5f, 0.5f, 0.5f, 32.0f);
	static const materialStruct Black_rubber	= materialStruct(0.4f, 0.4f, 0.4f, 10.0f);
}

class ObjectHandler
{
public:
	ObjectHandler();
	~ObjectHandler();

	bool SetGeometryPassObjectBuffers();
	
private:
	void CreatePerObjectConstantBuffer();
	void CreateMaterialConstantBuffer();

	struct cPerObjectBuffer
	{
		DirectX::XMFLOAT4X4 World;
	};
	static_assert((sizeof(cPerObjectBuffer) % 16) == 0, "cPerObjectBuffer size must be 16-byte aligned");

	//struct material WILL BE REWRITTEN WHIT CODE FROM OTHER BRANCH

	struct cMaterialBuffer
	{
		cMaterialBuffer(materialStruct mat = materialStruct()) : material(mat)
		{}
		materialStruct material;
	}; 
	static_assert((sizeof(cMaterialBuffer) % 16) == 0, "cMaterialBuffer size must be 16-byte aligned");


	ID3D11Buffer* gPerObjectBuffer;
	cPerObjectBuffer ObjectBufferData;
	ID3D11Buffer* gMaterialBuffer;
	cMaterialBuffer gMaterialBufferData;
	ID3D11ShaderResourceView* mTextureView;


};

#endif // !OBJECTHANDLER_HPP
