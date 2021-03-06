
#include "GlobalResources.hpp"
//
//#define DIRECTINPUT_VERSION 0x0800
//
//const double MATH_PI = 3.14159265358;
////const UINT GBUFFER_COUNT = 4; //MOVED TO GraphicsHandler.hpp
//const LONG SCREEN_WIDTH = 1920;//2*640;
//const LONG SCREEN_HEIGHT = 1080;//2*480;
//								//const int MAX_LIGHTS = 8; //moved to LightHandler.hpp
//const int NR_OF_OBJECTS = 1;
//float CUBE_ROTATION_SPEED = 0.01f;
//float LIGHT_ROTATION_SPEED = 0.001f;
////---------------Camera default values------------------------------------
//const DirectX::XMVECTOR CAMERA_STARTING_POS = DirectX::XMVectorSet(2.0f, 5.0f, 2.0f, 1.0f);
//DirectX::XMVECTOR CAM_POS = CAMERA_STARTING_POS;
//DirectX::XMVECTOR CAM_TARGET = DirectX::XMVectorZero();
//DirectX::XMVECTOR CAM_FORWARD = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
//DirectX::XMVECTOR CAM_RIGHT = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
//DirectX::XMVECTOR CAM_UP = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
//
//DirectX::XMVECTOR DEFAULT_FORWARD = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
//DirectX::XMVECTOR DEFAULT_RIGHT = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
//DirectX::XMVECTOR DEFAULT_UP = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
//float MOVE_LR = 0.0f, MOVE_BF = 0.0f, MOVE_UD = 0.0f, CAM_YAW = 0.0f, CAM_PITCH = 0.0f, SPEED = 15.0f;
//
//
////--------------Timer values---------------
//double COUNTS_PER_SECOND = 0.0;
//_int64 COUNTER_START = 0;
//int FRAME_COUNT = 0, FPS = 0;
//_int64 FRAME_TIME_OLD = 0;
////double FRAME_TIME;
//
////---------------------Heightmap values-----------------------------
//int NUMBER_OF_FACES = 0;
//int NUMBER_OF_VERTICES = 0;
//float WORLD_HEIGHT[200][200];
////struct HeightMapInfo {
////	int worldWidth;
////	int worldHeight;
////	DirectX::XMFLOAT3 *heightMap;
////};
//
////HWND InitWindow(HINSTANCE hInstance);
////LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
//
////HRESULT CreateDirect3DContext(HWND wndHandle);
//HRESULT gHR = 0;
//
//// Input devices
////HRESULT CreateDirect3DContext(HWND wndHandle);
////void InitializeDirectInput(HINSTANCE hInstance, HWND hwnd);
//
////IDirectInputDevice8* DIKeyboard;
////IDirectInputDevice8* DIMouse;
//
////DIMOUSESTATE MOUSE_LAST_STATE;
////LPDIRECTINPUT8 DirectInput;
//
//IDXGISwapChain* gSwapChain = nullptr;
//ID3D11Device* gDevice = nullptr;
//ID3D11DeviceContext* gDeviceContext = nullptr;
//
////MOVED TO GraphicsHandler.hpp
////// Depth Buffer
////ID3D11DepthStencilView* mDepthStecilView = nullptr;
////ID3D11Texture2D* mDepthStencilTexture = nullptr;
//
//// First Pass
//
//ID3D11Buffer* gSquareIndexBuffer = nullptr;
//ID3D11Buffer* gSquareVertBuffer = nullptr;
//
////ID3D11Buffer* gVertexBuffer = nullptr;
////ID3D11InputLayout* mVertexLayout = nullptr;
////ID3D11VertexShader* mGeometryPassVertexShader = nullptr;
////ID3D11GeometryShader* mGeometryPassGeometryShader = nullptr;
////ID3D11PixelShader* mGeometryPassPixelShader = nullptr;
//
////ID3D11ShaderResourceView* mTextureView = nullptr;
//
//// Last Pass
////ID3D11VertexShader* mLightPassVertexShader = nullptr;
////ID3D11PixelShader* mLightPassPixelShader = nullptr;
////ID3D11RenderTargetView* mBackbufferRTV = nullptr;
//
//
////struct Vertex {
////	Vertex() {}
////	Vertex(float x, float y, float z, float u, float v, float nx, float ny, float nz) :pos(x, y, z), texCoord(u, v), normal(nx, ny, nz) {}
////	DirectX::XMFLOAT3 pos;
////	DirectX::XMFLOAT2 texCoord;
////	DirectX::XMFLOAT3 normal;
////};
//
//
////MOVED TO GraphicsHandler.hpp
/////*--------------------------------------------------------------------------------------
////*			G-Buffer
////--------------------------------------------------------------------------------------*/
////
////struct gGraphicsBufferStruct {
////	ID3D11Texture2D* texture = nullptr;
////	ID3D11RenderTargetView* renderTargetView = nullptr;
////	ID3D11ShaderResourceView* shaderResourceView = nullptr;
////}; gGraphicsBufferStruct gGraphicsBuffer[GBUFFER_COUNT];
//
//
///*--------------------------------------------------------------------------------------
//*			Constant Buffers
//--------------------------------------------------------------------------------------*/
////ID3D11Buffer* mPerFrameBuffer = nullptr; //MOVED TO CameraHandler.hpp
//ID3D11Buffer* gPerObjectBuffer = nullptr;
//
////MOVED TO CameraHandler.hpp
////struct cPerFrameBuffer
////{
////	//XMFLOAT4X4 ViewProjection;
////	DirectX::XMFLOAT4X4 View;
////	DirectX::XMFLOAT4X4 Projection;
////}; cPerFrameBuffer VPBufferData;
//
////struct cPerObjectBuffer
////{
////	DirectX::XMFLOAT4X4 World;
////}; cPerObjectBuffer ObjectBufferData;
//
////------------------ Material (GBufferFragment.hlsl) -----------------------------------
//ID3D11Buffer* gMaterialBuffer = nullptr;
//
////struct materialStruct
////{
////	materialStruct(float r = 0.0f, float b = 0.0f, float g = 0.0f, float specPow = 128.0f)
////		: specularAlbedo(r, g, b), specularPower(specPow)
////	{}
////	DirectX::XMFLOAT3 specularAlbedo;
////	float specularPower;
////};
////
////struct cMaterialBuffer
////{
////	cMaterialBuffer(materialStruct mat = materialStruct()) : material(mat)
////	{}
////	materialStruct material;
////}; cMaterialBuffer gMaterialBufferData;
//
////------------------ Lights (LightFragment.hlsl) ---------------------------------------
////ID3D11Buffer* gLightBuffer = nullptr;
//
//
//// TODO: one default constructor instead of two
////struct Light
////{
////	Light(DirectX::XMFLOAT4 pos = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
////		DirectX::XMFLOAT4 col = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
////		float c_att = 1.0f,
////		float l_att = 0.0f,
////		float q_att = 0.0f,
////		float amb = 0.0f)
////		: PositionWS(pos),
////		Color(col),
////		constantAttenuation(c_att),
////		linearAttenuation(l_att),
////		quadraticAttenuation(q_att),
////		ambientCoefficient(amb)
////	{}
////	DirectX::XMFLOAT4 PositionWS;
////	DirectX::XMFLOAT4 Color;
////	float constantAttenuation;
////	float linearAttenuation;
////	float quadraticAttenuation;
////	float ambientCoefficient;
////};
////
////struct cLightBuffer
////{
////	cLightBuffer()
////		: cameraPositionWS(0.0f, 0.0f, 0.0f, 1.0f),
////		globalAmbient(0.2f, 0.2f, 0.2f, 1.0f)
////	{}
////	DirectX::XMFLOAT4 cameraPositionWS;
////	DirectX::XMFLOAT4 globalAmbient;
////	Light Lights[MAX_LIGHTS];
////}; cLightBuffer gLightBufferData;
//
////static_assert((sizeof(cPerFrameBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");
////static_assert((sizeof(cPerObjectBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");
////static_assert((sizeof(cMaterialBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");
////static_assert((sizeof(cLightBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");
//
//

