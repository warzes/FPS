#include "stdafx.h"
#include "CPUProfiler.h"
#include "Log.h"
//=============================================================================
void CPUProfiler::BeginCPUTime(const std::string& aEventName, bool toLog /*= true*/)
{
	auto startTimer = std::chrono::high_resolution_clock::now();
	auto it = m_eventsCPUTime.find(aEventName);
	if (it == m_eventsCPUTime.end())
		m_eventsCPUTime.emplace(aEventName, startTimer);
	else
		it->second = startTimer;
}
//=============================================================================
void CPUProfiler::EndCPUTime(const std::string& aEventName)
{
	auto endTimer = std::chrono::high_resolution_clock::now();
	auto it = m_eventsCPUTime.find(aEventName);
	if (it == m_eventsCPUTime.end())
		return;
	else
	{
		std::chrono::duration<double> finalTime = endTimer - it->second;
		std::string message = "[CPUProfiler] CPU time of <" + aEventName + "> is " + std::to_string(finalTime.count()) + "s\n";
		Print(message.c_str());
	}
}
//=============================================================================