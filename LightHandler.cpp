/*
* TODO: error handling, descriptions of functions, and add more lights
*
*/

#include "LightHandler.hpp"

// private ------------------------------------------------------------------------------



// public ------------------------------------------------------------------------------

LightHandler::LightHandler(DirectX::XMVECTOR CAM_POS)
{
	DirectX::XMStoreFloat4(&mLightBufferData.cameraPositionWS, CAM_POS);
	mLightBufferData.globalAmbient = DirectX::XMFLOAT4(0.05f, 0.05f, 0.05f, 0.05f);

	for (int i = 0; i < NR_OF_LIGHTS; i++)
	{
		mLightBufferData.LightArray[i] = Light();
	}
	mLightBuffer = nullptr;
}

LightHandler::~LightHandler()
{

}

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
	HRESULT gHR = Dev->CreateBuffer(&lightBufferDesc, nullptr, &mLightBuffer);
	if (FAILED(gHR))
	{
		// handle the error, could be fatal or a warning...
		exit(-1);
	}
	return true;
}

//used in 
//Currently only sets one light
bool LightHandler::InitializeLights(ID3D11Device* Dev, DirectX::XMVECTOR CAM_POS)
{
	CreateLightBuffer(Dev);

	DirectX::XMFLOAT4 light_position = { 10.0f, 10.0f, 100.0f, 1.0f };
	DirectX::XMFLOAT4 light_color = Colors::White;
	float c_att = 1.0f;
	float l_att = 0.0001f;
	float q_att = 0.001f;
	float amb = 0.0001f;

	Light test_light(light_position, light_color, c_att, l_att, q_att, amb);
	mLightBufferData.LightArray[0] = test_light;

	mLightBufferData.globalAmbient = DirectX::XMFLOAT4(0.05f, 0.05f, 0.05f, 1.0f);
	DirectX::XMStoreFloat4(&mLightBufferData.cameraPositionWS, CAM_POS);

	return true;
}

//TODO: Make the movement time dependant and not frame dependant
// The light currently follows the camera's position
bool LightHandler::BindLightBuffer(ID3D11DeviceContext* DevCon, DirectX::XMVECTOR CAM_POS)
{
	// Move light up and down
	/*static int lightYMovement = 249;
	lightYMovement = (lightYMovement + 1) % 1000;
	float yMovement = (lightYMovement < 500) ? -0.01f : 0.01f;
	DirectX::XMMATRIX yMove = DirectX::XMMatrixTranslation(0.0f, yMovement, 0.0f);
*/
	/*DirectX::XMVECTOR lightPos = DirectX::XMLoadFloat4(&mLightBufferData.LightArray[0].PositionWS);
	lightPos = DirectX::XMVector4Transform(lightPos, yMove);*/

	//update the camera position in the Light buffer
	DirectX::XMStoreFloat4(&mLightBufferData.cameraPositionWS, CAM_POS);
	//set the light position to the camera position
	DirectX::XMStoreFloat4(&mLightBufferData.LightArray[0].PositionWS, CAM_POS);

	// Map light buffer
	D3D11_MAPPED_SUBRESOURCE LightBufferPtr;
	DevCon->Map(mLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &LightBufferPtr);
	memcpy(LightBufferPtr.pData, &mLightBufferData, sizeof(cLightBuffer));
	DevCon->Unmap(mLightBuffer, 0);

	DevCon->PSSetConstantBuffers(0, 1, &mLightBuffer);

	return true;
}


