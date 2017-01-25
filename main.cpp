//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
#include <windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "bth_image.h"
#include <Windows.h>

#include<dinput.h>
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

using namespace DirectX;

const float SCREEN_WIDTH = 1920;
const float SCREEN_HEIGHT = 1080;

XMVECTOR CAM_POS = XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f);
XMVECTOR CAM_TARGET = XMVectorZero();
XMVECTOR CAM_FORWARD = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
XMVECTOR CAM_RIGHT = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
XMVECTOR CAM_UP = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

XMVECTOR DEFAULT_FORWARD = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
XMVECTOR DEFAULT_RIGHT = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

XMMATRIX CAM_ROT_MAT;

float gMoveLR = 0, gMoveBF = 0,gMoveUD=0, gCAM_YAW = 0, gCAM_PITCH = 0;

double gCounts_per_second = 0.0;
_int64 CounterStart = 0;
int gFrame_count = 0, gFPS = 0;
_int64 gFrame_time_old = 0;
double gFrame_time;


HWND InitWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HRESULT CreateDirect3DContext(HWND wndHandle);
bool InitDirectInput(HINSTANCE hInstance,HWND hwnd);

IDXGISwapChain* gSwapChain = nullptr;
ID3D11Device* gDevice = nullptr;
ID3D11DeviceContext* gDeviceContext = nullptr;
ID3D11RenderTargetView* gBackbufferRTV = nullptr;

ID3D11DepthStencilView* gDepthStecilView;
ID3D11Texture2D* gDepthStencilBuffer = NULL;


ID3D11Buffer* gVertexBuffer = nullptr;

ID3D11InputLayout* gVertexLayout = nullptr;
ID3D11VertexShader* gVertexShader = nullptr;
ID3D11PixelShader* gPixelShader = nullptr;
ID3D11GeometryShader* gGeometryShader = nullptr;

ID3D11ShaderResourceView* gTextureView = nullptr;

IDirectInputDevice8* DIKeyboard;
IDirectInputDevice8* DIMouse;

DIMOUSESTATE gMouse_last_state;
LPDIRECTINPUT8 DirectInput;

// https://msdn.microsoft.com/en-us/library/windows/desktop/ff476898(v=vs.85).aspx
ID3D11Buffer* gExampleBuffer = nullptr; // NEW
struct valuesFromCpu {					// NEW
	float value1;						// NEW
	float value2;						// NEW
	float value3;						// NEW
	float value4;						// NEW
};
valuesFromCpu globalValues{ 0.0,0.0,0.0,0.0 }; // NEW

void CreateConstantBufferExample() // NEW
{
	// initialize the description of the buffer.
	D3D11_BUFFER_DESC exampleBufferDesc;
	exampleBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	exampleBufferDesc.ByteWidth = sizeof(valuesFromCpu);
	exampleBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	exampleBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	exampleBufferDesc.MiscFlags = 0;
	exampleBufferDesc.StructureByteStride = 0;

	// check if the creation failed for any reason
	HRESULT hr = 0;
	hr = gDevice->CreateBuffer(&exampleBufferDesc, nullptr, &gExampleBuffer);
	if (FAILED(hr))
	{
		// handle the error, could be fatal or a warning...
		exit(-1);
	}
}


ID3D11Buffer* gWorldBuffer = nullptr;
struct valuesToWorld {
	XMFLOAT4X4 worldMatrix;
};
void CreateConstantBufferWorld() // NEW
{
	// initialize the description of the buffer.
	D3D11_BUFFER_DESC worldBufferDesc;
	worldBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	worldBufferDesc.ByteWidth = sizeof(valuesToWorld);
	worldBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	worldBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	worldBufferDesc.MiscFlags = 0;
	worldBufferDesc.StructureByteStride = 0;

	// check if the creation failed for any reason
	HRESULT hr = 0;
	hr = gDevice->CreateBuffer(&worldBufferDesc, nullptr, &gWorldBuffer);
	if (FAILED(hr))
	{
		// handle the error, could be fatal or a warning...
		exit(-1);
	}
}

