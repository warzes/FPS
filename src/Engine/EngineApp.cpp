#include "stdafx.h"
#include "EngineApp.h"
#include "Log.h"
//=============================================================================
bool IsRequestExit = false;
void RequestExit()
{
	IsRequestExit = true;
}
//=============================================================================
bool EngineApp::Create(const EngineAppCreateInfo& createInfo)
{
	IsRequestExit = true;

#if PLATFORM_WINDOWS && RENDER_D3D
	if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
	{
		Fatal("Failed to call CoInitializeEx");
		return false;
	}
#endif // PLATFORM_WINDOWS

#if defined(_DEBUG) && PLATFORM_WINDOWS
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);
#endif

	if (!m_log.Create(createInfo.log)) return false;
	if (!m_window.Create(createInfo.window)) return false;
	m_window.ConnectInputSystem(&m_input);
	if (!m_input.Create(createInfo.input)) return false;

	WindowData data;
#if PLATFORM_WINDOWS
	data.hwnd = m_window.GetHWND();
	data.handleInstance = m_window.GetWindowInstance();
#endif // PLATFORM_WINDOWS
	data.width = m_window.GetWidth();
	data.height = m_window.GetHeight();

	if (!m_render.Create(data, createInfo.render)) return false;

	for (auto component : m_coreComponents)
	{
		if (!component->Initialize()) return false;
	}

	m_CPUProfiler = new CPUProfiler();

	m_coreClock.Reset();

	IsRequestExit = false;
	return true;
}
//=============================================================================
void EngineApp::Destroy()
{
	m_coreComponents.clear();
	delete m_CPUProfiler;
	m_render.Destroy();
	m_input.Destroy();
	m_window.Destroy();
	m_log.Destroy();
	IsRequestExit = true;
}
//=============================================================================
bool EngineApp::IsShouldClose() const
{
	return IsRequestExit;
}
//=============================================================================
void EngineApp::BeginFrame()
{
	m_coreClock.UpdateGameTime(m_coreTime);

	m_window.PollEvent();
	if (m_window.IsShouldClose())
	{
		// TODO: здесь можно обработать закрытие окна
		RequestExit();
		return;
	}
	m_input.Update();

	m_render.Resize(m_window.GetWidth(), m_window.GetHeight());

	for (auto component : m_coreComponents)
	{
		if (component->Enabled())
		{
			component->Update(m_coreTime);
		}
	}
}
//=============================================================================
void EngineApp::EndFrame()
{
	m_render.Present();
}
//=============================================================================