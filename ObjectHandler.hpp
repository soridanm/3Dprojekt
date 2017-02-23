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

	void InitializeObjects(ID3D11Device* Dev);
	bool SetGeometryPassObjectBuffers(ID3D11DeviceContext* DevCon);
	
	int GetHeightMapNrOfFaces();
	int GetHeightMapNrOfVerticies();

	void CreateWorld(ID3D11Device* Dev);
private:
	bool LoadHeightMap(char* filename, HeightMapInfo &hminfo);

	void CreatePerObjectConstantBuffer(ID3D11Device* Dev);
	void CreateMaterialConstantBuffer(ID3D11Device* Dev);

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

	//Height map
	ID3D11Buffer* gSquareIndexBuffer = nullptr;
	ID3D11Buffer* gSquareVertBuffer = nullptr;
	int NUMBER_OF_FACES = 0;
	int NUMBER_OF_VERTICES = 0;
	float WORLD_HEIGHT[200][200];

};

#endif // !OBJECTHANDLER_HPP
