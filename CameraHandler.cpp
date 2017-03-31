#include "CameraHandler.hpp"

// public ------------------------------------------------------------------------------

CameraHandler::CameraHandler() : CAMERA_STARTING_POS(DirectX::XMVectorSet(10.0f, 40.0f, 10.0f, 1.0f))
{
	VPBufferData	= cPerFrameBuffer();
	SMBufferData = cPerFrameBuffer();
	mPerFrameBuffer = nullptr;
	mShadowMapBuffer = nullptr;
	freemoovingCamera = true;
	CAM_POS		= CAMERA_STARTING_POS;
	CAM_TARGET	= DirectX::XMVectorSet(20.0f, 5.0f, 20.0f, 0.0f);
	CAM_FORWARD = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	CAM_RIGHT	= DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	CAM_UP		= DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	DEFAULT_FORWARD = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	DEFAULT_RIGHT	= DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	DEFAULT_UP		= DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	MOVE_LR		= 0.0f;
	MOVE_BF		= 0.0f;
	MOVE_UD		= 0.0f;
	CAM_YAW		= 0.0f;
	CAM_PITCH	= 0.0f;
	SPEED		= 15.0f;
}

CameraHandler::~CameraHandler()
{

}

DirectX::XMFLOAT4 CameraHandler::GetCameraPosition()
{
	DirectX::XMFLOAT4 pos; 
	DirectX::XMStoreFloat4(&pos, CAM_POS);
	return pos;
}

//temp done
//Used in DetectInput()
void CameraHandler::UpdateCamera()
{
	//limits cam pitch in order to not spin around
	CAM_PITCH = std::min<float>(std::max<float>(CAM_PITCH, -1.57), 1.57);

	DirectX::XMMATRIX CAM_ROT_MAT;
	//transforms the cameras target
	CAM_ROT_MAT = DirectX::XMMatrixRotationRollPitchYaw(CAM_PITCH, CAM_YAW, 0.0f);
	CAM_TARGET	= DirectX::XMVector3TransformCoord(DEFAULT_FORWARD, CAM_ROT_MAT);
	CAM_TARGET	= DirectX::XMVector3Normalize(CAM_TARGET);

	DirectX::XMMATRIX YRotation_CAM_directions = DirectX::XMMatrixRotationY(CAM_YAW);
	//trnsforms the cameras directions
	CAM_RIGHT = DirectX::XMVector3TransformCoord(DEFAULT_RIGHT, YRotation_CAM_directions);

	////Camera follows the planes
	//CAM_FORWARD = XMVector3TransformCoord(DEFAULT_FORWARD, YRotation_CAM_directions);
	//CAM_UP = XMVector3TransformCoord(CAM_UP, YRotation_CAM_directions);

	//freelook camera
	CAM_FORWARD = DirectX::XMVector3Normalize(XMVector3TransformCoord(DEFAULT_FORWARD, CAM_ROT_MAT));
	CAM_UP		= DirectX::XMVector3Normalize(XMVector3TransformCoord(DEFAULT_UP, CAM_ROT_MAT));

	using DirectX::operator*;
	using DirectX::operator+=;
	using DirectX::operator+;

	//transforms the cameras position
	CAM_POS += MOVE_LR * CAM_RIGHT;
	CAM_POS += MOVE_BF * CAM_FORWARD;
	CAM_POS += MOVE_UD * CAM_UP;

	//following terrain
	int a= (int)DirectX::XMVectorGetX(CAM_POS), b =(int) DirectX::XMVectorGetZ(CAM_POS);
	if (a > 0 && b > 0 && a < terrain.worldDepth && b < terrain.worldWidth&&!freemoovingCamera) {
		CAM_POS = DirectX::XMVectorSet(DirectX::XMVectorGetX(CAM_POS), terrain.worldHeight[b][a] + 5, DirectX::XMVectorGetZ(CAM_POS), 1.0f);
	}

	MOVE_LR = 0.0f;
	MOVE_BF = 0.0f;
	MOVE_UD = 0.0f;

	CAM_TARGET = CAM_POS + CAM_TARGET;
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
		DirectX::XMMATRIX view = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(CAM_POS, CAM_TARGET, CAM_UP));
		DirectX::XMStoreFloat4x4(&VPBufferData.ViewProjection, DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&mCameraProjection), view));
		DirectX::XMStoreFloat4(&VPBufferData.CameraPosition, CAM_POS);
	}

	// TODO: check if map_write_discard is necessary and if it's required to make a mapped subresource
	D3D11_MAPPED_SUBRESOURCE viewProjectionMatrixPtr;
	DevCon->Map(mPerFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &viewProjectionMatrixPtr);
	memcpy(viewProjectionMatrixPtr.pData, &VPBufferData, sizeof(cPerFrameBuffer));
	DevCon->Unmap(mPerFrameBuffer, 0);

	DevCon->GSSetConstantBuffers(0, 1, &mPerFrameBuffer);

	return true;
}

bool CameraHandler::BindShadowMapPerFrameConstantBuffer(ID3D11DeviceContext* DevCon, RenderPassID passID)
{
	//TODO: this code block might not be needed
	// TODO: check if map_write_discard is necessary and if it's required to make a mapped subresource
	D3D11_MAPPED_SUBRESOURCE viewProjectionMatrixPtr;
	DevCon->Map(mShadowMapBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &viewProjectionMatrixPtr);
	memcpy(viewProjectionMatrixPtr.pData, &SMBufferData, sizeof(cPerFrameBuffer));
	DevCon->Unmap(mShadowMapBuffer, 0);
	
	if (passID == SHADOW_PASS) //ShadowVertex.hlsl
	{
		DevCon->VSSetConstantBuffers(0, 1, &mShadowMapBuffer);
	}
	
	if (passID == LIGHT_PASS) //LightFragment.hlsl
	{
		DevCon->PSSetConstantBuffers(0, 1, &mShadowMapBuffer);
	}

	return true;
}

