/**
* Course: DV142 - 3D-Programming
* Authors: Viktor Enfeldt, Peter Meunier
*
* File: Handler.hpp
*
* File summary:
*	Creates the application window and launches it.
*/

#define DIRECTINPUT_VERSION 0x0800

#include "Engine.hpp"


HWND InitWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	Engine DemoEngine;
	
	MSG msg = { 0 };
	HWND wndHandle = InitWindow(hInstance); // Create window
		
	DemoEngine.gCameraHandler.InitializeDirectInput(hInstance, wndHandle);

	if (wndHandle)
	{
		HRESULT hr = DemoEngine.CreateDirect3DContext(wndHandle);
		if (FAILED(hr))
		{
			OutputDebugString(L"\nmain.cpp : wWinMain() Failed to create 3DContext\n\n");
			exit(-1);
		}

		DemoEngine.Initialize();

		ShowWindow(wndHandle, nCmdShow);

		PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				DemoEngine.UpdateInput(wndHandle);
				DemoEngine.Render();

				//this should maybe be turned into a function
				DemoEngine.gSwapChain->Present(1, 0); // Show the back buffer which has just been rendered to
			}
		}
		//this REALLY should be turned into a function
		DemoEngine.gSwapChain->SetFullscreenState(true, NULL);
		DemoEngine.gCameraHandler.DIKeyboard->Unacquire();
		DemoEngine.gCameraHandler.DIMouse->Unacquire();
		DemoEngine.gCameraHandler.DirectInput->Release();
		/*gVertexBuffer->Release();
		gVertexLayout->Release();
		gVertexShader->Release();
		gPixelShader->Release();
		gGeometryShader	->Release();
		gDepthStecilView->Release();
		gDepthStencilTexture->Release();
		gBackbufferRTV->Release();*/
		DemoEngine.gSwapChain->Release();
		/*mDev->Release();
		mDevCon->Release();*/
		DestroyWindow(wndHandle);
	}

	return (int)msg.wParam;
}

HWND InitWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex		= { 0 };
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.hInstance		= hInstance;
	wcex.lpszClassName	= L"DV1542_3D_PROJECT";
	if (!RegisterClassEx(&wcex))
		return false;

	
	RECT rc = { 0, 0,  static_cast<LONG>(SCREEN_RESOLUTION.SCREEN_WIDTH), static_cast<LONG>(SCREEN_RESOLUTION.SCREEN_HEIGHT) };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND handle = CreateWindow(
		L"DV1542_3D_PROJECT",
		L"DV1542 - 3D Project",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	return handle;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
