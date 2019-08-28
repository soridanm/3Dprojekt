#include "pch.h"

#include "AudioManager.hpp"
#include "Channel.hpp"
#include "Sound.hpp"

constexpr int NUM_CHANNELS = 2;

AudioManager::AudioManager()
{

}


AudioManager::~AudioManager()
{
	system->release();
	system = 0;
}

void AudioManager::Init(FMOD_CREATESOUNDEXINFO info)
{
	ExitOnError(FMOD::System_Create(&system));
	ExitOnError(system->init(MAX_SOUND_CHANNELS, FMOD_INIT_NORMAL, 0));


	// Create a looping stream with FMOD_OPENUSER and the info
	FMOD::Sound* sound;
	FMOD_MODE mode = FMOD_LOOP_NORMAL | FMOD_OPENUSER;
	ExitOnError(system->createStream(0, mode, &info, &sound));
	ExitOnError(system->playSound(FMOD_CHANNEL_FREE, sound, false, 0));

}

void AudioManager::Update(float elapsed, DirectX::XMFLOAT4 camPos, DirectX::XMFLOAT4 camRight)
{
	// Update sound volumes
	for (auto &c : mChannels) {
		if (c.HasObject()) {
			float3 objPos = c.GetObjectPosition();
			float dist = GetDistance({ camPos.x, camPos.y, camPos.z }, objPos);

			c.SetVolumeBasedOnDistance(dist);

			// y is up so only x and z coords are used for panning
			c.CalcAndSetPan({ camPos.x, camPos.z }, { camRight.x, camRight.z }, { objPos.x, objPos.z });
		}
	}

	system->update();
}

void AudioManager::Load(const std::string& path) { LoadOrStream(path, false); }
void AudioManager::Stream(const std::string& path) { LoadOrStream(path, true); }

void AudioManager::LoadOrStream(const std::string& path, bool stream)
{
	// Ignore if sound is already loaded
	if (sounds.find(path) != sounds.end()) { return; }


	// store sound object in the map with the path as key
	sounds.insert(std::make_pair(path, new Sound(path.c_str())));
}


void AudioManager::Play(const std::string& path, float volume, float pitch, bool loop, Object *object)
{
	// Search for the sound
	SoundMap::iterator sound = sounds.find(path);

	// Early exit if no sound was found
	if (sound == sounds.end()) { return; }

	Channel c(volume, pitch, loop, object);
	c.Play(sound->second);
	mChannels.push_back(c);
}