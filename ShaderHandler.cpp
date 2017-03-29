/*
* TODO: Sampler states
*
*
*
*/

#include "ShaderHandler.hpp"


ShaderHandler::ShaderHandler()
{

}

ShaderHandler::~ShaderHandler()
{

}

//public


//bool?
//TODO: Not sure is reuse of pS is okay or not
//TODO: create texture sampler state
void ShaderHandler::CreateShaders(ID3D11Device* Dev)
{
	ID3DBlob* pS = nullptr;

	// Geometry Pass -----------------------------------------------
	CompileShader(&pS, L"GBufferVertex.hlsl", "VS_main", "vs_5_0");
	CreateShader(Dev, pS, VERTEX_SHADER, GEOMETRY_PASS);
	
	//create input layout
	CreateInputLayout(Dev, pS, GEOMETRY_PASS);

	pS->Release();
	CompileShader(&pS, L"GBufferGeometry.hlsl", "GS_main", "gs_5_0");
	CreateShader(Dev, pS, GEOMETRY_SHADER, GEOMETRY_PASS);

	pS->Release();
	CompileShader(&pS, L"GBufferFragment.hlsl", "PS_main", "ps_5_0");
	CreateShader(Dev, pS, PIXEL_SHADER, GEOMETRY_PASS);

	// Height map version
	D3D_SHADER_MACRO HeightMapMacros[] =
	{
		"HEIGHT_MAP",  "1",
		NULL, NULL
	};

	pS->Release();
	CompileShader(&pS, L"GBufferFragment.hlsl", "PS_main", "ps_5_0", HeightMapMacros);
	CreateShader(Dev, pS, PIXEL_SHADER_HEIGHTMAP_VERSION, GEOMETRY_PASS);

	// Shadow Pass -----------------------------------------------
	pS->Release();
	CompileShader(&pS, L"ShadowVertex.hlsl", "VS_main", "vs_5_0");
	CreateShader(Dev, pS, VERTEX_SHADER, SHADOW_PASS);

	//create input layout
	CreateInputLayout(Dev, pS, SHADOW_PASS);

	pS->Release();
	CompileShader(&pS, L"ShadowFragment.hlsl", "PS_main", "ps_5_0");
	CreateShader(Dev, pS, PIXEL_SHADER, SHADOW_PASS);

	// Light Pass -----------------------------------------------

	//input layout
	pS->Release();
	CompileShader(&pS, L"LightVertex.hlsl", "VS_main", "vs_5_0");
	CreateShader(Dev, pS, VERTEX_SHADER, LIGHT_PASS);

	//no input layout since geometry is generated in the vertex shader

	D3D_SHADER_MACRO LightPassFragmentMacros[] =
	{
		"SHADOW_MAP_SIZE",  SHADOW_QUALITY.SIZE_STRING,
		NULL, NULL
	};

	pS->Release();
	CompileShader(&pS, L"LightFragment.hlsl", "PS_main", "ps_5_0", LightPassFragmentMacros);
	CreateShader(Dev, pS, PIXEL_SHADER, LIGHT_PASS);

	// Compute Pass -----------------------------------------------

	D3D_SHADER_MACRO ComputeShaderMacros[] =
	{
		"TEXTURE_WIDTH",  SCREEN_RESOLUTION.WIDTH_STRING,
		"TEXTURE_HEIGHT", SCREEN_RESOLUTION.HEIGHT_STRING,
		NULL, NULL
	};

	pS->Release();
	CompileShader(&pS, L"FXAACompute.hlsl", "FXAA_main", "cs_5_0", ComputeShaderMacros);
	CreateShader(Dev, pS, COMPUTE_SHADER, COMPUTE_PASS);

	pS->Release();
	CompileShader(&pS, L"ComputePassVertex.hlsl", "VS_main", "vs_5_0");
	CreateShader(Dev, pS, VERTEX_SHADER, COMPUTE_PASS);

	//no input layout since geometry is generated in the vertex shader

	pS->Release();
	CompileShader(&pS, L"ComputePassPixel.hlsl", "PS_main", "ps_5_0");
	CreateShader(Dev, pS, PIXEL_SHADER, COMPUTE_PASS);

	pS->Release();
}

//private

void ShaderHandler::CompileShader(
	ID3DBlob** pShader, LPCWSTR shaderFileName, LPCSTR entryPoint, LPCSTR shaderModel,
	const D3D_SHADER_MACRO* pMacros, ID3DInclude* pInclude,
	UINT Flags1, UINT flags2, ID3DBlob** ppErrorMsgs) const
{
	HRESULT hr = D3DCompileFromFile(
		shaderFileName,
		pMacros,
		pInclude,
		entryPoint,
		shaderModel,
		Flags1,
		flags2,
		pShader,
		ppErrorMsgs
	);

	if (FAILED(hr))
	{
		OutputDebugString(L"\nShaderHandler::CompileShader() Failed to compile shader\n\n");
		exit(-1);
	}
}

