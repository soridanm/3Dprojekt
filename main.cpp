//--------------------------------------------------------------------------------------
// TODO: 
//  rewrite material functions with more values
//	viewprojection as single matrix instead of two seperate ones
// TODO?:
//	Turn global constants into getFunctions()
//--------------------------------------------------------------------------------------
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "bth_image.h"
#include <Windows.h>

// for reading obj
#include <string> //might not be necessary
#include <vector>
#include <fstream>
#include <istream>
#include <sstream> //looks like this is needed?

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

using namespace DirectX;

const double MATH_PI			= 3.14159265358;
const UINT GBUFFER_COUNT		= 4;
const LONG SCREEN_WIDTH			= 2*640;
const LONG SCREEN_HEIGHT		= 2*480;
const int MAX_LIGHTS			= 8;
const int NR_OF_OBJECTS			= 1;
const XMVECTOR CAMERA_STARTING_POS	= XMVectorSet(0.0f, 1.0f, 2.0f, 1.0f);
float CUBE_ROTATION_SPEED		= 0.01f;
float LIGHT_ROTATION_SPEED		= 0.001f;

HWND InitWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HRESULT CreateDirect3DContext(HWND wndHandle);
HRESULT gHR = 0;

IDXGISwapChain* gSwapChain						= nullptr;
ID3D11Device* gDevice							= nullptr;
ID3D11DeviceContext* gDeviceContext				= nullptr;

// Depth Buffer
ID3D11DepthStencilView* gDepthStecilView		= nullptr;
ID3D11Texture2D* gDepthStencilTexture			= nullptr;

// First Pass
ID3D11Buffer* gVertexBuffer						= nullptr;
ID3D11InputLayout* gVertexLayout				= nullptr;
ID3D11VertexShader* gVertexShader				= nullptr;
ID3D11GeometryShader* gGeometryShader			= nullptr;
ID3D11PixelShader* gPixelShader					= nullptr;

ID3D11ShaderResourceView* gTextureView			= nullptr;

// Last Pass
ID3D11VertexShader* gFullScreenTriangleShader	= nullptr;
ID3D11PixelShader* gLightPixelShader			= nullptr;

ID3D11RenderTargetView* gBackbufferRTV			= nullptr;

/*--------------------------------------------------------------------------------------
*			G-Buffer
--------------------------------------------------------------------------------------*/

struct gGraphicsBufferStruct {
	ID3D11Texture2D* texture						= nullptr;
	ID3D11RenderTargetView* renderTargetView		= nullptr;
	ID3D11ShaderResourceView* shaderResourceView	= nullptr;
}; gGraphicsBufferStruct gGraphicsBuffer[GBUFFER_COUNT];



/*--------------------------------------------------------------------------------------
*			Constant Buffers
--------------------------------------------------------------------------------------*/
ID3D11Buffer* gPerFrameBuffer	= nullptr;
ID3D11Buffer* gPerObjectBuffer	= nullptr;

struct cPerFrameBuffer
{
	//XMFLOAT4X4 ViewProjection;
	XMFLOAT4X4 View;
	XMFLOAT4X4 Projection;
}; cPerFrameBuffer VPBufferData;

struct cPerObjectBuffer
{
	XMFLOAT4X4 World;
}; cPerObjectBuffer ObjectBufferData;

//------------------ Material (GBufferFragment.hlsl) -----------------------------------
ID3D11Buffer* gMaterialBuffer = nullptr;

struct materialStruct
{
	/*materialStruct(float r = 0.0f, float b = 0.0f, float g = 0.0f, float specPow = 128.0f) 
		: specularAlbedo(r, g, b), specularPower(specPow) 
	{}*/
	XMFLOAT4 diffuseColor;
	std::wstring matName; 
	int texArrayIndex;
	bool hasTexture;
	float specularPower;
};

struct cMaterialBuffer
{
	cMaterialBuffer(materialStruct mat = materialStruct()) : material(mat) 
	{}
	materialStruct material;
}; cMaterialBuffer gMaterialBufferData;

//------------------ Lights (LightFragment.hlsl) ---------------------------------------
ID3D11Buffer* gLightBuffer = nullptr;

// TODO: one default constructor instead of two
struct Light
{
	Light(XMFLOAT4 pos	= XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), 
		XMFLOAT4 col	= XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		float c_att		= 1.0f, 
		float l_att		= 0.0f, 
		float q_att		= 0.0f, 
		float amb		= 0.0f)
		: PositionWS(pos), 
		Color(col),
		constantAttenuation(c_att), 
		linearAttenuation(l_att), 
		quadraticAttenuation(q_att),
		ambientCoefficient(amb) 
	{}
	XMFLOAT4 PositionWS;
	XMFLOAT4 Color;
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
	float ambientCoefficient;
};

struct cLightBuffer 
{
	cLightBuffer() 
		: cameraPositionWS	(0.0f, 0.0f, 0.0f, 1.0f),
		globalAmbient		(0.2f, 0.2f, 0.2f, 1.0f)
	{}
	XMFLOAT4 cameraPositionWS;
	XMFLOAT4 globalAmbient;
	Light Lights[MAX_LIGHTS];
}; cLightBuffer gLightBufferData;

static_assert((sizeof(cPerFrameBuffer) % 16)	== 0, "Constant Buffer size must be 16-byte aligned");
static_assert((sizeof(cPerObjectBuffer) % 16)	== 0, "Constant Buffer size must be 16-byte aligned");
static_assert((sizeof(cMaterialBuffer) % 16)	== 0, "Constant Buffer size must be 16-byte aligned");
static_assert((sizeof(cLightBuffer) % 16)		== 0, "Constant Buffer size must be 16-byte aligned");

/*--------------------------------------------------------------------------------------
*				Namespaces
--------------------------------------------------------------------------------------*/

