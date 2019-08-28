#pragma once

#define DIRECTINPUT_VERSION 0x0800

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <Windows.h>
#include <vector>
#include <dinput.h>
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include <objbase.h>

#include <string> //might not be necessary
#include <vector>
#include <fstream>
#include <istream>
#include <sstream>
#include <algorithm>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "Ole32.lib")







// AUDIO Stuff





















struct ScreenSize
{
	ScreenSize(UINT w, UINT h, LPCSTR ws, LPCSTR hs)
		: SCREEN_WIDTH(w), SCREEN_HEIGHT(h), WIDTH_STRING(ws), HEIGHT_STRING(hs)
	{}

	const UINT SCREEN_WIDTH;
	const UINT SCREEN_HEIGHT;
	const LPCSTR WIDTH_STRING;
	const LPCSTR HEIGHT_STRING;
};

// NOTE: Height must be divisible by 20, and width must be divisible by 40
namespace ScreenResolutionPresets
{
	static const ScreenSize LOW_480p = ScreenSize(640U, 480U, "640\n", "480\n");
	static const ScreenSize HD_720p = ScreenSize(1280U, 720U, "1280", "720");
	static const ScreenSize FHD_1080p = ScreenSize(1920U, 1080U, "1920\n", "1080\n");
	static const ScreenSize QHD_1440p = ScreenSize(2560U, 1440U, "2560\n", "1440\n");
	static const ScreenSize UHD_1 = ScreenSize(3840U, 2160U, "3840", "2160");
	static const ScreenSize FHD_1920x1000 = ScreenSize(1920U, 1000U, "1920\n", "1000\n");

};

struct ShadowQuality
{
	ShadowQuality(UINT w, LPCSTR s)
		: SHADOW_MAP_SIZE(w), SIZE_STRING(s)
	{}

	const UINT SHADOW_MAP_SIZE;
	const LPCSTR SIZE_STRING;
};

namespace ShadowMapPresets
{
	static const ShadowQuality LOW = ShadowQuality(256U, "256\n");
	static const ShadowQuality MEDIUM = ShadowQuality(512U, "512\n");
	static const ShadowQuality HIGH = ShadowQuality(1024U, "1024");
	static const ShadowQuality EXTREME = ShadowQuality(2048U, "2048\n");
	static const ShadowQuality ULTRA = ShadowQuality(4096U, "4096\n");
};

enum RenderPassID
{
	GEOMETRY_PASS,
	SHADOW_PASS,
	LIGHT_PASS,
	COMPUTE_PASS,
	SCREEN_PASS
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
