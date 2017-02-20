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
	bool BindPerFrameConstantBuffer();
	void InitializeCamera();
	void DetectInput(double time, HWND hwnd);
	void InitializeDirectInput(HINSTANCE hInstance, HWND hwnd);
private:
	void SetViewPort();
	bool CreatePerFrameConstantBuffer();

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
};


#endif // !CAMERAHANDLER_HPP
