/**
* Course: DV1542 - 3D-Programming
* Authors: Viktor Enfeldt, Peter Meunier
*
* File: Handler.hpp
*
* File summary:
*	Creates the application window and launches it.
*/



//
// E:	Several audio sources (more than two) in a 3D world
//		Volume based on distance
//
// D	Background noice played at low volume
//		Background noice looped
//		Background noice mixed with other sound
//
// C	Stereo panning depending on the direction of the camera to the audio source
//		Updated when the camera moves (or when the source moves)
//
// B	Time domain filter (e.g. echo effect)
//
// A	Resonator filter used to make the sound of an audio source with special tone color. 
//

#define DIRECTINPUT_VERSION 0x0800

#include "Engine.hpp"


#include "AudioEngine/AudioManager.hpp"
#include <chrono>

HWND InitWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	//AudioManager audio;


	//// Create and initialize a sound info structure
	//FMOD_CREATESOUNDEXINFO info;
	//memset(&info, 0, sizeof(FMOD_CREATESOUNDEXINFO));
	//info.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);

	//// Specify sampling rate, format, and number of channels to use
	//// In this case, 44100 Hz, signed 16-bit PCM, Stereo
	//info.defaultfrequency = 44100;
	//info.format = FMOD_SOUND_FORMAT_PCM16;
	//info.numchannels = 2;

	//// Size of the entire sound in bytes. Since the sound will be
	//// looping, it does not need to be too long. In this example
	//// we will be using the equivalent of a 5 seconds sound.
	//// i.e. sampleRate * channels * bytesPerSample * durationInSeconds
	//info.length = 44100 * 2 * sizeof(signed short) * 5;

	//// Number of samples we will be submitting at a time
	//// A smaller value results in less latency between operations
	//// but if it is too small we get problems in the sound
	//// In this case we will aim for a latency of 100ms
	//// i.e. sampleRate * durationInSeconds = 44100 * 0.1 = 4410
	//info.decodebuffersize = 4410;

	//// Specify the callback function that will provide the audio data
	//info.pcmreadcallback = &AudioManager::WriteSoundData;
	//info.userdata = &audio; // Get access to the audiomanager

	//audio.Init(info);



	//audio.Load("Sounds/1kHz.wav");

	//float elapsed = 0.0f;

	////audio.Play("1kHz.wav", 1.0f, 2.0f);
	//audio.Play("Sounds/1kHz.wav");
	//audio.Play("Sounds/1kHz.wav");



	//while (true) {
	//	audio.Update(elapsed);
	//}




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

		DemoEngine.gSwapChain->Release();

		DestroyWindow(wndHandle);
	}

	return (int)msg.wParam;
}

HWND InitWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = L"DV1542_3D_PROJECT";
	if (!RegisterClassEx(&wcex)) {
		return false;
	}

	RECT rc = { 0, 0,  static_cast<LONG>(SCREEN_RESOLUTION.SCREEN_WIDTH), static_cast<LONG>(SCREEN_RESOLUTION.SCREEN_HEIGHT) };
	AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW);

	return CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
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