ID3D11Buffer* gViewBuffer = nullptr;
struct valuesToView {
	XMFLOAT4X4 viewMatrix;
};
void CreateConstantBufferView() // NEW
{
	// initialize the description of the buffer.
	D3D11_BUFFER_DESC viewBufferDesc;
	viewBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	viewBufferDesc.ByteWidth = sizeof(valuesToView);
	viewBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	viewBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	viewBufferDesc.MiscFlags = 0;
	viewBufferDesc.StructureByteStride = 0;

	// check if the creation failed for any reason
	HRESULT hr = 0;
	hr = gDevice->CreateBuffer(&viewBufferDesc, nullptr, &gViewBuffer);
	if (FAILED(hr))
	{
		// handle the error, could be fatal or a warning...
		exit(-1);
	}
}

ID3D11Buffer* gProjectionBuffer = nullptr;
struct valuesToProject {
	XMFLOAT4X4 projectionMatrix;
};
void CreateConstantBufferProjection() // NEW
{
	// initialize the description of the buffer.
	D3D11_BUFFER_DESC projectionBufferDesc;
	projectionBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	projectionBufferDesc.ByteWidth = sizeof(valuesToProject);
	projectionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	projectionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	projectionBufferDesc.MiscFlags = 0;
	projectionBufferDesc.StructureByteStride = 0;

	// check if the creation failed for any reason
	HRESULT hr = 0;
	hr = gDevice->CreateBuffer(&projectionBufferDesc, nullptr, &gProjectionBuffer);
	if (FAILED(hr))
	{
		// handle the error, could be fatal or a warning...
		exit(-1);
	}
}

void CreateShaders()
{
	//create vertex shader
	ID3DBlob* pVS = nullptr;
	D3DCompileFromFile(
		L"Vertex.hlsl", // filename
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

	gDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &gVertexShader);

	//create input layout (verified using vertex shader)
	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};


	gDevice->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), &gVertexLayout);
	// we do not need anymore this COM object, so we release it.
	pVS->Release();

	//create pixel shader
	ID3DBlob* pPS = nullptr;
	D3DCompileFromFile(
		L"Fragment.hlsl", // filename
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

	gDevice->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &gPixelShader);
	// we do not need anymore this COM object, so we release it.
	pPS->Release();

	//create geometry shader
	ID3DBlob* pGS = nullptr;
	D3DCompileFromFile(
		L"Geometry.hlsl",
		nullptr,
		nullptr,
		"GS_main",
		"gs_5_0",
		0,
		0,
		&pGS,
		nullptr
	);
	gDevice->CreateGeometryShader(pGS->GetBufferPointer(), pGS->GetBufferSize(), nullptr, &gGeometryShader);


	pGS->Release();
}

