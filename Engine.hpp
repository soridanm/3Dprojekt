/**
* Course: DV142 - 3D-Programming
* Authors: Viktor Enfeldt, Peter Meunier
*
* File: Engine.hpp
*
* Class summary:
*	Creates and holds the Device, DeviceContext, and SwapChain pointers.
*	Is mainly responsible for the overaching structure of the render loop 
*	including which objects are to be rendered.
*	All [Noun]Handlers are ultimately members of this class.
*/

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "GlobalResources.hpp"
#include "GlobalSettings.hpp"

#include "TimeHandler.hpp"
#include "Lighthandler.hpp"
#include "Shaderhandler.hpp"
#include "Camerahandler.hpp"
#include "Objecthandler.hpp"

#include <vector>
#include <algorithm>

class Engine
{
public:
	Engine();
	~Engine();

	bool Initialize();
	bool Render();

	HRESULT CreateDirect3DContext(HWND &wndHandle);

	void UpdateInput(HWND &wndHandle);

	// Public since they're used in main.cpp
	CameraHandler gCameraHandler = CameraHandler();
	IDXGISwapChain* gSwapChain	 = nullptr;
private:
	void RenderGeometryPass();
	void RenderShadowPass();
	void RenderLightPass();
	void RenderComputePass();
	void RenderScreenPass();

	void SetHeightMapShaderResources();

	ID3D11Device* mDev = nullptr;
	ID3D11DeviceContext* mDevCon = nullptr;

	ID3D11ShaderResourceView* mGrassTextureView = nullptr;
	ID3D11ShaderResourceView* mStoneTextureView = nullptr;

	TimeHandler	  mTimeHandler   = TimeHandler();
	ObjectHandler mObjectHandler = ObjectHandler();
	ShaderHandler mShaderHandler = ShaderHandler();
	LightHandler  mLightHandler;
};


#endif // !ENGINE_HPP
