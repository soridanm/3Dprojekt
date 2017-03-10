#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <vector>

#include "GraphicsHandler.hpp"
#include "TimeHandler.hpp"

class Engine
{
public:
	Engine();
	//Engine(const Engine& other);
	~Engine();

	bool Initialize(ShadowQuality shadowQuality = SHADOWS_HIGH);
	bool Render();

	HRESULT eCreateDirect3DContext(HWND &wndHandle);

	void TimeFunction(HWND &wndHandle);

	ID3D11Device**	GetDevice();
	IDXGISwapChain**	GetSwapChain();
	ID3D11DeviceContext** GetDeviceContext();
	ID3D11RenderTargetView** GetBackBufferRTV();
	ID3D11ShaderResourceView** Engine::GetTextureView();

	ID3D11Device*	gDevice		= nullptr;
	IDXGISwapChain* gSwapChain	= nullptr;
	ID3D11DeviceContext* gDeviceContext = nullptr;


	GraphicsHandler mGraphicsHandler;
	TimeHandler		mTimeHandler;
private:


	//std::vector<Object> mObjects;



};


#endif // !ENGINE_HPP