namespace Colors
{
	static const XMFLOAT4 White				= { 1.0f, 1.0f, 1.0f, 1.0f };
	static const XMFLOAT4 Black				= { 0.0f, 0.0f, 0.0f, 1.0f };
	static const XMFLOAT4 LightSteelBlue	= { 0.69f, 0.77f, 0.87f, 1.0f };
	static const XMFLOAT4 Red				= { 1.0f, 0.0f, 0.0f, 1.0f };
	static const XMFLOAT4 Green				= { 0.0f, 1.0f, 0.0f, 1.0f };
	static const XMFLOAT4 Blue				= { 0.0f, 0.0f, 1.0f, 1.0f };
	static const float fWhite[4]			= { 1.0f, 1.0f, 1.0f, 1.0f };
	static const float fBlack[4]			= { 0.0f, 0.0f, 0.0f, 1.0f };
	static const float fLightSteelBlue[4]	= { 0.69f, 0.77f, 0.87f, 1.0f };
}

namespace Materials
{
	static const materialStruct Black_plastic	= materialStruct(0.5f, 0.5f, 0.5f, 32.0f);
	static const materialStruct Black_rubber	= materialStruct(0.4f, 0.4f, 0.4f, 10.0f);
}

/*--------------------------------------------------------------------------------------
*			Functions
--------------------------------------------------------------------------------------*/

//--------------------- Create Constant Buffers ----------------------------------------
void CreatePerFrameConstantBuffer()
{
	float aspect_ratio			= (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
	float degrees_field_of_view	= 90.0f;
	float near_plane			= 0.1f;
	float far_plane				= 20.f;

	//camera, look at, up
	XMVECTOR camera		= CAMERA_STARTING_POS;
	XMVECTOR look_at	= XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR up			= XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixTranspose(XMMatrixLookAtLH(camera, look_at, up));

	XMMATRIX projection = XMMatrixTranspose(XMMatrixPerspectiveFovLH(
		XMConvertToRadians(degrees_field_of_view), aspect_ratio, near_plane, far_plane));

	XMStoreFloat4x4(&VPBufferData.Projection, projection);
	XMStoreFloat4x4(&VPBufferData.View, view);

	D3D11_BUFFER_DESC VPBufferDesc;
	VPBufferDesc.Usage					= D3D11_USAGE_DYNAMIC;
	VPBufferDesc.ByteWidth				= sizeof(cPerFrameBuffer);
	VPBufferDesc.BindFlags				= D3D11_BIND_CONSTANT_BUFFER;
	VPBufferDesc.CPUAccessFlags			= D3D11_CPU_ACCESS_WRITE;
	VPBufferDesc.MiscFlags				= 0;
	VPBufferDesc.StructureByteStride	= 0;

	gHR = gDevice->CreateBuffer(&VPBufferDesc, nullptr, &gPerFrameBuffer);
	if (FAILED(gHR)) {
		exit(-1);
	}
}

void CreatePerObjectConstantBuffer()
{
	XMMATRIX world = XMMatrixTranspose(XMMatrixRotationY(0.0f));
	
	XMStoreFloat4x4(&ObjectBufferData.World, world);

	D3D11_BUFFER_DESC WBufferDesc;
	WBufferDesc.Usage				= D3D11_USAGE_DYNAMIC;
	WBufferDesc.ByteWidth			= sizeof(cPerObjectBuffer);
	WBufferDesc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	WBufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	WBufferDesc.MiscFlags			= 0;
	WBufferDesc.StructureByteStride	= 0;

	gHR = gDevice->CreateBuffer(&WBufferDesc, nullptr, &gPerObjectBuffer);
	if (FAILED(gHR)) {
		exit(-1);
	}
}

void CreateMaterialConstantBuffer() 
{
	D3D11_BUFFER_DESC materialBufferDesc;
	materialBufferDesc.Usage				= D3D11_USAGE_DYNAMIC;
	materialBufferDesc.ByteWidth			= sizeof(cMaterialBuffer);
	materialBufferDesc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	materialBufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	materialBufferDesc.MiscFlags			= 0;
	materialBufferDesc.StructureByteStride	= 0;

	// check if the creation failed for any reason
	HRESULT hr = 0;
	hr = gDevice->CreateBuffer(&materialBufferDesc, nullptr, &gMaterialBuffer);
	if (FAILED(hr))
	{
		// handle the error, could be fatal or a warning...
		exit(-1);
	}
}

// TODO: change MAX_LIGHTS to NR_OF_LIGHTS or add bool Enable to light struct
void CreateLightConstantBuffer() 
{
	// initialize the description of the buffer.
	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage				= D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth			= sizeof(cLightBuffer);
	lightBufferDesc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags			= 0;
	lightBufferDesc.StructureByteStride	= 0;

	// check if the creation failed for any reason
	HRESULT hr = 0;
	hr = gDevice->CreateBuffer(&lightBufferDesc, nullptr, &gLightBuffer);
	if (FAILED(hr))
	{
		// handle the error, could be fatal or a warning...
		exit(-1);
	}
}

//---------------------- Set Constant Buffers ------------------------------------------

// might need a rewrite? Not sure if this is the best way to do it
// storing them in an array could be a good idea
void SetMaterial(materialStruct matprop)
{
	gMaterialBufferData.material = matprop;
}

// Currently only creates one static light.
void setLights()
{
	XMFLOAT4 light_position = { 2.0f, 0.0f, 4.0f, 1.0f };
	XMFLOAT4 light_color	= Colors::White;
	float c_att	= 0.2f;
	float l_att	= 0.5f;
	float q_att	= 0.009f;
	float amb	= 0.01f;
	Light test_light(light_position, light_color, c_att, l_att, q_att, amb);

	gLightBufferData.Lights[0] = test_light;
	XMStoreFloat4(&gLightBufferData.cameraPositionWS, CAMERA_STARTING_POS);
	gLightBufferData.globalAmbient = { 0.04f, 0.04f, 0.04f, 1.0f };
}

void CreateShaders()
{
//---------------------------------- First Pass ----------------------------------------------------
	//create vertex shader
	ID3DBlob* pVS = nullptr;
	D3DCompileFromFile(
		L"GBufferVertex.hlsl", // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"VS_main",		// entry point
		"vs_5_0",		// shader model (target)
		0,				// shader compile options			// here DEBUGGING OPTIONS
		0,				// effect compile options
		&pVS,			// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
						// how to use the Error blob, see here
						// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
	);
	gHR = gDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &gVertexShader);
	if (FAILED(gHR)) {
		exit(-1);
	}

	//create input layout (verified using vertex shader)
	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	gDevice->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), &gVertexLayout);
	// we do not need anymore this COM object, so we release it.
	pVS->Release();

	//create geometry shader
	ID3DBlob* pGS = nullptr;
	D3DCompileFromFile(
		L"GBufferGeometry.hlsl",
		nullptr,
		nullptr,
		"GS_main",
		"gs_5_0",
		0,
		0,
		&pGS,
		nullptr
	);
	gHR = gDevice->CreateGeometryShader(pGS->GetBufferPointer(), pGS->GetBufferSize(), nullptr, &gGeometryShader);
	if (FAILED(gHR)) {
		exit(-1);
	}
	pGS->Release();

	//create pixel shader
	ID3DBlob* pPS = nullptr;
	D3DCompileFromFile(
		L"GBufferFragment.hlsl", // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"PS_main",		// entry point
		"ps_5_0",		// shader model (target)
		0,				// shader compile options
		0,				// effect compile options
		&pPS,			// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
						// how to use the Error blob, see here
						// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
	);
	gHR = gDevice->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &gPixelShader);
	if (FAILED(gHR)) {
		exit(-1);
	}
	// we do not need anymore this COM object, so we release it.
	pPS->Release();

