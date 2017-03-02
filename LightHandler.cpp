/*
* TODO: error handling, descriptions of functions, and add more lights
*
*/

#include "LightHandler.hpp"

// private ------------------------------------------------------------------------------



// public ------------------------------------------------------------------------------

LightHandler::LightHandler(DirectX::XMFLOAT4 CAM_POS)
{
	//DirectX::XMStoreFloat4(&mLightBufferData.cameraPositionWS, CAM_POS);
	mLightBufferData.cameraPositionWS = CAM_POS;
	mLightBufferData.globalAmbient = DirectX::XMFLOAT4(0.05f, 0.05f, 0.05f, 0.05f);

	//TODO: This for loop is already in .hpp
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
bool LightHandler::InitializeLights(ID3D11Device* Dev, DirectX::XMFLOAT4 CAM_POS)
{
	CreateLightBuffer(Dev);

	DirectX::XMFLOAT4 light_position = { 10.0f, 10.0f, 100.0f, 1.0f };
	DirectX::XMFLOAT3 light_color = Colors::Red;
	int has_s = 0;
	float c_att = 1.0f;
	float l_att = 0.0001f;
	float q_att = 0.001f;
	float amb   = 0.0001f;

	Light test_light(light_position, light_color, has_s, c_att, l_att, q_att, amb);
	mLightBufferData.LightArray[0] = test_light;

	DirectX::XMFLOAT4 shadowlight_position = { 100.0f, 100.0f, 100.0f, 0.0f };
	DirectX::XMFLOAT3 shadowlight_color = Colors::White;
	int sm_has_s = 1;
	float sm_c_att = 1.0f;
	float sm_l_att = 0.00001f;
	float sm_q_att = 0.0002f;
	float sm_amb   = 0.00001f;

	Light shadow_light(shadowlight_position, shadowlight_color, sm_has_s, sm_c_att, sm_l_att, sm_q_att, sm_amb);
	mLightBufferData.LightArray[1] = shadow_light;

	mLightBufferData.globalAmbient = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
	//DirectX::XMStoreFloat4(&mLightBufferData.cameraPositionWS, CAM_POS);
	mLightBufferData.cameraPositionWS = CAM_POS;

	return true;
}

//TODO: Make the movement time dependant and not frame dependant
// The light currently follows the camera's position
bool LightHandler::BindLightBuffer(ID3D11DeviceContext* DevCon, DirectX::XMFLOAT4 CAM_POS)
{
	// Move light up and down
	static int lightYMovement = 249;
	lightYMovement = (lightYMovement + 1) % 1000;
	float yMovement = (lightYMovement < 500) ? -100.1f : -100.1f;
	DirectX::XMMATRIX yMove = DirectX::XMMatrixTranslation(yMovement, 0.0f, 0.0f);

	DirectX::XMVECTOR lightPos = DirectX::XMLoadFloat4(&mLightBufferData.LightArray[0].PositionWS);
	lightPos = DirectX::XMVector4Transform(lightPos, yMove);

	//static double lightMovement = 0.01;
	//DirectX::XMVECTOR testMove = DirectX::XMVECTOR()

	//set the light position to the camera position
	//DirectX::XMStoreFloat4(&mLightBufferData.LightArray[0].PositionWS, CAM_POS);
	mLightBufferData.LightArray[0].PositionWS = CAM_POS;
	//update the camera position in the Light buffer
	//DirectX::XMStoreFloat4(&mLightBufferData.cameraPositionWS, CAM_POS);
	mLightBufferData.cameraPositionWS = CAM_POS;

	// Map light buffer
	D3D11_MAPPED_SUBRESOURCE LightBufferPtr;
	DevCon->Map(mLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &LightBufferPtr);
	memcpy(LightBufferPtr.pData, &mLightBufferData, sizeof(cLightBuffer));
	DevCon->Unmap(mLightBuffer, 0);

	DevCon->PSSetConstantBuffers(1, 1, &mLightBuffer);

	return true;
}

//TODO: look up what format is best to use. Do this for the GBuffer as well
bool LightHandler::CreateShadowMap(ID3D11Device* Dev)
{
	//Shadow map height and width
	UINT smHeight = 512;
	UINT smWidth = 512;

	//Shadow map texture desc
	D3D11_TEXTURE2D_DESC smTexDesc;
	smTexDesc.Width = smWidth;
	smTexDesc.Height = smHeight;
	smTexDesc.MipLevels = 1;
	smTexDesc.ArraySize = 1;
	smTexDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	smTexDesc.SampleDesc.Count = 1;
	smTexDesc.SampleDesc.Quality = 0;
	smTexDesc.Usage = D3D11_USAGE_DEFAULT;
	smTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; //
	smTexDesc.CPUAccessFlags = 0;
	smTexDesc.MiscFlags = 0;

	//Shadow map depth stencil view desc
	D3D11_DEPTH_STENCIL_VIEW_DESC smDescDSV{}; //{} might be unnecessary
	smDescDSV.Format = DXGI_FORMAT_D32_FLOAT;
	smDescDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	smDescDSV.Texture2D.MipSlice = 0;
	smDescDSV.Flags = 0;

	//Shadow map shader resource view desc
	D3D11_SHADER_RESOURCE_VIEW_DESC smSrvDesc {};
	smSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	smSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	smSrvDesc.Texture2D.MipLevels = smTexDesc.MipLevels;
	smSrvDesc.Texture2D.MostDetailedMip = 0;

	HRESULT hr;
	hr = Dev->CreateTexture2D(&smTexDesc, nullptr, &mShadowMap);
	if (FAILED(hr)) { return false; }
	hr = Dev->CreateDepthStencilView(mShadowMap, &smDescDSV, &mShadowMapDepthView);
	if (FAILED(hr)) { return false; }
	hr = Dev->CreateShaderResourceView(mShadowMap, &smSrvDesc, &mShadowMapSRView);
	if (FAILED(hr)) { return false; }

	return true;
}

