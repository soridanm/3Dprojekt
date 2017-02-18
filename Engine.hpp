#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <vector>

#include "Object.hpp"
#include "GraphicsHandler.hpp"


class Engine
{
public:
	Engine();
	//Engine(const Engine& other);
	~Engine();

	bool Initialize();
	bool Render();

private:


	GraphicsHandler mGraphicsHandler;
	std::vector<Object> mObjects;

};


#endif // !ENGINE_HPP
