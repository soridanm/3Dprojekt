#include "Engine.hpp"


bool Engine::Initialize()
{
	mObjectHandler.InitializeObjects(mDev, mDevCon);

	gCameraHandler.InitializeCamera(mDev, mDevCon, mObjectHandler.getWorldDepth(), mObjectHandler.getWorldWidth(), mObjectHandler.getWorldHeight());

	mLightHandler.InitializeLights(mDev, gCameraHandler.GetCameraPosition());

	mShaderHandler.InitializeShaders(mDev);

	mObjectHandler.mQuadtree.frustum = FrustumHandler(gCameraHandler.getProjection(), gCameraHandler.getView());

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	//TODO: move to function
	ID3D11Resource* textureGrass;
	HRESULT hr = DirectX::CreateDDSTextureFromFile(mDev, mDevCon, L"grass.dds", &textureGrass, &grassTextureView);
	if (FAILED(hr))
	{
		OutputDebugString(L"\nGraphicsHandler::InitializeGraphics() Failed to create DDS grass texture from file\n\n");
		exit(-1);
	}
	ID3D11Resource* textureStone;
	hr = DirectX::CreateDDSTextureFromFile(mDev, mDevCon, L"seamlessstone.dds", &textureStone, &stoneTextureView);
	if (FAILED(hr))
	{
		OutputDebugString(L"\nGraphicsHandler::InitializeGraphics() Failed to create DDS stone texture from file\n\n");
		exit(-1);
	}
	return true;
}

bool Engine::Render()
{
	RenderGeometryPass();
	RenderShadowPass();
	RenderLightPass();
	RenderComputePass();
	RenderScreenPass();
	return true;
}

HRESULT Engine::eCreateDirect3DContext(HWND &wndHandle)
{

	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                               // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;      // use 32-bit color
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
		&mDevCon);


	if (SUCCEEDED(hr))
	{
		// get the address of the back buffer
		ID3D11Texture2D* pBackBuffer = nullptr;
		gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

		// use the back buffer address to create the render target
		mDev->CreateRenderTargetView(pBackBuffer, NULL, mShaderHandler.GetBackBufferRTV());
		pBackBuffer->Release();

		// set the render target as the back buffer
		mDevCon->OMSetRenderTargets(1, mShaderHandler.GetBackBufferRTV(), NULL);
	}

	return hr;
}

Engine::Engine() : gCameraHandler(), mLightHandler(gCameraHandler.GetCameraPosition()), mShaderHandler()
{
	
}

Engine::~Engine()
{

}