void CreateTriangleData()
{
	struct TriangleVertex
	{
		float x, y, z;
		//float r,g,b;
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
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(triangleVertices);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = triangleVertices;
	gDevice->CreateBuffer(&bufferDesc, &data, &gVertexBuffer);
}

void SetViewport()
{
	D3D11_VIEWPORT vp;
	vp.Width = SCREEN_WIDTH;
	vp.Height = SCREEN_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	gDeviceContext->RSSetViewports(1, &vp);
}

void StartTimer() {
	LARGE_INTEGER frequency_count;
	QueryPerformanceFrequency(&frequency_count);

	gCounts_per_second = double(frequency_count.QuadPart);
	QueryPerformanceCounter(&frequency_count);
	CounterStart = frequency_count.QuadPart;
}
double GetTime() {
	LARGE_INTEGER current_time;
	QueryPerformanceCounter(&current_time);
	return double(current_time.QuadPart - CounterStart) / gCounts_per_second;
}
double GetFrameTime() {
	LARGE_INTEGER current_time;
	_int64 tick_count;
	QueryPerformanceCounter(&current_time);
	tick_count = current_time.QuadPart - gFrame_time_old;
	gFrame_time_old = current_time.QuadPart;

	if (tick_count < 0.0f) {
		tick_count = 0.0f;
	}
	return float(tick_count) / gCounts_per_second;
}

void UpdateCamera() {
	CAM_ROT_MAT = XMMatrixRotationRollPitchYaw(gCAM_PITCH, gCAM_YAW, 0.0f);
	CAM_TARGET = XMVector3TransformCoord(DEFAULT_FORWARD, CAM_ROT_MAT);
	CAM_TARGET = XMVector3Normalize(CAM_TARGET);

	XMMATRIX YRotation_CAM_directions = XMMatrixRotationY(gCAM_YAW);

	CAM_RIGHT = XMVector3TransformCoord(DEFAULT_RIGHT, YRotation_CAM_directions);
	CAM_UP = XMVector3TransformCoord(CAM_UP, YRotation_CAM_directions);
	CAM_FORWARD = XMVector3TransformCoord(DEFAULT_FORWARD, YRotation_CAM_directions);

	CAM_POS += gMoveLR*CAM_RIGHT;
	CAM_POS += gMoveBF*CAM_FORWARD;
	CAM_POS += gMoveUD*CAM_UP;

	gMoveLR = 0.0f;
	gMoveBF = 0.0f;
	gMoveUD = 0.0f;
	
	CAM_TARGET = CAM_POS + CAM_TARGET;

}

void Render(double time)
{
	float clearColor[] = { 0.1f, 0.1f, 0.1f, 1 }; //background color
	// set rendering state
	// if nothing changes, this does not have to be "re-done" every frame...

	UINT32 vertexSize = sizeof(float) * 5;
	UINT32 offset = 0;

	gDeviceContext->IASetVertexBuffers(0, 1, &gVertexBuffer, &vertexSize, &offset);
	gDeviceContext->IASetInputLayout(gVertexLayout);
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	gDeviceContext->VSSetShader(gVertexShader, nullptr, 0);
	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	gDeviceContext->GSSetShader(gGeometryShader, nullptr, 0);
	gDeviceContext->PSSetShader(gPixelShader, nullptr, 0);
	gDeviceContext->PSSetShaderResources(0, 1, &gTextureView);


	// NEW ========================================================
	// Map constant buffer so that we can write to it.
	D3D11_MAPPED_SUBRESOURCE dataPtr;
	gDeviceContext->Map(gExampleBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &dataPtr);
	// copy memory from CPU to GPU the entire struct
	globalValues.value1 += 0.005;
	memcpy(dataPtr.pData, &globalValues, sizeof(valuesFromCpu));
	// UnMap constant buffer so that we can use it again in the GPU
	gDeviceContext->Unmap(gExampleBuffer, 0);
	// set resource to Vertex Shader
	gDeviceContext->VSSetConstantBuffers(0, 1, &gExampleBuffer);
	// ==============================================================

	D3D11_MAPPED_SUBRESOURCE dataPtr1;
	gDeviceContext->Map(gWorldBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &dataPtr1);
	static float rotation;
	//rotation += 0.05;
	rotation += 1.0f*time;
	XMMATRIX W = XMMatrixRotationY(rotation);
	XMMATRIX WT = XMMatrixTranspose(W);
	memcpy(dataPtr1.pData, &WT, sizeof(valuesToWorld));
	gDeviceContext->Unmap(gWorldBuffer, 0);
	gDeviceContext->GSSetConstantBuffers(1, 1, &gWorldBuffer);

	D3D11_MAPPED_SUBRESOURCE dataPtr2;
	gDeviceContext->Map(gViewBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &dataPtr2);
	//	//XMVECTOR pos = XMVectorSet(0.0f, 1.0f, -2.0f, 1.0f);
	//	//XMVECTOR up = XMVectorSet(0.0f, 1.0f, -1.0f, 0.0f);

	//	XMVECTOR pos = XMVectorSet(0.0f, -1.0f, -2.0f, 1.0f);
	//	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 1.0f, 0.0f);

	////XMVECTOR pos = XMVectorSet(0.0f, 0.0f, -2.0f, 1.0f);
	////XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	//XMVECTOR target = XMVectorZero();
	//XMMATRIX V = XMMatrixLookAtLH(pos, target, up);

	XMMATRIX V = XMMatrixLookAtLH(CAM_POS, CAM_TARGET, CAM_UP);
	XMMATRIX VT = XMMatrixTranspose(V);
	memcpy(dataPtr2.pData, &VT, sizeof(valuesToView));
	gDeviceContext->Unmap(gWorldBuffer, 0);
	gDeviceContext->GSSetConstantBuffers(2, 1, &gViewBuffer);

	D3D11_MAPPED_SUBRESOURCE dataPtr3;
	gDeviceContext->Map(gProjectionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &dataPtr3);
	float fov = 0.45f*XM_PI;
	float ar = SCREEN_WIDTH / SCREEN_HEIGHT;
	float closer = 0.1f;
	float further = 20.0f;
	XMMATRIX P = XMMatrixPerspectiveFovLH(fov, ar, closer, further);
	XMMATRIX PT = XMMatrixTranspose(P);
	memcpy(dataPtr3.pData, &PT, sizeof(valuesToProject));
	gDeviceContext->Unmap(gProjectionBuffer, 0);
	gDeviceContext->GSSetConstantBuffers(3, 1, &gProjectionBuffer);


	// clear screen
	gDeviceContext->ClearRenderTargetView(gBackbufferRTV, clearColor);
	gDeviceContext->ClearDepthStencilView(gDepthStecilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// draw geometry
	gDeviceContext->Draw(36, 0);//number of vertices to draw
}

void DetectInput(double time,HWND hwnd) {
	DIMOUSESTATE mouse_current_state;
	BYTE keyboardState[256];
	DIKeyboard->Acquire();
	DIMouse->Acquire();

	DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouse_current_state);
	DIKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);
	if (keyboardState[DIK_ESCAPE] & 0x80) {
		PostMessage(hwnd, WM_DESTROY, 0, 0);
	}

	float speed = 15.0f*time;
	if (keyboardState[DIK_A] & 0x80) {
		gMoveLR -= speed;
	}
	if (keyboardState[DIK_D] & 0x80) {
		gMoveLR += speed;
	}
	if (keyboardState[DIK_W] & 0x80) {
		gMoveBF += speed;
	}
	if (keyboardState[DIK_S] & 0x80) {
		gMoveBF -= speed;
	}
	if (keyboardState[DIK_SPACE] & 0x80) {
		gMoveUD += speed;
	}
	if (keyboardState[DIK_C] & 0x80) {
		gMoveUD -= speed;
	}
	if ((mouse_current_state.lX != gMouse_last_state.lX) || (mouse_current_state.lY != gMouse_last_state.lY)) {
		gCAM_YAW += mouse_current_state.lX*0.001f;
		gCAM_PITCH += mouse_current_state.lY*0.001f;
		gMouse_last_state = mouse_current_state;
	}
	if (keyboardState[DIK_Q] & 0x80) {
		CAM_POS = XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f);
		CAM_TARGET = XMVectorZero();
		CAM_FORWARD = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		CAM_RIGHT = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		CAM_UP = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		gCAM_PITCH = 0.0f;
		gCAM_YAW = 0.0f;

	}

	gMouse_last_state = mouse_current_state;
	UpdateCamera();
}

