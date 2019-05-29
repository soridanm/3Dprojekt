#pragma once

#include "fmod.hpp"

#include <cmath>

constexpr float PI   = 3.14159265359f;
constexpr float PI_4 = 0.78539816339f; // PI/4
constexpr float PI2  = 6.28318530718f; // PI*2
constexpr float SQRT2_2 = 0.70710678118f; // SQRT(2)/2


typedef signed short PCM16;
typedef unsigned int U32;
typedef unsigned short U16;

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

	inline float length()
	{
		return (float)sqrt(x*x + z * z);
	}

	void normalize()
	{
		float l = length();
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



