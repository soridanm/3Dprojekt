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

	void UpdateCamera();
	bool BindPerFrameConstantBuffer();
	void InitializeCamera();
private:
	void SetViewPort();
	bool CreatePerFrameConstantBuffer();

	struct cPerFrameBuffer
	{
		//XMFLOAT4X4 ViewProjection;
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 Projection;
	}; 

	cPerFrameBuffer VPBufferData;
	ID3D11Buffer* mPerFrameBuffer;

};


#endif // !CAMERAHANDLER_HPP
