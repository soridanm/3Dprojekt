#include "CameraHandler.hpp"

/*============================================================================
 *						Public functions
 *===========================================================================*/

CameraHandler::CameraHandler()
{}

CameraHandler::~CameraHandler()
{}

void CameraHandler::InitializeCamera(ID3D11Device* Dev, ID3D11DeviceContext* DevCon, int worldWidth, int worldDepth, float** worldHeight)
{
	mTerrainValues.worldDepth = worldDepth;
	mTerrainValues.worldWidth = worldWidth;
	mTerrainValues.worldHeight = worldHeight;
	CreateViewPorts();
	CreatePerFrameConstantBuffer(Dev);
	CreateShadowMapConstantBuffer(Dev);
}

void CameraHandler::UpdateCamera()
{
	using DirectX::operator*;
	using DirectX::operator+=;
	using DirectX::operator+;

	// Limits the camera's pitch so that it doesn't do front-/backflips
	mCamPitch = std::min<float>(std::max<float>(mCamPitch, -1.57f), 1.57f);

	DirectX::XMMATRIX CAM_ROT_MAT;
	// Transforms the camera's target
	CAM_ROT_MAT = DirectX::XMMatrixRotationRollPitchYaw(mCamPitch, mCamYaw, 0.0f);
	mCamTarget	= DirectX::XMVector3TransformCoord(mDEFAULT_FORWARD, CAM_ROT_MAT);
	mCamTarget	= DirectX::XMVector3Normalize(mCamTarget);

	DirectX::XMMATRIX YRotation_CAM_directions = DirectX::XMMatrixRotationY(mCamYaw);
	
	// Transforms the camera's directions
	mCamRight = DirectX::XMVector3TransformCoord(mDEFAULT_RIGHT, YRotation_CAM_directions);

	// Freelook camera
	mCamForward = DirectX::XMVector3Normalize(XMVector3TransformCoord(mDEFAULT_FORWARD, CAM_ROT_MAT));
	mCamUp		= DirectX::XMVector3Normalize(XMVector3TransformCoord(mDEFAULT_UP, CAM_ROT_MAT));

	// Transforms the camera's position
	mCamPos += mLeftRightMovement * mCamRight;
	mCamPos += mBackForwardMovement * mCamForward;
	mCamPos += mUpDownMovement * mCamUp;

	// Following terrain
	int cam_x = static_cast<int>(DirectX::XMVectorGetX(mCamPos));
	int cam_y = static_cast<int>(DirectX::XMVectorGetZ(mCamPos));

	if (!mFreemoveEnabled 
		&& cam_x > 0 && cam_y > 0 
		&& cam_x < mTerrainValues.worldDepth && cam_y < mTerrainValues.worldWidth)
	{
		mCamPos = DirectX::XMVectorSet(DirectX::XMVectorGetX(mCamPos), mTerrainValues.worldHeight[cam_y][cam_x] + 5.0f, DirectX::XMVectorGetZ(mCamPos), 1.0f);
	}

	mLeftRightMovement = 0.0f;
	mBackForwardMovement = 0.0f;
	mUpDownMovement = 0.0f;

	mCamTarget = mCamPos + mCamTarget;
}

bool CameraHandler::BindPerFrameConstantBuffer(ID3D11DeviceContext* DevCon)
{
	if (GOD_CAMERA_ENABLED)
	{
		VPBufferData.ViewProjection = mGodCameraViewProjection;
		
		DirectX::XMStoreFloat4(&VPBufferData.CameraPosition, mGOD_CAM_POS);
	}
	else
	{
		DirectX::XMMATRIX view = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(mCamPos, mCamTarget, mCamUp));
		DirectX::XMStoreFloat4x4(&VPBufferData.ViewProjection, DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&mCameraProjection), view));
		DirectX::XMStoreFloat4(&VPBufferData.CameraPosition, mCamPos);
	}

	D3D11_MAPPED_SUBRESOURCE viewProjectionMatrixPtr;
	DevCon->Map(mPerFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &viewProjectionMatrixPtr);
	memcpy(viewProjectionMatrixPtr.pData, &VPBufferData, sizeof(cPerFrameBuffer));
	DevCon->Unmap(mPerFrameBuffer, 0);

	DevCon->GSSetConstantBuffers(0, 1, &mPerFrameBuffer);

	return true;
}

bool CameraHandler::BindShadowMapPerFrameConstantBuffer(ID3D11DeviceContext* DevCon, RenderPassID passID)
{
	if (GOD_CAMERA_ENABLED)
	{
		DirectX::XMStoreFloat4(&SMBufferData.CameraPosition, mGOD_CAM_POS);
	}
	else
	{
		// Update the camera position that's used for the light calculations
		DirectX::XMStoreFloat4(&SMBufferData.CameraPosition, mCamPos);
	}

	D3D11_MAPPED_SUBRESOURCE viewProjectionMatrixPtr;
	DevCon->Map(mShadowMapBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &viewProjectionMatrixPtr);
	memcpy(viewProjectionMatrixPtr.pData, &SMBufferData, sizeof(cPerFrameBuffer));
	DevCon->Unmap(mShadowMapBuffer, 0);
	
	if (passID == SHADOW_PASS)
	{
		DevCon->VSSetConstantBuffers(0, 1, &mShadowMapBuffer);
	}
	
	if (passID == LIGHT_PASS)
	{
		DevCon->PSSetConstantBuffers(0, 1, &mShadowMapBuffer);
	}

	return true;
}

