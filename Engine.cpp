#include "Engine.hpp"

/*============================================================================
*						Public functions
*===========================================================================*/

Engine::Engine()
{}

Engine::~Engine()
{}

bool Engine::Initialize()
{
	// AudioManager
	// Create and initialize a sound info structure
	FMOD_CREATESOUNDEXINFO info;
	memset(&info, 0, sizeof(FMOD_CREATESOUNDEXINFO));
	info.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);

	// Specify sampling rate, format, and number of channels to use
	// In this case, 44100 Hz, signed 16-bit PCM, Stereo
	info.defaultfrequency = 44100;
	info.format = FMOD_SOUND_FORMAT_PCM16;
	info.numchannels = 2;

	// Size of the entire sound in bytes. Since the sound will be
	// looping, it does not need to be too long. In this example
	// we will be using the equivalent of a 5 seconds sound.
	// i.e. sampleRate * channels * bytesPerSample * durationInSeconds
	info.length = 44100 * 2 * sizeof(signed short) * 5;

	// Number of samples we will be submitting at a time
	// A smaller value results in less latency between operations
	// but if it is too small we get problems in the sound
	// In this case we will aim for a latency of 100ms
	// i.e. sampleRate * durationInSeconds = 44100 * 0.1 = 4410
	info.decodebuffersize = 4410/2;

	// Specify the callback function that will provide the audio data
	info.pcmreadcallback = &AudioManager::WriteSoundData;
	info.userdata = &mAudioManager; // Get access to the audiomanager

	mAudioManager.Init(info);

	// Load all sounds
	mAudioManager.Load("Sounds/1kHz.wav");


	mObjectHandler.InitializeObjects(mDev, mDevCon);

	gCameraHandler.InitializeCamera(mDev, mDevCon, mObjectHandler.getWorldDepth(), mObjectHandler.getWorldWidth(), mObjectHandler.getWorldHeight());

	mLightHandler.InitializeLights(mDev);

	mShaderHandler.InitializeShaders(mDev, gSwapChain);

	mObjectHandler.mQuadtree.mFrustum = FrustumHandler(gCameraHandler.GetProjection(), gCameraHandler.GetView());

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	// Textures for the heightmap
	ID3D11Resource* textureGrass;
	HRESULT hr = DirectX::CreateDDSTextureFromFile(mDev, mDevCon, L"grass.dds", &textureGrass, &mGrassTextureView);
	if (FAILED(hr))
	{
		OutputDebugString(L"\nEngine::InitializeGraphics() Failed to create DDS grass texture from file\n\n");
		exit(-1);
	}

	ID3D11Resource* textureStone;
	hr = DirectX::CreateDDSTextureFromFile(mDev, mDevCon, L"seamlessstone.dds", &textureStone, &mStoneTextureView);
	if (FAILED(hr))
	{
		OutputDebugString(L"\nEngine::InitializeGraphics() Failed to create DDS stone texture from file\n\n");
		exit(-1);
	}

	textureGrass->Release();
	textureStone->Release();








	// Add Audio to the objects

	std::vector<Object> *objects = mObjectHandler.GetObjectArrayPtr(STATIC_OBJECT);

	mAudioManager.Play("Sounds/1kHz.wav", 1.0f, 1.0f, true, &objects->at(1));
	
	
	mAudioManager.Play("Sounds/1kHz.wav", 1.0f, 1.7f, true, &objects->at(9));





	return true;
}

bool Engine::Render()
{
	//construct mFrustum again every frame
	mObjectHandler.mQuadtree.mFrustum = FrustumHandler(gCameraHandler.GetProjection(), gCameraHandler.GetView());

	UpdateAudio();

	RenderGeometryPass();
	RenderShadowPass();
	RenderLightPass();
	RenderComputePass();
	RenderScreenPass();
	return true;
}

HRESULT Engine::CreateDirect3DContext(HWND &wndHandle)
{
	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                                // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// how swap chain is to be used
	scd.OutputWindow = wndHandle;                       // the window to be used
	scd.SampleDesc.Count = 1;                           // how many multisamples
	scd.Windowed = WINDOWED_MODE_SETTING;				// windowed/full-screen mode

	D3D_FEATURE_LEVEL MaxSupportedFeatureLevel = D3D_FEATURE_LEVEL_9_1;
	D3D_FEATURE_LEVEL FeatureLevels[] = 
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	// create a device, device context and swap chain using the information in the scd struct
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		FeatureLevels,
		ARRAYSIZE(FeatureLevels),
		D3D11_SDK_VERSION,
		&scd,
		&gSwapChain,
		&mDev,
		&MaxSupportedFeatureLevel,
		&mDevCon
	);

	return hr;
}

