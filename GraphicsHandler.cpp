/*
* TODO: Write error messages and use if(Function()) instead of just Funtion()
*		Update code with code from the .obj branch
*		See if the order of things should be changed
*		viewport and whatnot
*
* TODO? Instead of having one SetRenderPassWhatever() function per render pass have them all use 
*		the same one with RenderPassID as an input
*/

#include "GraphicsHandler.hpp"

// private ------------------------------------------------------------------------------
// MOVED TO SHADERHANDLER
//done for now, will add comments to this function later
//bool GraphicsHandler::CompileShader(
//	ID3DBlob** pShader,
//	LPCWSTR shaderFileName,
//	LPCSTR entryPoint,
//	LPCSTR shaderModel,
//	const D3D_SHADER_MACRO* pDefines,
//	ID3DInclude* pInclude,
//	UINT Flags1,
//	UINT flags2,
//	ID3DBlob** ppErrorMsgs)
//{
//	HRESULT gHR = D3DCompileFromFile(
//		shaderFileName,
//		pDefines,
//		pInclude,
//		entryPoint,
//		shaderModel,
//		Flags1,
//		flags2,
//		pShader,
//		ppErrorMsgs
//	);
//	
//	if (FAILED(gHR)) 
//	{
//		OutputDebugString(L"\nGraphicsHandler::CompileShader() Failed to compile shader\n\n");
//		return false;
//	}
//	else
//	{
//		return true;
//	}
//}

//MOVED TO SHADERHANDLER
//temp done
//bool GraphicsHandler::CreateInputLayout(ID3D11Device* Dev, ID3DBlob* pVS)
//{
//	HRESULT hr;
//	//create input layout (verified using vertex shader)
//	D3D11_INPUT_ELEMENT_DESC inputDesc[] = 
//	{
//		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0,	12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0,	20,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
//	};
//
//	hr = Dev->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), &mVertexLayout);
//
//	if (FAILED(hr))
//	{
//		OutputDebugString(L"\nGraphicsHandler::CreateInputLayout() Failed to create input layout\n\n");
//		exit(-1);
//	}
//	return true;
//}

// MOVED TO SHADERHANDLER
// TODO: Should probably add hr checks. also Relese()
//bool GraphicsHandler::InitializeGraphicsBuffer(ID3D11Device* Dev)
//{
//	// Create render target textures
//	D3D11_TEXTURE2D_DESC textureDesc{};
//	textureDesc.Width = SCREEN_RESOLUTION.SCREEN_WIDTH;
//	textureDesc.Height = SCREEN_RESOLUTION.SCREEN_HEIGHT;
//	textureDesc.MipLevels = 1;
//	textureDesc.ArraySize = 1;
//	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
//	textureDesc.SampleDesc.Count = 1;
//	textureDesc.SampleDesc.Quality = 0;
//	textureDesc.Usage = D3D11_USAGE_DEFAULT;
//	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
//	//textureDesc.CPUAccessFlags = 0;
//	//textureDesc.MiscFlags = 0;
//
//	for (UINT i = 0; i < GBUFFER_COUNT; i++) {
//		Dev->CreateTexture2D(&textureDesc, NULL, &mGraphicsBuffer[i].texture);
//	}
//
//	// Create render target views.
//	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
//	renderTargetViewDesc.Format = textureDesc.Format;
//	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
//
//	for (UINT i = 0; i < GBUFFER_COUNT; i++) {
//		Dev->CreateRenderTargetView(mGraphicsBuffer[i].texture, &renderTargetViewDesc, &mGraphicsBuffer[i].renderTargetView);
//	}
//
//	// Create the shader resource views
//	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
//	shaderResourceViewDesc.Format = textureDesc.Format;
//	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
//	shaderResourceViewDesc.Texture2D.MipLevels = 1;
//
//	for (UINT i = 0; i < GBUFFER_COUNT; i++) {
//		Dev->CreateShaderResourceView(mGraphicsBuffer[i].texture, &shaderResourceViewDesc, &mGraphicsBuffer[i].shaderResourceView);
//	}
//
//	//Create the depth stencil buffer texture
//	D3D11_TEXTURE2D_DESC depthDesc{};
//	depthDesc.Width = SCREEN_RESOLUTION.SCREEN_WIDTH;
//	depthDesc.Height = SCREEN_RESOLUTION.SCREEN_HEIGHT;
//	depthDesc.MipLevels = 1;
//	depthDesc.ArraySize = 1;
//	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
//	depthDesc.SampleDesc.Count = 1;
//	depthDesc.Usage = D3D11_USAGE_DEFAULT;
//	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
//
//	Dev->CreateTexture2D(&depthDesc, NULL, &mDepthStencilTexture);
//
//	// Create the depth stencil view
//	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
//	depthStencilViewDesc.Format = depthDesc.Format;
//	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
//
//	Dev->CreateDepthStencilView(
//		mDepthStencilTexture,	// Depth stencil texture
//		&depthStencilViewDesc,	// Depth stencil desc
//		&mDepthStecilView);		// [out] Depth stencil view
//
//	//Release
//
//	return true;
//}

