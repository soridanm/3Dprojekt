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

	bool Initialize();
	bool Render();



	IDXGISwapChain* GetSwapChain();
	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();
private:


	GraphicsHandler mGraphicsHandler;
	TimeHandler		mTimeHandler;
	//std::vector<Object> mObjects;

	IDXGISwapChain* gSwapChain;// = nullptr;
	ID3D11Device* gDevice;// = nullptr;
	ID3D11DeviceContext* gDeviceContext;// = nullptr;


};


#endif // !ENGINE_HPP