void Engine::RenderGeometryPass()
{
	//construct frustum again every frame
	mObjectHandler.mQuadtree.frustum = FrustumHandler(gCameraHandler.getProjection(), gCameraHandler.getView());

	std::vector<Object>* objectArray = nullptr;

	mDevCon->RSSetViewports(1, &gCameraHandler.playerVP);

	gCameraHandler.BindPerFrameConstantBuffer(mDevCon);

	// ------------------------------ Height Map ------------------------------------------------------

	mShaderHandler.PrepareRender(mDevCon, GEOMETRY_PASS, true, true);

	SetHeightMapShaderResources(); //TODO: Move to shaderhandler
	mObjectHandler.SetHeightMapBuffer(mDevCon, GEOMETRY_PASS);
	mDevCon->DrawIndexed(mObjectHandler.GetHeightMapNrOfFaces() * 3, 0, 0);

	// ------------------------------ Static Objects ------------------------------------------------------
	// NOTE: Quad-tree stuff goes here
	mShaderHandler.PrepareRender(mDevCon, GEOMETRY_PASS, false);

	objectArray = mObjectHandler.GetObjectArrayPtr(STATIC_OBJECT);

	// --------------------- RENDER ALL OBJECTS. NO TEST AGAINST QUADTREE ----------------------------------------
	//for (size_t i = 0; i < (*objectArray).size(); i++)
	//{
	//	for (int j = 0; j < (*objectArray)[i].GetNrOfMeshSubsets(); j++)
	//	{
	//		mObjectHandler.SetObjectBufferWithIndex(DevCon, GEOMETRY_PASS, STATIC_OBJECT, i, j);

	//		int indexStart = (*objectArray)[i].meshSubsetIndexStart[j];
	//		int indexDrawAmount = (*objectArray)[i].meshSubsetIndexStart[j + 1] - indexStart;
	//		//int indexStart = mObjectHandler.meshSubsetIndexStart[i];
	//		//int indexDrawAmount = mObjectHandler.meshSubsetIndexStart[i + 1] - indexStart;

	//		DevCon->DrawIndexed(indexDrawAmount, indexStart, 0);
	//	}
	//}
	// --------------------- END RENDER ALL OBJECTS. NO TEST AGAINST QUADTREE ----------------------------------------


	int objInd;
	std::vector<UINT> toDrawIndexes = mObjectHandler.mQuadtree.getObjects(mObjectHandler.mQuadtree.root);

	// TODO! Do this in the getObjects function
	// TEMPORARY SOLUTION TO DUPLICATE DRAWS
	//std::sort(toDrawIndexes.begin(), toDrawIndexes.end());
	//toDrawIndexes.erase(std::unique(toDrawIndexes.begin(), toDrawIndexes.end()), toDrawIndexes.end());
	// END TEMPORARY SOLUTION TO DUPLICATE DRAWS

	for (size_t i = 0; i < toDrawIndexes.size(); i++)
	{
		objInd = toDrawIndexes[i];
		for (int j = 0; j < (*objectArray)[objInd].GetNrOfMeshSubsets(); j++)
		{
			mObjectHandler.SetObjectBufferWithIndex(mDevCon, GEOMETRY_PASS, STATIC_OBJECT, objInd, j);

			int indexStart = (*objectArray)[objInd].meshSubsetIndexStart[j];
			int indexDrawAmount = (*objectArray)[objInd].meshSubsetIndexStart[j + 1] - indexStart;

			mDevCon->DrawIndexed(indexDrawAmount, indexStart, 0);
		}
	}

	// ------------------------------ Dynamic Objects ------------------------------------------------------
	// NOTE: Quad-tree stuff does NOT go here
	objectArray = mObjectHandler.GetObjectArrayPtr(DYNAMIC_OBJECT);
	for (size_t i = 0; i < (*objectArray).size(); i++)
	{
		for (int j = 0; j < (*objectArray)[i].GetNrOfMeshSubsets(); j++)
		{
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

	// ------------------------------ Height Map -----------------------------------------------------
	mObjectHandler.SetHeightMapBuffer(mDevCon, SHADOW_PASS);
	mDevCon->DrawIndexed(mObjectHandler.GetHeightMapNrOfFaces() * 3, 0, 0);


	// ------------------------------ Static Objects ------------------------------------------------------
	// NOTE: Quad-tree stuff does NOT go here
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

	// ------------------------------ Dynamic Objects ------------------------------------------------------
	// NOTE: Quad-tree stuff goes here
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

	// Draw full screen triangle
	mDevCon->Draw(3, 0);

	// Set render target to nullptr since the renderTexture can not be bound as a render target 
	// and used as a shader resource view at the same time
	// NOTE: If this doesn't work then DevCon->ClearState()
	//DevCon->OMSetRenderTargets(1, nullptr, nullptr);
	mDevCon->ClearState();
}

void Engine::RenderComputePass()
{
	mShaderHandler.PrepareRender(mDevCon, COMPUTE_PASS);

	static const UINT squaresWide = SCREEN_RESOLUTION.SCREEN_WIDTH / 40U;
	static const UINT squaresHigh = SCREEN_RESOLUTION.SCREEN_HEIGHT / 20U;

	mDevCon->Dispatch(squaresWide, squaresHigh, 1);

	mDevCon->ClearState(); // Used to make sure that mTempTextureUAV is free to use
}

void Engine::RenderScreenPass()
{
	mShaderHandler.PrepareRender(mDevCon, SCREEN_PASS);
	mDevCon->RSSetViewports(1, &gCameraHandler.playerVP);

	mDevCon->Draw(3, 0);

	mDevCon->ClearState();
}

//This will/should be renamed to something more descriptive
void Engine::TimeFunction(HWND &wndHandle)
{
	mTimeHandler.FrameRateCounter();
	gCameraHandler.DetectInput(mTimeHandler.GetFrameTime(), wndHandle);
}

void Engine::SetHeightMapShaderResources()
{
	mDevCon->PSSetShaderResources(1, 1, &grassTextureView);
	mDevCon->PSSetShaderResources(2, 1, &stoneTextureView);
}

