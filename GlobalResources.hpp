#ifndef GLOBALRESOURCES_HPP
#define GLOBALRESOURCES_HPP

#define DIRECTINPUT_VERSION 0x0800

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "bth_image.h"
#include <Windows.h>
#include <vector>
#include <dinput.h>
#include <WICTextureLoader.h>
#include <objbase.h>


#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"Ole32.lib")


const double MATH_PI = 3.14159265358;
const UINT GBUFFER_COUNT = 4;
const LONG SCREEN_WIDTH = 1920;//2*640;
const LONG SCREEN_HEIGHT = 1080;//2*480;
const int MAX_LIGHTS = 8;
const int NR_OF_OBJECTS = 1;
float CUBE_ROTATION_SPEED = 0.01f;
float LIGHT_ROTATION_SPEED = 0.001f;
//---------------Camera default values------------------------------------
const DirectX::XMVECTOR CAMERA_STARTING_POS = DirectX::XMVectorSet(2.0f, 5.0f, 2.0f, 1.0f);
DirectX::XMVECTOR CAM_POS = CAMERA_STARTING_POS;
DirectX::XMVECTOR CAM_TARGET = DirectX::XMVectorZero();
DirectX::XMVECTOR CAM_FORWARD = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
DirectX::XMVECTOR CAM_RIGHT = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
DirectX::XMVECTOR CAM_UP = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

DirectX::XMVECTOR DEFAULT_FORWARD = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
DirectX::XMVECTOR DEFAULT_RIGHT = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
DirectX::XMVECTOR DEFAULT_UP = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
float MOVE_LR = 0.0f, MOVE_BF = 0.0f, MOVE_UD = 0.0f, CAM_YAW = 0.0f, CAM_PITCH = 0.0f, SPEED = 15.0f;


//--------------Timer values---------------
double COUNTS_PER_SECOND = 0.0;
_int64 COUNTER_START = 0;
int FRAME_COUNT = 0, FPS = 0;
_int64 FRAME_TIME_OLD = 0;
double FRAME_TIME;

//---------------------Heightmap values-----------------------------
int NUMBER_OF_FACES = 0;
int NUMBER_OF_VERTICES = 0;
float WORLD_HEIGHT[200][200];
struct HeightMapInfo {
	int worldWidth;
	int worldHeight;
	DirectX::XMFLOAT3 *heightMap;
};

HWND InitWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HRESULT CreateDirect3DContext(HWND wndHandle);
HRESULT gHR = 0;

// Input devices
HRESULT CreateDirect3DContext(HWND wndHandle);
void InitDirectInput(HINSTANCE hInstance, HWND hwnd);

IDirectInputDevice8* DIKeyboard;
IDirectInputDevice8* DIMouse;

DIMOUSESTATE MOUSE_LAST_STATE;
LPDIRECTINPUT8 DirectInput;

IDXGISwapChain* gSwapChain = nullptr;
ID3D11Device* gDevice = nullptr;
ID3D11DeviceContext* gDeviceContext = nullptr;

// Depth Buffer
ID3D11DepthStencilView* gDepthStecilView = nullptr;
ID3D11Texture2D* gDepthStencilTexture = nullptr;

// First Pass

ID3D11Buffer* gSquareIndexBuffer = nullptr;
ID3D11Buffer* gSquareVertBuffer = nullptr;

ID3D11Buffer* gVertexBuffer = nullptr;
//ID3D11InputLayout* gVertexLayout = nullptr;
//ID3D11VertexShader* gVertexShader = nullptr;
//ID3D11GeometryShader* gGeometryShader = nullptr;
//ID3D11PixelShader* gPixelShader = nullptr;

//ID3D11ShaderResourceView* gTextureView = nullptr;

// Last Pass
//ID3D11VertexShader* gFullScreenTriangleShader = nullptr;
//ID3D11PixelShader* gLightPixelShader = nullptr;
//ID3D11RenderTargetView* gBackbufferRTV = nullptr;


