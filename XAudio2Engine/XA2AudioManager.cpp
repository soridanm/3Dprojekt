#include "pch.h"

#include "XA2AudioManager.h"

#include "AudioFileReader.h"
#include "OmnidirectionalSound.h"
#include "XAudio2Helpers.h"

#include <hrtfapoapi.h>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>

constexpr int NUM_CHANNELS = 2;

XA2AudioManager::XA2AudioManager()
{

}


XA2AudioManager::~XA2AudioManager()
{
	//system->release();
	//system = 0;
}

void XA2AudioManager::Init(FMOD_CREATESOUNDEXINFO info)
{


}

void XA2AudioManager::Update(float elapsed, 
	DirectX::XMFLOAT4 camPos, DirectX::XMFLOAT4 camRight, 
	DirectX::XMFLOAT4 camUp, DirectX::XMFLOAT4 camForward)
{
	// Update sound volumes
	//for (auto &c : mChannels) {
	//	if (c.HasObject()) {
	//		float3 objPos = c.GetObjectPosition();
	//		float dist = GetDistance({ camPos.x, camPos.y, camPos.z }, objPos);

	//		c.SetVolumeBasedOnDistance(dist);

	//		// y is up so only x and z coords are used for panning
	//		c.CalcAndSetPan({ camPos.x, camPos.z }, { camRight.x, camRight.z }, { objPos.x, objPos.z });
	//	}
	//}

	//system->update();

	using namespace winrt::Windows::Foundation;

	
	{

		Numerics::float3 headPosition = Numerics::float3{ camPos.x, camPos.y, camPos.z };
		Numerics::float3 headUp = Numerics::float3{ -camUp.x, -camUp.y, -camUp.z };
		Numerics::float3 headDirection = Numerics::float3{ camForward.x, camForward.y, camForward.z };


		Numerics::float3 negativeZAxis = Numerics::normalize(headDirection);
		Numerics::float3 positiveYAxisGuess = Numerics::normalize(headUp);
		Numerics::float3 positiveXAxis = Numerics::normalize(Numerics::cross(negativeZAxis, positiveYAxisGuess));
		Numerics::float3 positiveYAxis = Numerics::normalize(cross(negativeZAxis, positiveXAxis));

		Numerics::float4x4 rotationTransform{
			positiveXAxis.x, positiveYAxis.x, negativeZAxis.x, 0.f,
			positiveXAxis.y, positiveYAxis.y, negativeZAxis.y, 0.f,
			positiveXAxis.z, positiveYAxis.z, negativeZAxis.z, 0.f,
			0.f, 0.f, 0.f, 1.f,
		};


		// The translate transform can be constructed using the Windows::Foundation::Numerics API.
		Numerics::float4x4 translationTransform = Numerics::make_float4x4_translation(-headPosition);

		// Now, we have a basis transform from our spatial coordinate system to a device-relative
		// coordinate system.
		Numerics::float4x4 coordinateSystemTransform = translationTransform * rotationTransform;

		// Reinterpret the cube position in the device's coordinate system.
		Numerics::float3 cubeRelativeToHead = Numerics::transform(mSoundPosition2, coordinateSystemTransform);

		// Note that at (0, 0, 0) exactly, the HRTF audio will simply pass through audio. We can use a minimal offset
		// to simulate a zero distance when the hologram position vector is exactly at the device origin in order to
		// allow HRTF to continue functioning in this edge case.
		float distanceFromHologramToHead = length(cubeRelativeToHead);
		static const float distanceMin = 0.00001f;
		if(distanceFromHologramToHead < distanceMin) {
			cubeRelativeToHead = Numerics::float3(0.f, distanceMin, 0.f);
		}

		mSound2->OnUpdate(cubeRelativeToHead);

	}

	{
		Numerics::float3 headPosition = Numerics::float3{ camPos.x, camPos.y, camPos.z };
		Numerics::float3 headUp = Numerics::float3{ -camUp.x, -camUp.y, -camUp.z };
		Numerics::float3 headDirection = Numerics::float3{ camForward.x, camForward.y, camForward.z };


		Numerics::float3 negativeZAxis = Numerics::normalize(headDirection);
		Numerics::float3 positiveYAxisGuess = Numerics::normalize(headUp);
		Numerics::float3 positiveXAxis = Numerics::normalize(Numerics::cross(negativeZAxis, positiveYAxisGuess));
		Numerics::float3 positiveYAxis = Numerics::normalize(cross(negativeZAxis, positiveXAxis));

		Numerics::float4x4 rotationTransform{
			positiveXAxis.x, positiveYAxis.x, negativeZAxis.x, 0.f,
			positiveXAxis.y, positiveYAxis.y, negativeZAxis.y, 0.f,
			positiveXAxis.z, positiveYAxis.z, negativeZAxis.z, 0.f,
			0.f, 0.f, 0.f, 1.f,
		};


		// The translate transform can be constructed using the Windows::Foundation::Numerics API.
		Numerics::float4x4 translationTransform = Numerics::make_float4x4_translation(-headPosition);

		// Now, we have a basis transform from our spatial coordinate system to a device-relative
		// coordinate system.
		Numerics::float4x4 coordinateSystemTransform = translationTransform * rotationTransform;

		// Reinterpret the cube position in the device's coordinate system.
		Numerics::float3 cubeRelativeToHead = Numerics::transform(mSoundPosition, coordinateSystemTransform);

		// Note that at (0, 0, 0) exactly, the HRTF audio will simply pass through audio. We can use a minimal offset
		// to simulate a zero distance when the hologram position vector is exactly at the device origin in order to
		// allow HRTF to continue functioning in this edge case.
		float distanceFromHologramToHead = length(cubeRelativeToHead);
		static const float distanceMin = 0.00001f;
		if(distanceFromHologramToHead < distanceMin) {
			cubeRelativeToHead = Numerics::float3(0.f, distanceMin, 0.f);
		}

		mSound->OnUpdate(cubeRelativeToHead);


	}

	//



}

