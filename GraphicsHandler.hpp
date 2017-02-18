/*
* TODO: See what functions should be made private
*		Loop over objects in render function
*		Rename member pointers to mPointerName
*
*/


#ifndef GRAPHICSHANDLER_HPP
#define GRAPHICSHANDLER_HPP

#include "GlobalResources.hpp"

class GraphicsHandler
{
public:
	GraphicsHandler();
	~GraphicsHandler();

	bool CreateShaders();
	bool RenderGeometryPass();
	bool RenderLightPass();

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
	
	// Geometry Pass ----------------------------
	bool SetGeometryPassRenderTargets();
	bool SetGeometryPassShaders();
	bool SetGeometryPassViewProjectionBuffer();
	bool SetGeometryPassObjectBuffers();

	// Light Pass -------------------------------
	bool SetLightPassRenderTargets();
	bool SetLightPassShaders();
	bool SetLightPassLightBuffer();
	bool SetLightPassGBuffers();

	ID3D11InputLayout* gVertexLayout;
	ID3D11InputLayout* gVertexLayout;
	ID3D11VertexShader* gVertexShader;
	ID3D11GeometryShader* gGeometryShader;
	ID3D11PixelShader* gPixelShader;
	ID3D11ShaderResourceView* gTextureView;
	ID3D11VertexShader* gFullScreenTriangleShader;
	ID3D11PixelShader* gLightPixelShader;
	ID3D11RenderTargetView* gBackbufferRTV;
};


#endif // !GRAPHICSHANDLER_HPP
