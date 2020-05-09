#include <Windows.h>
#include "Timer.h"

Timer::Timer():
	secondsPerCount(0.0), deltaTime(0.0), baseTime(0), pausedTime(0), 
	stopTime(0), prevTime(0), curTime(0), stopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	secondsPerCount = 1.f / (double)countsPerSec;
}

float Timer::TotalTime() const
{
	if (stopped)
		return (float)(((stopTime - pausedTime) - baseTime) * secondsPerCount);
	else
		return (float)(((curTime - pausedTime) - baseTime) * secondsPerCount);
}

float Timer::DeltaTime() const
{
	return (float)deltaTime;
}

void Timer::Reset()
{
	__int64 t;
	QueryPerformanceCounter((LARGE_INTEGER*)&t);

	baseTime = t;
	prevTime = t;
	stopTime = 0;
	stopped = false;
}

void Timer::Start()
{
	__int64 t;
	QueryPerformanceCounter((LARGE_INTEGER*)&t);

	if (stopped)
	{
		pausedTime += (t - stopTime);

		prevTime = t;
		stopTime = 0;
		stopped = false;
	}
}

void Timer::Stop()
{
	if (!stopped)
	{
		__int64 t;
		QueryPerformanceCounter((LARGE_INTEGER*)&t);

		stopTime = t;
		stopped = true;
	}
}

void Timer::Tick()
{
	if (stopped)
	{
		deltaTime = 0.0;
		return;
	}

	__int64 t;
	QueryPerformanceCounter((LARGE_INTEGER*)&t);
	curTime = t;

	deltaTime = (curTime - prevTime) * secondsPerCount;
	prevTime = curTime;
	if (deltaTime < 0.0)
		deltaTime = 0;
}