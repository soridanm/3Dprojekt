#include "Engine.hpp"


// private ------------------------------------------------------------------------------

// public ------------------------------------------------------------------------------

//TODO: Create all objects here
bool Engine::Initialize()
{

	mGraphicsHandler.CreateShaders();
	return true;
}

bool Engine::Render()
{
	mGraphicsHandler.RenderGeometryPass();
	mGraphicsHandler.RenderLightPass();
	
	return true;
}

Engine::Engine()
{
	//mGraphicsHandler = GraphicsHandler();
	//TODO: init object vector
}

Engine::~Engine()
{

}

IDXGISwapChain* Engine::GetSwapChain()
{
	return gSwapChain;
}

ID3D11Device* Engine::GetDevice()
{
	return gDevice;
}

ID3D11DeviceContext* Engine::GetDeviceContext()
{
	return gDeviceContext;
}