//---------------------------------- Last Pass ----------------------------------------------------

	//create vertex shader
	ID3DBlob* pVS2 = nullptr;
	D3DCompileFromFile(
		L"LightVertex.hlsl", // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"VS_main",		// entry point
		"vs_5_0",		// shader model (target)
		0,				// shader compile options			// here DEBUGGING OPTIONS
		0,				// effect compile options
		&pVS2,			// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
						// how to use the Error blob, see here
						// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
	);
	gHR = gDevice->CreateVertexShader(pVS2->GetBufferPointer(), pVS2->GetBufferSize(), nullptr, &gFullScreenTriangleShader);
	if (FAILED(gHR)) {
		exit(-1);
	}
	// we do not need anymore this COM object, so we release it.
	pVS2->Release();

	//create pixel shader
	ID3DBlob* pPS2 = nullptr;
	D3DCompileFromFile(
		L"LightFragment.hlsl", // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"PS_main",		// entry point
		"ps_5_0",		// shader model (target)
		0,				// shader compile options
		0,				// effect compile options
		&pPS2,			// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
						// how to use the Error blob, see here
						// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
	);
	gHR = gDevice->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &gLightPixelShader);
	if (FAILED(gHR)) {
		exit(-1);
	}
	// we do not need anymore this COM object, so we release it.
	pPS2->Release();
}

void CreateTriangleData()
{
	struct TriangleVertex
	{
		float x, y, z;
		float u, v;
	};

	TriangleVertex triangleVertices[36] =
	{
		//front
				//lower left corner
			-0.5f, -0.5f, -0.5f,	//v0 pos
			0.0f, 1.0f,	//v0 texcoord
		//	1.0f,0.0f,0.0f,//color

			//upper left corner
			-0.5f, 0.5f, -0.5f,	//v1
			0.0f, 0.0f, 	//v1 texcoord
		//	0.0f,1.0f,0.0f,//color

			//lower right corner
			0.5f, -0.5f, -0.5f, //v2
			1.0f, 1.0f,	//v2 texcoord
		//	0.0f,0.0f,1.0f,//color

			//upper right corner
			0.5f,0.5f,-0.5f,//v3
			1.0f,0.0f,//v3 texcoord
		 // 1.0f,0.0f,1.0f,//color


		 //lower right corner
		 0.5f, -0.5f, -0.5f, //v2
		 1.0f, 1.0f,	//v2 texcoord

		 //upper left corner
		 -0.5f, 0.5f, -0.5f,	//v1
		 0.0f, 0.0f, 	//v1 texcoord


		 //left side

		//lower left corner
		-0.5f, -0.5f, 0.5f,	//v0 pos
		0.0f, 1.0f,	//v0 texcoord

		//upper left corner
		-0.5f, 0.5f, 0.5f,	//v1
		0.0f, 0.0f, 	//v1 texcoord

		//lower right corner
		-0.5f, -0.5f, -0.5f, //v2
		1.0f, 1.0f,	//v2 texcoord

		//upper right corner
		-0.5f,0.5f,-0.5f,//v3
		1.0f,0.0f,//v3 texcoord

		//lower right corner
		-0.5f, -0.5f, -0.5f, //v2
		1.0f, 1.0f,	//v2 texcoord

		//upper left corner
		-0.5f, 0.5f, 0.5f,	//v1
		0.0f, 0.0f, //v1 texcoord

		 //right side

		//lower left corner
		0.5f, -0.5f, -0.5f,
		0.0f, 1.0f,

		//upper left corner
		0.5f, 0.5f, -0.5f,
		0.0f, 0.0f,

		//lower right corner
		0.5f, -0.5f, 0.5f,
		1.0f, 1.0f,

		//upper right corner
		0.5f,0.5f,0.5f,
		1.0f,0.0f,

		//lower right corner
		0.5f, -0.5f, 0.5f,
		1.0f, 1.0f,

		//upper left corner
		0.5f, 0.5f, -0.5f,
		0.0f, 0.0f,

		//top

		//lower left corner
		-0.5f, 0.5f, -0.5f,
		0.0f, 1.0f,

		//upper left corner
		-0.5f, 0.5f, 0.5f,
		0.0f, 0.0f,

		//lower right corner
		0.5f, 0.5f, -0.5f,
		1.0f, 1.0f,

		//upper right corner
		0.5f,0.5f,0.5f,
		1.0f,0.0f,

		//lower right corner
		0.5f,0.5f, -0.5f,
		1.0f, 1.0f,

		//upper left corner
		-0.5f, 0.5f, 0.5f,
		0.0f, 0.0f,


			//bottom

			//lower left corner
			-0.5f, -0.5f, 0.5f,
			0.0f, 1.0f,

			//upper left corner
			-0.5f, -0.5f, -0.5f,
			0.0f, 0.0f,

			//lower right corner
			0.5f, -0.5f, 0.5f,
			1.0f, 1.0f,

			//upper right corner
			0.5f, -0.5f, -0.5f,
			1.0f, 0.0f,

			//lower right corner
			0.5f, -0.5f, 0.5f,
			1.0f, 1.0f,

			//upper left corner
			-0.5f, -0.5f, -0.5f,
			0.0f, 0.0f,

			//back

			//lower left corner
			0.5f, -0.5f, 0.5f,
			0.0f, 1.0f,

			//upper left corner
			0.5f, 0.5f, 0.5f,
			0.0f, 0.0f,

			//lower right corner
			-0.5f, -0.5f, 0.5f,
			1.0f, 1.0f,

			//upper right corner
			-0.5f, 0.5f, 0.5f,
			1.0f, 0.0f,

			//lower right corner
			-0.5f, -0.5f, 0.5f,
			1.0f, 1.0f,

			//upper left corner
			0.5f, 0.5f, 0.5f,
			0.0f, 0.0f,


	};

	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.BindFlags	= D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage		= D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth	= sizeof(triangleVertices);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = triangleVertices;
	gDevice->CreateBuffer(&bufferDesc, &data, &gVertexBuffer);
}

