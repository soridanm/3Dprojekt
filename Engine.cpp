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

