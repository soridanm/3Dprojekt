#pragma once

#include "OmnidirectionalSound.h"


#include "..\ObjectHandler.hpp"


#include <fmod.hpp>
#include <fmod_errors.h>

#include <iostream>
#include <map>
#include <vector>







//typedef std::map<std::string, OmnidirectionalSound*> SoundMap;

class XA2AudioManager
{
public:
	XA2AudioManager();
	~XA2AudioManager();

	void Update(
		float elapsed,
		DirectX::XMFLOAT4 camPos = DirectX::XMFLOAT4(),
		DirectX::XMFLOAT4 camRight = DirectX::XMFLOAT4(),
		DirectX::XMFLOAT4 camUp = DirectX::XMFLOAT4(),
		DirectX::XMFLOAT4 camForward = DirectX::XMFLOAT4()
		);

	void Load(const std::string& path);
	void Load2(const std::string& path);



	void Stream(const std::string& path);

	void Play(const std::string& path, float volume = 1.0f, float pitch = 1.0f, bool loop = false, Object *object = nullptr);
	
	void Play2(const std::string& path, float volume = 1.0f, float pitch = 1.0f, bool loop = false, Object *object = nullptr);

	void Init(FMOD_CREATESOUNDEXINFO info);



	//std::vector<Channel> mChannels;



	//// The callback function called by the OS
	//static FMOD_RESULT F_CALLBACK WriteSoundData(FMOD_SOUND *sound, void *data, unsigned int length)
	//{
	//	// Get access to the XA2AudioManager
	//	XA2AudioManager *am;
	//	((FMOD::Sound*)sound)->getUserData((void**)&am);

	//	// Clear output
	//	memset(data, 0, length);

	//	// Get data in the correct format and calculate sample count
	//	PCM16* pcmData = (PCM16*)data;
	//	int pcmDataCount = length / 2;

	//	// Tell all the channels to write to the output
	//	for (auto &c : am->mChannels) {
	//		c.WriteSoundData(pcmData, pcmDataCount);
	//	}

	//	return FMOD_OK;
	//}
private:
	void LoadOrStream(const std::string& path, bool stream);

	//FMOD::System* system;
	//SoundMap sounds;

	//// buffer has up to 1 second of audio data at 44100 Hz (Stereo)
	//// data[0] = left, data[1] = right, data[2] = left, etc.
	//// count = 44100 Hz * 1 second * 2 channels
	//PCM16 data[88200];


	//SoundMap sounds; // TODO? do differently
	OmnidirectionalSound *mSound;
	OmnidirectionalSound *mSound2;
	winrt::Windows::Foundation::Numerics::float3 mSoundPosition;
	winrt::Windows::Foundation::Numerics::float3 mSoundPosition2;

};