void CreateAllBuffers() {
	CreateConstantBufferExample();
	CreateConstantBufferWorld();
	CreateConstantBufferView();
	CreateConstantBufferProjection();
}
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	MSG msg = { 0 };
	HWND wndHandle = InitWindow(hInstance); //1. Skapa fönster
	if (InitDirectInput(hInstance,wndHandle)==false) {
		MessageBox(0, L"Direct Input Initialisation failed", L"Error", MB_OK);
		return 0;
	}
	if (wndHandle)
	{
		CreateDirect3DContext(wndHandle); //2. Skapa och koppla SwapChain, Device och Device Context

		SetViewport(); //3. Sätt viewport

		CreateShaders(); //4. Skapa vertex- och pixel-shaders

		CreateTriangleData(); //5. Definiera triangelvertiser, 6. Skapa vertex buffer, 7. Skapa input layout

		CreateAllBuffers();

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
				gFrame_count++;
				if (GetTime() > 1.0f) {
					gFPS = gFrame_count;
					gFrame_count = 0;
					StartTimer();
				}
				double time = GetFrameTime();
				DetectInput(time, wndHandle);
				Render(time); //8. Rendera

				gSwapChain->Present(1, 0); //9. Växla front- och back-buffer
			}
		}
		gSwapChain->SetFullscreenState(false, NULL);
		DIKeyboard->Unacquire();
		DIMouse->Unacquire();
		DirectInput->Release();
		gVertexBuffer->Release();
		gVertexLayout->Release();
		gVertexShader->Release();
		gPixelShader->Release();
		gGeometryShader->Release();
		gDepthStecilView->Release();
		gDepthStencilBuffer->Release();
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
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = L"BTH_D3D_DEMO";
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