/*=============================================================================
*						Private functions
*===========================================================================*/

void Engine::RenderGeometryPass()
{
	std::vector<Object>* objectArray = nullptr;

	mDevCon->RSSetViewports(1, &gCameraHandler.playerVP);

	gCameraHandler.BindPerFrameConstantBuffer(mDevCon);

	// Height Map -------------------------------------------------------------
	mShaderHandler.PrepareRender(mDevCon, GEOMETRY_PASS, true, true);

	SetHeightMapShaderResources();
	mObjectHandler.SetHeightMapBuffer(mDevCon, GEOMETRY_PASS);
	mDevCon->DrawIndexed(mObjectHandler.GetHeightMapNrOfFaces() * 3, 0, 0);

	// Static Objects ---------------------------------------------------------
	mShaderHandler.PrepareRender(mDevCon, GEOMETRY_PASS, false);

	objectArray = mObjectHandler.GetObjectArrayPtr(STATIC_OBJECT);

	// RENDER ALL OBJECTS. NO TEST AGAINST QUADTREE ---------------------------
	//for (size_t i = 0; i < (*objectArray).size(); i++)
	//{
	//	for (int j = 0; j < (*objectArray)[i].GetNrOfMeshSubsets(); j++)
	//	{
	//		mObjectHandler.SetObjectBufferWithIndex(DevCon, GEOMETRY_PASS, STATIC_OBJECT, i, j);
	//		int indexStart = (*objectArray)[i].meshSubsetIndexStart[j];
	//		int indexDrawAmount = (*objectArray)[i].meshSubsetIndexStart[j + 1] - indexStart;
	//		DevCon->DrawIndexed(indexDrawAmount, indexStart, 0);
	//	}
	//}
	// END RENDER ALL OBJECTS. NO TEST AGAINST QUADTREE -----------------------

	int objInd;
	std::vector<UINT> IndicesToDraw = mObjectHandler.mQuadtree.getVisibleObjectIndices(mObjectHandler.mQuadtree.root);

	for (size_t i = 0; i < IndicesToDraw.size(); i++)
	{
		objInd = IndicesToDraw[i];
		for (int j = 0; j < (*objectArray)[objInd].GetNrOfMeshSubsets(); j++)
		{
			// Audio stuff
			(*objectArray)[i].mUpdatedSinceLastFrame = false;

			mObjectHandler.SetObjectBufferWithIndex(mDevCon, GEOMETRY_PASS, STATIC_OBJECT, objInd, j);

			int indexStart = (*objectArray)[objInd].meshSubsetIndexStart[j];
			int indexDrawAmount = (*objectArray)[objInd].meshSubsetIndexStart[j + 1] - indexStart;

			mDevCon->DrawIndexed(indexDrawAmount, indexStart, 0);
		}
	}

	// Dynamic Objects --------------------------------------------------------
	objectArray = mObjectHandler.GetObjectArrayPtr(DYNAMIC_OBJECT);
	for (size_t i = 0; i < (*objectArray).size(); i++)
	{
		for (int j = 0; j < (*objectArray)[i].GetNrOfMeshSubsets(); j++)
		{
			// Audio stuff
			(*objectArray)[i].mUpdatedSinceLastFrame = false;


			mObjectHandler.SetObjectBufferWithIndex(mDevCon, GEOMETRY_PASS, DYNAMIC_OBJECT, i, j);
			
			int indexStart = (*objectArray)[i].meshSubsetIndexStart[j];
			int indexDrawAmount = (*objectArray)[i].meshSubsetIndexStart[j + 1] - indexStart;

			mDevCon->DrawIndexed(indexDrawAmount, indexStart, 0);
		}
	}
}

