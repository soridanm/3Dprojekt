/*
* TODO: Write error messages and use if(Function()) instead of just Funtion()
*		Update code with code from the .obj branch
*		See if the order of things should be changed
*
*/

#include "GraphicsHandler.hpp"

// private ------------------------------------------------------------------------------

//done for now, will add comments to this function later
bool GraphicsHandler::CompileShader(
	ID3DBlob** pShader,
	LPCWSTR shaderFileName,
	LPCSTR entryPoint,
	LPCSTR shaderModel,
	const D3D_SHADER_MACRO* pDefines,
	ID3DInclude* pInclude,
	UINT Flags1,
	UINT flags2,
	ID3DBlob** ppErrorMsgs)
{
	HRESULT gHR = D3DCompileFromFile(
		shaderFileName,
		pDefines,
		pInclude,
		entryPoint,
		shaderModel,
		Flags1,
		flags2,
		pShader,
		ppErrorMsgs
	);
	
	if (FAILED(gHR)) {
		return false;
	} else {
		return true;
	}
}

//temp done
bool GraphicsHandler::CreateInputLayout(ID3D11Device* Dev, ID3DBlob* pVS)
{
	//create input layout (verified using vertex shader)
	D3D11_INPUT_ELEMENT_DESC inputDesc[] = 
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0,	12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0,	20,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	Dev->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), &mVertexLayout);

	return true;
}

// TODO: Should probably add hr checks. also Relese()
bool GraphicsHandler::InitializeGraphicsBuffer(ID3D11Device* Dev)
{
	// Create render target textures
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = mCameraHandler.GetScreenWidth();
	textureDesc.Height = mCameraHandler.GetScreenHeight();
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
		Dev->CreateTexture2D(&textureDesc, NULL, &mGraphicsBuffer[i].texture);
	}

	// Create render target views.
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	for (UINT i = 0; i < GBUFFER_COUNT; i++) {
		Dev->CreateRenderTargetView(mGraphicsBuffer[i].texture, &renderTargetViewDesc, &mGraphicsBuffer[i].renderTargetView);
	}

	// Create the shader resource views
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	for (UINT i = 0; i < GBUFFER_COUNT; i++) {
		Dev->CreateShaderResourceView(mGraphicsBuffer[i].texture, &shaderResourceViewDesc, &mGraphicsBuffer[i].shaderResourceView);
	}

	//Create the depth stencil buffer texture
	D3D11_TEXTURE2D_DESC depthDesc{};
	depthDesc.Width = mCameraHandler.GetScreenWidth();
	depthDesc.Height = mCameraHandler.GetScreenHeight();
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	Dev->CreateTexture2D(&depthDesc, NULL, &mDepthStencilTexture);

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	Dev->CreateDepthStencilView(
		mDepthStencilTexture,	// Depth stencil texture
		&depthStencilViewDesc,	// Depth stencil desc
		&mDepthStecilView);		// [out] Depth stencil view

	//Release

	return true;
}


// public ------------------------------------------------------------------------------

