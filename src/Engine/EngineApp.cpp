#include "stdafx.h"
#include "EngineApp.h"
//=============================================================================
// TODO: возможно удалить =>
bool IsRequestExit = false;
void RequestExit()
{
	IsRequestExit = true;
}
//=============================================================================
bool EngineApp::Create(const EngineAppCreateInfo& createInfo)
{
	IsRequestExit = false;
	return true;
}
//=============================================================================
void EngineApp::Destroy()
{
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
}
//=============================================================================
void EngineApp::EndFrame()
{
}
//=============================================================================