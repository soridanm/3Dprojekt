#include "Engine.hpp"


// private ------------------------------------------------------------------------------

bool Engine::RenderGeometryPass()
{
	mGraphicsHandler.SetGeometryPassRenderTargets();
	mGraphicsHandler.SetGeometryPassShaders();
	mGraphicsHandler.SetGeometryPassViewProjectionBuffer();



	//draw geometry
	gDeviceContext->DrawIndexed(NUMBER_OF_FACES * 3, 0, 0);
	return true;
}


bool Engine::RenderLightPass()
{

	return true;
}

// public ------------------------------------------------------------------------------

Engine::Engine()
{

}

Engine::~Engine()
{

}

bool Engine::Initialize()
{

}

bool Engine::Render()
{

}