//---------------------- Load objects ------------------------------------------


ID3D11BlendState* Transparency;

ID3D11Buffer* meshVertBuff;
ID3D11Buffer* meshIndexBuff;
XMMATRIX meshWorld;
int meshSubsets = 0;
std::vector<int> meshSubsetIndexStart;
std::vector<int> meshSubsetTexture;

std::vector<ID3D11ShaderResourceView*> meshSRV;
std::vector<std::wstring> textureNameArray;

// TODO: write description of all the things this function does
// TODO? use std::getline instead
// Description (not complete): if no vertex normals are included in the .obj file then they will be computed
bool LoadObjectModel(std::wstring filename,
	ID3D11Buffer** vertBuff,
	ID3D11Buffer** indexBuff,
	std::vector<int>& subsetIndexStart,
	std::vector<int>& sunsetMaterialArray,
	std::vector<materialStruct>& material,
	int& subsetCount,
	bool isRHCoordSys,		//needed?
	bool computeNormals)	//needed?
{
	// Presentation version
	/*std::string myFile("myFile.obj"), special;
	std::string line2;
	std::ifstream file(myFile);
	std::istringstream inputString;
	struct VertexPos { float x, y, z; };
	std::vector<VertexPos> verticies;
	VertexPos vtx;
	while (std::getline(file, line2))
	{
		inputString.str(line2);
		if (line2.substr(0, 2) == "v ")
		{
			inputString >> special >> vtx.x >> vtx.y >> vtx.z;
			verticies.push_back(vtx);
		}
	}
	file.close();*/
	// End Presentation version

	std::wifstream fileIn(filename.c_str());    //Open file
	std::wstring meshMatLib;                    //String to hold our obj material library filename

		//Arrays to store our model's information
	std::vector<DWORD> indices;
	std::vector<XMFLOAT3> vertPos;
	std::vector<XMFLOAT3> vertNorm;
	std::vector<XMFLOAT2> vertTexCoord;
	std::vector<std::wstring> meshMaterials;

	//Vertex definition indices
	std::vector<int> vertPosIndex;
	std::vector<int> vertNormIndex;
	std::vector<int> vertTCIndex;

	//Make sure we have a default if no tex coords or normals are defined
	bool hasTexCoord = false;
	bool hasNorm = false;

	//Temp variables to store into vectors
	std::wstring meshMaterialsTemp;
	int vertPosIndexTemp;
	int vertNormIndexTemp;
	int vertTCIndexTemp;

	wchar_t checkChar;        //The variable we will use to store one char from file at a time
	std::wstring face;        //Holds the string containing our face vertices
	int vIndex = 0;           //Keep track of our vertex index count
	int triangleCount = 0;    //Total Triangles
	int totalVerts = 0;
	int meshTriangles = 0;

	//TODO: improve with an error message or something
	if (!fileIn)	//Early exit if the file doesn't open
		exit(-1);
	
	while (fileIn)
	{
		checkChar = fileIn.get();			//get the next char

		switch (checkChar)
		{
		case '#':							//Comment line
			while (checkChar != '\n')		//Keep reading chars until the line ends
				checkChar = fileIn.get();
			break;
		case 'v': // Get vertex descriptions
			checkChar = fileIn.get();
			if (checkChar == ' ')			//v - vertex position
			{ 
				float vx, vy, vz;
				fileIn >> vx >> vy >> vz;	//store the vertex positions
				vertPos.push_back(XMFLOAT3(vx, vy, (isRHCoordSys) ? vz * -1.0f : vz));		//invert Z-axiz if the OBJ is RH
			}
			if (checkChar == 't')			//vt - vertex texture coordinates
			{
				float vu, vv;
				fileIn >> vu >> vv;			//store the uv-coordinates
				vertTexCoord.push_back(XMFLOAT2(vu, (isRHCoordSys) ? 1.0f - vv : vv));		//Reverse the v-axis if the OBJ is RH
				hasTexCoord = true;
			}
			if (checkChar == 'n')			//vn - vertex normal
			{
				float vnx, vny, vnz;
				fileIn >> vnx >> vny >> vnz; //store the normal values
				vertNorm.push_back(XMFLOAT3(vnx, vny, (isRHCoordSys) ? vnz * -1.0f : vnz)); //Invert the z-axiz if the OBJ is RH
				hasNorm = true;
			}
			break;
		case 'g':							//g - group
			checkChar = fileIn.get();
			if (checkChar == ' ')
			{
				subsetIndexStart.push_back(vIndex);
				subsetCount++;
			}
			break;

		/* Breaks faces with more than three sides into multiple triangles */
		case 'f':							//f - faces. Vertex definition [vPos1/vTexCoord1/vNorm1 ... ]
			checkChar = fileIn.get();
			if (checkChar == ' ')
			{
				face = L"";				//clear the temporary wide string which will be used to store the line
				std::wstring VertDef;	//holds one vertex definition at a time
				triangleCount = 0;

				checkChar = fileIn.get();
				while (checkChar != '\n')	//read through the whole line
				{
					face += checkChar;		//add every char to the face string
					checkChar = fileIn.get();
					if (checkChar == ' ')
						triangleCount++;	//increase triangle count if it's a space
				}

				//check for space at the end of our face string
				if (face[face.length() - 1] == ' ')
					triangleCount--;
				triangleCount -= 1; //every vertex in the face after the first two are new faces

				std::wstringstream ss(face);
				if (face.length() > 0) // if the line isn't just simply 'f '
				{
					int firstVIndex, lastVIndex; // first and last vertice's index

					for (int i = 0; i < 3; i++) //First three vertices
					{
						ss >> VertDef; //get one vertex definition [vPos/vTexCoord/vNorm]
						std::wstring vertPart;
						int whichPart = 0; //vPos = 0, vTexCoord = 1, vNorm = 2
						//Parse the string
						for (int j = 0; j < VertDef.length(); j++)
						{
							if (VertDef[j] != '/')		//if there's no divider add char to vertPart
								vertPart += VertDef[j];
							//If the current char is a divider or the last character in the string
							if (VertDef[j] == '/' || j == VertDef.length() - 1)
							{
								std::wistringstream wstringToInt(vertPart); //used to convert wstring to int

								if (whichPart == 0) //if vPos
								{
									wstringToInt >> vertPosIndexTemp;
									vertPosIndexTemp -= 1; //subtract one since c++ starts arrays at 0 and .obj starts indexing at 1

									//check if the vert pos was the only thing specified
									if (j == VertDef.length() - 1)
									{
										vertNormIndexTemp = 0;
										vertTCIndexTemp = 0;
									}
								} //end if vPos
								else if (whichPart == 1) //if vTexCoord
								{
									if (vertPart != L"") //check if there is a tex coord
									{
										wstringToInt >> vertTCIndexTemp;
										vertTCIndexTemp -= 1; //subtract one since c++ starts arrays at 0 and .obj starts indexing at 1
									}
									else //if there is no tex coord 0 will be default index
										vertTCIndexTemp = 0;

									//if the current char is the second to last in the string then there must be no normal so a default normal is set to index 0
									if (j == VertDef.length() - 1)
										vertNormIndexTemp = 0;

								} //end if vTexCoord
								else if (whichPart == 2) //if vNorm
								{
									//std::wistringstream wstringToInt(vertPart);
									wstringToInt >> vertNormIndexTemp;
									vertNormIndexTemp -= 1; //subtract one since c++ starts arrays at 0 and .obj starts indexing at 1
								} //end if vNorm

								vertPart = L"";
								whichPart++;
							} //end if current char is a divider or the last char in the string
						} //end for j-loop
						
						  //TODO: storing the face
						//check to make sure there's at least one subset
						if (subsetCount == 0)
						{
							subsetIndexStart.push_back(vIndex); //start index for this subset
							subsetCount++;
						}
						//avoid duplicate vertices
						bool vertAlreadyExists = false;
						if (totalVerts >= 3) //make sure there's at least one triangle to check
						{
							//loop through all the verticies
							for (int iCheck = 0; iCheck < totalVerts; iCheck)
							{
								//if the vertex position and tex coord in memory are the same as the ones currently being read then 
								// this faces vertex index is set to the vertex's value in memory. 
								if (vertPosIndexTemp == vertPosIndex[iCheck] && !vertAlreadyExists)
								{
									if (vertTCIndexTemp == vertTCIndex[iCheck])
									{
										indices.push_back(iCheck);	//set the index for this vertex
										vertAlreadyExists = true;
									}
								}
							} //end loop through verticies
						} //end duplicate test

						if (!vertAlreadyExists)
						{
							vertPosIndex.push_back(vertPosIndexTemp);
							vertTCIndex.push_back(vertTCIndexTemp);
							vertNormIndex.push_back(vertNormIndexTemp);
							totalVerts++;						//new vertex created
							indices.push_back(totalVerts - 1);	//set index for this vertex
						}

						//make sure the rest of the triangles use the first vertex
						if (i == 0)
						{
							firstVIndex = indices[vIndex];		//the first vertex index of this face
						}

						//if this was the last vertex in the first triangle the next triangle will use it as well
						if (i == 2)
						{
							lastVIndex = indices[vIndex];		//the last vertex index of this triangle
						}
						vIndex++; //increment index count
					} //end for i-loop
					meshTriangles++; //one triangle down

					//convert the face to more than one triangle in case the face has more than three vertexes
					for (int l = 0; l < triangleCount - 1; ++l)
					{
						//first vertex of this triangle (the very first vertex of the face too)
						indices.push_back(firstVIndex); //set index for this vertex
						vIndex++;

						//second vertex of this triangle (the last vertex used in the tri before this one)
						indices.push_back(lastVIndex); //set index for this vertex
						vIndex++;

						//get the third vertex for this triangle
						ss >> VertDef;

						std::wstring vertPart;
						int whichPart = 0;

						//TODO: see if this duplicate code can be eliminated
						//parse this string (same procedure as in the for i-loop)
						for (int j = 0; j < VertDef.length(); j++)
						{
							if (VertDef[j] != '/')		//if there's no divider add char to vertPart
								vertPart += VertDef[j];
							//If the current char is a divider or the last character in the string
							if (VertDef[j] == '/' || j == VertDef.length() - 1)
							{
								std::wistringstream wstringToInt(vertPart); //used to convert wstring to int

								if (whichPart == 0) //if vPos
								{
									wstringToInt >> vertPosIndexTemp;
									vertPosIndexTemp -= 1; //subtract one since c++ starts arrays at 0 and .obj starts indexing at 1

														   //check if the vert pos was the only thing specified
									if (j == VertDef.length() - 1)
									{
										vertNormIndexTemp = 0;
										vertTCIndexTemp = 0;
									}
								} //end if vPos
								else if (whichPart == 1) //if vTexCoord
								{
									if (vertPart != L"") //check if there is a tex coord
									{
										wstringToInt >> vertTCIndexTemp;
										vertTCIndexTemp -= 1; //subtract one since c++ starts arrays at 0 and .obj starts indexing at 1
									}
									else //if there is no tex coord 0 will be default index
										vertTCIndexTemp = 0;

									//if the current char is the second to last in the string then there must be no normal so a default normal is set to index 0
									if (j == VertDef.length() - 1)
										vertNormIndexTemp = 0;

								} //end if vTexCoord
								else if (whichPart == 2) //if vNorm
								{
									//std::wistringstream wstringToInt(vertPart);
									wstringToInt >> vertNormIndexTemp;
									vertNormIndexTemp -= 1; //subtract one since c++ starts arrays at 0 and .obj starts indexing at 1
								} //end if vNorm

								vertPart = L"";
								whichPart++;
							} //end if current char is a divider or the last char in the string
						} //end for j-loop

						//check for duplicate vertices
						bool vertAlreadyExists = false;
						if (totalVerts >= 3) //make sure there's at least one triangle to check
						{
							//loop through all the verticies
							for (int iCheck = 0; iCheck < totalVerts; iCheck)
							{
								//if the vertex position and tex coord in memory are the same as the ones currently being read then 
								// this faces vertex index is set to the vertex's value in memory. 
								if (vertPosIndexTemp == vertPosIndex[iCheck] && !vertAlreadyExists)
								{
									if (vertTCIndexTemp == vertTCIndex[iCheck])
									{
										indices.push_back(iCheck);	//set the index for this vertex
										vertAlreadyExists = true;
									}
								}
							} //end loop through verticies
						} //end duplicate test

						if (!vertAlreadyExists)
						{
							vertPosIndex.push_back(vertPosIndexTemp);
							vertTCIndex.push_back(vertTCIndexTemp);
							vertNormIndex.push_back(vertNormIndexTemp);
							totalVerts++;						//new vertex created
							indices.push_back(totalVerts - 1);	//set index for this vertex
						}

						//set the second vertex for the next triangle to the last vertex
						lastVIndex = indices[vIndex]; //the last vertex index of this triangle

						meshTriangles++; //new triangle defined
						vIndex++;

					} //end for l-loop

				} //end if the line isn't empty
			}
			break;
		case 'm':							//mtllib - material library filename
			for (int i = 0; i < 6; i++) {	//loop over 'tllib '
				checkChar = fileIn.get();
			}
			fileIn >> meshMatLib;			//store the material library file name 
			break;
		case 'u':							//usemtl - which material to use
			for (int i = 0; i < 6; i++) {	//loop over 'semtl '
				checkChar = fileIn.get();
			}
			meshMaterialsTemp = L"";					//Clear the temporary wide string
			fileIn >> meshMaterialsTemp;				//store the material name as a wide string
			meshMaterials.push_back(meshMaterialsTemp); //store it in a vectore of wide strings
			break;
		case 's':

		default:
			break;
		}
	}

	subsetIndexStart.push_back(vIndex); //set index start

	//make sure the first subset does not contain "0" indices. Can happen if "g" is defined at the very top of the file
	if (subsetIndexStart[1] == 0)
	{
		subsetIndexStart.erase(subsetIndexStart.begin() + 1);
		meshSubsets--;
	}

	//set default tex coords and norms if those are not specified in the file
	if (!hasNorm)
		vertNorm.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	if (!hasTexCoord)
		vertTexCoord.push_back(XMFLOAT2(0.0f, 0.0f));

	//close the obj file and open the mtl file (if it exists)
	fileIn.close();
	fileIn.open(meshMatLib.c_str());

	std::wstring lastStringRead;
	int matCount = material.size(); //total materials

	bool kdset = false; //if diffuse was not set, use ambient. if diffuse WAS set, no need to set diffuse to amb.

	if (!fileIn) //early exit if the material fiel doesn't open
		exit(-1);
	while (fileIn)
	{
		checkChar = fileIn.get();

		switch (checkChar)
		{
		case '#': //comment
			while (checkChar != '\n')
				checkChar = fileIn.get();
			break;
		case 'K': //Diffuse color
			checkChar = fileIn.get();
			if (checkChar == 'd')
			{
				checkChar = fileIn.get(); //read over space
				
			}
			break;
		}
	}

}



