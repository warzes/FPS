#include "stdafx.h"
#include "Timer.h"

Timer::Timer()
{
	if (!QueryPerformanceFrequency(&m_qpcFrequency))
	{
		Fatal("QueryPerformanceFrequency failed");
		return;
	}

	if (!QueryPerformanceCounter(&m_qpcLastTime))
	{
		Fatal("QueryPerformanceCounter failed");
		return;
	}

	// Initialize max delta to 1/10 of a second.
	m_qpcMaxDelta = static_cast<uint64_t>(m_qpcFrequency.QuadPart / 10);
}

void Timer::ResetElapsedTime()
{
	if (!QueryPerformanceCounter(&m_qpcLastTime))
	{
		Fatal("QueryPerformanceCounter failed");
		return;
	}

	m_leftOverTicks = 0;
	m_framesPerSecond = 0;
	m_framesThisSecond = 0;
	m_qpcSecondCounter = 0;
}