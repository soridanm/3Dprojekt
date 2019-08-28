#include "pch.h"
#include "TimeHandler.hpp"

/*============================================================================
*						Public functions
*===========================================================================*/

TimeHandler::TimeHandler()
{}

TimeHandler::~TimeHandler()
{}

void TimeHandler::FrameRateCounter()
{
	FRAME_COUNT++;
	if (GetTime() > 1.0)
	{
		FPS = FRAME_COUNT;
		FRAME_COUNT = 0;
		StartTimer();
	}
}

//used in wWinMain
void TimeHandler::StartTimer()
{
	LARGE_INTEGER frequencyCount;
	QueryPerformanceFrequency(&frequencyCount);

	COUNTS_PER_SECOND = static_cast<double>(frequencyCount.QuadPart);
	QueryPerformanceCounter(&frequencyCount);
	COUNTER_START = frequencyCount.QuadPart;
}

void TimeHandler::SetFrameCount(int newCount)
{
	FRAME_COUNT = newCount;
}

//used in wWinMain
double TimeHandler::GetTime()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);

	return static_cast<double>(currentTime.QuadPart - COUNTER_START) / COUNTS_PER_SECOND;
}

//used by Engine::UpdateInput()
double TimeHandler::GetFrameTime()
{
	LARGE_INTEGER currentTime;
	__int64 tickCount;
	QueryPerformanceCounter(&currentTime);
	tickCount		= currentTime.QuadPart - FRAME_TIME_OLD;
	FRAME_TIME_OLD	= currentTime.QuadPart;

	if (tickCount < 0) tickCount = 0;

	return static_cast<double>(tickCount) / COUNTS_PER_SECOND;
}
