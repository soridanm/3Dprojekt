#include "ComputeShader.hpp"


ComputeShader::ComputeShader()
{

}

ComputeShader::~ComputeShader()
{

}



// Done for now
// TODO: Make a shader class so that these are created using the same function as the one in GraphicsHandler
// Used in GraphicsHandler::CreateShaders()
void ComputeShader::CreateComputePassShaders(ID3D11Device* Dev)
{
	HRESULT hr;

	// Compute Shader
	ID3DBlob* pCS = nullptr;
	D3DCompileFromFile(
		L"FXAACompute.hlsl",
		nullptr,
		nullptr,
		"FXAA_main",
		"cs_5_0",
		0,
		0,
		&pCS,
		nullptr
	);
	
	hr = Dev->CreateComputeShader(pCS->GetBufferPointer(), pCS->GetBufferSize(), nullptr, &mComputeShader);
	if (FAILED(hr))
	{
		OutputDebugString(L"\nComputeShader::CreateComputePassShaders() Failed to create Compute Shader\n\n");
	}
	
	// Vertex Shader
	ID3DBlob* pVS = nullptr;
	D3DCompileFromFile(
		L"ComputePassVertex.hlsl",
		nullptr,
		nullptr,
		"VS_main",
		"vs_5_0",
		0,
		0,
		&pVS,
		nullptr
	);

	hr = Dev->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &mComputePassVertexShader);
	if (FAILED(hr))
	{
		OutputDebugString(L"\nComputeShader::CreateComputePassShaders() Failed to create vertex Shader\n\n");
	}

	// Input Layout
	// No input layout is created here since the triangle is generated in the vertex shader from the VertexID

	// Pixel Shader
	ID3DBlob* pPS = nullptr;
	D3DCompileFromFile(
		L"ComputePassPixel.hlsl",
		nullptr,
		nullptr,
		"PS_main",
		"ps_5_0",
		0,
		0,
		&pPS,
		nullptr
	);

	hr = Dev->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &mComputePassPixelShader);
	if (FAILED(hr))
	{
		OutputDebugString(L"\nComputeShader::CreateComputePassShaders() Failed to create pixel Shader\n\n");
	}

	pCS->Release();
	pVS->Release();
	pPS->Release();
}

// Done for now
// Used in GraphicsHandler::InitializeGraphics()
void ComputeShader::CreateRenderTextures(ID3D11Device* Dev)
{
	HRESULT hr;
	
	// Texture that will be used as input for the Compute shader
	ID3D11Texture2D* texture;
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.Width = ScreenSize::SCREEN_WIDTH;
	texDesc.Height = ScreenSize::SCREEN_HEIGHT;
	texDesc.MipLevels = 1U;
	texDesc.ArraySize = 1U;
	texDesc.SampleDesc.Count = 1U;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //TODO: look into which file format would be most appropriate to use

	hr = Dev->CreateTexture2D(&texDesc, nullptr, &texture);
	if (FAILED(hr))
	{
		OutputDebugString(L"\nComputeShader::CreateRenderTextures() Failed to create compute shader input texture\n\n");
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
		OutputDebugString(L"\nComputeShader::CreateRenderTextures() Failed to create compute shader input texture\n\n");
		exit(-1);
	}

	Dev->CreateUnorderedAccessView(texture, nullptr, &mTempTextureUAV);
	Dev->CreateShaderResourceView(texture, nullptr, &mTempTextureSRV);

	texture->Release();
}



void ComputeShader::RenderComputeShader(
	ID3D11DeviceContext* DevCon, ID3D11RenderTargetView* BackBuffer, 
	const D3D11_VIEWPORT* VP, ID3D11RasterizerState* RS)
{
	// Compute Shader Pass

	ID3D11UnorderedAccessView* uav[] = { mTempTextureUAV };
	DevCon->CSSetUnorderedAccessViews(0U, 1U, uav, nullptr);

	DevCon->VSSetShader(nullptr, nullptr, 0);
	DevCon->HSSetShader(nullptr, nullptr, 0);
	DevCon->DSSetShader(nullptr, nullptr, 0);
	DevCon->GSSetShader(nullptr, nullptr, 0);
	DevCon->PSSetShader(nullptr, nullptr, 0);
	DevCon->CSSetShader(mComputeShader, nullptr, 0U);

	DevCon->CSSetShaderResources(0U, 1U, &mRenderTextureSRV);

	DevCon->Dispatch(1U, ScreenSize::SCREEN_HEIGHT, 1U);

	//TODO: See if this can be done without doing something as drastic as ClearState()
	DevCon->ClearState(); // Used to make sure that mTempTextureUAV is free to use
	//DevCon->CSSetShaderResources(0U, 0U, nullptr); //TODO: test if this one works
	
	// Full Screen triangle pass

	DevCon->RSSetViewports(1U, VP);
	DevCon->RSSetState(RS);

	DevCon->OMSetRenderTargets(1U, &BackBuffer, nullptr);
	DevCon->ClearRenderTargetView(BackBuffer, Colors::fBlack);

	DevCon->VSSetShader(mComputePassVertexShader, nullptr, 0);
	DevCon->HSSetShader(nullptr, nullptr, 0);
	DevCon->DSSetShader(nullptr, nullptr, 0);
	DevCon->GSSetShader(nullptr, nullptr, 0);
	DevCon->PSSetShader(mComputePassPixelShader, nullptr, 0);

	/* Full screen triangle is created in Vertex shader by using the vertexID so no input layout needed */
	const uintptr_t n0 = 0;
	DevCon->IASetInputLayout(nullptr);
	DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DevCon->IASetVertexBuffers(0, 0,
		reinterpret_cast<ID3D11Buffer *const *>(&n0),
		reinterpret_cast<const UINT *>(n0),
		reinterpret_cast<const UINT *>(&n0)
	);

	DevCon->PSSetShaderResources(0U, 1U, &mTempTextureSRV);

	DevCon->Draw(3U, 0U);

	DevCon->ClearState();
}
