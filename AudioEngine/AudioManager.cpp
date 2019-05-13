#include "AudioManager.hpp"
#include "Channel.hpp"
#include "Sound.hpp"

constexpr int NUM_CHANNELS = 2;

AudioManager::AudioManager()
{

}


void AudioManager::Init(FMOD_CREATESOUNDEXINFO info)
{
	ExitOnError(FMOD::System_Create(&system));
	ExitOnError(system->init(MAX_SOUND_CHANNELS, FMOD_INIT_NORMAL, 0));


	// Create a looping stream with FMOD_OPENUSER and the info we filled 
	FMOD::Sound* sound;
	FMOD_MODE mode = FMOD_LOOP_NORMAL | FMOD_OPENUSER;
	ExitOnError(system->createStream(0, mode, &info, &sound));
	ExitOnError(system->playSound(FMOD_CHANNEL_FREE, sound, false, 0));

}

AudioManager::~AudioManager()
{
	//for (auto &s : sounds) {
	//	s.second->release();
	//}
	//sounds.clear();

	system->release();
	system = 0;
}

void AudioManager::Update(float elapsed, bool cameraUpdated, DirectX::XMFLOAT4 cameraPos)
{
	// Update sound volumes
	for (auto &c : mChannels) {
		if (cameraUpdated || c.GetObjectUpdated()) {
			float dist = GetDistance({ cameraPos.x, cameraPos.y, cameraPos.z }, c.GetObjectPosition());

			c.SetVolumeBasedOnDistance(dist);
		}
	}

	system->update();
}

void AudioManager::LoadOrStream(const std::string& path, bool stream)
{
	// Ignore if sound is already loaded
	if (sounds.find(path) != sounds.end()) { return; }

	//// Load (or stream) file into a sound object
	//FMOD::Sound* sound;
	//if (stream) {
	//	system->createStream(path.c_str(), FMOD_DEFAULT, 0, &sound);
	//} else {
	//	system->createSound(path.c_str(), FMOD_DEFAULT, 0, &sound);
	//}


	/// low-level
	Sound* sound = new Sound(path.c_str());


	// store sound object in the map with the path as key
	sounds.insert(std::make_pair(path, sound));
}

void AudioManager::Load(const std::string& path) { LoadOrStream(path, false); }

void AudioManager::Stream(const std::string& path) { LoadOrStream(path, true); }

void AudioManager::Play(const std::string& path, float volume, float pitch, bool loop, Object *object)
{
	// Search for the sound
	SoundMap::iterator sound = sounds.find(path);

	// Early exit if no sound was found
	if (sound == sounds.end()) { return; }



	//Channel::getInstance().Play(sound->second);
	Channel c(volume, pitch, loop, object);
	c.Play(sound->second);
	mChannels.push_back(c);

	//mChannels.push_back(Channel{});
	//mChannelPlay(sound->second);

	///
	//FMOD::Channel* channel;

	// play the sound
	//system->playSound(FMOD_CHANNEL_FREE, sound->second, false, &channel);
	//channel->setVolume(0.2f);
}