void ShaderHandler::CreateShader(ID3D11Device* Dev, ID3DBlob* pS, ShaderType shaderType, RenderPassID passID)
{
	HRESULT hr;
	bool failed = false;
	ID3D11VertexShader** vertexShader		= nullptr;
	ID3D11GeometryShader** geometryShader	= nullptr;
	ID3D11PixelShader** pixelShader			= nullptr;
	ID3D11ComputeShader** computeShader		= nullptr;

	switch (passID)
	{
	case GEOMETRY_PASS:
		vertexShader	= &mGeometryPassVertexShader;
		geometryShader	= &mGeometryPassGeometryShader;
		pixelShader		= (shaderType == PIXEL_SHADER_HEIGHTMAP_VERSION) ? 
			&mGeometryPassPixelHeightMapShader : &mGeometryPassPixelShader;
		break;
	case SHADOW_PASS:
		vertexShader	= &mShadowPassVertexShader;
		pixelShader		= &mShadowPassPixelShader;
		break;

	case LIGHT_PASS:
		vertexShader	= &mLightPassVertexShader;
		pixelShader		= &mLightPassPixelShader;
		break;

	case COMPUTE_PASS:
		vertexShader	= &mComputePassVertexShader;
		pixelShader		= &mComputePassPixelShader;
		computeShader	= &mComputeShader;
		break;
	
	default:
		break;
	} //end passID switch
	
	switch (shaderType)
	{
	case VERTEX_SHADER:
		hr = Dev->CreateVertexShader(pS->GetBufferPointer(), pS->GetBufferSize(), nullptr, vertexShader);
		failed = (FAILED(hr)) ? true : false;
		break;

	case GEOMETRY_SHADER:
		hr = Dev->CreateGeometryShader(pS->GetBufferPointer(), pS->GetBufferSize(), nullptr, geometryShader);
		failed = (FAILED(hr)) ? true : false;
		break;

	case PIXEL_SHADER_HEIGHTMAP_VERSION: //Heightmap uses the same shader as the other objects but with a macro defined
	case PIXEL_SHADER:
		hr = Dev->CreatePixelShader(pS->GetBufferPointer(), pS->GetBufferSize(), nullptr, pixelShader);
		failed = (FAILED(hr)) ? true : false;
		break;

	case COMPUTE_SHADER:
		hr = Dev->CreateComputeShader(pS->GetBufferPointer(), pS->GetBufferSize(), nullptr, computeShader);
		failed = (FAILED(hr)) ? true : false;
		break;

	default:
		break;
	} //end shaderType switch

	if (failed)
	{
		OutputDebugString(L"\nShaderHandler::CreateShader() Failed to create shader\n\n");
		exit(-1);
	}
}

void ShaderHandler::CreateInputLayout(ID3D11Device* Dev, ID3DBlob* pS, RenderPassID passID)
{
	HRESULT hr;
	bool failed = false;

	switch (passID)
	{
	case GEOMETRY_PASS:
	{
		D3D11_INPUT_ELEMENT_DESC inputDesc[] =
		{
			{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0,	12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0,	20,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		hr = Dev->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), pS->GetBufferPointer(), pS->GetBufferSize(), &mVertexLayout);
		failed = (FAILED(hr)) ? true : false;
	}
		break;

	case SHADOW_PASS:
	{
		D3D11_INPUT_ELEMENT_DESC inputDesc[] =
		{
			{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0,	12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0,	20,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		hr = Dev->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), pS->GetBufferPointer(), pS->GetBufferSize(), &mVertexLayout);
		failed = (FAILED(hr)) ? true : false;
	}
		break;

	case LIGHT_PASS:
		break;
	case COMPUTE_PASS:
		break;
	default:
		break;
	}

	if (failed)
	{
		OutputDebugString(L"\nShaderHandler::CreateInputLayout() Failed to create input layout\n\n");
		exit(-1);
	}
}

