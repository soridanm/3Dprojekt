/*
* TODO: See what functions should be made private
*		Rename member pointers to mPointerName
*		Make a seperate shader class
*
*/


#ifndef GRAPHICSHANDLER_HPP
#define GRAPHICSHANDLER_HPP

#include "GlobalResources.hpp"
#include "LightHandler.hpp"
#include "CameraHandler.hpp"
#include "ObjectHandler.hpp"
#include "ComputeShader.hpp"
#include <algorithm>

//const UINT GBUFFER_COUNT = 4;

class GraphicsHandler
{
public:
	GraphicsHandler();
	~GraphicsHandler();

	bool InitializeGraphics(ID3D11Device* Dev, ID3D11DeviceContext* DevCon);
	//bool CreateShaders(ID3D11Device* Dev);															// MOVE TO SHADERHANDLER!
	void RenderGeometryPass(ID3D11DeviceContext* DevCon);
	void RenderShadowPass(ID3D11DeviceContext* DevCon);
	void RenderLightPass(ID3D11Device* Dev, ID3D11DeviceContext* DevCon, IDXGISwapChain* SwapChain);
	void RenderComputePass(ID3D11DeviceContext* DevCon);


	ID3D11ShaderResourceView* gTextureView;// = nullptr; //SHOULD BE MOVED TO MODEL
	ID3D11ShaderResourceView* sTextureView;
	ID3D11RenderTargetView* mBackbufferRTV; //might be moved to Engine
	//Really not sure if it's best to have these public or not
	LightHandler mLightHandler;
	//ShaderHandler mShaderHandler;
	ObjectHandler mObjectHandler; //dev/devcon set
	CameraHandler mCameraHandler; //dev/devcon set //move to engine? 
private:
	//bool CompileShader(																						// MOVE TO SHADERHANDLER!
	//	ID3DBlob** pShader,
	//	LPCWSTR shaderFileName,
	//	LPCSTR entryPoint,
	//	LPCSTR shaderModel,
	//	const D3D_SHADER_MACRO* pDefines = nullptr,
	//	ID3DInclude* pInclude = nullptr,
	//	UINT Flags1 = 0,
	//	UINT flags2 = 0,
	//	ID3DBlob** ppErrorMsgs = nullptr
	//);
	bool CreateInputLayout(ID3D11Device* Dev, ID3DBlob* pVS);												// MOVE TO SHADERHANDLER!
	bool InitializeGraphicsBuffer(ID3D11Device* Dev);
	bool CreateRasterizerStates(ID3D11Device* Dev);
	void SetRasterizerState(ID3D11DeviceContext* DevCon, RenderPassID passID);

	// Geometry Pass ----------------------------
	void SetGeometryPassRenderTargets(ID3D11DeviceContext* DevCon);											// MOVE TO SHADERHANDLER!
	void SetGeometryPassShaders(ID3D11DeviceContext* DevCon, bool isHeightMap = false);						// MOVE TO SHADERHANDLER!
	void SetGeometryPassShaderResources(ID3D11DeviceContext* DevCon, bool isHeightMap = false);

	// Shadow mapping Pass ----------------------
	void SetShadowMapPassRenderTargets(ID3D11DeviceContext* DevCon);										// MOVE TO SHADERHANDLER!
	void SetShadowMapPassShaders(ID3D11DeviceContext* DevCon);												// MOVE TO SHADERHANDLER!
	void SetShadowMapPassShaderResources(ID3D11DeviceContext* DevCon);

	// Light Pass -------------------------------
	bool SetLightPassRenderTargets(ID3D11Device* Dev, ID3D11DeviceContext* DevCon, IDXGISwapChain* SwapChain); // MOVE TO SHADERHANDLER!
	bool SetLightPassShaders(ID3D11DeviceContext* DevCon);														// MOVE TO SHADERHANDLER!
	bool SetLightPassGBuffers(ID3D11DeviceContext* DevCon);														// MOVE TO SHADERHANDLER!

	// Compute Pass -------------------------------
	/*void SetComputePassUnorderedAccessViews(ID3D11DeviceContext* DevCon);
	void SetComputePassShaders(ID3D11DeviceContext* DevCon);
	void SetComputePassShaderResources(ID3D11DeviceContext* DevCon);*/

	//struct GraphicsBuffer { 					// MOVE TO SHADERHANDLER!
	//	ID3D11Texture2D* texture = nullptr;
	//	ID3D11RenderTargetView* renderTargetView = nullptr;
	//	ID3D11ShaderResourceView* shaderResourceView = nullptr;
	//}; 
	
	//GraphicsBuffer mGraphicsBuffer[GBUFFER_COUNT];


	ID3D11DepthStencilView* mDepthStecilView = nullptr;
	ID3D11Texture2D* mDepthStencilTexture = nullptr;
	ID3D11InputLayout* mVertexLayout = nullptr;

	ID3D11RasterizerState* mRasterizerState[2];

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

	//Compute Pass
	ComputeShader mComputeShader;
};


#endif // !GRAPHICSHANDLER_HPP
