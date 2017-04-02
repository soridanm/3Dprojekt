/**
* Course: DV142 - 3D-Programming
* Authors: Viktor Enfeldt, Peter Meunier
*
* File: ObjectHandler.hpp
*
* Class summary:
*	Loads, stores, and moves all objects (including the heighmap). 
*
*	NOTE: The heightmap's two textures are stored in the Engine class but apart
*	from that all textures (as well as materials) are handled by this class.
*
*	This file also includes the definitions of several structs used by the class.
*/

#ifndef OBJECTHANDLER_HPP
#define OBJECTHANDLER_HPP

#include "GlobalResources.hpp"
#include "GlobalSettings.hpp"

#include "QuadtreeHandler.hpp"	

enum ObjectType : int
{
	DYNAMIC_OBJECT,
	STATIC_OBJECT
};

struct Vertex
{
	Vertex(
		float x = 0.0f, float y = 0.0f, float z = 0.0f,
		float u = 0.0f, float v = 0.0f,
		float nx = 0.0f, float ny = 0.0f, float nz = 0.0f)
		: pos(x, y, z), texCoord(u, v), normal(nx, ny, nz)
	{}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 texCoord;
	DirectX::XMFLOAT3 normal;
};

struct cMaterialBuffer
{
	cMaterialBuffer(
		float r = 0.0f, float g = 0.0f, float b = 0.0f, 
		float specPow = 128.0f, int hasTex = 0, int texInd = 0)
		: SpecularColor(r, g, b), SpecularPower(specPow), 
		DiffuseColor(r, g, b), HasTexture(hasTex), TexArrIndex(texInd), 
		padding(0.0f, 0.0f, 0.0f)
	{}

	DirectX::XMFLOAT3 SpecularColor;
	float SpecularPower;
	DirectX::XMFLOAT3 DiffuseColor;
	int TexArrIndex; // 1 = yes, 0 = no
	int HasTexture;  // 1 = yes, 0 = no

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
	ID3D11Buffer* meshIndexBuffer  = nullptr;

	int nrOfMeshSubsets = 0;
	std::vector<int> meshSubsetIndexStart;
	std::vector<int> meshSubsetMaterialIndex;

	// World Buffer. For static objects this will be set to an identity matrix
	DirectX::XMMATRIX worldMatrixPerObject;
	cPerObjectBuffer objectBufferData = cPerObjectBuffer();
	ID3D11Buffer* perObjectWorldBuffer = nullptr;

	int GetNrOfMeshSubsets() { return nrOfMeshSubsets; }
};

// Materials will usually be taken from the .mtl file but the heightmap will use the Grass preset
namespace Materials
{
	static const cMaterialBuffer Black_plastic	= cMaterialBuffer(0.5f, 0.5f, 0.5f, 32.0f);
	static const cMaterialBuffer Black_rubber	= cMaterialBuffer(0.4f, 0.4f, 0.4f, 10.0f);
	static const cMaterialBuffer Grass			= cMaterialBuffer(0.024f, 0.05f, 0.01f, 1.5f);
}

struct HeightMapInfo
{
	int worldWidth;
	int worldHeight;
	DirectX::XMFLOAT3 *heightMap;
};


class ObjectHandler
{
public:
	ObjectHandler();
	~ObjectHandler();
	
	void InitializeObjects(ID3D11Device* Dev, ID3D11DeviceContext* DevCon);
	bool SetHeightMapBuffer(ID3D11DeviceContext* DevCon, RenderPassID passID);
	bool SetObjectBufferWithIndex(ID3D11DeviceContext* DevCon, RenderPassID passID, 
		ObjectType objectType, int objectIndex, int materialIndex);


	std::vector<Object>* GetObjectArrayPtr(ObjectType objectType);

	// Height Map
	void CreateHeightMap(ID3D11Device* Dev);
	float** getWorldHeight() const;
	int getWorldDepth() const;
	int getWorldWidth() const;
	int GetHeightMapNrOfFaces() const;
	
	// Quadtree
	void MoveStaticObjects();
	void InsertToQuadtree();
	Quadtree mQuadtree;
private:
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

	
	std::vector<Object> mStaticObjects;  // Objects that are included in the quadtree
	std::vector<Object> mDynamicObjects; // Objects that are NOT included in the quadtree

	// Material
	std::vector<materialStruct> mMaterialArray; // Stores ALL objects' materials
	std::vector<std::wstring> mMaterialFileNameArray;
	std::vector<ID3D11Buffer*> mMaterialBufferArray;

	// Texture
	std::vector<ID3D11ShaderResourceView*> mMeshTextureSRV;
	std::vector<std::wstring> mTextureNameArray;


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
