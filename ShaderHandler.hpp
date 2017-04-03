/**
* Course: DV142 - 3D-Programming
* Authors: Viktor Enfeldt, Peter Meunier
*
* File: ShaderHandler.hpp
*
* File summary:
*	Is responsible for creating/setting all shaders as well as their affiliated
*	sampler states, resources, render textures, render targets, and rasterizer
*	states.
*/

#ifndef SHADERHANDLER_HPP
#define SHADERHANDLER_HPP

#include "GlobalResources.hpp"
#include "GlobalSettings.hpp"

const UINT GBUFFER_COUNT = 4;

enum ShaderType
{
	VERTEX_SHADER,
	GEOMETRY_SHADER,
	PIXEL_SHADER,
	PIXEL_SHADER_HEIGHTMAP_VERSION, // Heightmap requires two textures instead of one
	COMPUTE_SHADER
};

class ShaderHandler
{
public:
	ShaderHandler();
	~ShaderHandler();

	void InitializeShaders(ID3D11Device* Dev, IDXGISwapChain* SwapChain);
	void PrepareRender(ID3D11DeviceContext* DevCon, RenderPassID passID, bool clearRenderTargets = true, bool isHeightMap = false);

	ID3D11RenderTargetView** GetBackBufferRTV();

private:
	void CreateShaders(ID3D11Device* Dev);

	void SetRenderTargets(ID3D11DeviceContext* DevCon, RenderPassID passID, bool clearRenderTargets = true);
	void SetInputLayoutAndTopology(ID3D11DeviceContext* DevCon, RenderPassID passID);
	void SetShaders(ID3D11DeviceContext* DevCon, RenderPassID passID, bool isHeightMap = false);
	void SetSamplers(ID3D11DeviceContext* DevCon, RenderPassID passID);
	void SetShaderResources(ID3D11DeviceContext* DevCon, RenderPassID passID);
	void SetRasterizerState(ID3D11DeviceContext* DevCon, RenderPassID passID);


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

	void CreateShader(ID3D11Device* Dev, ID3DBlob* pS, ShaderType shaderType, RenderPassID passID);
	
	// Used in CreateShaders
	void CreateInputLayout(ID3D11Device* Dev, ID3DBlob* pS, RenderPassID passID);
	void CreateRenderTextures(ID3D11Device* Dev, IDXGISwapChain* SwapChain);
	void CreateSamplerStates(ID3D11Device* Dev);
	void CreateRasterizerStates(ID3D11Device* Dev);

	struct GraphicsBuffer 
	{
		ID3D11Texture2D* texture = nullptr;
		ID3D11RenderTargetView* renderTargetView = nullptr;
		ID3D11ShaderResourceView* shaderResourceView = nullptr;
	};


	ID3D11RasterizerState* mRasterizerState[2];

	// Graphics Buffer --------------------------------------------------------
	GraphicsBuffer mGraphicsBuffer[GBUFFER_COUNT];
	ID3D11DepthStencilView* mDepthStecilView = nullptr;
	ID3D11Texture2D* mDepthStencilTexture    = nullptr;

	// Geometry Pass ----------------------------------------------------------
	ID3D11SamplerState* mGeometryPassSampler  = nullptr; //TODO: Rename
	ID3D11InputLayout*  mGeometryPassInputLayout = nullptr;

	ID3D11VertexShader*   mGeometryPassVertexShader         = nullptr;
	ID3D11GeometryShader* mGeometryPassGeometryShader       = nullptr;
	ID3D11PixelShader*    mGeometryPassPixelShader          = nullptr;
	ID3D11PixelShader*    mGeometryPassPixelHeightMapShader = nullptr;

	// Shadow Pass ------------------------------------------------------------
	ID3D11InputLayout* mShadowPassInputLayout = nullptr;

	ID3D11VertexShader* mShadowPassVertexShader = nullptr;
	ID3D11PixelShader*  mShadowPassPixelShader  = nullptr;

	ID3D11Texture2D*          mShadowMap          = nullptr;
	ID3D11DepthStencilView*   mShadowMapDepthView = nullptr;
	ID3D11ShaderResourceView* mShadowMapSRView    = nullptr;

	// Light Pass -------------------------------------------------------------
	ID3D11SamplerState* mShadowSampler = nullptr;

	ID3D11VertexShader* mFullScreenVertexShader = nullptr;
	ID3D11PixelShader*  mLightPassPixelShader   = nullptr;

	ID3D11RenderTargetView*	mLightPassOutputTextureRTV = nullptr;

	// Compute Pass -----------------------------------------------------------
	ID3D11ComputeShader* mComputeShader = nullptr;

	ID3D11ShaderResourceView*  mComputePassOutputTextureSRV = nullptr;
	ID3D11UnorderedAccessView* mComputePassTempTextureUAV   = nullptr;

	// Screen Pass ------------------------------------------------------------
	ID3D11PixelShader* mScreenPassPixelShader = nullptr;

	ID3D11ShaderResourceView* mSreenPassInputTextureSRV = nullptr;
	ID3D11RenderTargetView* mBackbufferRTV = nullptr; //TODO: Remove this from other classes
};


#endif // !SHADERHANDLER_HPP
