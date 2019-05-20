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

constexpr float PI   = 3.14159265359;
constexpr float PI_4 = 0.78539816339; // PI/4
constexpr float PI2  = 6.28318530718; // PI*2
constexpr float SQRT2_2 = 0.70710678118; // SQRT(2)/2

//info.decodebuffersize = 4410;
//constexpr int DECODE_BUFFER_SIZE = 4410;
constexpr int DECODE_BUFFER_SIZE = 2205;

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



struct float2
{
	float x;
	float z;

	float2(float X = 0.0f, float Z = 0.0f) : x(X), z(Z) {}

	inline float2 operator+(const float2& rh) const
	{
		return float2(x + rh.x, z + rh.z);
	}

	inline float2 operator-(const float2& rh) const
	{
		return float2(x - rh.x, z - rh.z);
	}

	inline float2 operator/(const float& rh) const
	{
		return float2(x / rh, z / rh);
	}

	inline float2 operator*(const float& rh) const
	{
		return float2(x*rh, z*rh);
	}

	inline double length()
	{
		return sqrt(x*x + z * z);
	}

	void normalize()
	{
		double l = length();
		x = x / l;
		z = z / l;
	}

	float dot(const float2& rh) const
	{
		return (x*rh.x + z * rh.z);
	}
};





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

