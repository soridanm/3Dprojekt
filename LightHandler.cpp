#include "pch.h"

#include "LightHandler.hpp"

/*============================================================================
*						Public functions
*===========================================================================*/

LightHandler::LightHandler()
{}

LightHandler::~LightHandler()
{}

bool LightHandler::CreateLightBuffer(ID3D11Device* Dev)
{
	// initialize the description of the buffer.
	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(cLightBuffer);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// check if the creation failed for any reason
	HRESULT hr = Dev->CreateBuffer(&lightBufferDesc, nullptr, &mLightBuffer);
	if (FAILED(hr))
	{
		OutputDebugString(L"\nLightHandler::CreateLightBuffer() Failed to create buffer\n\n");
		exit(-1);
	}
	return true;
}

bool LightHandler::InitializeLights(ID3D11Device* Dev)
{
	CreateLightBuffer(Dev);

	// Light #1 (Follows the camera's position and doesn't cast a shadow)
	DirectX::XMFLOAT4 light_position = { 10.0f, 10.0f, 100.0f, 1.0f };
	DirectX::XMFLOAT3 light_color = Colors::Red;
	int has_s = 0;
	float c_att = 1.5f;
	float l_att = 0.001f;
	float q_att = 0.001f;
	float amb   = 0.001f;

	Light test_light(light_position, light_color, has_s, c_att, l_att, q_att, amb);
	mLightBufferData.LightArray[0] = test_light;

	// Light #2 (Stationary white light above the map that casts a shadow)
	DirectX::XMFLOAT4 shadowlight_position = { 100.0f, 100.0f, 100.0f, 0.0f };
	DirectX::XMFLOAT3 shadowlight_color = Colors::White;
	int sm_has_s = 1;
	float sm_c_att = 1.3f;
	float sm_l_att = 0.0001f;
	float sm_q_att = 0.0003f;
	float sm_amb   = 0.08f;

	Light shadow_light(shadowlight_position, shadowlight_color, sm_has_s, sm_c_att, sm_l_att, sm_q_att, sm_amb);
	mLightBufferData.LightArray[1] = shadow_light;

	// Other variables
	mLightBufferData.globalAmbient = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);

	return true;
}

bool LightHandler::BindLightBuffer(ID3D11DeviceContext* DevCon, DirectX::XMFLOAT4 camPos)
{
	//set light #1's position to the camera position
	mLightBufferData.LightArray[0].PositionWS = camPos;

	// Map light buffer
	D3D11_MAPPED_SUBRESOURCE LightBufferPtr;
	DevCon->Map(mLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &LightBufferPtr);
	memcpy(LightBufferPtr.pData, &mLightBufferData, sizeof(cLightBuffer));
	DevCon->Unmap(mLightBuffer, 0);

	DevCon->PSSetConstantBuffers(1, 1, &mLightBuffer);

	return true;
}
