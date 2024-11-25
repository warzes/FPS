#pragma once

#include "LogSystem.h"
#include "WindowSystem.h"

struct EngineAppCreateInfo final
{
	LogSystemCreateInfo    log{};
	WindowSystemCreateInfo window{};
};

class EngineApp final
{
public:
	bool Create(const EngineAppCreateInfo& createInfo);
	void Destroy();

	bool IsShouldClose() const;
	
	void BeginFrame();
	void EndFrame();

	auto& GetLogSystem() { return m_log; }
	auto& GetWindowSystem() { return m_window; }

private:
	LogSystem    m_log{};
	WindowSystem m_window{};
};