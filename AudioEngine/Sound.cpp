#include "Sound.hpp"

#include <iostream>
#include <fstream>


// Read and load .wav file
Sound::Sound(const char* path)
{
	// Open file stream for input as binary
	std::ifstream file(path, std::ios::in | std::ios::binary);

	// Read number of channels and sample rate
	file.seekg(22);
	file.read((char*)&numChannels, 2);
	file.read((char*)&samplingRate, 4);

	// Read bits per sample
	file.seekg(34);
	file.read((char*)&bitsPerSample, 2);

	// Read size of data in bytes
	U32 length;
	file.seekg(40);
	file.read((char*)&length, 4);

	// Allocate array to hold all the data as PCM samples
	count = length / 2;
	data = new PCM16[count];

	// Read PCM data
	file.read((char*)data, length);
}

Sound::~Sound()
{
	delete[] data;
}