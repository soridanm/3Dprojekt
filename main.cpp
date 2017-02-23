//--------------------------------------------------------------------------------------
// TODO: 
// Move resolution options to main.cpp
// 
// TODO?:
//	Turn global constants into getFunctions()
//--------------------------------------------------------------------------------------

#define DIRECTINPUT_VERSION 0x0800

#include "Engine.hpp"


HWND InitWindow(HINSTANCE hInstance, Engine engine);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//HRESULT CreateDirect3DContext(HWND wndHandle, Engine engine);
//HRESULT gHR; //WILL BE REMOVED

// Input devices
//void InitDirectInput(HINSTANCE hInstance, HWND hwnd); //was moved so will probably be removed



//PARTIALLY REWRITTEN IN GraphicsHandler.cpp
void CreateAllConstantBuffers() {
	//CreatePerFrameConstantBuffer(); //InitializeGraphics.InitializeCamera
	//CreatePerObjectConstantBuffer(); //InitializeGraphics.InitializeObjects
	//CreateMaterialConstantBuffer(); //InitializeGraphics.InitializeObjects
	//CreateLightConstantBuffer(); InitializeGraphics.InitializeLights
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	Engine DemoEngine;
	
	MSG msg = { 0 };
	HWND wndHandle = InitWindow(hInstance, DemoEngine); //1. Skapa fönster
	//InitDirectInput(hInstance, wndHandle);//creates input
		
	DemoEngine.mGraphicsHandler.mCameraHandler.InitializeDirectInput(hInstance, wndHandle);
	if (wndHandle)
	{

		//SetViewPort() //InitializeGraphics.InitializeCamera

		//HRESULT hr = CreateDirect3DContext(wndHandle, DemoEngine); //2. Skapa och koppla SwapChain, Device och Device Context
		HRESULT hr = DemoEngine.eCreateDirect3DContext(wndHandle);
		if (FAILED(hr))
		{
			exit(-1);
		}


		DemoEngine.Initialize(); /* SetViewPort(), CreatePerFrameConstantBuffer()
								  * CreateShaders(), Create-/SetLightBuffer(), InitializeGBuffer()
								  * CreateWorld(), 
								  *
								  */

		//CreateTriangleData(); //5. Definiera triangelvertiser, 6. Skapa vertex buffer, 7. Skapa input layout

		//CreateWorld(); //InitializeGraphics.InitializeObjects
		
		//CreateAllConstantBuffers();

		//initGraphicsBuffer(); // Create G-Buffer //InitializeGraphics

		//setLights(); //initializeGraphics.InitializeLights

		ShowWindow(wndHandle, nCmdShow);


		PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
		while (WM_QUIT != msg.message)
		{
			
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				DemoEngine.TimeFunction(wndHandle);
				DemoEngine.Render();

				////increases time
				//FRAME_COUNT++;
				//if (GetTime() > 1.0f) {
				//	FPS = FRAME_COUNT;
				//	FRAME_COUNT = 0;
				//	StartTimer();
				//}
				//double time = GetFrameTime();
				//DetectInput(time, wndHandle);
				//Render(); //8. Rendera

				//this should maybe be turned into a function
				DemoEngine.gSwapChain->Present(1, 0); //9. Växla front- och back-buffer
			}
		}
		//this REALLY should be turned into a function
		DemoEngine.gSwapChain->SetFullscreenState(false, NULL);
		DemoEngine.mGraphicsHandler.mCameraHandler.DIKeyboard->Unacquire();
		DemoEngine.mGraphicsHandler.mCameraHandler.DIMouse->Unacquire();
		DemoEngine.mGraphicsHandler.mCameraHandler.DirectInput->Release();
		/*gVertexBuffer->Release();
		gVertexLayout->Release();
		gVertexShader->Release();
		gPixelShader->Release();
		gGeometryShader	->Release();
		gDepthStecilView->Release();
		gDepthStencilTexture->Release();
		gBackbufferRTV->Release();*/
		DemoEngine.gSwapChain->Release();
		/*gDevice->Release();
		gDeviceContext->Release();*/
		DestroyWindow(wndHandle);
	}

	return (int)msg.wParam;
}

HWND InitWindow(HINSTANCE hInstance, Engine engine)
{
	WNDCLASSEX wcex		= { 0 };
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.hInstance		= hInstance;
	wcex.lpszClassName	= L"BTH_D3D_DEMO";
	if (!RegisterClassEx(&wcex))
		return false;

	LONG SCREEN_WIDTH = engine.mGraphicsHandler.mCameraHandler.GetScreenWidth();
	LONG SCREEN_HEIGHT = engine.mGraphicsHandler.mCameraHandler.GetScreenHeight();
	
	RECT rc = { 0, 0,  SCREEN_WIDTH, SCREEN_HEIGHT };
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
//HRESULT CreateDirect3DContext(HWND wndHandle, Engine engine)
//{
//	// create a struct to hold information about the swap chain
//	DXGI_SWAP_CHAIN_DESC scd;
//
//	// clear out the struct for use
//	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
//
//	// fill the swap chain description struct
//	scd.BufferCount			= 1;                               // one back buffer
//	scd.BufferDesc.Format	= DXGI_FORMAT_R8G8B8A8_UNORM;      // use 32-bit color
//	scd.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT; // how swap chain is to be used
//	scd.OutputWindow		= wndHandle;                       // the window to be used
//	scd.SampleDesc.Count	= 1;                               // how many multisamples
//	scd.Windowed			= TRUE;                            // windowed/full-screen mode
//
//	// create a device, device context and swap chain using the information in the scd struct
//	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL,
//		D3D_DRIVER_TYPE_HARDWARE,
//		NULL,
//		NULL,
//		NULL,
//		NULL,
//		D3D11_SDK_VERSION,
//		&scd,
//		&engine.gSwapChain,
//		&engine.gDevice,
//		NULL,
//		&engine.gDeviceContext);
//
//	if (SUCCEEDED(hr))
//	{
//		// get the address of the back buffer
//		ID3D11Texture2D* pBackBuffer = nullptr;
//		engine.gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
//
//		// use the back buffer address to create the render target
//		engine.gDevice->CreateRenderTargetView(pBackBuffer, NULL, engine.GetBackBufferRTV());
//		pBackBuffer->Release();
//
//		// set the render target as the back buffer
//		engine.gDeviceContext->OMSetRenderTargets(1, engine.GetBackBufferRTV(), NULL);
//	}
//
//
//	//pTexture->Release();
//	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
//
//	ID3D11Resource* texture;
//	DirectX::CreateWICTextureFromFile(engine.gDevice, engine.gDeviceContext, L"grass-free-texture.jpg", &texture, engine.GetTextureView());
//
//	return hr;
//}
