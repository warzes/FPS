#include "stdafx.h"
#include "CoreClock.h"
#include "CoreTime.h"
#include "Log.h"
#if PLATFORM_WINDOWS
//=============================================================================
CoreClock::CoreClock()
	: m_startTime()
	, m_currentTime()
	, m_lastTime()
	, m_frequency()
{
	m_frequency = GetFrequency();
	Reset();
}
//=============================================================================
void CoreClock::Reset()
{
	GetTime(m_startTime);
	m_currentTime = m_startTime;
	m_lastTime = m_currentTime;
}
//=============================================================================
double CoreClock::GetFrequency() const
{
	LARGE_INTEGER frequency;

	if (QueryPerformanceFrequency(&frequency) == false)
	{
		Fatal("QueryPerformanceFrequency() failed.");
	}

	return (double)frequency.QuadPart;
}
//=============================================================================
void CoreClock::GetTime(LARGE_INTEGER& time) const
{
	QueryPerformanceCounter(&time);
}
//=============================================================================
void CoreClock::UpdateGameTime(CoreTime& gameTime)
{
	GetTime(m_currentTime);
	gameTime.SetTotalCoreTime((m_currentTime.QuadPart - m_startTime.QuadPart) / m_frequency);
	gameTime.SetElapsedCoreTime((m_currentTime.QuadPart - m_lastTime.QuadPart) / m_frequency);

	m_lastTime = m_currentTime;
}
//=============================================================================
#endif // PLATFORM_WINDOWS