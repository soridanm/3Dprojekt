#ifndef COMPUTESHADER_HPP
#define COMPUTESHADER_HPP

#include "GlobalResources.hpp"

class ComputeShader
{
public:
	ComputeShader();
	~ComputeShader();

	void CreateComputePassShaders(ID3D11Device* Dev);
	void CreateRenderTextures(); //rename to something with computeshader in it?
	void RenderComputeShader(ID3D11DeviceContext* DevCon);

private:
	//ID3D11Buffer* mVertexBufferQuad; //no vertex buffer since the triangle is generated in the vertex shader

	//ID3D11InputLayout* gVertexLayoutQuad = nullptr; //no input layout since triangle is generated in the vertex shader
	ID3D11VertexShader* mComputePassVertexShader = nullptr;
	ID3D11PixelShader* mComputePassPixelShader = nullptr;

	ID3D11ComputeShader* mComputeShader = nullptr;

	ID3D11RenderTargetView*	mRenderTextureRTV = nullptr;
	ID3D11ShaderResourceView* mRenderTextureSRV = nullptr;

	ID3D11UnorderedAccessView* mTempTextureUAV = nullptr;
	ID3D11ShaderResourceView* mTempTextureSRV = nullptr;

};



#endif // !COMPUTESHADER_HPP