void SetViewport()
{
	D3D11_VIEWPORT vp;
	vp.Width	= (FLOAT)SCREEN_WIDTH;
	vp.Height	= (FLOAT)SCREEN_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.f;
	vp.TopLeftY = 0.f;
	gDeviceContext->RSSetViewports(1, &vp);
}

// TODO: Should probably add hr checks. also Relese()
void initGraphicsBuffer()
{
	// Create render target textures
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = SCREEN_WIDTH;
	textureDesc.Height = SCREEN_HEIGHT;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	//textureDesc.CPUAccessFlags = 0;
	//textureDesc.MiscFlags = 0;
	
	for (UINT i = 0; i < GBUFFER_COUNT; i++) {
		gDevice->CreateTexture2D(&textureDesc, NULL, &gGraphicsBuffer[i].texture);
	}

	// Create render target views.
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	for (UINT i = 0; i < GBUFFER_COUNT; i++) {
		gDevice->CreateRenderTargetView(gGraphicsBuffer[i].texture, &renderTargetViewDesc, &gGraphicsBuffer[i].renderTargetView);
	}

	// Create the shader resource views
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	for (UINT i = 0; i < GBUFFER_COUNT; i++) {
		gDevice->CreateShaderResourceView(gGraphicsBuffer[i].texture, &shaderResourceViewDesc, &gGraphicsBuffer[i].shaderResourceView);
	}

	//Create the depth stencil buffer texture
	D3D11_TEXTURE2D_DESC depthDesc{};
	depthDesc.Width = SCREEN_WIDTH;
	depthDesc.Height = SCREEN_HEIGHT;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	gDevice->CreateTexture2D(&depthDesc, NULL, &gDepthStencilTexture);

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	gDevice->CreateDepthStencilView(gDepthStencilTexture, // Depth stencil texture
		&depthStencilViewDesc, // Depth stencil desc
		&gDepthStecilView);  // [out] Depth stencil view

	//Relese
}

