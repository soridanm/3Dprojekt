#pragma once

#include "Sound.hpp"
#include "globals.hpp"
#include <algorithm>

constexpr float MIN_PITCH = 0.25f;
constexpr float MAX_PITCH = 4.0f;

class Channel
{
public:
	Channel(float volume = 1.0f, float pitch = 1.0f, bool loop = false)
		: mSound(0), mPosition(0)
	{
		SetVolume(volume);
		SetPitch(pitch);
		SetLoop(loop);
	}

	~Channel() {}


	bool GetLoop() const { return mLoop; }
	void SetLoop(bool loop) { mLoop = loop; }

	float GetVolume() const { return mVolume; }
	void SetVolume(float volume) { mVolume = std::clamp<float>(volume, 0.0f, 1.0f); }

	float GetPitch() const { return mPitch; }
	void SetPitch(float pitch) { mPitch = std::clamp<float>(pitch, MIN_PITCH, MAX_PITCH); }

	void Play(Sound* sound)
	{
		mSound = sound;
		mPosition = 0;
	}

	void Stop()
	{
		mSound = 0;
	}

	void WriteSoundData(PCM16* data, int count)
	{
		// If there is no sound assigned to the channel do nothing  
		if (mSound == 0) return;

		// We need to write "count" samples to the "data" array
		// Since output is stereo it is easier to advance in pairs
		for (int i = 0; i < count; i += 2) {

			// If we have reached the end of the sound, stop and return
			if (mPosition >= mSound->count) {
				if (mLoop) {
					mPosition = 0;
				} else {
					Stop();
					return;
				}
			}

			// Read value from the sound data at the current position




			//PCM16 value = static_cast<PCM16>(mSound->data[static_cast<int>(mPosition)] * mVolume);
			PCM16 value = static_cast<PCM16>(lineInter(mSound->data, mPosition) * mVolume);

			

			// Write value to both the left and right channels
			//data[i]     += static_cast<PCM16>(value * gVolume);
			//data[i + 1] += static_cast<PCM16>(value * gVolume);
			
			PCM16 t = SafeAdd(data[i], static_cast<PCM16>(value * gPerSoundGain));

			data[i]     = SafeAdd(data[i], static_cast<PCM16>(value * gPerSoundGain));
			data[i + 1] = SafeAdd(data[i + 1], static_cast<PCM16>(value * gPerSoundGain));

			// Advance the position by one sample
			mPosition += mPitch;
		}
	}


	//// globals
	//static Channel& getInstance()
	//{
	//	static Channel instance; // guaranteed to be destroyed. Instantiated on first use.
	//	return instance;
	//}


private:

	
	//Channel(Channel const&); //don't implement
	//void operator=(Channel const&); //don't implement

private:
	Sound* mSound;
	bool mLoop;
	float mVolume;
	
	float mPosition;
	float mPitch;
};