/*--------------------------------------------------------------------------------------
*			G-Buffer
--------------------------------------------------------------------------------------*/

struct gGraphicsBufferStruct {
	ID3D11Texture2D* texture = nullptr;
	ID3D11RenderTargetView* renderTargetView = nullptr;
	ID3D11ShaderResourceView* shaderResourceView = nullptr;
}; gGraphicsBufferStruct gGraphicsBuffer[GBUFFER_COUNT];


/*--------------------------------------------------------------------------------------
*			Constant Buffers
--------------------------------------------------------------------------------------*/
ID3D11Buffer* gPerFrameBuffer = nullptr;
ID3D11Buffer* gPerObjectBuffer = nullptr;

struct cPerFrameBuffer
{
	//XMFLOAT4X4 ViewProjection;
	DirectX::XMFLOAT4X4 View;
	DirectX::XMFLOAT4X4 Projection;
}; cPerFrameBuffer VPBufferData;

struct cPerObjectBuffer
{
	DirectX::XMFLOAT4X4 World;
}; cPerObjectBuffer ObjectBufferData;

//------------------ Material (GBufferFragment.hlsl) -----------------------------------
ID3D11Buffer* gMaterialBuffer = nullptr;

struct materialStruct
{
	materialStruct(float r = 0.0f, float b = 0.0f, float g = 0.0f, float specPow = 128.0f)
		: specularAlbedo(r, g, b), specularPower(specPow)
	{}
	DirectX::XMFLOAT3 specularAlbedo;
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
	Light(DirectX::XMFLOAT4 pos = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
		DirectX::XMFLOAT4 col = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		float c_att = 1.0f,
		float l_att = 0.0f,
		float q_att = 0.0f,
		float amb = 0.0f)
		: PositionWS(pos),
		Color(col),
		constantAttenuation(c_att),
		linearAttenuation(l_att),
		quadraticAttenuation(q_att),
		ambientCoefficient(amb)
	{}
	DirectX::XMFLOAT4 PositionWS;
	DirectX::XMFLOAT4 Color;
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
	float ambientCoefficient;
};

struct cLightBuffer
{
	cLightBuffer()
		: cameraPositionWS(0.0f, 0.0f, 0.0f, 1.0f),
		globalAmbient(0.2f, 0.2f, 0.2f, 1.0f)
	{}
	DirectX::XMFLOAT4 cameraPositionWS;
	DirectX::XMFLOAT4 globalAmbient;
	Light Lights[MAX_LIGHTS];
}; cLightBuffer gLightBufferData;

static_assert((sizeof(cPerFrameBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");
static_assert((sizeof(cPerObjectBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");
static_assert((sizeof(cMaterialBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");
static_assert((sizeof(cLightBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");

/*--------------------------------------------------------------------------------------
*				Namespaces
--------------------------------------------------------------------------------------*/

namespace Colors
{
	static const DirectX::XMFLOAT4 White = { 1.0f, 1.0f, 1.0f, 1.0f };
	static const DirectX::XMFLOAT4 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
	static const DirectX::XMFLOAT4 LightSteelBlue = { 0.69f, 0.77f, 0.87f, 1.0f };
	static const DirectX::XMFLOAT4 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
	static const DirectX::XMFLOAT4 Green = { 0.0f, 1.0f, 0.0f, 1.0f };
	static const DirectX::XMFLOAT4 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	static const float fWhite[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static const float fBlack[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	static const float fLightSteelBlue[4] = { 0.69f, 0.77f, 0.87f, 1.0f };
}

namespace Materials
{
	static const materialStruct Black_plastic = materialStruct(0.5f, 0.5f, 0.5f, 32.0f);
	static const materialStruct Black_rubber = materialStruct(0.4f, 0.4f, 0.4f, 10.0f);
}


#endif // !GLOBALRESOURCES_HPP
