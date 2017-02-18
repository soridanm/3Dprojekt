/*
* TODO: Write error messages and use if(Function()) instead of just Funtion()
*		Update code with code from the .obj branch
*		See if the order of things should be changed
*
*/


#include "GraphicsHandler.hpp"

// private ------------------------------------------------------------------------------

//done for now
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

	gDevice->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), &gVertexLayout);

	return true;
}

// public ------------------------------------------------------------------------------

GraphicsHandler::GraphicsHandler()
{
	gVertexLayout = nullptr;
	gVertexLayout = nullptr;
	gVertexShader = nullptr;
	gGeometryShader = nullptr;
	gPixelShader = nullptr;
	gTextureView = nullptr;
	gFullScreenTriangleShader = nullptr;
	gLightPixelShader = nullptr;
	gBackbufferRTV = nullptr;
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

	gHR = gDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &gVertexShader);
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

	gHR = gDevice->CreateGeometryShader(pGS->GetBufferPointer(), pGS->GetBufferSize(), nullptr, &gGeometryShader);
	if (FAILED(gHR)) {
		exit(-1);
	}

	//compile and create pixel shader
	ID3DBlob* pPS = nullptr;
	CompileShader(pPS, L"GBufferFragment.hlsl", "PS_main", "ps_5_0");

	gHR = gDevice->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &gPixelShader);
	if (FAILED(gHR)) {
		exit(-1);
	}

	//---------------------------------- Light Pass ----------------------------------------------------

		//compile and create vertex shader
	ID3DBlob* pVS2 = nullptr;
	CompileShader(pVS2, L"LightVertex.hlsl", "VS_main", "vs_5_0");
	gHR = gDevice->CreateVertexShader(pVS2->GetBufferPointer(), pVS2->GetBufferSize(), nullptr, &gFullScreenTriangleShader);
	if (FAILED(gHR)) {
		exit(-1);
	}

	//compile and create pixel shader
	ID3DBlob* pPS2 = nullptr;
	CompileShader(pPS2, L"LightFragment.hlsl", "PS_main", "ps_5_0");
	gHR = gDevice->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &gLightPixelShader);
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
bool GraphicsHandler::SetGeometryPassRenderTargets()
{
	ID3D11RenderTargetView* renderTargets[] =
	{
		gGraphicsBuffer[0].renderTargetView, /* Normal */
		gGraphicsBuffer[1].renderTargetView, /* PositionWS */
		gGraphicsBuffer[2].renderTargetView, /* Diffuse */
		gGraphicsBuffer[3].renderTargetView  /* Specular */
	};

	//set render targets
	gDeviceContext->OMSetRenderTargets(GBUFFER_COUNT, renderTargets, gDepthStecilView);

	//Clear the render targets
	gDeviceContext->ClearRenderTargetView(gGraphicsBuffer[0].renderTargetView, Colors::fBlack);
	gDeviceContext->ClearRenderTargetView(gGraphicsBuffer[1].renderTargetView, Colors::fLightSteelBlue);
	gDeviceContext->ClearRenderTargetView(gGraphicsBuffer[2].renderTargetView, Colors::fLightSteelBlue);
	gDeviceContext->ClearRenderTargetView(gGraphicsBuffer[3].renderTargetView, Colors::fBlack);
	gDeviceContext->ClearDepthStencilView(gDepthStecilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	//gDeviceContext->ClearDepthStencilView(gDepthStecilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	return true;
}

//TODO: update with correct code from both branches.
bool GraphicsHandler::SetGeometryPassShaders()
{
	// Set Vertex Shader input
	gDeviceContext->IASetInputLayout(gVertexLayout);
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set shaders
	gDeviceContext->VSSetShader(gVertexShader, nullptr, 0);
	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	gDeviceContext->GSSetShader(gGeometryShader, nullptr, 0);
	gDeviceContext->PSSetShader(gPixelShader, nullptr, 0);

	return true;
}

//TODO: Only do this when the camera updates
bool GraphicsHandler::SetGeometryPassViewProjectionBuffer()
{
	// TODO: check if map_write_discard is necessary and if it's required to make a mapped subresource
	D3D11_MAPPED_SUBRESOURCE viewProjectionMatrixPtr;
	gDeviceContext->Map(gPerFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &viewProjectionMatrixPtr);

	DirectX::XMMATRIX view = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(CAM_POS, CAM_TARGET, CAM_UP));
	XMStoreFloat4x4(&VPBufferData.View, view);

	memcpy(viewProjectionMatrixPtr.pData, &VPBufferData, sizeof(cPerFrameBuffer));
	//gDeviceContext->Unmap(gPerFrameBuffer, 0);
	gDeviceContext->GSSetConstantBuffers(0, 1, &gPerFrameBuffer);

	return true;
}