DirectX::XMFLOAT4 CameraHandler::GetCameraPosition()
{
	DirectX::XMFLOAT4 temp; 
	DirectX::XMStoreFloat4(&temp, mCamPos);
	return temp;
}

DirectX::XMFLOAT4 CameraHandler::GetCameraRight()
{
	DirectX::XMFLOAT4 temp;
	DirectX::XMStoreFloat4(&temp, mCamRight);
	return temp;
}

DirectX::XMFLOAT4X4 CameraHandler::GetProjection() 
{
	return mCameraProjection;
}

DirectX::XMFLOAT4X4 CameraHandler::GetView() 
{
	DirectX::XMFLOAT4X4 temp;
	DirectX::XMStoreFloat4x4(&temp, DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorScale(mCamPos, -1.0f), 
		DirectX::XMVectorScale(mCamTarget, -1.0f), 
		{ 0.0f, 1.0f, 0.0f })
	);
	 return temp;
}

void CameraHandler::DetectInput(double time, HWND &hwnd)
{
	DIMOUSESTATE mouse_current_state;
	BYTE keyboardState[256];
	DIKeyboard->Acquire();
	DIMouse->Acquire();

	DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouse_current_state);
	DIKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);

	// Closes the program
	if (keyboardState[DIK_ESCAPE] & 0x80) 
	{
		//PostMessage(hwnd, WM_DESTROY, 0, 0);
		PostMessage(hwnd, WM_CLOSE, 0, 0); // To prevent other applications from going fullscreen
	}

	mSpeed = (keyboardState[DIK_LSHIFT] & 0x80) ? 50.0f : 15.0f;

    float fTime = static_cast<float>(time);

	// WASD movement
	if (keyboardState[DIK_W] & 0x80) 
	{
		mBackForwardMovement += mSpeed*fTime;
	}
	if (keyboardState[DIK_A] & 0x80) 
	{
		mLeftRightMovement -= mSpeed*fTime;
	}
	if (keyboardState[DIK_S] & 0x80) 
	{
		mBackForwardMovement -= mSpeed*fTime;
	}
	if (keyboardState[DIK_D] & 0x80) 
	{
		mLeftRightMovement += mSpeed*fTime;
	}
	// Up-Down movement
	if (keyboardState[DIK_SPACE] & 0x80) 
	{
		mUpDownMovement += mSpeed*fTime;
	}
	if (keyboardState[DIK_C] & 0x80) 
	{
		mUpDownMovement -= mSpeed*fTime;
	}
	// Switch between freemove and following the terrain
	if (keyboardState[DIK_1] & 0x80) 
	{
		mFreemoveEnabled = true;
	}
	if (keyboardState[DIK_2] & 0x80) 
	{
		mFreemoveEnabled = false;
	}

	// Hold [E] to show the scene from above
	GOD_CAMERA_ENABLED = (keyboardState[DIK_E] & 0x80) ? true : false;

	// Camera movement if the mouse has moved
	if ((mouse_current_state.lX != mLastMouseState.lX) || (mouse_current_state.lY != mLastMouseState.lY)) 
	{
		mCamYaw += mouse_current_state.lX * 0.001f;
		mCamPitch += mouse_current_state.lY * 0.001f;
		mLastMouseState = mouse_current_state;
	}

	// Pressing [Q] resets the camera
	if (keyboardState[DIK_Q] & 0x80) 
	{
		mCamPos     = mCAMERA_STARTING_POS;
		mCamTarget  = DirectX::XMVectorSet(5.0f, 5.0f, 5.0f, 0.0f);
		mCamForward = mDEFAULT_FORWARD;
		mCamRight   = mDEFAULT_RIGHT;
		mCamUp      = mDEFAULT_UP;
		mCamPitch   = 0.0f;
		mCamYaw     = 0.0f;
	}
	mLastMouseState = mouse_current_state;
	UpdateCamera();
}

// Creates the DirectX input and sets the data format
void CameraHandler::InitializeDirectInput(HINSTANCE &hInstance, HWND &hwnd)
{
	HRESULT hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&DirectInput, NULL);

	hr = DirectInput->CreateDevice(GUID_SysKeyboard, &DIKeyboard, NULL);
	hr = DirectInput->CreateDevice(GUID_SysMouse, &DIMouse, NULL);

	hr = DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	hr = DIKeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE );

	hr = DIMouse->SetDataFormat(&c_dfDIMouse);
	hr = DIMouse->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);
}

/*=============================================================================
 *						Private functions
 *===========================================================================*/

