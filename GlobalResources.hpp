#ifndef GLOBALRESOURCES_HPP
#define GLOBALRESOURCES_HPP

#define DIRECTINPUT_VERSION 0x0800

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
//#include "bth_image.h"
#include <Windows.h>
#include <vector>
#include <dinput.h>
#include <WICTextureLoader.h>
#include <objbase.h>

// for reading obj
#include <string> //might not be necessary
#include <vector>
#include <fstream>
#include <istream>
#include <sstream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "Ole32.lib")

namespace ScreenSize
{
	const UINT SCREEN_WIDTH  = 1024U;
	const UINT SCREEN_HEIGHT = 720U;
};

enum ShadowQuality: UINT 
{
	SHADOWS_LOW		= 256U,
	SHADOWS_MEDIUM	= 512U,
	SHADOWS_HIGH	= 1024U,
	SHADOWS_EXTREME = 2048U,
	SHADOWS_ULTRA	= 4096U
};

enum RenderPassID
{
	GEOMETRY_PASS,
	SHADOW_PASS,
	LIGHT_PASS,
	COMPUTE_PASS
};

//TODO? Move to some class
struct HeightMapInfo {
	int worldWidth;
	int worldHeight;
	DirectX::XMFLOAT3 *heightMap;
};

struct Vertex {
	Vertex() {}
	Vertex(float x, float y, float z, float u, float v, float nx, float ny, float nz) 
		:pos(x, y, z), texCoord(u, v), normal(nx, ny, nz) 
	{}
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 texCoord;
	DirectX::XMFLOAT3 normal;
};

namespace Colors
{
	static const DirectX::XMFLOAT3 White = { 1.0f, 1.0f, 1.0f };
	static const DirectX::XMFLOAT3 Black = { 0.0f, 0.0f, 0.0f };
	static const DirectX::XMFLOAT3 LightSteelBlue = { 0.69f, 0.77f, 0.87f };
	static const DirectX::XMFLOAT3 Red = { 1.0f, 0.0f, 0.0f };
	static const DirectX::XMFLOAT3 Green = { 0.0f, 1.0f, 0.0f };
	static const DirectX::XMFLOAT3 Blue = { 0.0f, 0.0f, 1.0f };
	static const float fWhite[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static const float fBlack[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	static const float fLightSteelBlue[4] = { 0.69f, 0.77f, 0.87f, 1.0f };
}


#endif // !GLOBALRESOURCES_HPP