// TODO: Should probably add hr checks. also Relese()
void ShaderHandler::CreateRenderTextures(ID3D11Device* Dev)
{
	HRESULT hr;
	// --------------------------------- Graphics Buffer --------------------------------------------
	{
		// TODO: Not sure if this should be here
		for (int i = 0; i < GBUFFER_COUNT; i++)
		{
			mGraphicsBuffer[i] = GraphicsBuffer();
		}

		// Create render target textures
		D3D11_TEXTURE2D_DESC textureDesc{};
		textureDesc.Width = SCREEN_RESOLUTION.SCREEN_WIDTH;
		textureDesc.Height = SCREEN_RESOLUTION.SCREEN_HEIGHT;
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
		depthDesc.Width = SCREEN_RESOLUTION.SCREEN_WIDTH;
		depthDesc.Height = SCREEN_RESOLUTION.SCREEN_HEIGHT;
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

		//TODO: Release
	}

	// ------------------------------------ Shadow Map ----------------------------------------------
	{
		//Shadow map texture desc
		D3D11_TEXTURE2D_DESC smTexDesc;
		smTexDesc.Width = SHADOW_QUALITY.SHADOW_MAP_SIZE;
		smTexDesc.Height = SHADOW_QUALITY.SHADOW_MAP_SIZE;
		smTexDesc.MipLevels = 1;
		smTexDesc.ArraySize = 1;
		smTexDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		smTexDesc.SampleDesc.Count = 1;
		smTexDesc.SampleDesc.Quality = 0;
		smTexDesc.Usage = D3D11_USAGE_DEFAULT;
		smTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; //
		smTexDesc.CPUAccessFlags = 0;
		smTexDesc.MiscFlags = 0;

		//Shadow map depth stencil view desc
		D3D11_DEPTH_STENCIL_VIEW_DESC smDescDSV{}; //{} might be unnecessary
		smDescDSV.Format = DXGI_FORMAT_D32_FLOAT;
		smDescDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		smDescDSV.Texture2D.MipSlice = 0;
		smDescDSV.Flags = 0;

		//Shadow map shader resource view desc
		D3D11_SHADER_RESOURCE_VIEW_DESC smSrvDesc{};
		smSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		smSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		smSrvDesc.Texture2D.MipLevels = smTexDesc.MipLevels;
		smSrvDesc.Texture2D.MostDetailedMip = 0;

		HRESULT hr;
		hr = Dev->CreateTexture2D(&smTexDesc, nullptr, &mShadowMap);
		hr = Dev->CreateDepthStencilView(mShadowMap, &smDescDSV, &mShadowMapDepthView);
		hr = Dev->CreateShaderResourceView(mShadowMap, &smSrvDesc, &mShadowMapSRView);
	}

	// ----------------------------------- Compute Pass ---------------------------------------------
	{
		// Texture that will be used as input for the Compute shader
		ID3D11Texture2D* texture;
		D3D11_TEXTURE2D_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(texDesc));
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.Width = SCREEN_RESOLUTION.SCREEN_WIDTH;
		texDesc.Height = SCREEN_RESOLUTION.SCREEN_HEIGHT;
		texDesc.MipLevels = 1U;
		texDesc.ArraySize = 1U;
		texDesc.SampleDesc.Count = 1U;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //TODO: look into which file format would be most appropriate to use

		hr = Dev->CreateTexture2D(&texDesc, nullptr, &texture);
		if (FAILED(hr))
		{
			OutputDebugString(L"\nShaderHandler::CreateComputePassRenderTextures() Failed to create compute shader input texture\n\n");
			exit(-1);
		}

		Dev->CreateRenderTargetView(texture, nullptr, &mRenderTextureRTV);
		Dev->CreateShaderResourceView(texture, nullptr, &mRenderTextureSRV);
		texture->Release();

		// Texture that will be used as output from the Compute shader

		// using the same description for the second texture but with different bind flags
		texDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;

		hr = Dev->CreateTexture2D(&texDesc, nullptr, &texture);
		if (FAILED(hr))
		{
			OutputDebugString(L"\nShaderHandler::CreateComputePassRenderTextures() Failed to create compute shader input texture\n\n");
			exit(-1);
		}

		Dev->CreateUnorderedAccessView(texture, nullptr, &mTempTextureUAV);
		Dev->CreateShaderResourceView(texture, nullptr, &mTempTextureSRV);

		texture->Release();
	}

}

void ShaderHandler::CreateRasterizerStates(ID3D11Device* Dev)
{
	HRESULT hr;

	D3D11_RASTERIZER_DESC BFSstate;
	BFSstate.FillMode = D3D11_FILL_SOLID;
	BFSstate.CullMode = D3D11_CULL_NONE;
	BFSstate.FrontCounterClockwise = FALSE;
	BFSstate.DepthBias = 0;
	BFSstate.DepthBiasClamp = 0.0f;
	BFSstate.SlopeScaledDepthBias = 0.0f;
	BFSstate.DepthClipEnable = TRUE;
	BFSstate.ScissorEnable = FALSE;
	BFSstate.MultisampleEnable = FALSE;
	BFSstate.AntialiasedLineEnable = FALSE;

	hr = Dev->CreateRasterizerState(&BFSstate, &mRasterizerState[0]);
	if (FAILED(hr))
	{
		OutputDebugString(L"\nShaderHandler::CreateRasterizerStates() Failed to create first rasterizer state\n\n");
		exit(-1);
	}

	D3D11_RASTERIZER_DESC noBFSstate;
	noBFSstate.FillMode = D3D11_FILL_SOLID;
	noBFSstate.CullMode = D3D11_CULL_BACK;
	noBFSstate.FrontCounterClockwise = FALSE;
	noBFSstate.DepthBias = 0;
	noBFSstate.DepthBiasClamp = 0.0f;
	noBFSstate.SlopeScaledDepthBias = 0.0f;
	noBFSstate.DepthClipEnable = TRUE;
	noBFSstate.ScissorEnable = FALSE;
	noBFSstate.MultisampleEnable = FALSE;
	noBFSstate.AntialiasedLineEnable = FALSE;

	hr = Dev->CreateRasterizerState(&noBFSstate, &mRasterizerState[1]);
	if (FAILED(hr))
	{
		OutputDebugString(L"\nShaderHandler::CreateRasterizerStates() Failed to create second rasterizer state\n\n");
		exit(-1);
	}
}

