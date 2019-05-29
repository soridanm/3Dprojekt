#pragma once

#include "Sound.hpp"
#include "globals.hpp"

#include "../ObjectHandler.hpp"

#include <algorithm>

constexpr float MIN_PITCH = 0.25f;
constexpr float MAX_PITCH = 4.0f;

struct volumePoint
{
	float volume;
	float leftGain;
	float rightGain;

	volumePoint(float v = 0.0f, float l = 0.0f, float r = 0.0f) : volume(v), leftGain(l), rightGain(r) {}

	inline volumePoint operator+(const volumePoint& rh) const
	{
		return volumePoint(volume + rh.volume, leftGain + rh.leftGain, rightGain + rh.rightGain);
	}

	inline volumePoint operator*(const float& m) const
	{
		return volumePoint(volume*m, leftGain*m, rightGain*m);
	}
};

class Channel
{
public:
	Channel(float volume = 1.0f, float pitch = 1.0f, bool loop = false, Object *object = nullptr)
		: mSound(0), mPosition(0), mObject(object), mPan(0.0f), mHasObject((object != nullptr))
	{
		SetVolume(volume);
		SetPitch(pitch);
		SetLoop(loop);
		UpdatePan();
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

	bool HasObject() const { return mHasObject; }

	bool GetLoop() const { return mLoop; }
	void SetLoop(bool loop) { mLoop = loop; }

	float GetVolume() const { return mVolPoint.volume; }

	void SetVolume(float volume) 
	{
		mVolPoint.volume = std::clamp<float>(volume, 0.0f, 1.0f);
	}

	float GetPitch() const { return mPitch; }

	void SetPitch(float pitch) { mPitch = std::clamp<float>(pitch, MIN_PITCH, MAX_PITCH); }

	float GetPan() const { return mPan; }


	void CalcAndSetPan(float2 camPos, float2 camRight, float2 objPos)
	{
		float2 toObj = objPos - camPos;
		toObj.normalize();
		camRight.normalize();

		// [0,pi]
		float angle = acos(camRight.dot(toObj));

		// [1,-1]
		float pan = -(((angle*2.0f) / PI) - 1.0f);

		mPan = std::clamp<float>(pan, -1.0f, 1.0f);
		UpdatePan();
	}


	// Constant power panning
	void UpdatePan()
	{
		const double angle = mPan * PI_4;
		constexpr float mg = 0.1f; // min gain



		mVolPoint.leftGain = static_cast<float>(mg + (1.0f - mg) * SQRT2_2 * (cos(angle) - sin(angle)));
		mVolPoint.rightGain = static_cast<float>(mg + (1.0f - mg) * SQRT2_2 * (cos(angle) + sin(angle)));
	}

	// Volume based on the inverse distance.
	void SetVolumeBasedOnDistance(float dist)
	{
		//float vol = 1.0f / (1.0f + (dist / 10.f));
		float vol = std::min<float>(1.0f, (10.0f / dist));
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

			// Fade between old values and new ones to prevent popping
			volumePoint vp;

			if (mHasObject) {
				float fade = static_cast<float>(i) / static_cast<float>(count-2);
				vp = (mVolPoint * fade) + (mOldVolPoint * (1.0f - fade));
			} else {
				vp = mVolPoint;
			}

			// Read value from the sound data at the current position
			const PCM16 value = static_cast<PCM16>(lineInter(mSound->data, mPosition) * vp.volume);

			// Write value to both the left and right channels
			data[i]     = SafeAdd(data[i],     static_cast<PCM16>(value * gPerSoundGain * vp.leftGain));
			data[i + 1] = SafeAdd(data[i + 1], static_cast<PCM16>(value * gPerSoundGain * vp.rightGain));

			// Advance the position by one sample
			mPosition += mPitch;
		}

		mOldVolPoint = mVolPoint;
	}


private:
	Object* mObject = nullptr; // The object associated with the sound
	const bool mHasObject;
	Sound* mSound;

	bool mLoop;
	float mPosition;
	float mPitch;

	float mPan;

	volumePoint mVolPoint;
	volumePoint mOldVolPoint = { 0.0f, 0.0f, 0.0f };

};