bool GraphicsHandler::InitializeGraphics(ID3D11Device* Dev, ID3D11DeviceContext* DevCon)
{
	mCameraHandler.InitializeCamera(Dev, DevCon);

	mLightHandler.InitializeLights(Dev, mCameraHandler.GetCameraPosition());

	mLightHandler.CreateShadowMap(Dev);

	mObjectHandler.InitializeObjects(Dev);

	CreateShaders(Dev); //TODO: rewrite with shader class

	InitializeGraphicsBuffer(Dev);


	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	ID3D11Resource* texture;
	HRESULT hr = DirectX::CreateWICTextureFromFile(Dev, DevCon, L"grass-free-texture.jpg", &texture, &gTextureView);
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

//TODO: initialization list probably doesn't work!
GraphicsHandler::GraphicsHandler() 
	: mCameraHandler(), mLightHandler(mCameraHandler.GetCameraPosition())
{
	for (int i = 0; i < GBUFFER_COUNT; i++)
	{
		mGraphicsBuffer[i] = GraphicsBuffer();
	}
	mDepthStecilView = nullptr;
	mDepthStencilTexture = nullptr;
	mVertexLayout = nullptr;
	mGeometryPassVertexShader = nullptr;
	mGeometryPassGeometryShader = nullptr;
	mGeometryPassPixelShader = nullptr;
	//mTextureView = nullptr;
	mLightPassVertexShader = nullptr;
	mLightPassPixelShader = nullptr;
	mBackbufferRTV = nullptr;

	//mCameraHandler = CameraHandler();
	//mLightHandler = LightHandler(mCameraHandler.GetCameraPosition());
}

GraphicsHandler::~GraphicsHandler()
{
	
}

//TODO: clean up and write better error messages
bool GraphicsHandler::CreateShaders(ID3D11Device* Dev)
{
	//---------------------------------- Geometry Pass ----------------------------------------------------

	//compile and create vertex shader
	ID3DBlob* pVS = nullptr;
	CompileShader(&pVS, L"GBufferVertex.hlsl", "VS_main", "vs_5_0");

	HRESULT gHR = Dev->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &mGeometryPassVertexShader);
	if (FAILED(gHR)) {
		exit(-1);
	}

	//Create an input layout for the vertex shader
	if (!CreateInputLayout(Dev, pVS)) {
		exit(-1);
	}

	//compile and create geometry shader
	ID3DBlob* pGS = nullptr;
	CompileShader(&pGS, L"GBufferGeometry.hlsl", "GS_main", "gs_5_0");

	gHR = Dev->CreateGeometryShader(pGS->GetBufferPointer(), pGS->GetBufferSize(), nullptr, &mGeometryPassGeometryShader);
	if (FAILED(gHR)) {
		exit(-1);
	}

	//compile and create pixel shader
	ID3DBlob* pPS = nullptr;
	CompileShader(&pPS, L"GBufferFragment.hlsl", "PS_main", "ps_5_0");

	gHR = Dev->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &mGeometryPassPixelShader);
	if (FAILED(gHR)) {
		exit(-1);
	}

	//---------------------------------- Light Pass ----------------------------------------------------

		//compile and create vertex shader
	ID3DBlob* pVS2 = nullptr;
	CompileShader(&pVS2, L"LightVertex.hlsl", "VS_main", "vs_5_0");
	gHR = Dev->CreateVertexShader(pVS2->GetBufferPointer(), pVS2->GetBufferSize(), nullptr, &mLightPassVertexShader);
	if (FAILED(gHR)) {
		exit(-1);
	}

	//compile and create pixel shader
	ID3DBlob* pPS2 = nullptr;
	CompileShader(&pPS2, L"LightFragment.hlsl", "PS_main", "ps_5_0");
	gHR = Dev->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &mLightPassPixelShader);
	if (FAILED(gHR)) {
		exit(-1);
	}

	//Clean up
	pVS->Release();
	pGS->Release();
	pPS->Release();
	pVS2->Release();
	pPS2->Release();

	return true;
}

// ------------------------------ Geometry Pass ------------------------------------------------------

