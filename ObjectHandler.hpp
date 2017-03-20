/* TODO: Might merge the perObject- and materialBuffers
*
*
*/


#ifndef OBJECTHANDLER_HPP
#define OBJECTHANDLER_HPP

#include "GlobalResources.hpp"
#include "QuadtreeHandler.hpp"	
enum ObjectType : int
{
	DYNAMIC_OBJECT,
	STATIC_OBJECT
};


//Might be moved into the class
struct cMaterialBuffer
{
	cMaterialBuffer(float r = 0.0f, float g = 0.0f, float b = 0.0f, float specPow = 128.0f, int hasTex = 0, int texInd = 0)
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

struct cPerObjectBuffer
{
	DirectX::XMFLOAT4X4 World;
};
static_assert((sizeof(cPerObjectBuffer) % 16) == 0, "cPerObjectBuffer size must be 16-byte aligned");

struct Object
{
	//From .obj
	std::vector<Vertex> meshVertexData;
	ID3D11Buffer* meshVertexBuffer = nullptr;
	ID3D11Buffer* meshIndexBuffer = nullptr;
	ID3D11BlendState* transparency = nullptr; // transparency is not implemented so this will likely be removed
	cMaterialBuffer materialBufferData = cMaterialBuffer();
	ID3D11Buffer* materialBuffer = nullptr;
	int nrOfMeshSubsets = 0;

	std::vector<int> meshSubsetIndexStart; // needed?
										   // TEXTURES ARE NOT YET IMPLEMENTED
	std::vector<int> meshSubsetTexture;
	//std::vector<ID3D11ShaderResourceView*> meshTextureSRV; //not yet implemented
	std::vector<std::wstring> textureNameArray; // might be implemented like this or with vector<materialStruct>

												// World Buffer. For static objects this will at some point be set to an identity matrix
	cPerObjectBuffer objectBufferData = cPerObjectBuffer();
	ID3D11Buffer* perObjectWorldBuffer = nullptr;
	int GetNrOfMeshSubsets() { return nrOfMeshSubsets; }

	DirectX::XMMATRIX worldMatrixPerObject;

};

namespace Materials
{
	static const cMaterialBuffer Black_plastic	= cMaterialBuffer(/*L"Black plastic",*/	0.5f, 0.5f, 0.5f, 32.0f);
	static const cMaterialBuffer Black_rubber	= cMaterialBuffer(/*L"Black rubber",*/	0.4f, 0.4f, 0.4f, 10.0f);
	static const cMaterialBuffer Grass = cMaterialBuffer(/*L"Grass",*/	0.024f, 0.05f, 0.01f, 1.5f);
}

class ObjectHandler
{
public:
	
	ObjectHandler();
	~ObjectHandler();
	void InitializeObjects(ID3D11Device* Dev, ID3D11DeviceContext* DevCon);
	bool SetHeightMapBuffer(ID3D11DeviceContext* DevCon, RenderPassID passID);
	bool SetObjectBufferWithIndex(ID3D11DeviceContext* DevCon, RenderPassID passID, ObjectType objectType, int objectIndex, int materialIndex);
	const int GetHeightMapNrOfFaces();
	int GetHeightMapNrOfVerticies();

	void CreateWorld(ID3D11Device* Dev);

	std::vector<Object>* GetObjectArrayPtr(ObjectType objectType);

	float** getWorldHeight();
	int getWorldDepth();
	int getWorldWidth();
	//int GetNrOfMeshSubsets();
	void moveObjects();
	void insertToQuadtree();
	//std::vector<int> meshSubsetIndexStart;	//TODO: Turn into get function

	bool SetQuadtreeBuffer(ID3D11DeviceContext* DevCon, RenderPassID passID);
	cPerObjectBuffer quadtreeWorldBufferData = cPerObjectBuffer();

	Quadtree mQuadtree; //Should probably be a member of GraphicsHandler.hpp instead
private:
	//TODO: Turn some of these into member variables
	bool LoadObjectModel(
		ID3D11Device* Dev, 
		ID3D11DeviceContext* DevCon, 
		std::wstring filename,
		ObjectType objectType,
		bool isRHCoordSys,
		bool computeNormals);

	bool LoadHeightMap(char* filename, HeightMapInfo &hminfo);

	void CreatePerObjectConstantBuffers(ID3D11Device* Dev);
	void CreateMaterialConstantBuffers(ID3D11Device* Dev);

	
	std::vector<Object> mStaticObjects;  // objects that will be included in quad-tree
	std::vector<Object> mDynamicObjects; // Objects that will NOT be included int quad-tree

	std::vector<materialStruct> mMaterialVector; // Stores ALL objects' materials
	std::vector<ID3D11ShaderResourceView*> mMeshTextureSRV; //not yet implemented


	cPerObjectBuffer mHeightMapWorldBufferData = cPerObjectBuffer();
	ID3D11Buffer* mHeightMapMaterialBuffer = nullptr;
	cMaterialBuffer mHeightMapMaterialBufferData = cMaterialBuffer();

	//Height map
	ID3D11Buffer* gSquareIndexBuffer = nullptr;
	ID3D11Buffer* gSquareVertBuffer = nullptr;
	ID3D11ShaderResourceView* mTextureView = nullptr;
	ID3D11Buffer* mHeightMapWorldBuffer = nullptr;
	int NUMBER_OF_FACES = 0;
	int NUMBER_OF_VERTICES = 0;
	float** WORLD_HEIGHT;
	int WORLD_WIDTH = 0, WORLD_DEPTH = 0;

	//Loading .obj files
	//ID3D11BlendState* Transparency;
	//ID3D11Buffer* meshVertBuff;
	//ID3D11Buffer* meshIndexBuff;
	DirectX::XMMATRIX meshWorld; //not used????
	//int meshSubsets = 0;					//number of subsets
	//std::vector<int> meshSubsetTexture;
	//std::vector<ID3D11ShaderResourceView*> meshSRV;
	//std::vector<std::wstring> textureNameArray;

};

#endif // !OBJECTHANDLER_HPP
