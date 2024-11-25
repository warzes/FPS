#pragma once

#include "LogSystem.h"

struct EngineAppCreateInfo final
{
	LogSystemCreateInfo log;
};

class EngineApp final
{
public:
	bool Create(const EngineAppCreateInfo& createInfo);
	void Destroy();

	bool IsShouldClose() const;
	
	void BeginFrame();
	void EndFrame();

private:
	LogSystem m_log;
};