//TODO: Rewrite with oject index as input
bool GraphicsHandler::SetGeometryPassObjectBuffers()
{
	UINT32 vertexSize = sizeof(float) * 5;
	UINT32 offset = 0;
	UINT32 squareVertexSize = sizeof(float) * 8;

	// set textures and constant buffers
	gDeviceContext->PSSetShaderResources(0, 1, &gTextureView);
	//gDeviceContext->PSSetConstantBuffers(0, 1, &gMaterialBuffer);

	// HEIGHT-MAP BEGIN ---------------------------------------------------------------------------

	//gDeviceContext->IASetVertexBuffers(0, 1, &gVertexBuffer, &vertexSize, &offset);
	gDeviceContext->IASetIndexBuffer(gSquareIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	gDeviceContext->IASetVertexBuffers(0, 1, &gSquareVertBuffer, &squareVertexSize, &offset);

	// HEIGHT-MAP END ---------------------------------------------------------------------------

	// update per-object buffer to spin cube
	static float rotation = 0.0f;
	//rotation += CUBE_ROTATION_SPEED;

	DirectX::XMStoreFloat4x4(&ObjectBufferData.World, DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationY(rotation)));

	D3D11_MAPPED_SUBRESOURCE worldMatrixPtr;
	gDeviceContext->Map(gPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &worldMatrixPtr);
	// copy memory from CPU to GPU of the entire struct
	memcpy(worldMatrixPtr.pData, &ObjectBufferData, sizeof(cPerObjectBuffer));
	// Unmap constant buffer so that we can use it again in the GPU
	gDeviceContext->Unmap(gPerObjectBuffer, 0);
	// set resource to Geometry Shader
	gDeviceContext->GSSetConstantBuffers(1, 1, &gPerObjectBuffer);

	// Map material properties buffer

	//SetMaterial(Materials::Black_plastic);
	gMaterialBufferData.material = Materials::Black_plastic;

	D3D11_MAPPED_SUBRESOURCE materialPtr;
	gDeviceContext->Map(gMaterialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &materialPtr);
	memcpy(materialPtr.pData, &gMaterialBufferData, sizeof(cMaterialBuffer));
	//gDeviceContext->Unmap(gPerFrameBuffer, 0);
	gDeviceContext->PSSetConstantBuffers(0, 1, &gMaterialBuffer);

	return true;
}

bool GraphicsHandler::RenderGeometryPass()
{
	SetGeometryPassRenderTargets();
	SetGeometryPassShaders();
	SetGeometryPassViewProjectionBuffer();

	//LOOP OVER OBJECTS FROM HERE

	SetGeometryPassObjectBuffers();

	gDeviceContext->DrawIndexed(NUMBER_OF_FACES * 3, 0, 0);

	//LOOP OVER OBJECTS TO HERE

	return true;
}

// ------------------------------ Light Pass ------------------------------------------------------

//TODO: check if pBackBuffer should be declared here or earlier
bool GraphicsHandler::SetLightPassRenderTargets()
{
	// get the address of the back buffer
	ID3D11Texture2D* pBackBuffer = nullptr;
	gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	// use the back buffer address to create the render target
	gDevice->CreateRenderTargetView(pBackBuffer, NULL, &gBackbufferRTV);
	pBackBuffer->Release();

	// set the render target as the back buffer
	gDeviceContext->OMSetRenderTargets(1, &gBackbufferRTV, nullptr);

	//Clear screen
	gDeviceContext->ClearRenderTargetView(gBackbufferRTV, Colors::fWhite);

	return true;
}

bool GraphicsHandler::SetLightPassShaders()
{
	/* Full screen triangle is created in Vertex shader so no input buffer needed */
	const uintptr_t n0 = 0;
	gDeviceContext->IASetInputLayout(nullptr);
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gDeviceContext->IASetVertexBuffers(0, 0,
		reinterpret_cast<ID3D11Buffer *const *>(&n0),
		reinterpret_cast<const UINT *>(n0),
		reinterpret_cast<const UINT *>(&n0)
	);

	gDeviceContext->VSSetShader(gFullScreenTriangleShader, nullptr, 0);
	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	gDeviceContext->GSSetShader(nullptr, nullptr, 0);
	gDeviceContext->PSSetShader(gLightPixelShader, nullptr, 0);

	return true;
}

//TODO: There might be a memory leak here
bool GraphicsHandler::SetLightPassGBuffers()
{
	ID3D11ShaderResourceView* GBufferTextureViews[] =
	{
		gGraphicsBuffer[0].shaderResourceView, // Normal
		gGraphicsBuffer[1].shaderResourceView, // PositionWS
		gGraphicsBuffer[2].shaderResourceView, // Diffuse
		gGraphicsBuffer[3].shaderResourceView  // Specular
	};

	gDeviceContext->PSSetShaderResources(0, GBUFFER_COUNT, GBufferTextureViews);

	return true;
}

bool GraphicsHandler::SetLightPassLightBuffer()
{
	// Move light up and down
	static int lightYMovement = 249;
	lightYMovement = (lightYMovement + 1) % 1000;
	float yMovement = (lightYMovement < 500) ? -0.01f : 0.01f;
	DirectX::XMMATRIX yMove = DirectX::XMMatrixTranslation(0.0f, yMovement, 0.0f);

	DirectX::XMVECTOR lightPos = DirectX::XMLoadFloat4(&gLightBufferData.Lights[0].PositionWS);
	lightPos = DirectX::XMVector4Transform(lightPos, yMove);
	XMStoreFloat4(&gLightBufferData.Lights[0].PositionWS, CAM_POS);
	XMStoreFloat4(&gLightBufferData.cameraPositionWS, CAM_POS);

	// Map light buffer
	D3D11_MAPPED_SUBRESOURCE LightBufferPtr;
	gDeviceContext->Map(gLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &LightBufferPtr);
	memcpy(LightBufferPtr.pData, &gLightBufferData, sizeof(cLightBuffer));
	gDeviceContext->Unmap(gLightBuffer, 0);
	gDeviceContext->PSSetConstantBuffers(0, 1, &gLightBuffer);

	return true;
}

bool GraphicsHandler::RenderLightPass()
{
	SetLightPassRenderTargets();
	SetLightPassShaders();
	SetLightPassLightBuffer();
	SetLightPassLightBuffer();

	// Draw full screen triangle
	gDeviceContext->Draw(3, 0);

	return true;
}
