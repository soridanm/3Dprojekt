/*
* TODO: Move ALL global camera variables to this class
* If they need to be accessed from other classes then write getVar() funtions for them
*
*/

#ifndef CAMERAHANDLER_HPP
#define CAMERAHANDLER_HPP

#include "GlobalResources.hpp"

class CameraHandler
{
public:
	CameraHandler();
	~CameraHandler();

	DirectX::XMVECTOR GetCameraPosition();

	void UpdateCamera();
	bool BindPerFrameConstantBuffer(ID3D11DeviceContext* DevCon);
	void InitializeCamera(ID3D11Device* Dev, ID3D11DeviceContext* DevCon);
	void DetectInput(double time, HWND hwnd);
	void InitializeDirectInput(HINSTANCE hInstance, HWND hwnd);
	const LONG GetScreenWidth();
	const LONG GetScreenHeight();
private:
	void SetViewPort(ID3D11DeviceContext* DevCon);
	bool CreatePerFrameConstantBuffer(ID3D11Device* Dev);

	struct cPerFrameBuffer
	{
		//XMFLOAT4X4 ViewProjection;
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 Projection;
	};
	static_assert((sizeof(cPerFrameBuffer) % 16) == 0, "cPerFrameBuffer size must be 16-byte aligned");


	cPerFrameBuffer VPBufferData;
	ID3D11Buffer* mPerFrameBuffer;

	const DirectX::XMVECTOR CAMERA_STARTING_POS;
	DirectX::XMVECTOR CAM_POS;
	DirectX::XMVECTOR CAM_TARGET;
	DirectX::XMVECTOR CAM_FORWARD;
	DirectX::XMVECTOR CAM_RIGHT;
	DirectX::XMVECTOR CAM_UP;

	DirectX::XMVECTOR DEFAULT_FORWARD;
	DirectX::XMVECTOR DEFAULT_RIGHT;
	DirectX::XMVECTOR DEFAULT_UP;
	float MOVE_LR;
	float MOVE_BF;
	float MOVE_UD;
	float CAM_YAW;
	float CAM_PITCH;
	float SPEED;

	//input ------------------------------------------------------
	IDirectInputDevice8* DIKeyboard;
	IDirectInputDevice8* DIMouse;

	DIMOUSESTATE MOUSE_LAST_STATE;
	LPDIRECTINPUT8 DirectInput;

	const LONG SCREEN_WIDTH = 1920;
	const LONG SCREEN_HEIGHT = 1080;
};


#endif // !CAMERAHANDLER_HPP
