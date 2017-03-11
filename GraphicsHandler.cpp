/*
* TODO: Write error messages and use if(Function()) instead of just Funtion()
*		Update code with code from the .obj branch
*		See if the order of things should be changed
*		viewport and whatnot
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
	
	if (FAILED(gHR)) 
	{
		OutputDebugString(L"\nGraphicsHandler::CompileShader() Failed to compile shader\n\n");
		return false;
	}
	else
	{
		return true;
	}
}

//temp done
bool GraphicsHandler::CreateInputLayout(ID3D11Device* Dev, ID3DBlob* pVS)
{
	HRESULT hr;
	//create input layout (verified using vertex shader)
	D3D11_INPUT_ELEMENT_DESC inputDesc[] = 
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0,	12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0,	20,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	hr = Dev->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), &mVertexLayout);

	if (FAILED(hr))
	{
		OutputDebugString(L"\nGraphicsHandler::CreateInputLayout() Failed to create input layout\n\n");
		exit(-1);
	}
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

bool GraphicsHandler::CreateRasterizerStates(ID3D11Device* Dev)
{
	HRESULT hr;

	D3D11_RASTERIZER_DESC BFSstate;
	BFSstate.FillMode = D3D11_FILL_SOLID;
	BFSstate.CullMode = D3D11_CULL_NONE;
	BFSstate.FrontCounterClockwise	= FALSE;
	BFSstate.DepthBias				= 0;
	BFSstate.DepthBiasClamp			= 0.0f;
	BFSstate.SlopeScaledDepthBias	= 0.0f;
	BFSstate.DepthClipEnable		= TRUE;
	BFSstate.ScissorEnable			= FALSE;
	BFSstate.MultisampleEnable		= FALSE;
	BFSstate.AntialiasedLineEnable	= FALSE;

	hr = Dev->CreateRasterizerState(&BFSstate, &mRasterizerState[0]);
	if (FAILED(hr)) 
	{
		OutputDebugString(L"\nGraphicsHandler::CreateRasterizerStates() Failed to create first rasterizer state\n\n");
		exit(-1);
	}


	D3D11_RASTERIZER_DESC noBFSstate;
	noBFSstate.FillMode = D3D11_FILL_SOLID;
	noBFSstate.CullMode = D3D11_CULL_BACK;
	noBFSstate.FrontCounterClockwise	= FALSE;
	noBFSstate.DepthBias				= 0;
	noBFSstate.DepthBiasClamp			= 0.0f;
	noBFSstate.SlopeScaledDepthBias		= 0.0f;
	noBFSstate.DepthClipEnable			= TRUE;
	noBFSstate.ScissorEnable			= FALSE;
	noBFSstate.MultisampleEnable		= FALSE;
	noBFSstate.AntialiasedLineEnable	= FALSE;

	hr = Dev->CreateRasterizerState(&noBFSstate, &mRasterizerState[1]);
	if (FAILED(hr))
	{
		OutputDebugString(L"\nGraphicsHandler::CreateRasterizerStates() Failed to create second rasterizer state\n\n");
		exit(-1);
	}
	return true;
}

void GraphicsHandler::SetRasterizerState(ID3D11DeviceContext* DevCon, RenderPassID passID)
{
	if (passID == GEOMETRY_PASS)
	{
		DevCon->RSSetState(mRasterizerState[0]);
	}
	if (passID == SHADOW_PASS)
	{
		DevCon->RSSetState(mRasterizerState[1]);
	}
	if (passID == LIGHT_PASS)
	{
		DevCon->RSSetState(mRasterizerState[1]);
	}
}

// public ------------------------------------------------------------------------------

bool GraphicsHandler::InitializeGraphics(ID3D11Device* Dev, ID3D11DeviceContext* DevCon, ShadowQuality shadowQuality)
{
	mObjectHandler.InitializeObjects(Dev);

	mCameraHandler.InitializeCamera(Dev, DevCon, shadowQuality, mObjectHandler.getWorldDepth(), mObjectHandler.getWorldWidth(), mObjectHandler.getWorldHeight());

	mLightHandler.InitializeLights(Dev, mCameraHandler.GetCameraPosition());

	mLightHandler.CreateShadowMap(Dev, shadowQuality);

	CreateShaders(Dev); //TODO: rewrite with shader class

	CreateRasterizerStates(Dev);

	InitializeGraphicsBuffer(Dev);

	
	quadtree = Quadtree::Quadtree(DirectX::XMVectorSet(0, 0, 0, 0), DirectX::XMVectorSet(mObjectHandler.getWorldWidth(), 200.0f, mObjectHandler.getWorldDepth(), 0), 1);
	quadtree.frustum=FrustumHandler::FrustumHandler(mCameraHandler.getProjection(), mCameraHandler.getView());

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);


	//ID3D11Resource* textureStone;
	//HRESULT hr = DirectX::CreateWICTextureFromFile(Dev, DevCon, L"stone.jpg", &textureStone, &sTextureView);
	//if (FAILED(hr))
	//{
	//	OutputDebugString(L"\nGraphicsHandler::InitializeGraphics() Failed to create WIC stone texture from file\n\n");
	//	exit(-1);
	//}
	//ID3D11Resource* textureGrass;
	// hr = DirectX::CreateWICTextureFromFile(Dev, DevCon, L"grass-free-texture.jpg", &textureGrass, &gTextureView);
	//if (FAILED(hr)) 
	//{
	//	OutputDebugString(L"\nGraphicsHandler::InitializeGraphics() Failed to create WIC grass texture from file\n\n");
	//	exit(-1);
	//}

	ID3D11Resource* textureGrass;
	HRESULT hr = DirectX::CreateDDSTextureFromFile(Dev, DevCon, L"grass.dds", &textureGrass, &gTextureView);
	if (FAILED(hr)) 
	{
		OutputDebugString(L"\nGraphicsHandler::InitializeGraphics() Failed to create DDS grass texture from file\n\n");
		exit(-1);
	}
	ID3D11Resource* textureStone;
	 hr = DirectX::CreateDDSTextureFromFile(Dev, DevCon, L"seamlessstone.dds", &textureStone, &sTextureView);
	if (FAILED(hr))
	{
		OutputDebugString(L"\nGraphicsHandler::InitializeGraphics() Failed to create DDS stone texture from file\n\n");
		exit(-1);
	}
	return true;
}

//TODO: Should probably be rewritten
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
	mShadowPassVertexShader = nullptr;
	mShadowPassPixelShader = nullptr;

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
//		Create a seperate input layout for the shadow pass
bool GraphicsHandler::CreateShaders(ID3D11Device* Dev)
{
	//---------------------------------- Geometry Pass ----------------------------------------------------

	//compile and create vertex shader
	ID3DBlob* pVS = nullptr;
	if (!CompileShader(&pVS, L"GBufferVertex.hlsl", "VS_main", "vs_5_0"))
	{
		OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to compile geometry pass geometry shader\n\n");
		exit(-1);
	}
	HRESULT gHR = Dev->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &mGeometryPassVertexShader);
	if (FAILED(gHR)) 
	{
		OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to create geometry pass vertex shader\n\n");
		exit(-1);
	}

	//Create an input layout for the vertex shader
	CreateInputLayout(Dev, pVS);

	//compile and create geometry shader
	ID3DBlob* pGS = nullptr;
	if (!CompileShader(&pGS, L"GBufferGeometry.hlsl", "GS_main", "gs_5_0"))
	{
		OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to compile geometry pass geometry shader\n\n");
		exit(-1);
	}
	gHR = Dev->CreateGeometryShader(pGS->GetBufferPointer(), pGS->GetBufferSize(), nullptr, &mGeometryPassGeometryShader);
	if (FAILED(gHR)) 
	{
		OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to create geometry pass geometry shader\n\n");
		exit(-1);
	}

	//compile and create pixel shader
	ID3DBlob* pPS = nullptr;
	if (!CompileShader(&pPS, L"GBufferFragment.hlsl", "PS_main", "ps_5_0"))
	{
		OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to compile geometry pass pixel shader\n\n");
		exit(-1);
	}
	gHR = Dev->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &mGeometryPassPixelShader);
	if (FAILED(gHR)) 
	{
		OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to create geometry pass pixel shader\n\n");
		exit(-1);
	}

	//describe and create sampler state
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter	 = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias     = 0.0f;
	samplerDesc.MaxAnisotropy  = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	gHR = Dev->CreateSamplerState(&samplerDesc, &mSampleState);
	if (FAILED(gHR))
	{
		OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to create geometry pass sampler state\n\n");
		exit(-1);
	}


	//---------------------------------- Shadow Pass ----------------------------------------------------

	//compile and create vertex shader
	ID3DBlob* pVS2 = nullptr;
	if (!CompileShader(&pVS2, L"ShadowVertex.hlsl", "VS_main", "vs_5_0"))
	{
		OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to compile shadow pass vertex shader\n\n");
		exit(-1);
	}
	gHR = Dev->CreateVertexShader(pVS2->GetBufferPointer(), pVS2->GetBufferSize(), nullptr, &mShadowPassVertexShader);
	if (FAILED(gHR)) 
	{
		OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to create shadow pass vertex shader\n\n");
		exit(-1);
	}

	//Create an input layout for the shadow vertex shader
	CreateInputLayout(Dev, pVS2);

	//compile and create pixel shader
	ID3DBlob* pPS2 = nullptr;
	if (!CompileShader(&pPS2, L"ShadowFragment.hlsl", "PS_main", "ps_5_0"))
	{
	OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to compile shadow pass pixel shader\n\n");
	exit(-1);
	}
	gHR = Dev->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &mShadowPassPixelShader);
	if (FAILED(gHR)) 
	{
		OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to create shadow pass pixel shader\n\n");
		exit(-1);
	}

	//---------------------------------- Light Pass ----------------------------------------------------

		//compile and create vertex shader
	ID3DBlob* pVS3 = nullptr;
	if (!CompileShader(&pVS3, L"LightVertex.hlsl", "VS_main", "vs_5_0"))
	{
	OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to compile light pass vertex shader\n\n");
	exit(-1);
	}
	gHR = Dev->CreateVertexShader(pVS3->GetBufferPointer(), pVS3->GetBufferSize(), nullptr, &mLightPassVertexShader);
	if (FAILED(gHR)) 
	{
		OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to create light pass vertex shader\n\n");
		exit(-1);
	}

	//compile and create pixel shader
	ID3DBlob* pPS3 = nullptr;
	if (!CompileShader(&pPS3, L"LightFragment.hlsl", "PS_main", "ps_5_0"))
	{
		OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to compile light pass pixel shader\n\n");
		exit(-1);
	}
	gHR = Dev->CreatePixelShader(pPS3->GetBufferPointer(), pPS3->GetBufferSize(), nullptr, &mLightPassPixelShader);
	if (FAILED(gHR)) 
	{
		OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to create light pass pixel shader\n\n");
		exit(-1);
	}

	D3D11_SAMPLER_DESC shadowSamplerDesc{};
	shadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.MipLODBias = 0.0f;
	shadowSamplerDesc.MaxAnisotropy = 16;
	shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSamplerDesc.BorderColor[0] = 1.0f;
	shadowSamplerDesc.BorderColor[1] = 1.0f;
	shadowSamplerDesc.BorderColor[2] = 1.0f;
	shadowSamplerDesc.BorderColor[3] = 1.0f;
	shadowSamplerDesc.MinLOD = 0.0f;
	shadowSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		// Create the texture sampler state.
	gHR = Dev->CreateSamplerState(&shadowSamplerDesc, &mShadowSampler);
	if (FAILED(gHR)) 
	{
		OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to create shadow pass sampler state\n\n");
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
	DevCon->ClearRenderTargetView(mGraphicsBuffer[0].renderTargetView, Colors::fBlack);				//Normal
	DevCon->ClearRenderTargetView(mGraphicsBuffer[1].renderTargetView, Colors::fLightSteelBlue);	//Position
	DevCon->ClearRenderTargetView(mGraphicsBuffer[2].renderTargetView, Colors::fBlack);				//Diffuse
	DevCon->ClearRenderTargetView(mGraphicsBuffer[3].renderTargetView, Colors::fBlack);				//Specular
	DevCon->ClearDepthStencilView(mDepthStecilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	//DevCon->ClearDepthStencilView(mDepthStecilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//construct frustum again every frame
	quadtree.frustum = FrustumHandler::FrustumHandler(mCameraHandler.getProjection(), mCameraHandler.getView());

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
	DevCon->PSSetSamplers(0, 1, &mSampleState);
}

void GraphicsHandler::SetGeometryPassShaderResources(ID3D11DeviceContext* DevCon)
{
	DevCon->PSSetShaderResources(1,1, &gTextureView);
	DevCon->PSSetShaderResources(2, 1, &sTextureView);
}

void GraphicsHandler::RenderGeometryPass(ID3D11DeviceContext* DevCon)
{
	std::vector<Object>* objectArray = nullptr;

	DevCon->RSSetViewports(1, &mCameraHandler.playerVP);

	SetGeometryPassRenderTargets(DevCon);
	SetGeometryPassShaders(DevCon);
	SetRasterizerState(DevCon, GEOMETRY_PASS);
	mCameraHandler.BindPerFrameConstantBuffer(DevCon);

	// ------------------------------ Height Map ------------------------------------------------------
	SetGeometryPassShaderResources(DevCon);
	mObjectHandler.SetHeightMapBuffer(DevCon, GEOMETRY_PASS);
	DevCon->DrawIndexed(mObjectHandler.GetHeightMapNrOfFaces() * 3, 0, 0);

	// ------------------------------ Static Objects ------------------------------------------------------
	// NOTE: Quad-tree stuff goes here
	objectArray = mObjectHandler.GetObjectArrayPtr(STATIC_OBJECT);
	for (size_t i = 0; i < (*objectArray).size(); i++)
	{
		for (int j = 0; j < (*objectArray)[i].GetNrOfMeshSubsets(); j++)
		{
			mObjectHandler.SetObjectBufferWithIndex(DevCon, GEOMETRY_PASS, STATIC_OBJECT, i, j);

			int indexStart = (*objectArray)[i].meshSubsetIndexStart[j];
			int indexDrawAmount = (*objectArray)[i].meshSubsetIndexStart[j + 1] - indexStart;
			//int indexStart = mObjectHandler.meshSubsetIndexStart[i];
			//int indexDrawAmount = mObjectHandler.meshSubsetIndexStart[i + 1] - indexStart;

			DevCon->DrawIndexed(indexDrawAmount, indexStart, 0);
		}
	}

	// ------------------------------ Dynamic Objects ------------------------------------------------------
	// NOTE: Quad-tree stuff goes here
	objectArray = mObjectHandler.GetObjectArrayPtr(DYNAMIC_OBJECT);
	for (size_t i = 0; i < (*objectArray).size(); i++)
	{
		for (int j = 0; j < (*objectArray)[i].GetNrOfMeshSubsets(); j++)
		{
			mObjectHandler.SetObjectBufferWithIndex(DevCon, GEOMETRY_PASS, DYNAMIC_OBJECT, i, j);

			int indexStart = (*objectArray)[i].meshSubsetIndexStart[j];
			int indexDrawAmount = (*objectArray)[i].meshSubsetIndexStart[j + 1] - indexStart;
			//int indexStart = mObjectHandler.meshSubsetIndexStart[i];
			//int indexDrawAmount = mObjectHandler.meshSubsetIndexStart[i + 1] - indexStart;

			DevCon->DrawIndexed(indexDrawAmount, indexStart, 0);
		}
	}
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
	//doesn't need any resources since it only cares about depth.
	//remove this function later
}

//TODO: set the position and whatnot for the light
void GraphicsHandler::RenderShadowPass(ID3D11DeviceContext* DevCon)
{
	std::vector<Object>* objectArray = nullptr;

	DevCon->RSSetViewports(1, &mCameraHandler.lightVP);
	SetShadowMapPassRenderTargets(DevCon);
	SetShadowMapPassShaders(DevCon);
	SetRasterizerState(DevCon, SHADOW_PASS);
	mCameraHandler.BindShadowMapPerFrameConstantBuffer(DevCon, SHADOW_PASS);

	// ------------------------------ Height Map -----------------------------------------------------
	SetShadowMapPassShaderResources(DevCon); // Currently does nothing
	mObjectHandler.SetHeightMapBuffer(DevCon, SHADOW_PASS);
	DevCon->DrawIndexed(mObjectHandler.GetHeightMapNrOfFaces() * 3, 0, 0);

	// ------------------------------ Static Objects ------------------------------------------------------
	// NOTE: Quad-tree stuff goes here
	objectArray = mObjectHandler.GetObjectArrayPtr(STATIC_OBJECT);
	for (size_t i = 0; i < (*objectArray).size(); i++)
	{
		for (int j = 0; j < (*objectArray)[i].GetNrOfMeshSubsets(); j++)
		{
			mObjectHandler.SetObjectBufferWithIndex(DevCon, SHADOW_PASS, STATIC_OBJECT, i, j);

			int indexStart = (*objectArray)[i].meshSubsetIndexStart[j];
			int indexDrawAmount = (*objectArray)[i].meshSubsetIndexStart[j + 1] - indexStart;
			//int indexStart = mObjectHandler.meshSubsetIndexStart[i];
			//int indexDrawAmount = mObjectHandler.meshSubsetIndexStart[i + 1] - indexStart;

			DevCon->DrawIndexed(indexDrawAmount, indexStart, 0);
		}
	}

	// ------------------------------ Dynamic Objects ------------------------------------------------------
	// NOTE: Quad-tree stuff goes here
	objectArray = mObjectHandler.GetObjectArrayPtr(DYNAMIC_OBJECT);
	for (size_t i = 0; i < (*objectArray).size(); i++)
	{
		for (int j = 0; j < (*objectArray)[i].GetNrOfMeshSubsets(); j++)
		{
			mObjectHandler.SetObjectBufferWithIndex(DevCon, SHADOW_PASS, DYNAMIC_OBJECT, i, j);

			int indexStart = (*objectArray)[i].meshSubsetIndexStart[j];
			int indexDrawAmount = (*objectArray)[i].meshSubsetIndexStart[j + 1] - indexStart;
			//int indexStart = mObjectHandler.meshSubsetIndexStart[i];
			//int indexDrawAmount = mObjectHandler.meshSubsetIndexStart[i + 1] - indexStart;

			DevCon->DrawIndexed(indexDrawAmount, indexStart, 0);
		}
	}
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
	DevCon->PSSetSamplers(0, 1, &mShadowSampler);

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
	DevCon->RSSetViewports(1, &mCameraHandler.playerVP);
	SetLightPassRenderTargets(Dev, DevCon, SwapChain);
	SetLightPassShaders(DevCon);
	SetLightPassGBuffers(DevCon);
	SetRasterizerState(DevCon, LIGHT_PASS);
	mCameraHandler.BindShadowMapPerFrameConstantBuffer(DevCon, LIGHT_PASS);
	
	DevCon->PSSetShaderResources(4, 1, &mLightHandler.mShadowMapSRView);

	mLightHandler.BindLightBuffer(DevCon, mCameraHandler.GetCameraPosition());


	// Draw full screen triangle
	DevCon->Draw(3, 0);
}
