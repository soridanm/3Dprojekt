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
	void CreateInputLayout(ID3D11Device* Dev, ID3DBlob* pS, RenderPassID passID);

	//create textures/srv
	//void CreateComputePassRenderTextures(ID3D11Device* Dev);

	//create/init GBuffer
	void CreateRenderTextures(ID3D11Device* Dev);

	//rasterizerStates
	void CreateRasterizerStates(ID3D11Device* Dev);

	struct GraphicsBuffer 
	{
		ID3D11Texture2D* texture = nullptr;
		ID3D11RenderTargetView* renderTargetView = nullptr;
		ID3D11ShaderResourceView* shaderResourceView = nullptr;
	};
	GraphicsBuffer mGraphicsBuffer[GBUFFER_COUNT];

	ID3D11DepthStencilView* mDepthStecilView = nullptr;
	ID3D11Texture2D* mDepthStencilTexture = nullptr;

	//Geometry Pass
	ID3D11InputLayout* mVertexLayout = nullptr;
	ID3D11VertexShader* mGeometryPassVertexShader = nullptr;
	ID3D11GeometryShader* mGeometryPassGeometryShader = nullptr;
	ID3D11PixelShader* mGeometryPassPixelShader = nullptr;
	ID3D11PixelShader* mGeometryPassPixelHeightMapShader = nullptr;

	//Shadow Pass
	ID3D11VertexShader* mShadowPassVertexShader = nullptr;
	ID3D11PixelShader* mShadowPassPixelShader = nullptr;

	ID3D11Texture2D* mShadowMap;
	ID3D11DepthStencilView* mShadowMapDepthView;
	ID3D11ShaderResourceView* mShadowMapSRView;

	//Light Pass
	ID3D11SamplerState* mSampleState = nullptr;
	//ID3D11ShaderResourceView* mTextureView; //moved to ObjectHandler.hpp
	ID3D11VertexShader* mLightPassVertexShader = nullptr;
	ID3D11PixelShader* mLightPassPixelShader = nullptr;
	ID3D11SamplerState* mShadowSampler = nullptr;
	//Compute Pass
	ID3D11VertexShader* mComputePassVertexShader = nullptr;
	ID3D11PixelShader* mComputePassPixelShader = nullptr;

	ID3D11ComputeShader* mComputeShader = nullptr;

	ID3D11ShaderResourceView* mRenderTextureSRV = nullptr;

	ID3D11UnorderedAccessView* mTempTextureUAV = nullptr;
	ID3D11ShaderResourceView* mTempTextureSRV = nullptr;
	ID3D11RenderTargetView*	mRenderTextureRTV = nullptr; //TODO:public?

	ID3D11RasterizerState* mRasterizerState[2];

};


#endif // !SHADERHANDLER_HPP
