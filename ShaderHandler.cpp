
#include "ShaderHandler.hpp"


/*============================================================================
*						Public functions
*===========================================================================*/

ShaderHandler::ShaderHandler()
{
	for (int i = 0; i < GBUFFER_COUNT; i++)
	{
		mGraphicsBuffer[i] = GraphicsBuffer();
	}
}

ShaderHandler::~ShaderHandler()
{

}

void ShaderHandler::InitializeShaders(ID3D11Device* Dev, IDXGISwapChain* SwapChain)
{
	CreateShaders(Dev);
	CreateRenderTextures(Dev, SwapChain);
	CreateSamplerStates(Dev);
	CreateRasterizerStates(Dev);
}

void ShaderHandler::PrepareRender(ID3D11DeviceContext* DevCon, RenderPassID passID, bool clearRenderTargets, bool isHeightMap)
{
	SetRenderTargets(DevCon, passID, clearRenderTargets);
	SetInputLayoutAndTopology(DevCon, passID);
	SetShaders(DevCon, passID, isHeightMap);
	SetShaderResources(DevCon, passID);
	SetSamplers(DevCon, passID);
	SetRasterizerState(DevCon, passID);
}

ID3D11RenderTargetView** ShaderHandler::GetBackBufferRTV()
{
	return &mBackbufferRTV;
}


/*=============================================================================
*						Private functions
*===========================================================================*/

void ShaderHandler::CreateShaders(ID3D11Device* Dev)
{
	ID3DBlob* pS = nullptr;

	// Geometry Pass -----------------------------------------------
	{
		CompileShader(&pS, L"GBufferVertex.hlsl", "VS_main", "vs_5_0");
		CreateShader(Dev, pS, VERTEX_SHADER, GEOMETRY_PASS);

		CreateInputLayout(Dev, pS, GEOMETRY_PASS);

		pS->Release();
		CompileShader(&pS, L"GBufferGeometry.hlsl", "GS_main", "gs_5_0");
		CreateShader(Dev, pS, GEOMETRY_SHADER, GEOMETRY_PASS);

		pS->Release();
		CompileShader(&pS, L"GBufferPixel.hlsl", "PS_main", "ps_5_0");
		CreateShader(Dev, pS, PIXEL_SHADER, GEOMETRY_PASS);

		// Height map version ------------------
		D3D_SHADER_MACRO HeightMapMacros[] =
		{
			"IS_HEIGHT_MAP",  "1",
			NULL, NULL
		};

		pS->Release();
		CompileShader(&pS, L"GBufferPixel.hlsl", "PS_main", "ps_5_0", HeightMapMacros);
		CreateShader(Dev, pS, PIXEL_SHADER_HEIGHTMAP_VERSION, GEOMETRY_PASS);
	}

	// Shadow Pass -----------------------------------------------
	{
		pS->Release();
		CompileShader(&pS, L"ShadowVertex.hlsl", "VS_main", "vs_5_0");
		CreateShader(Dev, pS, VERTEX_SHADER, SHADOW_PASS);

		CreateInputLayout(Dev, pS, SHADOW_PASS);

		pS->Release();
		CompileShader(&pS, L"ShadowPixel.hlsl", "PS_main", "ps_5_0");
		CreateShader(Dev, pS, PIXEL_SHADER, SHADOW_PASS);
	}

	// Light Pass -----------------------------------------------
	{
		pS->Release();
		CompileShader(&pS, L"FullScreenTriangleVertex.hlsl", "VS_main", "vs_5_0");
		CreateShader(Dev, pS, VERTEX_SHADER, LIGHT_PASS);

		//no input layout since geometry is generated in the vertex shader

		D3D_SHADER_MACRO LightPassPixelMacros[] =
		{
			"SHADOW_MAP_SIZE",  SHADOW_QUALITY.SIZE_STRING,
			NULL, NULL
		};

		pS->Release();
		CompileShader(&pS, L"LightPixel.hlsl", "PS_main", "ps_5_0", LightPassPixelMacros);
		CreateShader(Dev, pS, PIXEL_SHADER, LIGHT_PASS);
	}

	// Compute Pass -----------------------------------------------
	{
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
	}

	// Screen Pass -----------------------------------------------
	{
		// Full Screen vertex shader already compiled
		
		// no input layout since geometry is generated in the vertex shader

		CompileShader(&pS, L"ScreenPassPixel.hlsl", "PS_main", "ps_5_0");
		CreateShader(Dev, pS, PIXEL_SHADER, SCREEN_PASS);

		pS->Release();
	}
}

