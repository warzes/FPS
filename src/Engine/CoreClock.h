#pragma once

// TODO: std::chrono???

class CoreTime;

class CoreClock final
{
public:
	CoreClock();
	const auto& StartTime() const { return m_startTime; }
	const auto& CurrentTime() const { return m_currentTime; }
	const auto& LastTime() const { return m_lastTime; }

	void Reset();
	double GetFrequency() const;
#if PLATFORM_WINDOWS
	void GetTime(LARGE_INTEGER& time) const;
#endif
	void UpdateGameTime(CoreTime& gameTime);

private:
	CoreClock(const CoreClock&) = delete;
	CoreClock& operator=(const CoreClock&) = delete;

#if PLATFORM_WINDOWS
	LARGE_INTEGER m_startTime;
	LARGE_INTEGER m_currentTime;
	LARGE_INTEGER m_lastTime;
#endif
	double        m_frequency;
};