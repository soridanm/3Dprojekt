#pragma once

#include "fmod.hpp"


#include <cmath>
#include <limits>


#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif


typedef signed short PCM16;
typedef unsigned int U32;
typedef unsigned short U16;

template <typename T>
inline T SafeAdd(T a, T b)
{
	static const T min = std::numeric_limits<T>::min();
	static const T max = std::numeric_limits<T>::max();

	return (b < 0) ? (((min - b) < a) ? (a+b) : min) : ((max - a) > b) ? (a + b) : max;
}

typedef union
{
	struct { float x, y, z; };
	struct { float r, g, b; };
} float3;


inline float GetDistance(float3 a, float3 b)
{
	return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}


static const int testData = 432;


// Lower values prevent clipping when many sounds are present
static const float gPerSoundGain = 0.25f;


// return a linearly interpolated value between two samples
inline float lineInter(PCM16* data, float pos)
{
	int a = floor(pos);
	float mix = pos - a;

	PCM16 t1 = data[a];
	PCM16 t2 = data[a + 1];

	return (data[a] * (1.0f - mix) + data[a + 1] * mix);
}

