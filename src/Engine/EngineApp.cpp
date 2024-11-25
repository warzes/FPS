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

	IsRequestExit = false;
	return true;
}
//=============================================================================
void EngineApp::Destroy()
{
	m_window.Destroy();
	m_log.Destroy();
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