void XA2AudioManager::Load(const std::string& path) { LoadOrStream(path, false); }
void XA2AudioManager::Stream(const std::string& path) { LoadOrStream(path, true); }

void XA2AudioManager::Load2(const std::string& path) {
	mSound2 = new OmnidirectionalSound(path.c_str());
}
void XA2AudioManager::LoadOrStream(const std::string& path, bool stream)
{
	// Ignore if sound is already loaded
	//if (sounds.find(path) != sounds.end()) { return; }


	// store sound object in the map with the path as key
	//sounds.insert(std::make_pair(path, new Sound(path.c_str())));
	//sounds.insert(std::make_pair(path, new OmnidirectionalSound(path.c_str())));
	mSound = new OmnidirectionalSound(path.c_str());

}


void XA2AudioManager::Play(const std::string& path, float volume, float pitch, bool loop, Object *object)
{
	// Search for the sound
	//SoundMap::iterator sound = sounds.find(path);

	// Early exit if no sound was found
	if (!mSound) { return; }

	//Channel c(volume, pitch, loop, object);
	//c.Play(sound->second);
	//mChannels.push_back(c);

	float3 temp = object->GetPosition();
	mSoundPosition = winrt::Windows::Foundation::Numerics::float3(temp.x, temp.y, temp.z);

	mSound->SetEnvironment(HrtfEnvironment::Outdoors);
	//mSound->OnUpdate(object->GetPosition());
	mSound->OnUpdate(mSoundPosition);
	mSound->Start();
}


void XA2AudioManager::Play2(const std::string& path, float volume, float pitch, bool loop, Object *object)
{
	// Search for the sound
	//SoundMap::iterator sound = sounds.find(path);

	// Early exit if no sound was found
	if (!mSound2) { return; }

	//Channel c(volume, pitch, loop, object);
	//c.Play(sound->second);
	//mChannels.push_back(c);

	float3 temp = object->GetPosition();
	mSoundPosition2 = winrt::Windows::Foundation::Numerics::float3(temp.x, temp.y, temp.z);

	mSound2->SetEnvironment(HrtfEnvironment::Outdoors);
	//mSound->OnUpdate(object->GetPosition());
	mSound2->OnUpdate(mSoundPosition2);
	mSound2->Start();
}