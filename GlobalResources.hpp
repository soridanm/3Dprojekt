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
#include <DDSTextureLoader.h>
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

//namespace ScreenSize
//{
//	const UINT SCREEN_WIDTH  = 1024U;
//	const UINT SCREEN_HEIGHT = 720U;
//};

struct ScreenSize
{
	ScreenSize(UINT w, UINT h, LPCSTR ws, LPCSTR hs) 
		: SCREEN_WIDTH(w), SCREEN_HEIGHT(h), WIDTH_STRING(ws), HEIGHT_STRING(hs) {}

	const UINT SCREEN_WIDTH;
	const UINT SCREEN_HEIGHT;
	const LPCSTR WIDTH_STRING;
	const LPCSTR HEIGHT_STRING;
};

namespace ScreenResolution
{
	static const ScreenSize LOW_480p	= ScreenSize(640U, 480U, "640\n", "480\n");
	static const ScreenSize HD_720p		= ScreenSize(1280U, 720U, "1280", "720");
	static const ScreenSize FHD_1080p	= ScreenSize(1920U, 1080U, "1920\n", "1080\n");
	static const ScreenSize QHD_1440p	= ScreenSize(2560U, 1440U, "2560\n", "1440\n");
	static const ScreenSize UHD_1		= ScreenSize(3840U, 2160U, "3840", "2160");
};

//static const ScreenSize SCREEN_RESOLUTION = ScreenResolution::LOW_480p;
//static const ScreenSize SCREEN_RESOLUTION = ScreenResolution::HD_720p;
//static const ScreenSize SCREEN_RESOLUTION = ScreenResolution::FHD_1080p;
//static const ScreenSize SCREEN_RESOLUTION = ScreenResolution::QHD_1440p;
static const ScreenSize SCREEN_RESOLUTION = ScreenResolution::UHD_1;


struct ShadowQuality
{
	ShadowQuality(UINT w, LPCSTR s) 
		: SHADOW_MAP_SIZE(w), SIZE_STRING(s) {}

	const UINT SHADOW_MAP_SIZE;
	const LPCSTR SIZE_STRING;
};

namespace ShadowMapPresets
{
	static const ShadowQuality LOW		= ShadowQuality(256U, "256\n");
	static const ShadowQuality MEDIUM	= ShadowQuality(512U, "512\n");
	static const ShadowQuality HIGH		= ShadowQuality(1024U, "1024");
	static const ShadowQuality EXTREME	= ShadowQuality(2048U, "2048\n");
	static const ShadowQuality ULTRA	= ShadowQuality(4096U, "4096\n");
};

//static const ShadowQuality SHADOW_QUALITY = ShadowMapPresets::LOW;
//static const ShadowQuality SHADOW_QUALITY = ShadowMapPresets::MEDIUM;
static const ShadowQuality SHADOW_QUALITY = ShadowMapPresets::HIGH;
//static const ShadowQuality SHADOW_QUALITY = ShadowMapPresets::EXTREME;
//static const ShadowQuality SHADOW_QUALITY = ShadowMapPresets::ULTRA;

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
struct Node {
	int levels;
	DirectX::XMVECTOR boxMin;
	DirectX::XMVECTOR boxMax;
	Node* children[4];
	std::vector<UINT>objects;
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
