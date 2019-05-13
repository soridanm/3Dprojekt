#pragma once

#include "globals.hpp"

class Sound
{
public:
	Sound(const char* path);
	~Sound();

	U32 samplingRate;
	U16 numChannels;
	U16 bitsPerSample;
	PCM16* data;
	U32 count;
};