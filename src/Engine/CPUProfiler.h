#pragma once

typedef std::chrono::high_resolution_clock::time_point TimePoint;

class CPUProfiler final
{
public:
	void BeginCPUTime(const std::string& aEventName, bool toLog = true);
	void EndCPUTime(const std::string& aEventName);

private:
	std::map<std::string, TimePoint> m_eventsCPUTime;
};