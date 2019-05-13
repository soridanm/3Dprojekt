#pragma once

#include "globals.hpp"

#include "Channel.hpp"
#include "Sound.hpp"

#include "..\ObjectHandler.hpp"


#include <fmod.hpp>
#include <fmod_errors.h>

#include <iostream>
#include <map>
#include <vector>






constexpr int MAX_SOUND_CHANNELS = 100;


inline void ExitOnError(FMOD_RESULT result)
{
	if (result != FMOD_OK) {
		std::cout << FMOD_ErrorString(result) << std::endl;
		exit(-1);
	}
}




//typedef std::map<std::string, FMOD::Sound*> SoundMap;
typedef std::map<std::string, Sound*> SoundMap;


class AudioManager
{
public:
	
	static FMOD_RESULT F_CALLBACK
		WriteSoundData(FMOD_SOUND *sound, void *data, unsigned int length)
	{
		// Get the object we are using
		// Note that FMOD_Sound * must be cast to FMOD::Sound * to access it
		AudioManager *me;
		((FMOD::Sound *) sound)->getUserData((void **)&me);

		// Clear output
		memset(data, 0, length);

		// Get data in the correct format and calculate sample count
		PCM16* pcmData = (PCM16*)data;
		int pcmDataCount = length / 2;


		// Tell the channel to write to the output
		//Channel::getInstance().WriteSoundData(pcmData, pcmDataCount);
		for (auto &c : me->mChannels) {
			c.WriteSoundData(pcmData, pcmDataCount);
		}

		//me->mChannel.WriteSoundData(pcmData, pcmDataCount);

		//AudioManager::getInstance().mChannel.WriteSoundData(pcmData, pcmDataCount);

		return FMOD_OK;
	}

	void Update(float elapsed, bool cameraUpdated, DirectX::XMFLOAT4 cameraPos = DirectX::XMFLOAT4{});
	void Load(const std::string& path);
	void Stream(const std::string& path);
	void Play(const std::string& path, float volume = 1.0f, float pitch = 1.0f, bool loop = false, Object *object = nullptr);

	AudioManager();
	~AudioManager();

	void Init(FMOD_CREATESOUNDEXINFO info);

	

	std::vector<Channel> mChannels;
	//Channel mChannel;


	// Singleton
	//static AudioManager& getInstance()
	//{
	//	static AudioManager instance;
	//	return instance;
	//}


private:

	//AudioManager(AudioManager const&);
	//void operator=(AudioManager const&);


	void LoadOrStream(const std::string& path, bool stream);

	FMOD::System* system;
	SoundMap sounds;



	/// low-level audio

	// sound = 44100 Hz * 1 second * 1 channel
	//PCM16 data[44100];


	// 1 second of audio data at 44100 Hz (Stereo)
	// data[0] = left, data[1] = right, data[2] = left, etc.
	// count = 44100 Hz * 1 second * 2 channels
	PCM16 data[88200];


};