#include "ObjectHandler.hpp"


ObjectHandler::ObjectHandler()
{
	gPerObjectBuffer	= nullptr;
	ObjectBufferData	= cPerObjectBuffer();
	gMaterialBuffer		= nullptr;
	gMaterialBufferData = cMaterialBuffer();
	mTextureView		= nullptr;
}

ObjectHandler::~ObjectHandler()
{

}

bool ObjectHandler::SetGeometryPassObjectBuffers()
{
	UINT32 vertexSize = sizeof(float) * 5;
	UINT32 offset = 0;
	UINT32 squareVertexSize = sizeof(float) * 8;

	// set textures and constant buffers
	gDeviceContext->PSSetShaderResources(0, 1, &mTextureView);
	//gDeviceContext->PSSetConstantBuffers(0, 1, &gMaterialBuffer);

	// HEIGHT-MAP BEGIN ---------------------------------------------------------------------------

	//gDeviceContext->IASetVertexBuffers(0, 1, &gVertexBuffer, &vertexSize, &offset);
	gDeviceContext->IASetIndexBuffer(gSquareIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	gDeviceContext->IASetVertexBuffers(0, 1, &gSquareVertBuffer, &squareVertexSize, &offset);

	// HEIGHT-MAP END ---------------------------------------------------------------------------

	// update per-object buffer to spin cube
	static float rotation = 0.0f;
	//rotation += CUBE_ROTATION_SPEED;

	DirectX::XMStoreFloat4x4(&ObjectBufferData.World, DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationY(rotation)));

	D3D11_MAPPED_SUBRESOURCE worldMatrixPtr;
	gDeviceContext->Map(gPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &worldMatrixPtr);
	// copy memory from CPU to GPU of the entire struct
	memcpy(worldMatrixPtr.pData, &ObjectBufferData, sizeof(cPerObjectBuffer));
	// Unmap constant buffer so that we can use it again in the GPU
	gDeviceContext->Unmap(gPerObjectBuffer, 0);
	// set resource to Geometry Shader
	gDeviceContext->GSSetConstantBuffers(1, 1, &gPerObjectBuffer);

	// Map material properties buffer

	//SetMaterial(Materials::Black_plastic);
	gMaterialBufferData.material = Materials::Black_plastic;

	D3D11_MAPPED_SUBRESOURCE materialPtr;
	gDeviceContext->Map(gMaterialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &materialPtr);
	memcpy(materialPtr.pData, &gMaterialBufferData, sizeof(cMaterialBuffer));
	//gDeviceContext->Unmap(mPerFrameBuffer, 0);
	gDeviceContext->PSSetConstantBuffers(0, 1, &gMaterialBuffer);

	return true;
}

// private ---------------------------------------------------------------------------------------

void ObjectHandler::CreatePerObjectConstantBuffer()
{
	DirectX::XMMATRIX world = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationY(0.0f));

	DirectX::XMStoreFloat4x4(&ObjectBufferData.World, world);

	D3D11_BUFFER_DESC WBufferDesc;
	WBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	WBufferDesc.ByteWidth = sizeof(cPerObjectBuffer);
	WBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	WBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	WBufferDesc.MiscFlags = 0;
	WBufferDesc.StructureByteStride = 0;

	gHR = gDevice->CreateBuffer(&WBufferDesc, nullptr, &gPerObjectBuffer);
	if (FAILED(gHR)) {
		exit(-1);
	}
}

void ObjectHandler::CreateMaterialConstantBuffer()
{
	D3D11_BUFFER_DESC materialBufferDesc;
	materialBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	materialBufferDesc.ByteWidth = sizeof(cMaterialBuffer);
	materialBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	materialBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	materialBufferDesc.MiscFlags = 0;
	materialBufferDesc.StructureByteStride = 0;

	// check if the creation failed for any reason
	HRESULT hr = 0;
	hr = gDevice->CreateBuffer(&materialBufferDesc, nullptr, &gMaterialBuffer);
	if (FAILED(hr))
	{
		// handle the error, could be fatal or a warning...
		exit(-1);
	}
}
