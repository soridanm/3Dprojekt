#ifndef SHADERHANDLER_HPP
#define SHADERHANDLER_HPP

#include "GlobalResources.hpp"

const UINT GBUFFER_COUNT = 4;

enum ShaderType
{
	VERTEX_SHADER,
	GEOMETRY_SHADER,
	PIXEL_SHADER,
	PIXEL_SHADER_HEIGHTMAP_VERSION,
	COMPUTE_SHADER
};

class ShaderHandler
{
public:
	ShaderHandler();
	~ShaderHandler();

	void CreateShaders(ID3D11Device* Dev);
	void SetShaders(RenderPassID passID);

private:
	//compile shader
	void CompileShader(
		ID3DBlob** pShader,
		LPCWSTR shaderFileName,
		LPCSTR entryPoint,
		LPCSTR shaderModel,
		const D3D_SHADER_MACRO* pMacros = nullptr,
		ID3DInclude* pInclude = nullptr,
		UINT Flags1 = 0,
		UINT flags2 = 0,
		ID3DBlob** ppErrorMsgs = nullptr
	) const;



	//create shader
	void CreateShader(ID3D11Device* Dev, ID3DBlob* pS, ShaderType shaderType, RenderPassID passID);
	
	//create input layout

	//create textures/srv

	//create/init GBuffer

	struct GraphicsBuffer 
	{
		ID3D11Texture2D* texture = nullptr;
		ID3D11RenderTargetView* renderTargetView = nullptr;
		ID3D11ShaderResourceView* shaderResourceView = nullptr;
	};
	GraphicsBuffer mGraphicsBuffer[GBUFFER_COUNT];


	//Geometry Pass
	ID3D11VertexShader* mGeometryPassVertexShader = nullptr;
	ID3D11GeometryShader* mGeometryPassGeometryShader = nullptr;
	ID3D11PixelShader* mGeometryPassPixelShader = nullptr;
	ID3D11PixelShader* mGeometryPassPixelHeightMapShader = nullptr;

	//Shadow Pass
	ID3D11VertexShader* mShadowPassVertexShader = nullptr;
	ID3D11PixelShader* mShadowPassPixelShader = nullptr;
	//Light Pass
	ID3D11SamplerState* mSampleState = nullptr;
	//ID3D11ShaderResourceView* mTextureView; //moved to ObjectHandler.hpp
	ID3D11VertexShader* mLightPassVertexShader = nullptr;
	ID3D11PixelShader* mLightPassPixelShader = nullptr;
	ID3D11SamplerState* mShadowSampler = nullptr;


};


#endif // !SHADERHANDLER_HPP