//TODO: move as much out of render loop as possible. Will need a loop to loop through all ojects in the future
void RenderFirstPass()
{
	ID3D11RenderTargetView* renderTargets[] =
	{
		gGraphicsBuffer[0].renderTargetView, /* Normal */
		gGraphicsBuffer[1].renderTargetView, /* PositionWS */
		gGraphicsBuffer[2].renderTargetView, /* Diffuse */
		gGraphicsBuffer[3].renderTargetView, /* Specular */

	};

	//set render targets
	gDeviceContext->OMSetRenderTargets(GBUFFER_COUNT, renderTargets, gDepthStecilView);

	//Clear the render targets
	gDeviceContext->ClearRenderTargetView(gGraphicsBuffer[0].renderTargetView, Colors::fBlack);
	gDeviceContext->ClearRenderTargetView(gGraphicsBuffer[1].renderTargetView, Colors::fLightSteelBlue);
	gDeviceContext->ClearRenderTargetView(gGraphicsBuffer[2].renderTargetView, Colors::fLightSteelBlue);
	gDeviceContext->ClearRenderTargetView(gGraphicsBuffer[3].renderTargetView, Colors::fLightSteelBlue);
	gDeviceContext->ClearDepthStencilView(gDepthStecilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	//gDeviceContext->ClearDepthStencilView(gDepthStecilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	UINT32 vertexSize = sizeof(float) * 5;
	UINT32 offset = 0;

	// Set Vertex Shader input
	gDeviceContext->IASetVertexBuffers(0, 1, &gVertexBuffer, &vertexSize, &offset);
	gDeviceContext->IASetInputLayout(gVertexLayout);
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set shaders
	gDeviceContext->VSSetShader(gVertexShader, nullptr, 0);
	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	gDeviceContext->GSSetShader(gGeometryShader, nullptr, 0);
	gDeviceContext->PSSetShader(gPixelShader, nullptr, 0);

	// set textures and constant buffers
	gDeviceContext->PSSetShaderResources(0, 1, &gTextureView);
	//gDeviceContext->PSSetConstantBuffers(0, 1, &gMaterialBuffer);

	// set constant buffers

	// TODO: check if map_write_discard is necessary and if it's required to make a mapped subresource
	D3D11_MAPPED_SUBRESOURCE viewProjectionMatrixPtr;
	gDeviceContext->Map(gPerFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &viewProjectionMatrixPtr);
	memcpy(viewProjectionMatrixPtr.pData, &VPBufferData, sizeof(cPerFrameBuffer));
	//gDeviceContext->Unmap(gPerFrameBuffer, 0);
	gDeviceContext->GSSetConstantBuffers(0, 1, &gPerFrameBuffer);

// LOOP OVER OBJECTS FROM HERE -----------------------------------------------------------------

	// update per-object buffer to spin cube
	static float rotation = 0.0f;
	rotation += CUBE_ROTATION_SPEED;

	XMStoreFloat4x4(&ObjectBufferData.World, XMMatrixTranspose(XMMatrixRotationY(rotation)));

	D3D11_MAPPED_SUBRESOURCE worldMatrixPtr;
	gDeviceContext->Map(gPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &worldMatrixPtr);
	// copy memory from CPU to GPU of the entire struct
	memcpy(worldMatrixPtr.pData, &ObjectBufferData, sizeof(cPerObjectBuffer));
	// Unmap constant buffer so that we can use it again in the GPU
	gDeviceContext->Unmap(gPerObjectBuffer, 0);
	// set resource to Geometry Shader
	gDeviceContext->GSSetConstantBuffers(1, 1, &gPerObjectBuffer);

	// Map material properties buffer
	SetMaterial(Materials::Black_plastic);
	D3D11_MAPPED_SUBRESOURCE materialPtr;
	gDeviceContext->Map(gMaterialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &materialPtr);
	memcpy(materialPtr.pData, &gMaterialBufferData, sizeof(cMaterialBuffer));
	//gDeviceContext->Unmap(gPerFrameBuffer, 0);
	gDeviceContext->PSSetConstantBuffers(0, 1, &gMaterialBuffer);

	// draw geometry
	gDeviceContext->Draw(36, 0);//number of vertices to draw

// LOOP OVER OBJECTS TO HERE -----------------------------------------------------------------

}

//TODO: move as much out of render loop as possible
void RenderLastPass()
{
	// get the address of the back buffer
	ID3D11Texture2D* pBackBuffer = nullptr;
	gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	// use the back buffer address to create the render target
	gDevice->CreateRenderTargetView(pBackBuffer, NULL, &gBackbufferRTV);
	pBackBuffer->Release();

	// set the render target as the back buffer
	gDeviceContext->OMSetRenderTargets(1, &gBackbufferRTV, nullptr);

	/* Full screen triangle is created in Vertex shader so no input buffer needed */
	const uintptr_t n0 = 0;
	gDeviceContext->IASetInputLayout(nullptr);
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gDeviceContext->IASetVertexBuffers(0, 0, 
		reinterpret_cast<ID3D11Buffer *const *>(&n0), 
		reinterpret_cast<const UINT *>(n0), 
		reinterpret_cast<const UINT *>(&n0)
		);

	gDeviceContext->VSSetShader(gFullScreenTriangleShader, nullptr, 0);
	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	gDeviceContext->GSSetShader(nullptr, nullptr, 0);
	gDeviceContext->PSSetShader(gLightPixelShader, nullptr, 0);

	ID3D11ShaderResourceView* GBufferTextureViews[] =
	{
		gGraphicsBuffer[0].shaderResourceView,
		gGraphicsBuffer[1].shaderResourceView,
		gGraphicsBuffer[2].shaderResourceView,
		gGraphicsBuffer[3].shaderResourceView
	};

	gDeviceContext->PSSetShaderResources(0, GBUFFER_COUNT, GBufferTextureViews);

	// Move light up and down
	static int lightYMovement = 249;
	lightYMovement	= (lightYMovement + 1) % 1000;
	float yMovement	= (lightYMovement < 500) ? -0.01f : 0.01f;
	XMMATRIX yMove	= XMMatrixTranslation(0.0f, yMovement, 0.0f);

	XMVECTOR lightPos = XMLoadFloat4(&gLightBufferData.Lights[0].PositionWS);
	lightPos = XMVector4Transform(lightPos, yMove);
	XMStoreFloat4(&gLightBufferData.Lights[0].PositionWS, lightPos);

	// Map light buffer
	D3D11_MAPPED_SUBRESOURCE LightBufferPtr;
	gDeviceContext->Map(gLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &LightBufferPtr);
	memcpy(LightBufferPtr.pData, &gLightBufferData, sizeof(cLightBuffer));
	gDeviceContext->Unmap(gLightBuffer, 0);
	gDeviceContext->PSSetConstantBuffers(0, 1, &gLightBuffer);

	// Clear screen
	gDeviceContext->ClearRenderTargetView(gBackbufferRTV, Colors::fWhite);

	// Draw full screen triangle
	gDeviceContext->Draw(3, 0);
}

void Render()
{
	RenderFirstPass();
	RenderLastPass();
}

void CreateAllConstantBuffers() {
	CreatePerFrameConstantBuffer();
	CreatePerObjectConstantBuffer();
	CreateMaterialConstantBuffer();
	CreateLightConstantBuffer();
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	MSG msg = { 0 };
	HWND wndHandle = InitWindow(hInstance); //1. Skapa fönster

	if (wndHandle)
	{
		CreateDirect3DContext(wndHandle); //2. Skapa och koppla SwapChain, Device och Device Context

		SetViewport(); //3. Sätt viewport

		CreateShaders(); //4. Skapa vertex- och pixel-shaders

		CreateTriangleData(); //5. Definiera triangelvertiser, 6. Skapa vertex buffer, 7. Skapa input layout

		CreateAllConstantBuffers();

		initGraphicsBuffer(); // Create G-Buffer

		setLights();

		ShowWindow(wndHandle, nCmdShow);

		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				Render(); //8. Rendera

				gSwapChain->Present(1, 0); //9. Växla front- och back-buffer
			}
		}

		gVertexBuffer->Release();
		gVertexLayout->Release();
		gVertexShader->Release();
		gPixelShader->Release();
		gGeometryShader	->Release();
		gDepthStecilView->Release();
		gDepthStencilTexture->Release();
		gBackbufferRTV->Release();
		gSwapChain->Release();
		gDevice->Release();
		gDeviceContext->Release();
		DestroyWindow(wndHandle);
	}

	return (int)msg.wParam;
}

