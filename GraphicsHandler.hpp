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

const UINT GBUFFER_COUNT = 4;

class GraphicsHandler
{
public:
	GraphicsHandler();
	~GraphicsHandler();

	bool Initialize();
	bool CreateShaders();
	void RenderGeometryPass();
	void RenderLightPass();

private:
	bool CompileShader(
		ID3DBlob* pShader,
		LPCWSTR shaderFileName,
		LPCSTR entryPoint,
		LPCSTR shaderModel,
		const D3D_SHADER_MACRO* pDefines = nullptr,
		ID3DInclude* pInclude = nullptr,
		UINT Flags1 = 0,
		UINT flags2 = 0,
		ID3DBlob** ppErrorMsgs = nullptr
	);
	bool CreateInputLayout(ID3DBlob* pVS);
	bool InitializeGraphicsBuffer();
	void CreateAllConstantBuffers();
	
	// Geometry Pass ----------------------------
	void SetGeometryPassRenderTargets();
	void SetGeometryPassShaders(); //move to shader class
	bool SetGeometryPassObjectBuffers(); //move to object class

	// Light Pass -------------------------------
	bool SetLightPassRenderTargets();
	bool SetLightPassShaders();
	bool SetLightPassGBuffers();


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
	ID3D11ShaderResourceView* mTextureView;
	ID3D11VertexShader* mLightPassVertexShader;
	ID3D11PixelShader* mLightPassPixelShader;
	ID3D11RenderTargetView* mBackbufferRTV;

	LightHandler mLightHandler;
	//ShaderHandler mShaderHandler;
	//ObjectHandler mObjectHandler;
	CameraHandler mCameraHandler;
};


#endif // !GRAPHICSHANDLER_HPP
