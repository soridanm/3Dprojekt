#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "GlobalResources.hpp"
#include "GlobalSettings.hpp"

#include "TimeHandler.hpp"
#include "lighthandler.hpp"
#include "shaderhandler.hpp"
#include "camerahandler.hpp"
#include "objecthandler.hpp"

#include <vector>
#include <algorithm>

class Engine
{
public:
	Engine();
	~Engine();

	bool Initialize();
	bool Render();

	HRESULT eCreateDirect3DContext(HWND &wndHandle);

	void TimeFunction(HWND &wndHandle);

	// Public since it's used in main.cpp
	CameraHandler gCameraHandler; //dev/devcon set //move to engine? 
	IDXGISwapChain* gSwapChain	= nullptr;
private:
	void RenderGeometryPass();
	void RenderShadowPass();
	void RenderLightPass();
	void RenderComputePass();
	void RenderScreenPass();
	void SetHeightMapShaderResources(bool isHeightMap = false);

	ID3D11Device*	mDev		= nullptr;
	ID3D11DeviceContext* mDevCon = nullptr;

	ID3D11ShaderResourceView* grassTextureView;// = nullptr; //SHOULD BE MOVED TO MODEL
	ID3D11ShaderResourceView* stoneTextureView;

	TimeHandler	mTimeHandler;
	LightHandler mLightHandler;
	ObjectHandler mObjectHandler;
	ShaderHandler mShaderHandler;
};


#endif // !ENGINE_HPP
