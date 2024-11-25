#include "stdafx.h"
#include "RenderSystem.h"
//=============================================================================
RenderSystem::~RenderSystem()
{
}
//=============================================================================
bool RenderSystem::Create(const WindowPrivateData& data, const RenderSystemCreateInfo& createInfo)
{
	if (!createAPI(data, createInfo)) return false;

	return true;
}
//=============================================================================
void RenderSystem::Destroy()
{
	destroyAPI();
}
//=============================================================================
void RenderSystem::Resize(uint32_t width, uint32_t height)
{
}
//=============================================================================
void RenderSystem::Present()
{
}
//=============================================================================