void ShaderHandler::SetRenderTargets(ID3D11DeviceContext* DevCon, RenderPassID passID, bool clearRenderTargets)
{
	switch (passID)
	{
	case GEOMETRY_PASS:
	{
		ID3D11RenderTargetView* renderTargets[] =
		{
			mGraphicsBuffer[0].renderTargetView, /* Normal */
			mGraphicsBuffer[1].renderTargetView, /* PositionWS */
			mGraphicsBuffer[2].renderTargetView, /* Diffuse */
			mGraphicsBuffer[3].renderTargetView  /* Specular */
		};

		//set render targets
		DevCon->OMSetRenderTargets(GBUFFER_COUNT, renderTargets, mDepthStecilView);
		if (clearRenderTargets)
		{
			DevCon->ClearRenderTargetView(mGraphicsBuffer[0].renderTargetView, Colors::fBlack);				//Normal
			DevCon->ClearRenderTargetView(mGraphicsBuffer[1].renderTargetView, Colors::fLightSteelBlue);	//Position
			DevCon->ClearRenderTargetView(mGraphicsBuffer[2].renderTargetView, Colors::fBlack);				//Diffuse
			DevCon->ClearRenderTargetView(mGraphicsBuffer[3].renderTargetView, Colors::fBlack);				//Specular
			DevCon->ClearDepthStencilView(mDepthStecilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
		}
	}
	break;
	case SHADOW_PASS:
	{
		DevCon->OMSetRenderTargets(0, 0, mShadowMapDepthView);
		if (clearRenderTargets)
		{
			DevCon->ClearDepthStencilView(mShadowMapDepthView, D3D11_CLEAR_DEPTH, 1.0f, 0);
		}
	}
	break;
	case LIGHT_PASS:
	{
		// set the render target as the texture that'll be used as input to the compute shader
		DevCon->OMSetRenderTargets(1, &mLightPassOutputTextureRTV, nullptr);
		if (clearRenderTargets)
		{
			DevCon->ClearRenderTargetView(mLightPassOutputTextureRTV, Colors::fBlack);
		}
	}
	break;
	case COMPUTE_PASS:
	{
		ID3D11UnorderedAccessView* uav[] = { mComputePassTempTextureUAV };
		DevCon->CSSetUnorderedAccessViews(0, 1, uav, nullptr);
	}
	break;
	case SCREEN_PASS:
	{
		DevCon->OMSetRenderTargets(1, &mBackbufferRTV, nullptr);
		if (clearRenderTargets)
		{
			DevCon->ClearRenderTargetView(mBackbufferRTV, Colors::fBlack);
		}
	}
	break;
	default:
		break;
	} //end passID switch
}

void ShaderHandler::SetInputLayoutAndTopology(ID3D11DeviceContext* DevCon, RenderPassID passID)
{
	switch (passID)
	{
	case GEOMETRY_PASS:
	{
		// Set Vertex Shader input
		DevCon->IASetInputLayout(mGeometryPassInputLayout);
		DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
		break;
	case SHADOW_PASS:
	{
		DevCon->IASetInputLayout(mShadowPassInputLayout);
		DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
		break;

	case LIGHT_PASS:
	case SCREEN_PASS:
	{
		/* Full screen triangle is created in Vertex shader by using the vertexID so no input layout is set to nullptr*/
		const uintptr_t n0 = 0;
		DevCon->IASetInputLayout(nullptr);
		DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		DevCon->IASetVertexBuffers(0, 0,
			reinterpret_cast<ID3D11Buffer *const *>(&n0),
			reinterpret_cast<const UINT *>(n0),
			reinterpret_cast<const UINT *>(&n0)
		);
	}
		break;
	default:
		break;
	} //end passID switch
}

void ShaderHandler::SetShaders(ID3D11DeviceContext* DevCon, RenderPassID passID, bool isHeightMap)
{
	switch (passID)
	{
	case GEOMETRY_PASS:
	{
		ID3D11PixelShader* pixelShader = (isHeightMap) ? mGeometryPassPixelHeightMapShader : mGeometryPassPixelShader;

		DevCon->VSSetShader(mGeometryPassVertexShader, nullptr, 0);
		DevCon->HSSetShader(nullptr, nullptr, 0);
		DevCon->DSSetShader(nullptr, nullptr, 0);
		DevCon->GSSetShader(mGeometryPassGeometryShader, nullptr, 0);
		DevCon->PSSetShader(pixelShader, nullptr, 0);
	}
	break;
	case SHADOW_PASS:
	{
		DevCon->VSSetShader(mShadowPassVertexShader, nullptr, 0);
		DevCon->HSSetShader(nullptr, nullptr, 0);
		DevCon->DSSetShader(nullptr, nullptr, 0);
		DevCon->GSSetShader(nullptr, nullptr, 0);
		DevCon->PSSetShader(mShadowPassPixelShader, nullptr, 0);
	}
	break;
	case LIGHT_PASS:
	{
		DevCon->VSSetShader(mFullScreenVertexShader, nullptr, 0);
		DevCon->HSSetShader(nullptr, nullptr, 0);
		DevCon->DSSetShader(nullptr, nullptr, 0);
		DevCon->GSSetShader(nullptr, nullptr, 0);
		DevCon->PSSetShader(mLightPassPixelShader, nullptr, 0);
	}
	break;
	case COMPUTE_PASS:
	{
		DevCon->VSSetShader(nullptr, nullptr, 0);
		DevCon->HSSetShader(nullptr, nullptr, 0);
		DevCon->DSSetShader(nullptr, nullptr, 0);
		DevCon->GSSetShader(nullptr, nullptr, 0);
		DevCon->PSSetShader(nullptr, nullptr, 0);
		DevCon->CSSetShader(mComputeShader, nullptr, 0U);
	}
	break;
	case SCREEN_PASS:
	{
		DevCon->VSSetShader(mFullScreenVertexShader, nullptr, 0);
		DevCon->HSSetShader(nullptr, nullptr, 0);
		DevCon->DSSetShader(nullptr, nullptr, 0);
		DevCon->GSSetShader(nullptr, nullptr, 0);
		DevCon->PSSetShader(mScreenPassPixelShader, nullptr, 0);
	}
	break;
	default:
		break;
	} //end passID switch
}

void ShaderHandler::SetSamplers(ID3D11DeviceContext* DevCon, RenderPassID passID)
{
	switch (passID)
	{
	case GEOMETRY_PASS:
	{
		DevCon->PSSetSamplers(0, 1, &mGeometryPassSampler);
	}
	break;
	case LIGHT_PASS:
	{
		DevCon->PSSetSamplers(0, 1, &mShadowSampler);
	}
	break;
	default:
		break;
	} //end passID switch
}

void ShaderHandler::SetShaderResources(ID3D11DeviceContext* DevCon, RenderPassID passID)
{
	switch (passID)
	{
	case LIGHT_PASS:
	{
		ID3D11ShaderResourceView* GBufferTextureViews[] =
		{
			mGraphicsBuffer[0].shaderResourceView, // Normal
			mGraphicsBuffer[1].shaderResourceView, // PositionWS
			mGraphicsBuffer[2].shaderResourceView, // Diffuse
			mGraphicsBuffer[3].shaderResourceView  // Specular
		};

		DevCon->PSSetShaderResources(0, GBUFFER_COUNT, GBufferTextureViews);
		DevCon->PSSetShaderResources(4, 1, &mShadowMapSRView);
	}
	break;
	case COMPUTE_PASS:
	{
		DevCon->CSSetShaderResources(0, 1, &mComputePassOutputTextureSRV);
	}
	break;
	case SCREEN_PASS:
	{
		DevCon->PSSetShaderResources(0, 1, &mSreenPassInputTextureSRV);
	}
	break;
	default:
		break;
	} //end passID switch
}

void ShaderHandler::SetRasterizerState(ID3D11DeviceContext* DevCon, RenderPassID passID)
{
	switch (passID)
	{
	case GEOMETRY_PASS:
	case SHADOW_PASS:
		DevCon->RSSetState(mRasterizerState[0]);
		break;
	case LIGHT_PASS:
	case COMPUTE_PASS:
	case SCREEN_PASS:
		DevCon->RSSetState(mRasterizerState[1]);
		break;
	default:
		break;
	} //end passID switch
}

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
		vertexShader = &mGeometryPassVertexShader;
		geometryShader = &mGeometryPassGeometryShader;
		pixelShader = (shaderType == PIXEL_SHADER_HEIGHTMAP_VERSION) ?
			&mGeometryPassPixelHeightMapShader : &mGeometryPassPixelShader;
		break;
	case SHADOW_PASS:
		vertexShader = &mShadowPassVertexShader;
		pixelShader = &mShadowPassPixelShader;
		break;

	case LIGHT_PASS:
		vertexShader = &mFullScreenVertexShader;
		pixelShader = &mLightPassPixelShader;
		break;

	case COMPUTE_PASS:
		vertexShader = &mFullScreenVertexShader;
		computeShader = &mComputeShader;
		break;
	case SCREEN_PASS:
		pixelShader = &mScreenPassPixelShader;
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
		hr = Dev->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), pS->GetBufferPointer(), pS->GetBufferSize(), &mGeometryPassInputLayout);
		failed = (FAILED(hr)) ? true : false;
	}
	break;
	case SHADOW_PASS:
	{
		D3D11_INPUT_ELEMENT_DESC inputDesc[] =
		{
			{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	0,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		hr = Dev->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), pS->GetBufferPointer(), pS->GetBufferSize(), &mShadowPassInputLayout);
		failed = (FAILED(hr)) ? true : false;
	}
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