void Engine::RenderShadowPass()
{
	std::vector<Object>* objectArray = nullptr;

	mDevCon->RSSetViewports(1, &gCameraHandler.lightVP);

	mShaderHandler.PrepareRender(mDevCon, SHADOW_PASS);

	gCameraHandler.BindShadowMapPerFrameConstantBuffer(mDevCon, SHADOW_PASS);

	// Height Map -------------------------------------------------------------
	mObjectHandler.SetHeightMapBuffer(mDevCon, SHADOW_PASS);
	mDevCon->DrawIndexed(mObjectHandler.GetHeightMapNrOfFaces() * 3, 0, 0);

	// Static Objects ---------------------------------------------------------
	objectArray = mObjectHandler.GetObjectArrayPtr(STATIC_OBJECT);
	for (size_t i = 0; i < (*objectArray).size(); i++)
	{
		for (int j = 0; j < (*objectArray)[i].GetNrOfMeshSubsets(); j++)
		{
			mObjectHandler.SetObjectBufferWithIndex(mDevCon, SHADOW_PASS, STATIC_OBJECT, i, j);

			int indexStart = (*objectArray)[i].meshSubsetIndexStart[j];
			int indexDrawAmount = (*objectArray)[i].meshSubsetIndexStart[j + 1] - indexStart;

			mDevCon->DrawIndexed(indexDrawAmount, indexStart, 0);
		}
	}

	// Dynamic Objects ---------------------------------------------------------
	objectArray = mObjectHandler.GetObjectArrayPtr(DYNAMIC_OBJECT);
	for (size_t i = 0; i < (*objectArray).size(); i++)
	{
		for (int j = 0; j < (*objectArray)[i].GetNrOfMeshSubsets(); j++)
		{
			mObjectHandler.SetObjectBufferWithIndex(mDevCon, SHADOW_PASS, DYNAMIC_OBJECT, i, j);

			int indexStart = (*objectArray)[i].meshSubsetIndexStart[j];
			int indexDrawAmount = (*objectArray)[i].meshSubsetIndexStart[j + 1] - indexStart;

			mDevCon->DrawIndexed(indexDrawAmount, indexStart, 0);
		}
	}
}

void Engine::RenderLightPass()
{
	mDevCon->RSSetViewports(1, &gCameraHandler.playerVP);

	mShaderHandler.PrepareRender(mDevCon, LIGHT_PASS);

	gCameraHandler.BindShadowMapPerFrameConstantBuffer(mDevCon, LIGHT_PASS);

	mLightHandler.BindLightBuffer(mDevCon, gCameraHandler.GetCameraPosition());

	// Draw a full screen triangle
	mDevCon->Draw(3, 0);

	// Clear state to set render target to nullptr since the renderTexture can 
	// not be bound as a render target and used as a shader resource view at 
	// the same time
	mDevCon->ClearState();
}

void Engine::RenderComputePass()
{
	mShaderHandler.PrepareRender(mDevCon, COMPUTE_PASS);

	static const UINT squaresWide = SCREEN_RESOLUTION.SCREEN_WIDTH / 40U;
	static const UINT squaresHigh = SCREEN_RESOLUTION.SCREEN_HEIGHT / 20U;

	mDevCon->Dispatch(squaresWide, squaresHigh, 1);

	mDevCon->ClearState(); // Used to make sure that mShaderHandler.mComputePassTempTextureUAV is free to use
}

void Engine::RenderScreenPass()
{
	mShaderHandler.PrepareRender(mDevCon, SCREEN_PASS);
	mDevCon->RSSetViewports(1, &gCameraHandler.playerVP);

	// Draw a full screen triangle
	mDevCon->Draw(3, 0);

	mDevCon->ClearState();
}

void Engine::SetHeightMapShaderResources()
{
	mDevCon->PSSetShaderResources(1, 1, &mGrassTextureView);
	mDevCon->PSSetShaderResources(2, 1, &mStoneTextureView);
}

void Engine::UpdateInput(HWND &wndHandle)
{
	mTimeHandler.FrameRateCounter();
	gCameraHandler.DetectInput(mTimeHandler.GetFrameTime(), wndHandle);
}

void Engine::UpdateAudio()
{
	
	// todo only if camera moved
	mAudioManager.Update(0.0f, true, gCameraHandler.GetCameraPosition(), gCameraHandler.GetCameraRight());

	//else
	//mAudioManager.Update(0.0f, false);

}