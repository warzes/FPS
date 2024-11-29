#pragma once

#include "LogSystem.h"
#include "WindowSystem.h"
#include "InputSystem.h"
#include "RenderSystem.h"

struct EngineAppCreateInfo final
{
	LogSystemCreateInfo    log{};
	WindowSystemCreateInfo window{};
	InputSystemCreateInfo  input{};
	RenderSystemCreateInfo render{};
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
	auto& GetInputSystem() { return m_input; }
	auto& GetRenderSystem() { return m_render; }

private:
	LogSystem    m_log{};
	WindowSystem m_window{};
	InputSystem  m_input{};
	RenderSystem m_render{};
};