void CameraHandler::CreateViewPorts()
{
	playerVP.Width	  = static_cast<FLOAT>(SCREEN_RESOLUTION.SCREEN_WIDTH);
	playerVP.Height	  = static_cast<FLOAT>(SCREEN_RESOLUTION.SCREEN_HEIGHT);
	playerVP.MinDepth = 0.0f;
	playerVP.MaxDepth = 1.0f;
	playerVP.TopLeftX = 0.f;
	playerVP.TopLeftY = 0.f;

	lightVP.Width    = static_cast<FLOAT>(SHADOW_QUALITY.SHADOW_MAP_SIZE);
	lightVP.Height	 = static_cast<FLOAT>(SHADOW_QUALITY.SHADOW_MAP_SIZE);
	lightVP.MinDepth = 0.0f;
	lightVP.MaxDepth = 1.0f;
	lightVP.TopLeftX = 0.f;
	lightVP.TopLeftY = 0.f;
}

bool CameraHandler::CreatePerFrameConstantBuffer(ID3D11Device* Dev)
{
	float aspect_ratio = static_cast<float>(SCREEN_RESOLUTION.SCREEN_WIDTH) / static_cast<float>(SCREEN_RESOLUTION.SCREEN_HEIGHT);
	float degrees_field_of_view = 90.0f;
	float near_plane = 0.1f;
	float far_plane	 = 500.f;

	DirectX::XMMATRIX view = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(mCamPos, mCamTarget, mCamUp));
	DirectX::XMMATRIX projection = DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(degrees_field_of_view), aspect_ratio, near_plane, far_plane));
	DirectX::XMMATRIX vp = DirectX::XMMatrixMultiply(projection, view);

	DirectX::XMStoreFloat4x4(&VPBufferData.ViewProjection, vp);
	DirectX::XMStoreFloat4x4(&mCameraProjection, projection);

	// God camera for seeing the effects of teh mFrustum culling against quadtree
	const DirectX::XMVECTOR GOD_CAM_TARGET = DirectX::XMVectorSet(100.0f, 0.0f, 99.0f, 0.0f);
	const DirectX::XMVECTOR GOD_CAM_UP = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	const DirectX::XMMATRIX GOD_CAM_VIEW = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(mGOD_CAM_POS, GOD_CAM_TARGET, GOD_CAM_UP));

	DirectX::XMMATRIX godVp = DirectX::XMMatrixMultiply(projection, GOD_CAM_VIEW);
	DirectX::XMStoreFloat4x4(&mGodCameraViewProjection, godVp);


	D3D11_BUFFER_DESC VPBufferDesc;
	VPBufferDesc.Usage			= D3D11_USAGE_DYNAMIC;
	VPBufferDesc.ByteWidth		= sizeof(cPerFrameBuffer);
	VPBufferDesc.BindFlags		= D3D11_BIND_CONSTANT_BUFFER;
	VPBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	VPBufferDesc.MiscFlags				= 0;
	VPBufferDesc.StructureByteStride	= 0;

	HRESULT hr = Dev->CreateBuffer(&VPBufferDesc, nullptr, &mPerFrameBuffer);
	if (FAILED(hr)) 
	{
		OutputDebugString(L"\nCameraHandler::CreatePerFrameConstantBuffers() Failed to create mPerFrameBuffer\n\n");
		exit(-1);
	}
	return true;
}

bool CameraHandler::CreateShadowMapConstantBuffer(ID3D11Device* Dev)
{
	float aspect_ratio = 1.0f;
	float degrees_field_of_view = 90.0f;
	float near_plane = 0.1f;
	float far_plane  = 150.f;

	DirectX::XMVECTOR LIGHT_POS = DirectX::XMVectorSet(100.0f, 120.0f, 100.0f, 0.0f);
	DirectX::XMVECTOR LIGHT_TARGET = DirectX::XMVectorSet(100.0f, 0.0f, 99.0f, 0.0f);
	DirectX::XMVECTOR LIGHT_UP = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	DirectX::XMMATRIX projection = DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(degrees_field_of_view), aspect_ratio, near_plane, far_plane));

	DirectX::XMMATRIX view = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(LIGHT_POS, LIGHT_TARGET, LIGHT_UP));
	
	DirectX::XMMATRIX vp = DirectX::XMMatrixMultiply(projection, view);

	DirectX::XMStoreFloat4x4(&SMBufferData.ViewProjection, vp);

	D3D11_BUFFER_DESC SMBufferDesc;
	SMBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	SMBufferDesc.ByteWidth = sizeof(cPerFrameBuffer);
	SMBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	SMBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	SMBufferDesc.MiscFlags = 0;
	SMBufferDesc.StructureByteStride = 0;

	HRESULT hr = Dev->CreateBuffer(&SMBufferDesc, nullptr, &mShadowMapBuffer);
	if (FAILED(hr)) 
	{
		OutputDebugString(L"\nCameraHandler::CreateShadowMapConstantBuffer() Failed to create mShadowMapBuffer\n\n");
		exit(-1);
	}

	return true;
}