void ShaderHandler::CreateRenderTextures(ID3D11Device* Dev, IDXGISwapChain* SwapChain)
{
	HRESULT hr;
	// --------------------------------- Graphics Buffer --------------------------------------------
	{
		// Define render target textures
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

		// Define render target views.
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		// Define the shader resource views
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		// Create render target textures, render target views, and shader resource views
		for (UINT i = 0; i < GBUFFER_COUNT; i++) {
			Dev->CreateTexture2D(&textureDesc, NULL, &mGraphicsBuffer[i].texture);
			Dev->CreateRenderTargetView(mGraphicsBuffer[i].texture, &renderTargetViewDesc, &mGraphicsBuffer[i].renderTargetView);
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
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.SampleDesc.Count = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		hr = Dev->CreateTexture2D(&texDesc, nullptr, &texture);
		if (FAILED(hr))
		{
			OutputDebugString(L"\nShaderHandler::CreateComputePassRenderTextures() Failed to create compute shader input texture\n\n");
			exit(-1);
		}

		Dev->CreateRenderTargetView(texture, nullptr, &mLightPassOutputTextureRTV);
		Dev->CreateShaderResourceView(texture, nullptr, &mComputePassOutputTextureSRV);
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

		Dev->CreateUnorderedAccessView(texture, nullptr, &mComputePassTempTextureUAV);
		Dev->CreateShaderResourceView(texture, nullptr, &mSreenPassInputTextureSRV);

		texture->Release();
	}

	// ----------------------------------- Screen Pass ---------------------------------------------
	{
		ID3D11Texture2D* pTempBackBuffer = nullptr;
		// Gets the adress of teh back buffer
		SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pTempBackBuffer);

		// Create render target by using buffer adress
		Dev->CreateRenderTargetView(pTempBackBuffer, nullptr, &mBackbufferRTV);

		pTempBackBuffer->Release();
	}

}

