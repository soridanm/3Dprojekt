/*
* TODO: See what functions should be made private
*		Loop over objects in render function
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

const UINT GBUFFER_COUNT = 4;

class GraphicsHandler
{
public:
	GraphicsHandler();
	~GraphicsHandler();

	bool InitializeGraphics(ID3D11Device* Dev, ID3D11DeviceContext* DevCon);
	bool CreateShaders(ID3D11Device* Dev);
	void RenderGeometryPass(ID3D11DeviceContext* DevCon);
	void RenderLightPass(ID3D11Device* Dev, ID3D11DeviceContext* DevCon, IDXGISwapChain* SwapChain);

	ID3D11ShaderResourceView* gTextureView = nullptr; //SHOULD BE MOVED TO MODEL
	ID3D11RenderTargetView* mBackbufferRTV; //might be moved to Engine


	//Really not sure if it's best to have these public or not
	LightHandler mLightHandler;
	//ShaderHandler mShaderHandler;
	ObjectHandler mObjectHandler; //dev/devcon set
	CameraHandler mCameraHandler; //dev/devcon set //move to engine? 
private:
	bool CompileShader(
		ID3DBlob** pShader,
		LPCWSTR shaderFileName,
		LPCSTR entryPoint,
		LPCSTR shaderModel,
		const D3D_SHADER_MACRO* pDefines = nullptr,
		ID3DInclude* pInclude = nullptr,
		UINT Flags1 = 0,
		UINT flags2 = 0,
		ID3DBlob** ppErrorMsgs = nullptr
	);
	bool CreateInputLayout(ID3D11Device* Dev, ID3DBlob* pVS);
	bool InitializeGraphicsBuffer(ID3D11Device* Dev);
	
	// Geometry Pass ----------------------------
	void SetGeometryPassRenderTargets(ID3D11DeviceContext* DevCon);
	void SetGeometryPassShaders(ID3D11DeviceContext* DevCon); //move to shader class
	void SetGeometryPassShaderResources(ID3D11DeviceContext* DevCon);

	// Light Pass -------------------------------
	bool SetLightPassRenderTargets(ID3D11Device* Dev, ID3D11DeviceContext* DevCon, IDXGISwapChain* SwapChain);
	bool SetLightPassShaders(ID3D11DeviceContext* DevCon);
	bool SetLightPassGBuffers(ID3D11DeviceContext* DevCon);


	struct GraphicsBuffer {
		ID3D11Texture2D* texture = nullptr;
		ID3D11RenderTargetView* renderTargetView = nullptr;
		ID3D11ShaderResourceView* shaderResourceView = nullptr;
	}; 
	
	GraphicsBuffer mGraphicsBuffer[GBUFFER_COUNT];


	ID3D11DepthStencilView* mDepthStecilView;
	ID3D11Texture2D* mDepthStencilTexture;
	ID3D11InputLayout* mVertexLayout;
	ID3D11VertexShader* mGeometryPassVertexShader;
	ID3D11GeometryShader* mGeometryPassGeometryShader;
	ID3D11PixelShader* mGeometryPassPixelShader;
	ID3D11SamplerState* mSampleState;
	//ID3D11ShaderResourceView* mTextureView; //moved to ObjectHandler.hpp
	ID3D11VertexShader* mLightPassVertexShader;
	ID3D11PixelShader* mLightPassPixelShader;

};


#endif // !GRAPHICSHANDLER_HPP
