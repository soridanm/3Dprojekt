#include "Engine.hpp"


// private ------------------------------------------------------------------------------

// public ------------------------------------------------------------------------------

//TODO: Create all objects here
bool Engine::Initialize()
{
	mGraphicsHandler.InitializeGraphics(gDevice, gDeviceContext);
	return true;
}

bool Engine::Render()
{
	mGraphicsHandler.RenderGeometryPass(gDeviceContext);
	mGraphicsHandler.RenderLightPass(gDevice, gDeviceContext, gSwapChain);
	
	return true;
}

HRESULT Engine::eCreateDirect3DContext(HWND &wndHandle)
{

	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                               // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;      // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // how swap chain is to be used
	scd.OutputWindow = wndHandle;                       // the window to be used
	scd.SampleDesc.Count = 1;                               // how many multisamples
	scd.Windowed = TRUE;                            // windowed/full-screen mode

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
		gDevice->CreateRenderTargetView(pBackBuffer, NULL, GetBackBufferRTV());
		pBackBuffer->Release();

		// set the render target as the back buffer
		gDeviceContext->OMSetRenderTargets(1, GetBackBufferRTV(), NULL);
	}


	//pTexture->Release();
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	ID3D11Resource* texture;
	DirectX::CreateWICTextureFromFile(gDevice, gDeviceContext, L"grass-free-texture.jpg", &texture, GetTextureView());

	return hr;
}

Engine::Engine()
{
	//mGraphicsHandler.InitializeGraphics(gDevice, gDeviceContext);
}

Engine::~Engine()
{

}

//This will/should be renamed to something more descriptive
void Engine::TimeFunction(HWND &wndHandle)
{
	mTimeHandler.TimeHandlerTimeFunction();
	double time = mTimeHandler.GetFrameTime();
	mGraphicsHandler.mCameraHandler.DetectInput(time, wndHandle);
}


IDXGISwapChain** Engine::GetSwapChain()
{
	return &gSwapChain;
}

ID3D11Device** Engine::GetDevice()
{
	return &gDevice;
}

ID3D11DeviceContext** Engine::GetDeviceContext()
{
	return &gDeviceContext;
}

ID3D11RenderTargetView** Engine::GetBackBufferRTV()
{
	return &mGraphicsHandler.mBackbufferRTV;
}

ID3D11ShaderResourceView** Engine::GetTextureView()
{
	return &mGraphicsHandler.gTextureView;
}

