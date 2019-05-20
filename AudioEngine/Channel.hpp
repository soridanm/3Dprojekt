#pragma once

#include "Sound.hpp"
#include "globals.hpp"

#include "../ObjectHandler.hpp"

#include <algorithm>

constexpr float MIN_PITCH = 0.25f;
constexpr float MAX_PITCH = 4.0f;



class Channel
{
public:
	Channel(float volume = 1.0f, float pitch = 1.0f, bool loop = false, Object *object = nullptr)
		: mSound(0), mPosition(0), mObject(object), mPan(0.0f)
	{
		SetVolume(volume);
		SetPitch(pitch);
		SetLoop(loop);
	}

	~Channel() {}

	bool GetObjectUpdated() const
	{
		if (mObject) {
			return mObject->GetUpdatedSinceLastFrame();
		} 
		return false;
	}

	float3 GetObjectPosition() const
	{
		return mObject->mPosition;
	}

	bool GetLoop() const { return mLoop; }
	void SetLoop(bool loop) { mLoop = loop; }

	float GetVolume() const { return mVolume; }
	void SetVolume(float volume) { mVolume = std::clamp<float>(volume, 0.0f, 1.0f); }

	float GetPitch() const { return mPitch; }
	void SetPitch(float pitch) { mPitch = std::clamp<float>(pitch, MIN_PITCH, MAX_PITCH); }

	float GetPan() const { return mPan; }
	void SetPan(float val)
	{
		mPan = std::clamp<float>(val, -1.0f, 1.0f);
		UpdatePan();
	}

	// Constant power panning
	void UpdatePan()
	{
		const double angle = mPan * PI_4;
		mLeftGain  = static_cast<float>(SQRT2_2 * (cos(angle) - sin(angle)));
		mRightGain = static_cast<float>(SQRT2_2 * (cos(angle) + sin(angle)));

	}

	// todo change falloff function
	void SetVolumeBasedOnDistance(float dist)
	{
		float vol = 1.0f / (1.0f + (dist / 10.f));
		SetVolume(vol);
	}


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


		// todo update the volume based on distance between the camera and the object
		if (mObject) {



		}

		// todo interpolate between previous pan and gain values to prevent clipping sounds

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

			data[i]     = SafeAdd(data[i], static_cast<PCM16>(value * gPerSoundGain * mLeftGain));
			data[i + 1] = SafeAdd(data[i + 1], static_cast<PCM16>(value * gPerSoundGain * mRightGain));

			// Advance the position by one sample
			mPosition += mPitch;
		}
	}


private:
	Object* mObject = nullptr; // The object associated with the sound



	Sound* mSound;
	bool mLoop;
	float mVolume;

	float mLeftGain;
	float mRightGain;
	
	float mPan;


	float mPosition;
	float mPitch;
};