//MOVED TO SHADERHANDLER
//TODO: Look through settings again before handing in
//bool GraphicsHandler::CreateRasterizerStates(ID3D11Device* Dev)
//{
//	HRESULT hr;
//
//	D3D11_RASTERIZER_DESC BFSstate;
//	BFSstate.FillMode = D3D11_FILL_SOLID;
//	BFSstate.CullMode = D3D11_CULL_NONE;
//	BFSstate.FrontCounterClockwise	= FALSE;
//	BFSstate.DepthBias				= 0;
//	BFSstate.DepthBiasClamp			= 0.0f;
//	BFSstate.SlopeScaledDepthBias	= 0.0f;
//	BFSstate.DepthClipEnable		= TRUE;
//	BFSstate.ScissorEnable			= FALSE;
//	BFSstate.MultisampleEnable		= FALSE;
//	BFSstate.AntialiasedLineEnable	= FALSE;
//
//	hr = Dev->CreateRasterizerState(&BFSstate, &mRasterizerState[0]);
//	if (FAILED(hr)) 
//	{
//		OutputDebugString(L"\nGraphicsHandler::CreateRasterizerStates() Failed to create first rasterizer state\n\n");
//		exit(-1);
//	}
//
//	D3D11_RASTERIZER_DESC noBFSstate;
//	noBFSstate.FillMode = D3D11_FILL_SOLID;
//	noBFSstate.CullMode = D3D11_CULL_BACK;
//	noBFSstate.FrontCounterClockwise	= FALSE;
//	noBFSstate.DepthBias				= 0;
//	noBFSstate.DepthBiasClamp			= 0.0f;
//	noBFSstate.SlopeScaledDepthBias		= 0.0f;
//	noBFSstate.DepthClipEnable			= TRUE;
//	noBFSstate.ScissorEnable			= FALSE;
//	noBFSstate.MultisampleEnable		= FALSE;
//	noBFSstate.AntialiasedLineEnable	= FALSE;
//
//	hr = Dev->CreateRasterizerState(&noBFSstate, &mRasterizerState[1]);
//	if (FAILED(hr))
//	{
//		OutputDebugString(L"\nGraphicsHandler::CreateRasterizerStates() Failed to create second rasterizer state\n\n");
//		exit(-1);
//	}
//	return true;
//}

// MOVED TO SHADERHANDLER
//void GraphicsHandler::SetRasterizerState(ID3D11DeviceContext* DevCon, RenderPassID passID)
//{
//	if (passID == GEOMETRY_PASS)
//	{
//		DevCon->RSSetState(mRasterizerState[0]);
//	}
//	if (passID == SHADOW_PASS)
//	{
//		DevCon->RSSetState(mRasterizerState[1]);
//	}
//	if (passID == LIGHT_PASS)
//	{
//		DevCon->RSSetState(mRasterizerState[1]);
//	}
//	if (passID == COMPUTE_PASS)
//	{
//		DevCon->RSSetState(mRasterizerState[1]);
//	}
//}

// public ------------------------------------------------------------------------------

ID3D11RenderTargetView** GraphicsHandler::GetBackBufferRTV()
{
	return mShaderHandler.GetBackBufferRTV();
}