//used in GraphicsHandler::InitializeGraphics
void CameraHandler::InitializeCamera(ID3D11Device* Dev, ID3D11DeviceContext* DevCon, int worldWidth, int worldDepth, float** worldHeight)
{

	terrain.worldDepth = worldDepth;
	terrain.worldWidth = worldWidth;
	terrain.worldHeight = worldHeight;
	CreateViewPorts();
	CreatePerFrameConstantBuffer(Dev);
	CreateShadowMapConstantBuffer(Dev);
}

//move to input class?
//used in wWinMain()
void CameraHandler::DetectInput(double time, HWND &hwnd)
{
	DIMOUSESTATE mouse_current_state;
	BYTE keyboardState[256];
	DIKeyboard->Acquire();
	DIMouse->Acquire();

	DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouse_current_state);
	DIKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);


	//closes the program
	if (keyboardState[DIK_ESCAPE] & 0x80) {
		PostMessage(hwnd, WM_DESTROY, 0, 0);
	}
	//all the different movements
	if (keyboardState[DIK_LSHIFT] & 0x80) {
		SPEED = 45.0f;
	}
	if (keyboardState[DIK_LCONTROL] & 0x80) {
		SPEED = 15.0f;
	}

	if (keyboardState[DIK_A] & 0x80) {

		MOVE_LR -= SPEED*time;
	}
	if (keyboardState[DIK_D] & 0x80) {
		MOVE_LR += SPEED*time;
	}
	if (keyboardState[DIK_W] & 0x80) {
		MOVE_BF += SPEED*time;
	}
	if (keyboardState[DIK_S] & 0x80) {
		MOVE_BF -= SPEED*time;
	}
	if (keyboardState[DIK_SPACE] & 0x80) {
		MOVE_UD += SPEED*time;
	}
	if (keyboardState[DIK_C] & 0x80) {
		MOVE_UD -= SPEED*time;
	}
	if (keyboardState[DIK_1] & 0x80) {
		freemoovingCamera = true;
	}
	if (keyboardState[DIK_2] & 0x80) {
		freemoovingCamera = false;
	}
	if (keyboardState[DIK_5] & 0x80) {
		GOD_CAMERA_ENABLED = true;
	}
	if (keyboardState[DIK_6] & 0x80) {
		GOD_CAMERA_ENABLED = false;
	}


	//mouse movement do change camera directions
	if ((mouse_current_state.lX != MOUSE_LAST_STATE.lX) || (mouse_current_state.lY != MOUSE_LAST_STATE.lY)) 
	{
		CAM_YAW += mouse_current_state.lX*0.001f;
		CAM_PITCH += mouse_current_state.lY*0.001f;
		MOUSE_LAST_STATE = mouse_current_state;
	}

	//reset camera directions and position
	if (keyboardState[DIK_Q] & 0x80) 
	{
		CAM_POS = CAMERA_STARTING_POS;
		CAM_TARGET = DirectX::XMVectorSet(5.0f, 5.0f, 5.0f, 0.0f);
		CAM_FORWARD = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		CAM_RIGHT = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		CAM_UP = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		CAM_PITCH = 0.0f;
		CAM_YAW = 0.0f;
	}
	MOUSE_LAST_STATE = mouse_current_state;
	UpdateCamera();
}

//used in wWinMain()
void CameraHandler::InitializeDirectInput(HINSTANCE &hInstance, HWND &hwnd) //creates the directx input, sets the data format
{
	HRESULT hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&DirectInput, NULL);

	hr = DirectInput->CreateDevice(GUID_SysKeyboard, &DIKeyboard, NULL);
	hr = DirectInput->CreateDevice(GUID_SysMouse, &DIMouse, NULL);

	hr = DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	hr = DIKeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE );

	hr = DIMouse->SetDataFormat(&c_dfDIMouse);
	hr = DIMouse->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);
}


// private ------------------------------------------------------------------------------

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
	float near_plane			= 0.1f;
	float far_plane				= 500.f;

	DirectX::XMMATRIX view = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(CAM_POS, CAM_TARGET, CAM_UP));
	DirectX::XMMATRIX projection = DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(degrees_field_of_view), aspect_ratio, near_plane, far_plane));
	DirectX::XMMATRIX vp = DirectX::XMMatrixMultiply(projection, view);

	DirectX::XMStoreFloat4x4(&VPBufferData.ViewProjection, vp);
	DirectX::XMStoreFloat4x4(&mCameraProjection, projection);

	// God camera for seeing the effects of teh frustum culling against quadtree
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
	float far_plane = 150.f;

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

DirectX::XMFLOAT4X4 CameraHandler::getProjection() {
	return mCameraProjection;
}
DirectX::XMFLOAT4X4 CameraHandler::getView() {
	DirectX::XMFLOAT4X4 temp;
	DirectX::XMStoreFloat4x4(&temp, DirectX::XMMatrixLookAtLH(DirectX::XMVectorScale(CAM_POS, -1.0f), DirectX::XMVectorScale(CAM_TARGET, -1.0f), { 0.0f, 1.0f, 0.0f }));
	 return temp;
}