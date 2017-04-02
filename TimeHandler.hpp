/**
 * Course: DV142 - 3D-Programming
 * Authors: Viktor Enfeldt, Peter Meunier
 * 
 * File: Handler.hpp 
 *
 * File summary:
 *
 *
 *
 *
 *
 */

#ifndef TIMEHANDLER_HPP
#define TIMEHANDLER_HPP

#include "GlobalResources.hpp"

class TimeHandler
{
public:
	TimeHandler();
	~TimeHandler();

	void FrameRateCounter();
	void StartTimer();
	double GetTime();
	double GetFrameTime();
	void SetFrameCount(int newCount);
private:
	int FRAME_COUNT	= 0;
	double COUNTS_PER_SECOND = 0.0;
	__int64 COUNTER_START = 0;
	int FPS	= 0;
	__int64 FRAME_TIME_OLD = 0;
};


#endif // !TIMEHANDLER_HPP
