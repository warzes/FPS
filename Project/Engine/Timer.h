#pragma once

#include "Log.h"

class SystemTime final
{
public:
	// Query the performance counter frequency
	static void Initialize();

	// Query the current value of the performance counter
	static int64_t GetCurrentTick();

	static void BusyLoopSleep(float SleepTime);

	static double TicksToSeconds(int64_t TickCount)
	{
		return TickCount * sm_cpuTickDelta;
	}

	static double TicksToMillisecs(int64_t TickCount)
	{
		return TickCount * sm_cpuTickDelta * 1000.0;
	}

	static double TimeBetweenTicks(int64_t tick1, int64_t tick2)
	{
		return TicksToSeconds(tick2 - tick1);
	}
private:
	// The amount of time that elapses between ticks of the performance counter
	static double sm_cpuTickDelta;
};

class CpuTimer final
{
public:
	CpuTimer() = default;

	void Start()
	{
		if (m_StartTick == 0ll)
			m_StartTick = SystemTime::GetCurrentTick();
	}

	void Stop()
	{
		if (m_StartTick != 0ll)
		{
			m_ElapsedTicks += SystemTime::GetCurrentTick() - m_StartTick;
			m_StartTick = 0ll;
		}
	}

	void Reset()
	{
		m_ElapsedTicks = 0ll;
		m_StartTick = 0ll;
	}

	double GetTime() const
	{
		return SystemTime::TicksToSeconds(m_ElapsedTicks);
	}

private:
	int64_t m_StartTick = 0ll;
	int64_t m_ElapsedTicks = 0ll;
};