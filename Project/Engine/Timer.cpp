#include "stdafx.h"
#include "Timer.h"

double SystemTime::sm_cpuTickDelta = 0.0;

// Query the performance counter frequency
void SystemTime::Initialize(void)
{
	LARGE_INTEGER frequency;
	assert(TRUE == QueryPerformanceFrequency(&frequency) && "Unable to query performance counter frequency");
	sm_cpuTickDelta = 1.0 / static_cast<double>(frequency.QuadPart);
}

// Query the current value of the performance counter
int64_t SystemTime::GetCurrentTick(void)
{
	LARGE_INTEGER currentTick;
	assert(TRUE == QueryPerformanceCounter(&currentTick) && "Unable to query performance counter value");
	return static_cast<int64_t>(currentTick.QuadPart);
}

void SystemTime::BusyLoopSleep(float SleepTime)
{
	int64_t finalTick = (int64_t)((double)SleepTime / sm_cpuTickDelta) + GetCurrentTick();
	while (GetCurrentTick() < finalTick);
}