//temp done
void GraphicsHandler::SetGeometryPassRenderTargets(ID3D11DeviceContext* DevCon)
{
	//Clear the render targets
	DevCon->ClearRenderTargetView(mGraphicsBuffer[0].renderTargetView, Colors::fBlack);
	DevCon->ClearRenderTargetView(mGraphicsBuffer[1].renderTargetView, Colors::fLightSteelBlue);
	DevCon->ClearRenderTargetView(mGraphicsBuffer[2].renderTargetView, Colors::fLightSteelBlue);
	DevCon->ClearRenderTargetView(mGraphicsBuffer[3].renderTargetView, Colors::fBlack);
	DevCon->ClearDepthStencilView(mDepthStecilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	//DevCon->ClearDepthStencilView(mDepthStecilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	ID3D11RenderTargetView* renderTargets[] =
	{
		mGraphicsBuffer[0].renderTargetView, /* Normal */
		mGraphicsBuffer[1].renderTargetView, /* PositionWS */
		mGraphicsBuffer[2].renderTargetView, /* Diffuse */
		mGraphicsBuffer[3].renderTargetView  /* Specular */
	};

	//set render targets
	DevCon->OMSetRenderTargets(GBUFFER_COUNT, renderTargets, mDepthStecilView);
}

//TODO: update with correct code from both branches.
void GraphicsHandler::SetGeometryPassShaders(ID3D11DeviceContext* DevCon)
{
	// Set Vertex Shader input
	DevCon->IASetInputLayout(mVertexLayout);
	DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set shaders
	DevCon->VSSetShader(mGeometryPassVertexShader, nullptr, 0);
	DevCon->HSSetShader(nullptr, nullptr, 0);
	DevCon->DSSetShader(nullptr, nullptr, 0);
	DevCon->GSSetShader(mGeometryPassGeometryShader, nullptr, 0);
	DevCon->PSSetShader(mGeometryPassPixelShader, nullptr, 0);
}

void GraphicsHandler::SetGeometryPassShaderResources(ID3D11DeviceContext* DevCon)
{
	DevCon->PSSetShaderResources(1, 1, &gTextureView);
}

void GraphicsHandler::RenderGeometryPass(ID3D11DeviceContext* DevCon)
{
	SetGeometryPassRenderTargets(DevCon);
	SetGeometryPassShaders(DevCon);
	mCameraHandler.BindPerFrameConstantBuffer(DevCon);
	//LOOP OVER OBJECTS FROM HERE

	SetGeometryPassShaderResources(DevCon);

	mObjectHandler.SetGeometryPassHeightMapBuffer(DevCon);
	DevCon->DrawIndexed(mObjectHandler.GetHeightMapNrOfFaces() * 3, 0, 0);

	for (int i = 0; i < mObjectHandler.GetNrOfMeshSubsets(); i++)
	{
		mObjectHandler.SetGeometryPassObjectBufferWithIndex(DevCon, i);

		int indexStart = mObjectHandler.meshSubsetIndexStart[i];
		int indexDrawAmount = mObjectHandler.meshSubsetIndexStart[i + 1] - indexStart;

		DevCon->DrawIndexed(indexDrawAmount, indexStart, 0);
	}
	//LOOP OVER OBJECTS TO HERE
}

// ------------------------------ Shadow Map Pass ------------------------------------------------------


//TODO: move to LightHandler.cpp
void GraphicsHandler::SetShadowMapPassRenderTargets(ID3D11DeviceContext* DevCon)
{
	DevCon->OMSetRenderTargets(0, 0, mLightHandler.mShadowMapDepthView);
	DevCon->ClearDepthStencilView(mLightHandler.mShadowMapDepthView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

//TODO: make a seperate input layout for the shadow pass since it only needs the position
void GraphicsHandler::SetShadowMapPassShaders(ID3D11DeviceContext* DevCon)
{
	// Set Vertex Shader input
	DevCon->IASetInputLayout(mVertexLayout);
	DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set shaders
	DevCon->VSSetShader(mShadowPassVertexShader, nullptr, 0);
	DevCon->HSSetShader(nullptr, nullptr, 0);
	DevCon->DSSetShader(nullptr, nullptr, 0);
	DevCon->GSSetShader(nullptr, nullptr, 0);
	DevCon->PSSetShader(mShadowPassPixelShader, nullptr, 0);
}

void GraphicsHandler::SetShadowMapPassShaderResources(ID3D11DeviceContext* DevCon)
{

}

// ------------------------------ Light Pass ------------------------------------------------------

//TODO: check if pBackBuffer should be declared here or earlier
bool GraphicsHandler::SetLightPassRenderTargets(ID3D11Device* Dev, ID3D11DeviceContext* DevCon, IDXGISwapChain* SwapChain)
{
	// get the address of the back buffer
	ID3D11Texture2D* pBackBuffer = nullptr;
	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	// use the back buffer address to create the render target
	Dev->CreateRenderTargetView(pBackBuffer, NULL, &mBackbufferRTV);
	pBackBuffer->Release();

	// set the render target as the back buffer
	DevCon->OMSetRenderTargets(1, &mBackbufferRTV, nullptr);

	//Clear screen
	DevCon->ClearRenderTargetView(mBackbufferRTV, Colors::fWhite);

	return true;
}

//DONE!
bool GraphicsHandler::SetLightPassShaders(ID3D11DeviceContext* DevCon)
{
	/* Full screen triangle is created in Vertex shader using vertexID so no input layout needed */
	const uintptr_t n0 = 0;
	DevCon->IASetInputLayout(nullptr);
	DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DevCon->IASetVertexBuffers(0, 0,
		reinterpret_cast<ID3D11Buffer *const *>(&n0),
		reinterpret_cast<const UINT *>(n0),
		reinterpret_cast<const UINT *>(&n0)
	);

	DevCon->VSSetShader(mLightPassVertexShader, nullptr, 0);
	DevCon->HSSetShader(nullptr, nullptr, 0);
	DevCon->DSSetShader(nullptr, nullptr, 0);
	DevCon->GSSetShader(nullptr, nullptr, 0);
	DevCon->PSSetShader(mLightPassPixelShader, nullptr, 0);

	return true;
}

//TODO: There might be a memory leak here
bool GraphicsHandler::SetLightPassGBuffers(ID3D11DeviceContext* DevCon)
{
	ID3D11ShaderResourceView* GBufferTextureViews[] =
	{
		mGraphicsBuffer[0].shaderResourceView, // Normal
		mGraphicsBuffer[1].shaderResourceView, // PositionWS
		mGraphicsBuffer[2].shaderResourceView, // Diffuse
		mGraphicsBuffer[3].shaderResourceView  // Specular
	};

	DevCon->PSSetShaderResources(0, GBUFFER_COUNT, GBufferTextureViews);

	return true;
}

//DONE
void GraphicsHandler::RenderLightPass(ID3D11Device* Dev, ID3D11DeviceContext* DevCon, IDXGISwapChain* SwapChain)
{
	SetLightPassRenderTargets(Dev, DevCon, SwapChain);
	SetLightPassShaders(DevCon);
	SetLightPassGBuffers(DevCon);
	mLightHandler.BindLightBuffer(DevCon, mCameraHandler.GetCameraPosition());

	// Draw full screen triangle
	DevCon->Draw(3, 0);
}
