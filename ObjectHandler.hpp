/* TODO: Might merge the perObject- and materialBuffers
*
*
*/


#ifndef OBJECTHANDLER_HPP
#define OBJECTHANDLER_HPP

#include "GlobalResources.hpp"
	

//Might be moved into the class
struct cMaterialBuffer
{
	cMaterialBuffer(float r = 0.0f, float g = 0.0f, float b = 0.0f, float specPow = 128.0f, bool hasTex = false, int texInd = 0)
		: SpecularColor(r, g, b), SpecularPower(specPow), DiffuseColor(r, g, b), HasTexture(hasTex), TexArrIndex(texInd), padding(0.0f, 0.0f, 0.0f)
	{}
	DirectX::XMFLOAT3 SpecularColor;
	float SpecularPower;
	DirectX::XMFLOAT3 DiffuseColor;
	int TexArrIndex;
	int HasTexture;

	DirectX::XMFLOAT3 padding;
}; 
static_assert((sizeof(cMaterialBuffer) % 16) == 0, "cMaterialBuffer size must be 16-byte aligned");

struct materialStruct
{
	materialStruct() : Data(cMaterialBuffer()), matName(L"No name") {}

	cMaterialBuffer Data;
	std::wstring matName;
};

namespace Materials
{
	static const cMaterialBuffer Black_plastic	= cMaterialBuffer(/*L"Black plastic",*/	0.5f, 0.5f, 0.5f, 32.0f);
	static const cMaterialBuffer Black_rubber	= cMaterialBuffer(/*L"Black rubber",*/	0.4f, 0.4f, 0.4f, 10.0f);
}

class ObjectHandler
{
public:
	ObjectHandler();
	~ObjectHandler();

	void InitializeObjects(ID3D11Device* Dev);
	bool SetHeightMapBuffer(ID3D11DeviceContext* DevCon, int passID);
	bool SetObjectBufferWithIndex(ID3D11DeviceContext* DevCon, int i, int passID);
	
	int GetHeightMapNrOfFaces();
	int GetHeightMapNrOfVerticies();

	void CreateWorld(ID3D11Device* Dev);

	int GetNrOfMeshSubsets();

	std::vector<int> meshSubsetIndexStart;	//TODO: Turn into get function
private:
	//TODO: Turn some of these into member variables
	bool LoadObjectModel(
		ID3D11Device* Dev, 
		std::wstring filename,
		bool isRHCoordSys,
		bool computeNormals);

	bool LoadHeightMap(char* filename, HeightMapInfo &hminfo);

	void CreatePerObjectConstantBuffer(ID3D11Device* Dev);
	void CreateMaterialConstantBuffer(ID3D11Device* Dev);

	struct cPerObjectBuffer
	{
		DirectX::XMFLOAT4X4 World;
	};
	static_assert((sizeof(cPerObjectBuffer) % 16) == 0, "cPerObjectBuffer size must be 16-byte aligned");

	std::vector<materialStruct> materialVector;

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

	//Loading .obj files
	ID3D11BlendState* Transparency;
	ID3D11Buffer* meshVertBuff;
	ID3D11Buffer* meshIndexBuff;
	DirectX::XMMATRIX meshWorld;			//not used????
	int meshSubsets = 0;					//number of objects
	std::vector<int> meshSubsetTexture;
	std::vector<ID3D11ShaderResourceView*> meshSRV;
	std::vector<std::wstring> textureNameArray;

};

#endif // !OBJECTHANDLER_HPP
