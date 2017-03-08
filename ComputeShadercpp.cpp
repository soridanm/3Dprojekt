#include "ComputeShader.hpp"


ComputeShader::ComputeShader()
{

}

ComputeShader::~ComputeShader()
{

}

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
		L"ComputeVertex.hlsl",
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
		L"ComputeFragment.hlsl",
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

void ComputeShader::CreateRenderTextures()
{
	ID3D11Texture2D* texture;
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texture));
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	//texDesc = 
}

void ComputeShader::RenderComputeShader(ID3D11DeviceContext* DevCon)
{

}