bool InitDirectInput(HINSTANCE hInstance, HWND hwnd) {
	HRESULT hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&DirectInput, NULL);

	hr = DirectInput->CreateDevice(GUID_SysKeyboard, &DIKeyboard, NULL);
	hr = DirectInput->CreateDevice(GUID_SysMouse, &DIMouse, NULL);
	

	hr = DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	hr = DIKeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	hr = DIMouse->SetDataFormat(&c_dfDIMouse);
	hr = DIMouse->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);

	return true;

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

HRESULT CreateDirect3DContext(HWND wndHandle)
{
	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow = wndHandle;                           // the window to be used
	scd.SampleDesc.Count = 4;                               // how many multisamples
	scd.Windowed = FALSE;                                    // windowed/full-screen mode

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

	D3D11_TEXTURE2D_DESC depthDesc;
	depthDesc.Width = SCREEN_WIDTH;
	depthDesc.Height = SCREEN_HEIGHT;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthDesc.SampleDesc.Count = 4;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;
	gDevice->CreateTexture2D(&depthDesc, NULL, &gDepthStencilBuffer);


	// Create the depth stencil view
	gDevice->CreateDepthStencilView(gDepthStencilBuffer, // Depth stencil texture
		nullptr, // Depth stencil desc
		&gDepthStecilView);  // [out] Depth stencil view

				 // Bind the depth stencil view
	gDeviceContext->OMSetRenderTargets(1,          // One rendertarget view
		&gBackbufferRTV,      // Render target view, created earlier
		gDepthStecilView);     // Depth stencil view for the render target


	D3D11_TEXTURE2D_DESC bthTexDesc;
	ZeroMemory(&bthTexDesc, sizeof(bthTexDesc));
	bthTexDesc.Width = BTH_IMAGE_WIDTH;
	bthTexDesc.Height = BTH_IMAGE_HEIGHT;
	bthTexDesc.MipLevels = bthTexDesc.ArraySize = 1;
	bthTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bthTexDesc.SampleDesc.Count = 1;
	bthTexDesc.SampleDesc.Quality = 0;
	bthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	bthTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bthTexDesc.MiscFlags = 0;
	bthTexDesc.CPUAccessFlags = 0;

	ID3D11Texture2D *pTexture = NULL;

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = (void*)BTH_IMAGE_DATA;
	data.SysMemPitch = BTH_IMAGE_WIDTH * 4 * sizeof(char);
	gDevice->CreateTexture2D(&bthTexDesc, &data, &pTexture);

	D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
	ZeroMemory(&resourceViewDesc, sizeof(resourceViewDesc));
	resourceViewDesc.Format = bthTexDesc.Format;
	resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resourceViewDesc.Texture2D.MipLevels = bthTexDesc.MipLevels;
	resourceViewDesc.Texture2D.MostDetailedMip = 0;
	gDevice->CreateShaderResourceView(pTexture, &resourceViewDesc, &gTextureView);

	pTexture->Release();

	return hr;
}