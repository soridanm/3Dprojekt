//#pragma once
//
//#include "Sound.hpp"
//#include "globals.hpp"
//
//#include "../ObjectHandler.hpp"
//
//#include <algorithm>
//#include <limits>
//
//#ifdef max
//#undef max
//#endif
//
//#ifdef min
//#undef min
//#endif
//
//constexpr float MIN_PITCH = 0.25f;
//constexpr float MAX_PITCH = 4.0f;
//
//// Lower values prevent clipping when many sounds are present
//static const float gPerSoundGain = 0.25f;
//
//#pragma region Helperfunctions
//
//// return a linearly interpolated value between two samples
//inline float linInter(PCM16* data, float pos)
//{
//	int a = (int)floor(pos);
//	float mix = pos - a;
//
//	return (data[a] * (1.0f - mix) + data[a + 1] * mix);
//}
//
//
//// Prevent under-/overflow
//template <typename T>
//inline T SafeAdd(T a, T b)
//{
//	static const T min = std::numeric_limits<T>::min();
//	static const T max = std::numeric_limits<T>::max();
//
//	return (b < 0) ? (((min - b) < a) ? (a + b) : min) : ((max - a) > b) ? (a + b) : max;
//}
//
//#pragma endregion
//
//
//
//// volume settings for an object at a point in time
//struct volumePoint
//{
//	float volume;
//	float leftGain;
//	float rightGain;
//
//	volumePoint(float v = 0.0f, float l = 0.0f, float r = 0.0f) : volume(v), leftGain(l), rightGain(r) {}
//
//	inline volumePoint operator+(const volumePoint& rh) const
//	{
//		return volumePoint(volume + rh.volume, leftGain + rh.leftGain, rightGain + rh.rightGain);
//	}
//
//	inline volumePoint operator*(const float& m) const
//	{
//		return volumePoint(volume*m, leftGain*m, rightGain*m);
//	}
//};
//
//class Channel
//{
//public:
//	Channel(float volume = 1.0f, float pitch = 1.0f, bool loop = false, Object *object = nullptr)
//		: mSound(0), mPosition(0), mObject(object), mPan(0.0f), mHasObject((object != nullptr))
//	{
//		SetVolume(volume);
//		SetPitch(pitch);
//		SetLoop(loop);
//		UpdatePan();
//	}
//
//	~Channel() {}
//
//	bool GetObjectUpdated() const
//	{
//		if (mObject) {
//			return mObject->GetUpdatedSinceLastFrame();
//		} 
//		return false;
//	}
//
//	float3 GetObjectPosition() const
//	{
//		return mObject->mPosition;
//	}
//
//
//	void SetVolume(float volume) 
//	{
//		mVolPoint.volume = std::clamp<float>(volume, 0.0f, 1.0f);
//	}
//
//	void SetPitch(float pitch) 
//	{ 
//		mPitch = std::clamp<float>(pitch, MIN_PITCH, MAX_PITCH); 
//	}
//
//	void SetLoop(bool loop) { mLoop = loop; }
//
//	bool  HasObject() const { return mHasObject; }
//	bool  GetLoop()   const { return mLoop; }
//	float GetVolume() const { return mVolPoint.volume; }
//	float GetPitch()  const { return mPitch; }
//	float GetPan()    const { return mPan; }
//
//
//	void CalcAndSetPan(float2 camPos, float2 camRight, float2 objPos)
//	{
//		float2 toObj = objPos - camPos;
//		toObj.normalize();
//		camRight.normalize();
//
//		// [0,pi]
//		float angle = acos(camRight.dot(toObj));
//
//		// [1,-1]
//		float pan = -(((angle*2.0f) / PI) - 1.0f);
//
//		mPan = std::clamp<float>(pan, -1.0f, 1.0f);
//		UpdatePan();
//	}
//
//
//	// Constant power panning
//	void UpdatePan()
//	{
//		const double angle = mPan * PI_4;
//		constexpr float mg = 0.1f; // min gain per ear
//
//		mVolPoint.leftGain  = (float)(mg + (1.0f - mg) * SQRT2_2 * (cos(angle) - sin(angle)));
//		mVolPoint.rightGain = (float)(mg + (1.0f - mg) * SQRT2_2 * (cos(angle) + sin(angle)));
//	}
//
//	// Volume based on the inverse distance.
//	void SetVolumeBasedOnDistance(float dist)
//	{
//		float vol = std::min<float>(1.0f, (10.0f / dist));
//		SetVolume(vol);
//	}
//
//
//	void Play(Sound* sound)
//	{
//		mSound = sound;
//		mPosition = 0;
//	}
//
//	void Stop()
//	{
//		mSound = 0;
//	}
//
//	void WriteSoundData(PCM16* data, int count)
//	{
//		// If there is no sound assigned to the channel do nothing  
//		if (mSound == 0) return;
//
//		volumePoint vp;
//		float fade = 0.0f;
//		float delta = 2.0f / (float)(count - 2);
//
//		// We need to write "count" samples to the "data" array
//		// Since output is stereo it is easier to advance in pairs
//		for (int i = 0; i < count; i += 2) {
//			// If we have reached the end of the sound, stop and return
//			if (mPosition >= mSound->count) {
//				if (mLoop) {
//					mPosition = 0;
//				} else {
//					Stop();
//					return;
//				}
//			}
//
//			// Fade between old volume and new one to prevent popping
//			if (mHasObject) {
//				vp = (mVolPoint * fade) + (mOldVolPoint * (1.0f - fade));
//			} else {
//				vp = mVolPoint;
//			}
//
//			// Read value from the sound data at the current position
//			const PCM16 value = (PCM16)(linInter(mSound->data, mPosition) * vp.volume);
//
//			// Write value to both the left and right channels
//			data[i]     = SafeAdd(data[i],     (PCM16)(value * gPerSoundGain * vp.leftGain));
//			data[i + 1] = SafeAdd(data[i + 1], (PCM16)(value * gPerSoundGain * vp.rightGain));
//
//			// Advance the position by one sample and adjust the volume fading
//			mPosition += mPitch;
//			fade += delta;
//		}
//		mOldVolPoint = mVolPoint;
//	}
//
//
//private:
//	Object* mObject = nullptr; // The object associated with the sound
//	const bool mHasObject;
//	Sound* mSound;
//
//	bool mLoop;
//	float mPosition;
//	float mPitch;
//
//	float mPan;
//
//	volumePoint mVolPoint    = { 0.0f, 0.0f, 0.0f };
//	volumePoint mOldVolPoint = { 0.0f, 0.0f, 0.0f };
//};