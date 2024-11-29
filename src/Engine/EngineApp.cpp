#include "stdafx.h"
#include "EngineApp.h"
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

	IsRequestExit = false;
	return true;
}
//=============================================================================
void EngineApp::Destroy()
{
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
	m_window.PollEvent();
	if (m_window.IsShouldClose())
	{
		// TODO: здесь можно обработать закрытие окна
		RequestExit();
		return;
	}
}
//=============================================================================
void EngineApp::EndFrame()
{
}
//=============================================================================