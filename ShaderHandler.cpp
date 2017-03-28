#include "ShaderHandler.hpp"


ShaderHandler::ShaderHandler()
{

}

ShaderHandler::~ShaderHandler()
{

}

//public


//bool?
//TODO: create texture sampler state
void ShaderHandler::CreateShaders(ID3D11Device* Dev)
{
	ID3DBlob* pS = nullptr;
	// Shader macros ----------------------------------------------


	D3D_SHADER_MACRO HeightMapMacros[] =
	{
		"HEIGHT_MAP",  "1",
		NULL, NULL
	};



	// Geometry Pass -----------------------------------------------
	CompileShader(&pS, L"GBufferVertex.hlsl", "VS_main", "vs_5_0");
	CreateShader(Dev, pS, VERTEX_SHADER, GEOMETRY_PASS);
	//create input layout

	CompileShader(&pS, L"GBufferGeometry.hlsl", "GS_main", "gs_5_0");
	CreateShader(Dev, pS, GEOMETRY_SHADER, GEOMETRY_PASS);

	CompileShader(&pS, L"GBufferFragment.hlsl", "PS_main", "ps_5_0");
	CreateShader(Dev, pS, PIXEL_SHADER, GEOMETRY_PASS);

	// Height map version
	CompileShader(&pS, L"GBufferFragment.hlsl", "PS_main", "ps_5_0", HeightMapMacros);
	CreateShader(Dev, pS, PIXEL_SHADER_HEIGHTMAP_VERSION, GEOMETRY_PASS);

	// Shadow Pass -----------------------------------------------
	//input layout
	CompileShader(&pS, L"ShadowVertex.hlsl", "VS_main", "vs_5_0");
	CreateShader(Dev, pS, VERTEX_SHADER, SHADOW_PASS);

	CompileShader(&pS, L"ShadowFragment.hlsl", "PS_main", "ps_5_0");
	CreateShader(Dev, pS, PIXEL_SHADER, SHADOW_PASS);

	// Light Pass -----------------------------------------------

	//input layout
	CompileShader(&pS, L"LightVertex.hlsl", "VS_main", "vs_5_0");
	CreateShader(Dev, pS, VERTEX_SHADER, LIGHT_PASS);

	CompileShader(&pS, L"LightFragment.hlsl", "PS_main", "ps_5_0");
	CreateShader(Dev, pS, PIXEL_SHADER, LIGHT_PASS);

	// Compute Pass -----------------------------------------------

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

		break;
	
	default:
		break;
	}
	
	switch (shaderType)
	{
	case VERTEX_SHADER:
		hr = Dev->CreateVertexShader(pS->GetBufferPointer(), pS->GetBufferSize(), nullptr, vertexShader);
		break;

	case GEOMETRY_SHADER:
		hr = Dev->CreateGeometryShader(pS->GetBufferPointer(), pS->GetBufferSize(), nullptr, geometryShader);
		break;

	case PIXEL_SHADER_HEIGHTMAP_VERSION:
	case PIXEL_SHADER:
		hr = Dev->CreatePixelShader(pS->GetBufferPointer(), pS->GetBufferSize(), nullptr, pixelShader);
		break;

	case COMPUTE_SHADER:
		hr = Dev->CreateComputeShader(pS->GetBufferPointer(), pS->GetBufferSize(), nullptr, computeShader);
		break;

	default:
		break;
	}

	if (FAILED(hr))
	{
		OutputDebugString(L"\nShaderHandler::CreateShader() Failed to create shader\n\n");
		exit(-1);
	}
}
