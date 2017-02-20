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
	ID3DBlob* pShader,
	LPCWSTR shaderFileName,
	LPCSTR entryPoint,
	LPCSTR shaderModel,
	const D3D_SHADER_MACRO* pDefines,
	ID3DInclude* pInclude,
	UINT Flags1,
	UINT flags2,
	ID3DBlob** ppErrorMsgs)
{
	gHR = D3DCompileFromFile(
		shaderFileName,
		pDefines,
		pInclude,
		entryPoint,
		shaderModel,
		Flags1,
		flags2,
		&pShader,
		ppErrorMsgs
	);
	
	if (FAILED(gHR)) {
		return false;
	} else {
		return true;
	}
}

//temp done
bool GraphicsHandler::CreateInputLayout(ID3DBlob* pVS)
{
	//create input layout (verified using vertex shader)
	D3D11_INPUT_ELEMENT_DESC inputDesc[] = 
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0,	12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0,	20,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	gDevice->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), &mVertexLayout);

	return true;
}

// TODO: Should probably add hr checks. also Relese()
bool GraphicsHandler::InitializeGraphicsBuffer()
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
		gDevice->CreateTexture2D(&textureDesc, NULL, &mGraphicsBuffer[i].texture);
	}

	// Create render target views.
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	for (UINT i = 0; i < GBUFFER_COUNT; i++) {
		gDevice->CreateRenderTargetView(mGraphicsBuffer[i].texture, &renderTargetViewDesc, &mGraphicsBuffer[i].renderTargetView);
	}

	// Create the shader resource views
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	for (UINT i = 0; i < GBUFFER_COUNT; i++) {
		gDevice->CreateShaderResourceView(mGraphicsBuffer[i].texture, &shaderResourceViewDesc, &mGraphicsBuffer[i].shaderResourceView);
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

	gDevice->CreateTexture2D(&depthDesc, NULL, &mDepthStencilTexture);

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	gDevice->CreateDepthStencilView(
		mDepthStencilTexture,	// Depth stencil texture
		&depthStencilViewDesc,	// Depth stencil desc
		&mDepthStecilView);		// [out] Depth stencil view

	//Release

	return true;
}

//will probably be removed
void GraphicsHandler::CreateAllConstantBuffers()
{
	//CreatePerFrameConstantBuffer();


	//CreatePerObjectConstantBuffer();


	//CreateMaterialConstantBuffer();


	//CreateLightConstantBuffer();
	//mLightHandler.CreateLightBuffer();
}

// public ------------------------------------------------------------------------------

