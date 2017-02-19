#include "CameraHandler.hpp"


// public ------------------------------------------------------------------------------

CameraHandler::CameraHandler()
{
	VPBufferData = cPerFrameBuffer();
	mPerFrameBuffer = nullptr;
}

CameraHandler::~CameraHandler()
{

}

//temp done
void CameraHandler::UpdateCamera()
{
	//limits cam pitch in order to not spin around
	if (CAM_PITCH < -1.5f)
	{
		CAM_PITCH = -1.5f;
	}
	if (CAM_PITCH > 1.5f)
	{
		CAM_PITCH = 1.5f;
	}
	DirectX::XMMATRIX CAM_ROT_MAT;
	//transforms the cameras target
	CAM_ROT_MAT = DirectX::XMMatrixRotationRollPitchYaw(CAM_PITCH, CAM_YAW, 0.0f);
	CAM_TARGET = DirectX::XMVector3TransformCoord(DEFAULT_FORWARD, CAM_ROT_MAT);
	CAM_TARGET = DirectX::XMVector3Normalize(CAM_TARGET);

	DirectX::XMMATRIX YRotation_CAM_directions = DirectX::XMMatrixRotationY(CAM_YAW);
	//trnsforms the cameras directions
	CAM_RIGHT = DirectX::XMVector3TransformCoord(DEFAULT_RIGHT, YRotation_CAM_directions);

	////Camera follows the planes
	//CAM_FORWARD = XMVector3TransformCoord(DEFAULT_FORWARD, YRotation_CAM_directions);
	//CAM_UP = XMVector3TransformCoord(CAM_UP, YRotation_CAM_directions);

	//freelook camera
	CAM_FORWARD = DirectX::XMVector3Normalize(XMVector3TransformCoord(DEFAULT_FORWARD, CAM_ROT_MAT));
	CAM_UP = DirectX::XMVector3Normalize(XMVector3TransformCoord(DEFAULT_UP, CAM_ROT_MAT));

	using DirectX::operator*;
	using DirectX::operator+=;
	using DirectX::operator+;

	//transforms the cameras position
	CAM_POS += MOVE_LR * CAM_RIGHT;
	CAM_POS += MOVE_BF * CAM_FORWARD;
	CAM_POS += MOVE_UD * CAM_UP;

	//following terrain
	//int a= XMVectorGetX(CAM_POS), b = XMVectorGetZ(CAM_POS);
	//if (a > 0 && b > 0 && a < 200 && b < 200) {
	//	CAM_POS = XMVectorSet(XMVectorGetX(CAM_POS), WORLD_HEIGHT[b][a] + 3, XMVectorGetZ(CAM_POS), 1.0f);
	//}

	MOVE_LR = 0.0f;
	MOVE_BF = 0.0f;
	MOVE_UD = 0.0f;

	CAM_TARGET = CAM_POS + CAM_TARGET;
}

bool CameraHandler::BindPerFrameConstantBuffer()
{
	// TODO: check if map_write_discard is necessary and if it's required to make a mapped subresource
	D3D11_MAPPED_SUBRESOURCE viewProjectionMatrixPtr;
	gDeviceContext->Map(mPerFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &viewProjectionMatrixPtr);

	DirectX::XMMATRIX view = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(CAM_POS, CAM_TARGET, CAM_UP));
	XMStoreFloat4x4(&VPBufferData.View, view);

	memcpy(viewProjectionMatrixPtr.pData, &VPBufferData, sizeof(cPerFrameBuffer));
	//gDeviceContext->Unmap(mPerFrameBuffer, 0);
	gDeviceContext->GSSetConstantBuffers(0, 1, &mPerFrameBuffer);

	return true;
}

void CameraHandler::InitializeCamera()
{
	SetViewPort();
	CreatePerFrameConstantBuffer();
}

// private ------------------------------------------------------------------------------

void CameraHandler::SetViewPort()
{
	D3D11_VIEWPORT vp;
	vp.Width	= (FLOAT)SCREEN_WIDTH;
	vp.Height	= (FLOAT)SCREEN_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.f;
	vp.TopLeftY = 0.f;

	gDeviceContext->RSSetViewports(1, &vp);
}

bool CameraHandler::CreatePerFrameConstantBuffer()
{
	float aspect_ratio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
	float degrees_field_of_view = 90.0f;
	float near_plane = 0.1f;
	float far_plane = 1000.f;

	//camera, look at, up
	DirectX::XMVECTOR camera = CAMERA_STARTING_POS;
	DirectX::XMVECTOR look_at = CAM_TARGET;
	DirectX::XMVECTOR up = CAM_UP;

	DirectX::XMMATRIX view = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(camera, look_at, up));

	DirectX::XMMATRIX projection = DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(degrees_field_of_view), aspect_ratio, near_plane, far_plane));

	DirectX::XMStoreFloat4x4(&VPBufferData.Projection, projection);
	DirectX::XMStoreFloat4x4(&VPBufferData.View, view);

	D3D11_BUFFER_DESC VPBufferDesc;
	VPBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	VPBufferDesc.ByteWidth = sizeof(cPerFrameBuffer);
	VPBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	VPBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	VPBufferDesc.MiscFlags = 0;
	VPBufferDesc.StructureByteStride = 0;

	gHR = gDevice->CreateBuffer(&VPBufferDesc, nullptr, &mPerFrameBuffer);
	if (FAILED(gHR)) {
		exit(-1);
	}

	return true;
}
