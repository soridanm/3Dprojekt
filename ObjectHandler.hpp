

#ifndef OBJECTHANDLER_HPP
#define OBJECTHANDLER_HPP

#include "GlobalResources.hpp"
#include "GlobalSettings.hpp"
#include "QuadtreeHandler.hpp"	
#include <algorithm>


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

	int nrOfMeshSubsets = 0;
	std::vector<int> meshSubsetIndexStart; // needed?
	std::vector<int> meshSubsetMaterialIndex;

	// World Buffer. For static objects this will at some point be set to an identity matrix
	cPerObjectBuffer objectBufferData = cPerObjectBuffer();
	ID3D11Buffer* perObjectWorldBuffer = nullptr;
	int GetNrOfMeshSubsets() { return nrOfMeshSubsets; }

	DirectX::XMMATRIX worldMatrixPerObject;
};

namespace Materials
{
	static const cMaterialBuffer Black_plastic	= cMaterialBuffer(0.5f, 0.5f, 0.5f, 32.0f);
	static const cMaterialBuffer Black_rubber	= cMaterialBuffer(0.4f, 0.4f, 0.4f, 10.0f);
	static const cMaterialBuffer Grass			= cMaterialBuffer(0.024f, 0.05f, 0.01f, 1.5f);
}

class ObjectHandler
{
public:
	ObjectHandler();
	~ObjectHandler();
	
	void InitializeObjects(ID3D11Device* Dev, ID3D11DeviceContext* DevCon);
	bool SetHeightMapBuffer(ID3D11DeviceContext* DevCon, RenderPassID passID);
	bool SetObjectBufferWithIndex(ID3D11DeviceContext* DevCon, RenderPassID passID, ObjectType objectType, int objectIndex, int materialIndex);
	int GetHeightMapNrOfFaces() const;

	void CreateHeightMap(ID3D11Device* Dev); //TODO: Rename

	std::vector<Object>* GetObjectArrayPtr(ObjectType objectType);

	float** getWorldHeight() const;
	int getWorldDepth() const;
	int getWorldWidth() const;
	void MoveStaticObjects();
	void InsertToQuadtree();


	Quadtree mQuadtree; //Should probably be a member of Engine.hpp instead
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

	// Material
	std::vector<materialStruct> mMaterialArray; // Stores ALL objects' materials
	std::vector<std::wstring> mMaterialFileNameArray;
	std::vector<ID3D11Buffer*> mMaterialBufferArray;

	// Texture
	std::vector<ID3D11ShaderResourceView*> mMeshTextureSRV; //not yet implemented
	std::vector<std::wstring> mTextureNameArray; // might be implemented like this or with vector<materialStruct>


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

};

#endif // !OBJECTHANDLER_HPP