bool GraphicsHandler::InitializeGraphics(ID3D11Device* Dev, ID3D11DeviceContext* DevCon)
{
	mObjectHandler.InitializeObjects(Dev, DevCon);

	mCameraHandler.InitializeCamera(Dev, DevCon, mObjectHandler.getWorldDepth(), mObjectHandler.getWorldWidth(), mObjectHandler.getWorldHeight());

	mLightHandler.InitializeLights(Dev, mCameraHandler.GetCameraPosition());

	//mLightHandler.CreateShadowMap(Dev);

	//CreateShaders(Dev); //TODO: rewrite with shader class
	mShaderHandler.InitializeShaders(Dev);

	//mComputeShader.CreateRenderTextures(Dev);

	//CreateRasterizerStates(Dev);

	//InitializeGraphicsBuffer(Dev);

	mObjectHandler.mQuadtree.frustum = FrustumHandler(mCameraHandler.getProjection(), mCameraHandler.getView());


	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	//TODO: move to function
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
	: mCameraHandler(), mLightHandler(mCameraHandler.GetCameraPosition()), mShaderHandler()
{
	/*for (int i = 0; i < GBUFFER_COUNT; i++)
	{
		mGraphicsBuffer[i] = GraphicsBuffer();
	}*/
	//mDepthStecilView = nullptr;
	//mDepthStencilTexture = nullptr;
	//mVertexLayout = nullptr;
	//mGeometryPassVertexShader = nullptr;
	//mGeometryPassGeometryShader = nullptr;
	//mGeometryPassPixelShader = nullptr;
	////mTextureView = nullptr;
	//mShadowPassVertexShader = nullptr;
	//mShadowPassPixelShader = nullptr;

	//mLightPassVertexShader = nullptr;
	//mLightPassPixelShader = nullptr;
	//mBackbufferRTV = nullptr;


	//mCameraHandler = CameraHandler();
	//mLightHandler = LightHandler(mCameraHandler.GetCameraPosition());
}

GraphicsHandler::~GraphicsHandler()
{
	
}

// MOVED TO SHADERHANDLER
//TODO: clean up and write better error messages
//		Create a seperate input layout for the shadow pass
//bool GraphicsHandler::CreateShaders(ID3D11Device* Dev)
//{
	////---------------------------------- Geometry Pass ----------------------------------------------------
	////compile and create vertex shader
	//ID3DBlob* pVS = nullptr;
	//if (!CompileShader(&pVS, L"GBufferVertex.hlsl", "VS_main", "vs_5_0"))
	//{
	//	OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to compile geometry pass geometry shader\n\n");
	//	exit(-1);
	//}
	//HRESULT gHR = Dev->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &mGeometryPassVertexShader);
	//if (FAILED(gHR)) 
	//{
	//	OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to create geometry pass vertex shader\n\n");
	//	exit(-1);
	//}
	////Create an input layout for the vertex shader
	//CreateInputLayout(Dev, pVS);
	////compile and create geometry shader
	//ID3DBlob* pGS = nullptr;
	//if (!CompileShader(&pGS, L"GBufferGeometry.hlsl", "GS_main", "gs_5_0"))
	//{
	//	OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to compile geometry pass geometry shader\n\n");
	//	exit(-1);
	//}
	//gHR = Dev->CreateGeometryShader(pGS->GetBufferPointer(), pGS->GetBufferSize(), nullptr, &mGeometryPassGeometryShader);
	//if (FAILED(gHR)) 
	//{
	//	OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to create geometry pass geometry shader\n\n");
	//	exit(-1);
	//}
	////compile and create pixel shader
	//ID3DBlob* pPS = nullptr;
	//if (!CompileShader(&pPS, L"GBufferFragment.hlsl", "PS_main", "ps_5_0"))
	//{
	//	OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to compile geometry pass pixel shader\n\n");
	//	exit(-1);
	//}
	//gHR = Dev->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &mGeometryPassPixelShader);
	//if (FAILED(gHR)) 
	//{
	//	OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to create geometry pass pixel shader\n\n");
	//	exit(-1);
	//}
	////compile and create pixel heightmap shader
	//D3D_SHADER_MACRO HeightMapMacros[] =
	//{
	//	"HEIGHT_MAP",  "1",
	//	NULL, NULL
	//};
	//ID3DBlob* pPHMS = nullptr;
	//if (!CompileShader(&pPHMS, L"GBufferFragment.hlsl", "PS_main", "ps_5_0", HeightMapMacros))
	//{
	//	OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to compile geometry pass pixel shader\n\n");
	//	exit(-1);
	//}
	//gHR = Dev->CreatePixelShader(pPHMS->GetBufferPointer(), pPHMS->GetBufferSize(), nullptr, &mGeometryPassPixelHeightMapShader);
	//if (FAILED(gHR))
	//{
	//	OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to create geometry pass pixel shader\n\n");
	//	exit(-1);
	//}
	////describe and create sampler state
	//D3D11_SAMPLER_DESC samplerDesc;
	//samplerDesc.Filter	 = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	//samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	//samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	//samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	//samplerDesc.MipLODBias     = 0.0f;
	//samplerDesc.MaxAnisotropy  = 1;
	//samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	//samplerDesc.BorderColor[0] = 0;
	//samplerDesc.BorderColor[1] = 0;
	//samplerDesc.BorderColor[2] = 0;
	//samplerDesc.BorderColor[3] = 0;
	//samplerDesc.MinLOD = 0;
	//samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	//// Create the texture sampler state.
	//gHR = Dev->CreateSamplerState(&samplerDesc, &mSampleState);
	//if (FAILED(gHR))
	//{
	//	OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to create geometry pass sampler state\n\n");
	//	exit(-1);
	//}
	////---------------------------------- Shadow Pass ----------------------------------------------------
	////compile and create vertex shader
	//ID3DBlob* pVS2 = nullptr;
	//if (!CompileShader(&pVS2, L"ShadowVertex.hlsl", "VS_main", "vs_5_0"))
	//{
	//	OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to compile shadow pass vertex shader\n\n");
	//	exit(-1);
	//}
	//gHR = Dev->CreateVertexShader(pVS2->GetBufferPointer(), pVS2->GetBufferSize(), nullptr, &mShadowPassVertexShader);
	//if (FAILED(gHR)) 
	//{
	//	OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to create shadow pass vertex shader\n\n");
	//	exit(-1);
	//}
	////Create an input layout for the shadow vertex shader
	//CreateInputLayout(Dev, pVS2);
	////compile and create pixel shader
	//ID3DBlob* pPS2 = nullptr;
	//if (!CompileShader(&pPS2, L"ShadowFragment.hlsl", "PS_main", "ps_5_0"))
	//{
	//OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to compile shadow pass pixel shader\n\n");
	//exit(-1);
	//}
	//gHR = Dev->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &mShadowPassPixelShader);
	//if (FAILED(gHR)) 
	//{
	//	OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to create shadow pass pixel shader\n\n");
	//	exit(-1);
	//}
	////---------------------------------- Light Pass ----------------------------------------------------
	//	//compile and create vertex shader
	//ID3DBlob* pVS3 = nullptr;
	//if (!CompileShader(&pVS3, L"LightVertex.hlsl", "VS_main", "vs_5_0"))
	//{
	//OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to compile light pass vertex shader\n\n");
	//exit(-1);
	//}
	//gHR = Dev->CreateVertexShader(pVS3->GetBufferPointer(), pVS3->GetBufferSize(), nullptr, &mLightPassVertexShader);
	//if (FAILED(gHR)) 
	//{
	//	OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to create light pass vertex shader\n\n");
	//	exit(-1);
	//}
	//D3D_SHADER_MACRO LightPassFragmentMacros[] =
	//{
	//	"SHADOW_MAP_SIZE",  SHADOW_QUALITY.SIZE_STRING,
	//	NULL, NULL
	//};
	////compile and create pixel shader
	//ID3DBlob* pPS3 = nullptr;
	//if (!CompileShader(&pPS3, L"LightFragment.hlsl", "PS_main", "ps_5_0", LightPassFragmentMacros))
	//{
	//	OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to compile light pass pixel shader\n\n");
	//	exit(-1);
	//}
	//gHR = Dev->CreatePixelShader(pPS3->GetBufferPointer(), pPS3->GetBufferSize(), nullptr, &mLightPassPixelShader);
	//if (FAILED(gHR)) 
	//{
	//	OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to create light pass pixel shader\n\n");
	//	exit(-1);
	//}
	//D3D11_SAMPLER_DESC shadowSamplerDesc{};
	//shadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	//shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	//shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	//shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	//shadowSamplerDesc.MipLODBias = 0.0f;
	//shadowSamplerDesc.MaxAnisotropy = 16;
	//shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	//shadowSamplerDesc.BorderColor[0] = 1.0f;
	//shadowSamplerDesc.BorderColor[1] = 1.0f;
	//shadowSamplerDesc.BorderColor[2] = 1.0f;
	//shadowSamplerDesc.BorderColor[3] = 1.0f;
	//shadowSamplerDesc.MinLOD = 0.0f;
	//shadowSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	//	// Create the texture sampler state.
	//gHR = Dev->CreateSamplerState(&shadowSamplerDesc, &mShadowSampler);
	//if (FAILED(gHR)) 
	//{
	//	OutputDebugString(L"\nGraphicsHandler::CreateShaders() Failed to create shadow pass sampler state\n\n");
	//	exit(-1);
	//}
	////Clean up
	//pVS->Release();
	//pGS->Release();
	//pPS->Release();
	//pVS2->Release();
	//pPS2->Release();
	////---------------------------------- Compute Pass ----------------------------------------------------
	//
	//mComputeShader.CreateComputePassShaders(Dev);
	//return true;
//}

// ------------------------------ Geometry Pass ------------------------------------------------------
// MOVED TO SHADERHANDLER
//temp done
//void GraphicsHandler::SetGeometryPassRenderTargets(ID3D11DeviceContext* DevCon)
//{
//	//Clear the render targets
//	DevCon->ClearRenderTargetView(mGraphicsBuffer[0].renderTargetView, Colors::fBlack);				//Normal
//	DevCon->ClearRenderTargetView(mGraphicsBuffer[1].renderTargetView, Colors::fLightSteelBlue);	//Position
//	DevCon->ClearRenderTargetView(mGraphicsBuffer[2].renderTargetView, Colors::fBlack);				//Diffuse
//	DevCon->ClearRenderTargetView(mGraphicsBuffer[3].renderTargetView, Colors::fBlack);				//Specular
//	DevCon->ClearDepthStencilView(mDepthStecilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
//	//DevCon->ClearDepthStencilView(mDepthStecilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
//
//	//construct frustum again every frame
//	mObjectHandler.mQuadtree.frustum = FrustumHandler(mCameraHandler.getProjection(), mCameraHandler.getView());
//
//	ID3D11RenderTargetView* renderTargets[] =
//	{
//		mGraphicsBuffer[0].renderTargetView, /* Normal */
//		mGraphicsBuffer[1].renderTargetView, /* PositionWS */
//		mGraphicsBuffer[2].renderTargetView, /* Diffuse */
//		mGraphicsBuffer[3].renderTargetView  /* Specular */
//	};
//
//	//set render targets
//	DevCon->OMSetRenderTargets(GBUFFER_COUNT, renderTargets, mDepthStecilView);
//}

// MOVED TO SHADERHANDLER
//TODO: update with correct code from both branches.
//void GraphicsHandler::SetGeometryPassShaders(ID3D11DeviceContext* DevCon, bool isHeightMap)
//{
//	// Set Vertex Shader input
//	DevCon->IASetInputLayout(mVertexLayout);
//	DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	// Set shaders
//	DevCon->VSSetShader(mGeometryPassVertexShader, nullptr, 0);
//	DevCon->HSSetShader(nullptr, nullptr, 0);
//	DevCon->DSSetShader(nullptr, nullptr, 0);
//	DevCon->GSSetShader(mGeometryPassGeometryShader, nullptr, 0);
//	if (!isHeightMap)
//	{
//		DevCon->PSSetShader(mGeometryPassPixelShader, nullptr, 0);
//	} else {
//		DevCon->PSSetShader(mGeometryPassPixelHeightMapShader, nullptr, 0);
//	}
//	
//	DevCon->PSSetSamplers(0, 1, &mSampleState);
//}

void GraphicsHandler::SetHeightMapShaderResources(ID3D11DeviceContext* DevCon, bool isHeightMap)
{
	if (isHeightMap)
	{
		DevCon->PSSetShaderResources(1, 1, &gTextureView);
		DevCon->PSSetShaderResources(2, 1, &sTextureView);
	}
}

void GraphicsHandler::RenderGeometryPass(ID3D11DeviceContext* DevCon)
{
	//construct frustum again every frame
	mObjectHandler.mQuadtree.frustum = FrustumHandler(mCameraHandler.getProjection(), mCameraHandler.getView());

	std::vector<Object>* objectArray = nullptr;

	DevCon->RSSetViewports(1, &mCameraHandler.playerVP);

	///SetGeometryPassRenderTargets(DevCon);
	//construct frustum again every frame
	mObjectHandler.mQuadtree.frustum = FrustumHandler(mCameraHandler.getProjection(), mCameraHandler.getView());
	//SetRasterizerState(DevCon, GEOMETRY_PASS); //TODO: move to shaderhandler
	mCameraHandler.BindPerFrameConstantBuffer(DevCon);

	// ------------------------------ Height Map ------------------------------------------------------
	
	
	mShaderHandler.PrepareRender(DevCon, GEOMETRY_PASS, true, true);
	
	//SetGeometryPassShaders(DevCon, true);
	SetHeightMapShaderResources(DevCon, true); //TODO: Move to shaderhandler
	mObjectHandler.SetHeightMapBuffer(DevCon, GEOMETRY_PASS);
	DevCon->DrawIndexed(mObjectHandler.GetHeightMapNrOfFaces() * 3, 0, 0);

	// ------------------------------ Static Objects ------------------------------------------------------
	// NOTE: Quad-tree stuff goes here
	mShaderHandler.PrepareRender(DevCon, GEOMETRY_PASS, false);

	//SetGeometryPassShaders(DevCon, false);
	//SetHeightMapShaderResources(DevCon, false);
	objectArray = mObjectHandler.GetObjectArrayPtr(STATIC_OBJECT);

	// --------------------- RENDER ALL OBJECTS. NO TEST AGAINST QUADTREE ----------------------------------------
	//for (size_t i = 0; i < (*objectArray).size(); i++)
	//{
	//	for (int j = 0; j < (*objectArray)[i].GetNrOfMeshSubsets(); j++)
	//	{
	//		mObjectHandler.SetObjectBufferWithIndex(DevCon, GEOMETRY_PASS, STATIC_OBJECT, i, j);

	//		int indexStart = (*objectArray)[i].meshSubsetIndexStart[j];
	//		int indexDrawAmount = (*objectArray)[i].meshSubsetIndexStart[j + 1] - indexStart;
	//		//int indexStart = mObjectHandler.meshSubsetIndexStart[i];
	//		//int indexDrawAmount = mObjectHandler.meshSubsetIndexStart[i + 1] - indexStart;

	//		DevCon->DrawIndexed(indexDrawAmount, indexStart, 0);
	//	}
	//}
	// --------------------- END RENDER ALL OBJECTS. NO TEST AGAINST QUADTREE ----------------------------------------


	int objInd;
	std::vector<UINT> toDrawIndexes = mObjectHandler.mQuadtree.getObjects(mObjectHandler.mQuadtree.root);
	
	// TODO! Do this in the getObjects function
	// TEMPORARY SOLUTION TO DUPLICATE DRAWS
	std::sort(toDrawIndexes.begin(), toDrawIndexes.end());
	toDrawIndexes.erase(std::unique(toDrawIndexes.begin(), toDrawIndexes.end()), toDrawIndexes.end());
	// END TEMPORARY SOLUTION TO DUPLICATE DRAWS

	for (size_t i = 0; i < toDrawIndexes.size(); i++)
	{
		objInd = toDrawIndexes[i];
		for (int j = 0; j < (*objectArray)[objInd].GetNrOfMeshSubsets(); j++)
		{
			mObjectHandler.SetObjectBufferWithIndex(DevCon, GEOMETRY_PASS, STATIC_OBJECT, objInd, j);

			int indexStart = (*objectArray)[objInd].meshSubsetIndexStart[j];
			int indexDrawAmount = (*objectArray)[objInd].meshSubsetIndexStart[j + 1] - indexStart;

			DevCon->DrawIndexed(indexDrawAmount, indexStart, 0);
		}
	}

	// ------------------------------ Dynamic Objects ------------------------------------------------------
	// NOTE: Quad-tree stuff does NOT go here
	objectArray = mObjectHandler.GetObjectArrayPtr(DYNAMIC_OBJECT);
	for (size_t i = 0; i < (*objectArray).size(); i++)
	{
		for (int j = 0; j < (*objectArray)[i].GetNrOfMeshSubsets(); j++)
		{
			mObjectHandler.SetObjectBufferWithIndex(DevCon, GEOMETRY_PASS, DYNAMIC_OBJECT, i, j);

			int indexStart = (*objectArray)[i].meshSubsetIndexStart[j];
			int indexDrawAmount = (*objectArray)[i].meshSubsetIndexStart[j + 1] - indexStart;

			DevCon->DrawIndexed(indexDrawAmount, indexStart, 0);
		}
	}
}

// ------------------------------ Shadow Map Pass ------------------------------------------------------

// MOVED TO SHADERHANDLER
//TODO: move to LightHandler.cpp
//void GraphicsHandler::SetShadowMapPassRenderTargets(ID3D11DeviceContext* DevCon)
//{
//	DevCon->OMSetRenderTargets(0, 0, mLightHandler.mShadowMapDepthView);
//	DevCon->ClearDepthStencilView(mLightHandler.mShadowMapDepthView, D3D11_CLEAR_DEPTH, 1.0f, 0);
//}

// MOVED TO SHADERHANDLER
//TODO: make a seperate input layout for the shadow pass since it only needs the position
//void GraphicsHandler::SetShadowMapPassShaders(ID3D11DeviceContext* DevCon)
//{
//	// Set Vertex Shader input
//	DevCon->IASetInputLayout(mVertexLayout);
//	DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	// Set shaders
//	DevCon->VSSetShader(mShadowPassVertexShader, nullptr, 0);
//	DevCon->HSSetShader(nullptr, nullptr, 0);
//	DevCon->DSSetShader(nullptr, nullptr, 0);
//	DevCon->GSSetShader(nullptr, nullptr, 0);
//	DevCon->PSSetShader(mShadowPassPixelShader, nullptr, 0);
//}

//void GraphicsHandler::SetShadowMapPassShaderResources(ID3D11DeviceContext* DevCon)
//{
//	//doesn't need any resources since it only cares about depth.
//	//remove this function later
//}

//TODO: set the position and whatnot for the light
void GraphicsHandler::RenderShadowPass(ID3D11DeviceContext* DevCon)
{
	std::vector<Object>* objectArray = nullptr;

	DevCon->RSSetViewports(1, &mCameraHandler.lightVP);
	
	mShaderHandler.PrepareRender(DevCon, SHADOW_PASS);
	
	//SetShadowMapPassRenderTargets(DevCon);
	//SetShadowMapPassShaders(DevCon);
	//SetRasterizerState(DevCon, SHADOW_PASS);
	mCameraHandler.BindShadowMapPerFrameConstantBuffer(DevCon, SHADOW_PASS);

	// ------------------------------ Height Map -----------------------------------------------------
	//SetShadowMapPassShaderResources(DevCon); // Currently does nothing
	mObjectHandler.SetHeightMapBuffer(DevCon, SHADOW_PASS);
	DevCon->DrawIndexed(mObjectHandler.GetHeightMapNrOfFaces() * 3, 0, 0);


	// TODO: Change the static object loop to a quadtree one
	// ------------------------------ Static Objects ------------------------------------------------------
	// NOTE: Quad-tree stuff does NOT go here
	objectArray = mObjectHandler.GetObjectArrayPtr(STATIC_OBJECT);
	for (size_t i = 0; i < (*objectArray).size(); i++)
	{
		for (int j = 0; j < (*objectArray)[i].GetNrOfMeshSubsets(); j++)
		{
			mObjectHandler.SetObjectBufferWithIndex(DevCon, SHADOW_PASS, STATIC_OBJECT, i, j);

			int indexStart = (*objectArray)[i].meshSubsetIndexStart[j];
			int indexDrawAmount = (*objectArray)[i].meshSubsetIndexStart[j + 1] - indexStart;

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

			DevCon->DrawIndexed(indexDrawAmount, indexStart, 0);
		}
	}
}

// ------------------------------ Light Pass ------------------------------------------------------

// MOVED TO SHADERHANDLER
//TODO: check if pBackBuffer should be declared here or earlier
//bool GraphicsHandler::SetLightPassRenderTargets(ID3D11Device* Dev, ID3D11DeviceContext* DevCon, IDXGISwapChain* SwapChain)
//{
//	// set the render target as the texture that'll be used as input to the compute shader
//	DevCon->OMSetRenderTargets(1, &mComputeShader.mRenderTextureRTV, nullptr);
//
//	//Clear the render texture 
//	DevCon->ClearRenderTargetView(mComputeShader.mRenderTextureRTV, Colors::fBlack);
//
//	return true;
//}

// MOVED TO SHADRHANDLER
//DONE!
//bool GraphicsHandler::SetLightPassShaders(ID3D11DeviceContext* DevCon)
//{
//	/* Full screen triangle is created in Vertex shader by using the vertexID so no input layout needed */
//	const uintptr_t n0 = 0;
//	DevCon->IASetInputLayout(nullptr);
//	DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//	DevCon->IASetVertexBuffers(0, 0,
//		reinterpret_cast<ID3D11Buffer *const *>(&n0),
//		reinterpret_cast<const UINT *>(n0),
//		reinterpret_cast<const UINT *>(&n0)
//	);
//
//	DevCon->VSSetShader(mLightPassVertexShader, nullptr, 0);
//	DevCon->HSSetShader(nullptr, nullptr, 0);
//	DevCon->DSSetShader(nullptr, nullptr, 0);
//	DevCon->GSSetShader(nullptr, nullptr, 0);
//	DevCon->PSSetShader(mLightPassPixelShader, nullptr, 0);
//	DevCon->PSSetSamplers(0, 1, &mShadowSampler);
//
//	return true;
//}

// MOVED TO SHADERHANDLER
//TODO: There might be a memory leak here
//bool GraphicsHandler::SetLightPassGBuffers(ID3D11DeviceContext* DevCon)
//{
//	ID3D11ShaderResourceView* GBufferTextureViews[] =
//	{
//		mGraphicsBuffer[0].shaderResourceView, // Normal
//		mGraphicsBuffer[1].shaderResourceView, // PositionWS
//		mGraphicsBuffer[2].shaderResourceView, // Diffuse
//		mGraphicsBuffer[3].shaderResourceView  // Specular
//	};
//
//	DevCon->PSSetShaderResources(0, GBUFFER_COUNT, GBufferTextureViews);
//
//	return true;
//}

//DONE
void GraphicsHandler::RenderLightPass(ID3D11Device* Dev, ID3D11DeviceContext* DevCon, IDXGISwapChain* SwapChain)
{

	DevCon->RSSetViewports(1U, &mCameraHandler.playerVP);
	
	mShaderHandler.PrepareRender(DevCon, LIGHT_PASS);
	
	//SetLightPassRenderTargets(Dev, DevCon, SwapChain);
	//SetLightPassShaders(DevCon);
	//SetLightPassGBuffers(DevCon);
	//SetRasterizerState(DevCon, LIGHT_PASS);
	mCameraHandler.BindShadowMapPerFrameConstantBuffer(DevCon, LIGHT_PASS);

	//DevCon->PSSetShaderResources(4U, 1U, &mLightHandler.mShadowMapSRView);

	mLightHandler.BindLightBuffer(DevCon, mCameraHandler.GetCameraPosition());

	// Draw full screen triangle
	DevCon->Draw(3, 0);

	// Set render target to nullptr since the renderTexture can not be bound as a render target 
	// and used as a shader resource view at the same time
	// NOTE: If this doesn't work then DevCon->ClearState()
	//DevCon->OMSetRenderTargets(1, nullptr, nullptr);
	DevCon->ClearState();
}

// ------------------------------ Compute Pass ------------------------------------------------------

void GraphicsHandler::RenderComputePass(ID3D11DeviceContext* DevCon)
{
	//mComputeShader.RenderComputeShader(DevCon, mBackbufferRTV, &mCameraHandler.playerVP, mRasterizerState[1]);
	mShaderHandler.PrepareRender(DevCon, COMPUTE_PASS);
	
	static const UINT squaresWide = SCREEN_RESOLUTION.SCREEN_WIDTH / 40U;
	static const UINT squaresHigh = SCREEN_RESOLUTION.SCREEN_HEIGHT / 20U;

	DevCon->Dispatch(squaresWide, squaresHigh, 1);

	//TODO: See if this can be done without doing something as drastic as ClearState()
	DevCon->ClearState(); // Used to make sure that mTempTextureUAV is free to use
	//DevCon->CSSetShaderResources(0U, 0U, nullptr); //TODO: test if this one works
}

void GraphicsHandler::RenderScreenPass(ID3D11DeviceContext* DevCon)
{
	mShaderHandler.PrepareRender(DevCon, SCREEN_PASS);
	DevCon->RSSetViewports(1, &mCameraHandler.playerVP);

	DevCon->Draw(3, 0);
	
	DevCon->ClearState();
}