void ShaderHandler::CreateSamplerStates(ID3D11Device* Dev)
{
	HRESULT hr;
	// --------------------------------- Geometry Pass --------------------------------------------
	// Used for sampling model and heightmap textures
	{
		D3D11_SAMPLER_DESC samplerDesc;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		// Create the texture sampler state.
		hr = Dev->CreateSamplerState(&samplerDesc, &mGeometryPassSampler);
		if (FAILED(hr))
		{
			OutputDebugString(L"\nShaderHandler::CreateShaders() Failed to create geometry pass sampler state\n\n");
			exit(-1);
		}
	}
	// --------------------------------- Light Pass --------------------------------------------
	// A comparison sampler used to sample the shadow map
	{
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
		hr = Dev->CreateSamplerState(&shadowSamplerDesc, &mShadowSampler);
		if (FAILED(hr))
		{
			OutputDebugString(L"\nShaderHandler::CreateShaders() Failed to create shadow pass sampler state\n\n");
			exit(-1);
		}
	}
}

void ShaderHandler::CreateRasterizerStates(ID3D11Device* Dev)
{
	HRESULT hr;

	D3D11_RASTERIZER_DESC RSState1;
	RSState1.FillMode = D3D11_FILL_SOLID;
	RSState1.CullMode = D3D11_CULL_NONE;
	RSState1.FrontCounterClockwise = FALSE;
	RSState1.DepthBias = 0;
	RSState1.DepthBiasClamp = 0.0f;
	RSState1.SlopeScaledDepthBias = 0.0f;
	RSState1.DepthClipEnable = TRUE;
	RSState1.ScissorEnable = FALSE;
	RSState1.MultisampleEnable = FALSE;
	RSState1.AntialiasedLineEnable = FALSE;

	hr = Dev->CreateRasterizerState(&RSState1, &mRasterizerState[0]);
	if (FAILED(hr))
	{
		OutputDebugString(L"\nShaderHandler::CreateRasterizerStates() Failed to create first rasterizer state\n\n");
		exit(-1);
	}

	D3D11_RASTERIZER_DESC RSState2;
	RSState2.FillMode = D3D11_FILL_SOLID;
	RSState2.CullMode = D3D11_CULL_BACK;
	RSState2.FrontCounterClockwise = FALSE;
	RSState2.DepthBias = 0;
	RSState2.DepthBiasClamp = 0.0f;
	RSState2.SlopeScaledDepthBias = 0.0f;
	RSState2.DepthClipEnable = TRUE;
	RSState2.ScissorEnable = FALSE;
	RSState2.MultisampleEnable = FALSE;
	RSState2.AntialiasedLineEnable = FALSE;

	hr = Dev->CreateRasterizerState(&RSState2, &mRasterizerState[1]);
	if (FAILED(hr))
	{
		OutputDebugString(L"\nShaderHandler::CreateRasterizerStates() Failed to create second rasterizer state\n\n");
		exit(-1);
	}
}