HWND InitWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex		= { 0 };
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.hInstance		= hInstance;
	wcex.lpszClassName	= L"BTH_D3D_DEMO";
	if (!RegisterClassEx(&wcex))
		return false;

	RECT rc = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND handle = CreateWindow(
		L"BTH_D3D_DEMO",
		L"BTH Direct3D Demo",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	return handle;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

// CreateRnederTargetView and OMSetRenderTargets are probably unnecessary here
HRESULT CreateDirect3DContext(HWND wndHandle)
{
	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount			= 1;                               // one back buffer
	scd.BufferDesc.Format	= DXGI_FORMAT_R8G8B8A8_UNORM;      // use 32-bit color
	scd.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT; // how swap chain is to be used
	scd.OutputWindow		= wndHandle;                       // the window to be used
	scd.SampleDesc.Count	= 1;                               // how many multisamples
	scd.Windowed			= TRUE;                            // windowed/full-screen mode

	// create a device, device context and swap chain using the information in the scd struct
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&gSwapChain,
		&gDevice,
		NULL,
		&gDeviceContext);

	if (SUCCEEDED(hr))
	{
		// get the address of the back buffer
		ID3D11Texture2D* pBackBuffer = nullptr;
		gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

		// use the back buffer address to create the render target
		gDevice->CreateRenderTargetView(pBackBuffer, NULL, &gBackbufferRTV);
		pBackBuffer->Release();

		// set the render target as the back buffer
		gDeviceContext->OMSetRenderTargets(1, &gBackbufferRTV, NULL);
	}

	D3D11_TEXTURE2D_DESC bthTexDesc;
	ZeroMemory(&bthTexDesc, sizeof(bthTexDesc));
	bthTexDesc.Width				= BTH_IMAGE_WIDTH;
	bthTexDesc.Height				= BTH_IMAGE_HEIGHT;
	bthTexDesc.MipLevels			= bthTexDesc.ArraySize = 1;
	bthTexDesc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
	bthTexDesc.SampleDesc.Count		= 1;
	bthTexDesc.SampleDesc.Quality	= 0;
	bthTexDesc.Usage				= D3D11_USAGE_DEFAULT;
	bthTexDesc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
	bthTexDesc.MiscFlags			= 0;
	bthTexDesc.CPUAccessFlags		= 0;

	ID3D11Texture2D *pTexture = NULL;

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = (void*)BTH_IMAGE_DATA;
	data.SysMemPitch = BTH_IMAGE_WIDTH * 4 * sizeof(char);
	gDevice->CreateTexture2D(&bthTexDesc, &data, &pTexture);

	D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
	ZeroMemory(&resourceViewDesc, sizeof(resourceViewDesc));
	resourceViewDesc.Format						= bthTexDesc.Format;
	resourceViewDesc.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURE2D;
	resourceViewDesc.Texture2D.MipLevels		= bthTexDesc.MipLevels;
	resourceViewDesc.Texture2D.MostDetailedMip	= 0;
	gDevice->CreateShaderResourceView(pTexture, &resourceViewDesc, &gTextureView);

	pTexture->Release();

	return hr;
}