bool GraphicsHandler::Initialize()
{
	mCameraHandler.InitializeCamera();

	CreateShaders(); //TODO: rewrite with shader class

	mLightHandler.InitializeLights(mCameraHandler.GetCameraPosition());

	//create world/models

	//create constant buffers

	InitializeGraphicsBuffer();


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
bool GraphicsHandler::CreateShaders()
{
	//---------------------------------- Geometry Pass ----------------------------------------------------

	//compile and create vertex shader
	ID3DBlob* pVS = nullptr;
	CompileShader(pVS, L"GBufferVertex.hlsl", "VS_main", "vs_5_0");

	gHR = gDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &mGeometryPassVertexShader);
	if (FAILED(gHR)) {
		exit(-1);
	}

	//Create an input layout for the vertex shader
	if (!CreateInputLayout(pVS)) {
		exit(-1);
	}

	//compile and create geometry shader
	ID3DBlob* pGS = nullptr;
	CompileShader(pGS, L"GBufferGeometry.hlsl", "GS_main", "gs_5_0");

	gHR = gDevice->CreateGeometryShader(pGS->GetBufferPointer(), pGS->GetBufferSize(), nullptr, &mGeometryPassGeometryShader);
	if (FAILED(gHR)) {
		exit(-1);
	}

	//compile and create pixel shader
	ID3DBlob* pPS = nullptr;
	CompileShader(pPS, L"GBufferFragment.hlsl", "PS_main", "ps_5_0");

	gHR = gDevice->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &mGeometryPassPixelShader);
	if (FAILED(gHR)) {
		exit(-1);
	}

	//---------------------------------- Light Pass ----------------------------------------------------

		//compile and create vertex shader
	ID3DBlob* pVS2 = nullptr;
	CompileShader(pVS2, L"LightVertex.hlsl", "VS_main", "vs_5_0");
	gHR = gDevice->CreateVertexShader(pVS2->GetBufferPointer(), pVS2->GetBufferSize(), nullptr, &mLightPassVertexShader);
	if (FAILED(gHR)) {
		exit(-1);
	}

	//compile and create pixel shader
	ID3DBlob* pPS2 = nullptr;
	CompileShader(pPS2, L"LightFragment.hlsl", "PS_main", "ps_5_0");
	gHR = gDevice->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &mLightPassPixelShader);
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
void GraphicsHandler::SetGeometryPassRenderTargets()
{
	//Clear the render targets
	gDeviceContext->ClearRenderTargetView(mGraphicsBuffer[0].renderTargetView, Colors::fBlack);
	gDeviceContext->ClearRenderTargetView(mGraphicsBuffer[1].renderTargetView, Colors::fLightSteelBlue);
	gDeviceContext->ClearRenderTargetView(mGraphicsBuffer[2].renderTargetView, Colors::fLightSteelBlue);
	gDeviceContext->ClearRenderTargetView(mGraphicsBuffer[3].renderTargetView, Colors::fBlack);
	gDeviceContext->ClearDepthStencilView(mDepthStecilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	//gDeviceContext->ClearDepthStencilView(mDepthStecilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	ID3D11RenderTargetView* renderTargets[] =
	{
		mGraphicsBuffer[0].renderTargetView, /* Normal */
		mGraphicsBuffer[1].renderTargetView, /* PositionWS */
		mGraphicsBuffer[2].renderTargetView, /* Diffuse */
		mGraphicsBuffer[3].renderTargetView  /* Specular */
	};

	//set render targets
	gDeviceContext->OMSetRenderTargets(GBUFFER_COUNT, renderTargets, mDepthStecilView);
}

//TODO: update with correct code from both branches.
void GraphicsHandler::SetGeometryPassShaders()
{
	// Set Vertex Shader input
	gDeviceContext->IASetInputLayout(mVertexLayout);
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set shaders
	gDeviceContext->VSSetShader(mGeometryPassVertexShader, nullptr, 0);
	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	gDeviceContext->GSSetShader(mGeometryPassGeometryShader, nullptr, 0);
	gDeviceContext->PSSetShader(mGeometryPassPixelShader, nullptr, 0);
}

void GraphicsHandler::RenderGeometryPass()
{
	SetGeometryPassRenderTargets();
	SetGeometryPassShaders();
	mCameraHandler.BindPerFrameConstantBuffer();
	//LOOP OVER OBJECTS FROM HERE

	mObjectHandler.SetGeometryPassObjectBuffers();

	gDeviceContext->DrawIndexed(NUMBER_OF_FACES * 3, 0, 0);

	//LOOP OVER OBJECTS TO HERE
}

// ------------------------------ Light Pass ------------------------------------------------------

//TODO: check if pBackBuffer should be declared here or earlier
bool GraphicsHandler::SetLightPassRenderTargets()
{
	// get the address of the back buffer
	ID3D11Texture2D* pBackBuffer = nullptr;
	gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	// use the back buffer address to create the render target
	gDevice->CreateRenderTargetView(pBackBuffer, NULL, &mBackbufferRTV);
	pBackBuffer->Release();

	// set the render target as the back buffer
	gDeviceContext->OMSetRenderTargets(1, &mBackbufferRTV, nullptr);

	//Clear screen
	gDeviceContext->ClearRenderTargetView(mBackbufferRTV, Colors::fWhite);

	return true;
}

//DONE!
bool GraphicsHandler::SetLightPassShaders()
{
	/* Full screen triangle is created in Vertex shader using vertexID so no input layout needed */
	const uintptr_t n0 = 0;
	gDeviceContext->IASetInputLayout(nullptr);
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gDeviceContext->IASetVertexBuffers(0, 0,
		reinterpret_cast<ID3D11Buffer *const *>(&n0),
		reinterpret_cast<const UINT *>(n0),
		reinterpret_cast<const UINT *>(&n0)
	);

	gDeviceContext->VSSetShader(mLightPassVertexShader, nullptr, 0);
	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	gDeviceContext->GSSetShader(nullptr, nullptr, 0);
	gDeviceContext->PSSetShader(mLightPassPixelShader, nullptr, 0);

	return true;
}

//TODO: There might be a memory leak here
bool GraphicsHandler::SetLightPassGBuffers()
{
	ID3D11ShaderResourceView* GBufferTextureViews[] =
	{
		mGraphicsBuffer[0].shaderResourceView, // Normal
		mGraphicsBuffer[1].shaderResourceView, // PositionWS
		mGraphicsBuffer[2].shaderResourceView, // Diffuse
		mGraphicsBuffer[3].shaderResourceView  // Specular
	};

	gDeviceContext->PSSetShaderResources(0, GBUFFER_COUNT, GBufferTextureViews);

	return true;
}

//DONE
void GraphicsHandler::RenderLightPass()
{
	SetLightPassRenderTargets();
	SetLightPassShaders();
	SetLightPassGBuffers();
	mLightHandler.BindLightBuffer(mCameraHandler.GetCameraPosition());

	// Draw full screen triangle
	gDeviceContext->Draw(3, 0);
}
