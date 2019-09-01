/**
* Course: DV1542 - 3D-Programming
* Authors: Viktor Enfeldt, Peter Meunier
*
* File: CameraHandler.hpp
*
* Class summary:
*	Is responsible for updating the camera's various values, creating/binding the
*	ViewProjection-matrix constant buffers, as well as processing the input from
*	the mouse and keyboard.
*/


#ifndef CAMERAHANDLER_HPP
#define CAMERAHANDLER_HPP

#include "GlobalResources.hpp"
#include "GlobalSettings.hpp"

#include "FrustumHandler.hpp"

class CameraHandler
{
public:
	CameraHandler();
	~CameraHandler();

	void InitializeCamera(ID3D11Device* Dev, ID3D11DeviceContext* DevCon, int worldWidth, int worldDepth, float** worldHeight);
	void UpdateCamera();
	bool BindPerFrameConstantBuffer(ID3D11DeviceContext* DevCon);
	bool BindShadowMapPerFrameConstantBuffer(ID3D11DeviceContext* DevCon, RenderPassID passID);
	
	DirectX::XMFLOAT4   GetCameraPosition();
	DirectX::XMFLOAT4   GetCameraRight();
	DirectX::XMFLOAT4   GetCameraUp();
	DirectX::XMFLOAT4   GetCameraForward();
	DirectX::XMFLOAT4X4 GetProjection();
	DirectX::XMFLOAT4X4 GetView();

	D3D11_VIEWPORT lightVP;
	D3D11_VIEWPORT playerVP;
	
	//input ----------------------------------------------------------------------
	void DetectInput(double time, HWND &hwnd);
	void InitializeDirectInput(HINSTANCE &hInstance, HWND &hwnd);

	LPDIRECTINPUT8 DirectInput;
	IDirectInputDevice8* DIKeyboard = nullptr;
	IDirectInputDevice8* DIMouse	= nullptr;
private:
	void CreateViewPorts();
	bool CreatePerFrameConstantBuffer(ID3D11Device* Dev);
	bool CreateShadowMapConstantBuffer(ID3D11Device* Dev);
	
	struct cPerFrameBuffer
	{
		DirectX::XMFLOAT4X4 ViewProjection;
		DirectX::XMFLOAT4 CameraPosition;
	};
	static_assert((sizeof(cPerFrameBuffer) % 16) == 0, "cPerFrameBuffer size must be 16-byte aligned");

	// Constant Buffers ----------------------------------------------------------
	cPerFrameBuffer VPBufferData  = cPerFrameBuffer();
	ID3D11Buffer* mPerFrameBuffer = nullptr;

	cPerFrameBuffer SMBufferData   = cPerFrameBuffer();
	ID3D11Buffer* mShadowMapBuffer = nullptr;

	// Camera movement -----------------------------------------------------------
	bool mFreemoveEnabled = true;
	const DirectX::XMVECTOR mCAMERA_STARTING_POS = DirectX::XMVectorSet(10.0f, 40.0f, 10.0f, 1.0f);
	const DirectX::XMVECTOR mDEFAULT_FORWARD     = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	const DirectX::XMVECTOR mDEFAULT_RIGHT       = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	const DirectX::XMVECTOR mDEFAULT_UP          = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	
	DirectX::XMVECTOR mCamTarget  = DirectX::XMVectorSet(20.0f, 5.0f, 20.0f, 0.0f);
	DirectX::XMVECTOR mCamPos     = mCAMERA_STARTING_POS;
	DirectX::XMVECTOR mCamForward = mDEFAULT_FORWARD;
	DirectX::XMVECTOR mCamRight	  = mDEFAULT_RIGHT;
	DirectX::XMVECTOR mCamUp      = mDEFAULT_UP;

	float mBackForwardMovement = 0.0f;
	float mLeftRightMovement   = 0.0f;
	float mUpDownMovement      = 0.0f;
	float mCamPitch = 0.0f;
	float mCamYaw   = 0.0f;
	float mSpeed    = 15.0f;

	DirectX::XMFLOAT4X4 mCameraProjection;

	// God camera to show the effects of the mFrustum culling ---------------------
	const DirectX::XMVECTOR mGOD_CAM_POS = DirectX::XMVectorSet(100.0f, 100.0f, 100.0f, 0.0f);
	DirectX::XMFLOAT4X4 mGodCameraViewProjection;

	// For walking on the mTerrainValues ------------------------------------------------
	struct terrainValues 
	{
		int worldWidth;
		int worldDepth;
		float** worldHeight;
	};
	terrainValues mTerrainValues;

	// Input ---------------------------------------------------------------------
	DIMOUSESTATE mLastMouseState;

};

#endif // !CAMERAHANDLER_HPP
